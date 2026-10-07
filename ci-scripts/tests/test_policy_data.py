#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Tests for the UDR endpoints a PCF uses during AM/SM policy creation
[3GPP TS 29.519]:

    GET    /nudr-dr/{version}/policy-data/ues/{ueId}/am-data
    GET    /nudr-dr/{version}/policy-data/ues/{ueId}/sm-data
    GET    /nudr-dr/{version}/policy-data/ues/{ueId}/ue-policy-set
    GET    /nudr-dr/{version}/application-data/influenceData
    POST   /nudr-dr/{version}/application-data/influenceData/subs-to-notify
    DELETE /nudr-dr/{version}/application-data/influenceData/subs-to-notify/{subscriptionId}

With `http_version: 2` the UDR serves requests from udr-http2-server.cpp,
which routes nothing else under policy-data (no PUT/PATCH/DELETE, no
sponsor-connectivity-data, bdt-data or usage-monitoring-information); the
Pistache *ApiImpl.cpp stubs for those only answer over HTTP/1.1. The UDR has
no Traffic Influence Data store yet, so the influenceData GET always returns
an empty array and subscriptions live in memory only.

The UDR's HTTP/2 server speaks h2c *with prior knowledge* (cleartext HTTP/2,
no TLS/ALPN, no Upgrade dance). Neither package `requests` (HTTP/1.1 only) nor
`httpx`/`httpcore` (HTTP/2 only via TLS ALPN) can produce that request, so
requests are shelled out to `curl --http2-prior-knowledge` for transport;
everything else -- building query strings, parsing responses, asserting on
them -- is plain Python/`json`. No pip dependencies.

Configuration (environment variables):
    UDR_HOST          UDR SBI address                          (default: 127.0.0.1)
    UDR_PORT          UDR SBI port                              (default: 8080)
    UDR_CONTAINER     If set, curl runs as `docker exec $UDR_CONTAINER curl ...`
                      (for topologies where the test driver isn't on the UDR's
                      Docker network). If unset, curl runs directly.
    API_VERSIONS      Comma-separated API versions to exercise  (default: v1,v2)
    UE_IDS_STANDARD   Comma-separated standard-tier SUPIs       (default from oai_db_policy_data.sql)
    UE_IDS_PREMIUM    Comma-separated premium-tier SUPIs        (default from oai_db_policy_data.sql)
    UE_ID_UNKNOWN     A SUPI not present in the seeded DB       (default: 999999999999999)
    SNSSAI_SST        Slice SST present in the seeded SM data   (default: 222)
    SNSSAI_SD         Slice SD (hex) present in the seeded data (default: 00007b)
    DNN               DNN present in the seeded SM data         (default: default)
    NOTIF_URI         notificationUri sent when subscribing to influence data
                      (default: http://127.0.0.1:8081/influence-data-notify; the
                      UDR never calls it since there is no data to change)

Usage:
    ./test_policy_data.py suite
    ./test_policy_data.py am-data <ueId> [--version v2] [--expect-status 404]
    ./test_policy_data.py sm-data <ueId> [--version v1] [--snssai '{"sst":222,"sd":"00007b"}'] [--dnn default]
    ./test_policy_data.py ue-policy-set <ueId> [--version v1]
    ./test_policy_data.py influence-data [--version v2]
    ./test_policy_data.py influence-data-subscribe [--version v2]
    ./test_policy_data.py influence-data-unsubscribe <subscriptionId> [--version v2] [--expect-status 404]

Every subcommand exits 0 if all its assertions passed, 1 otherwise.
`suite` is the CI entrypoint: it runs every scenario below and reports one
aggregate pass/fail count.
"""

from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
import urllib.parse
from dataclasses import dataclass, field

# ===========================================================================
# CONFIG
# ===========================================================================

UDR_HOST = os.environ.get("UDR_HOST", "127.0.0.1")
UDR_PORT = os.environ.get("UDR_PORT", "8080")
UDR_CONTAINER = os.environ.get("UDR_CONTAINER", "")

API_VERSIONS = [v for v in os.environ.get("API_VERSIONS", "v1,v2").split(",") if v]

# Sample UE IDs seeded by build/scripts/oai_db_policy_data.sql
UE_IDS_STANDARD = os.environ.get(
    "UE_IDS_STANDARD", "208950000000031,208950000000032"
).split(",")
UE_IDS_PREMIUM = os.environ.get(
    "UE_IDS_PREMIUM", "208950000000125,208950000000126"
).split(",")
UE_ID_UNKNOWN = os.environ.get("UE_ID_UNKNOWN", "999999999999999")

# SNSSAI from sample data: sst=222 (0xde), sd=00007b -> hex key "de00007b"
SNSSAI_SST = int(os.environ.get("SNSSAI_SST", "222"))
SNSSAI_SD = os.environ.get("SNSSAI_SD", "00007b")
DNN = os.environ.get("DNN", "default")
NOTIF_URI = os.environ.get("NOTIF_URI", "http://127.0.0.1:8081/influence-data-notify")

REQUEST_TIMEOUT = float(os.environ.get("REQUEST_TIMEOUT", "10"))

_STATUS_MARKER = "###UDR_TEST_STATUS###"


def _snssai() -> dict:
    return {"sst": SNSSAI_SST, "sd": SNSSAI_SD} if SNSSAI_SD else {"sst": SNSSAI_SST}


def _snssai_json(snssai: dict) -> str:
    # Compact separators: the UDR's query-string decoder percent-decodes but
    # does not treat '+' as a form-urlencoded space, so a JSON value with
    # embedded spaces (json.dumps()'s default) can round-trip incorrectly
    # depending on the encoder used. Compact JSON sidesteps the ambiguity.
    return json.dumps(snssai, separators=(",", ":"))


def _snssai_hex_key() -> str:
    return f"{SNSSAI_SST:02x}{SNSSAI_SD}" if SNSSAI_SD else f"{SNSSAI_SST:02x}"


# ===========================================================================
# TRANSPORT -- curl over HTTP/2 prior-knowledge
# ===========================================================================


@dataclass
class Response:
    status: int
    headers: dict[str, str] = field(default_factory=dict)
    body: str = ""

    def header(self, name: str) -> str | None:
        return self.headers.get(name.lower())

    def json(self) -> dict | list | None:
        if not self.body.strip():
            return None
        try:
            return json.loads(self.body)
        except json.JSONDecodeError:
            return None


def curl_request(
    method: str, path: str, params: dict | None = None, body: dict | None = None
) -> Response:
    """Issue one request to http://UDR_HOST:UDR_PORT + path over HTTP/2
    prior-knowledge. `body`, if given, is sent as JSON.

    Returns a Response with the parsed status code, headers (lower-cased
    names), and raw body text.
    """
    url = f"http://{UDR_HOST}:{UDR_PORT}{path}"
    if params:
        # quote_via=quote (RFC 3986 %20) rather than the urlencode() default
        # of quote_plus ('+' for space) -- the UDR's query-string decoder
        # percent-decodes but does not additionally decode '+' as space.
        url += "?" + urllib.parse.urlencode(params, quote_via=urllib.parse.quote)

    argv = [
        "curl",
        "--http2-prior-knowledge",
        "-s",
        "-D",
        "-",
        "-X",
        method,
        "-H",
        "Accept: application/json",
        "--max-time",
        str(REQUEST_TIMEOUT),
        "--connect-timeout",
        "5",
    ]
    if body is not None:
        argv += ["-H", "Content-Type: application/json", "-d", json.dumps(body)]
    argv += ["-w", f"\n{_STATUS_MARKER}%{{http_code}}\n", url]

    full_argv = ["docker", "exec", UDR_CONTAINER] + argv if UDR_CONTAINER else argv

    result = subprocess.run(
        full_argv, capture_output=True, text=True, check=False,
        timeout=REQUEST_TIMEOUT + 5,
    )
    if result.returncode != 0:
        raise RuntimeError(
            f"curl failed (exit {result.returncode}) for {method} {url}: "
            f"{result.stderr.strip()}"
        )

    return _parse_response(result.stdout)


def _parse_response(raw: str) -> Response:
    marker_pos = raw.rfind(_STATUS_MARKER)
    if marker_pos == -1:
        raise RuntimeError(f"curl output missing status marker; got: {raw!r}")
    head_and_body = raw[:marker_pos]
    status_text = raw[marker_pos + len(_STATUS_MARKER) :].strip()
    status = int(status_text)

    # curl's `-D -` prints the synthesized status line ("HTTP/2 200"), then
    # header lines, then a blank line, then (since -o wasn't given) the body.
    if "\r\n\r\n" in head_and_body:
        header_block, _, body = head_and_body.partition("\r\n\r\n")
    else:
        header_block, _, body = head_and_body.partition("\n\n")

    headers: dict[str, str] = {}
    for line in header_block.splitlines()[1:]:  # skip the "HTTP/2 NNN" line
        if ":" in line:
            name, _, value = line.partition(":")
            headers[name.strip().lower()] = value.strip()

    return Response(status=status, headers=headers, body=body.strip())


# ===========================================================================
# ASSERTIONS / REPORTING
# ===========================================================================


class TestReport:
    """Accumulates pass/fail assertions without raising, so one failed check
    doesn't stop the rest of a scenario from running and reporting."""

    def __init__(self, name: str):
        self.name = name
        self.passed = 0
        self.failed = 0

    def check(self, description: str, condition: bool, detail: str = "") -> bool:
        if condition:
            self.passed += 1
            print(f"  [PASS] {description}")
        else:
            self.failed += 1
            suffix = f" -- {detail}" if detail else ""
            print(f"  [FAIL] {description}{suffix}")
        return condition

    def check_eq(self, description: str, expected, actual) -> bool:
        return self.check(
            description, expected == actual, f"expected {expected!r}, got {actual!r}"
        )

    def check_in(self, description: str, item, container) -> bool:
        return self.check(
            description, item in container, f"expected {item!r} in {container!r}"
        )

    def check_not_in(self, description: str, item, container) -> bool:
        return self.check(
            description, item not in container, f"expected {item!r} not in {container!r}"
        )

    def summary(self) -> bool:
        print(f"\n== {self.name}: {self.passed} passed, {self.failed} failed ==")
        return self.failed == 0

    def merge(self, other: "TestReport") -> None:
        self.passed += other.passed
        self.failed += other.failed


def _check_problem_details(report: TestReport, resp: Response) -> None:
    """A 4xx/5xx body should be ProblemDetails [TS 29.500 §5.2.7.2, RFC 7807]."""
    body = resp.json()
    is_dict = isinstance(body, dict)
    report.check(
        "error body is ProblemDetails with a 'title'",
        is_dict and "title" in body,
        f"got: {body!r}",
    )
    report.check_eq(
        "ProblemDetails 'status' matches the HTTP status",
        resp.status,
        body.get("status") if is_dict else None,
    )


def _safe_request(
    report: TestReport,
    method: str,
    path: str,
    params: dict | None = None,
    body: dict | None = None,
) -> Response:
    """curl_request(), but a transport failure (timeout, connection reset,
    server hang) becomes one failed check instead of an exception that would
    abort the whole suite -- a single unresponsive endpoint shouldn't hide
    results for every other scenario."""
    try:
        return curl_request(method, path, params, body)
    except RuntimeError as exc:
        report.check(f"{method} {path} completed", False, str(exc))
        return Response(status=-1)


# ===========================================================================
# GET am-data -- GET /policy-data/ues/{ueId}/am-data
# ===========================================================================


def get_am_data(
    ue_id: str,
    version: str = "v1",
    report: TestReport | None = None,
    expect_status: int = 200,
) -> Response:
    own_report = report or TestReport(f"get_am_data[{version}]")
    resp = _safe_request(
        own_report, "GET", f"/nudr-dr/{version}/policy-data/ues/{ue_id}/am-data"
    )
    if resp.status == -1:
        if report is None:
            own_report.summary()
        return resp

    own_report.check_eq(
        f"[{version}] GET am-data for UE {ue_id} returns {expect_status}",
        expect_status,
        resp.status,
    )

    data = resp.json()
    if expect_status == 200:
        own_report.check(
            "response is a JSON object", isinstance(data, dict), f"got: {data!r}"
        )
        if isinstance(data, dict):
            own_report.check_in("response has 'subscCats'", "subscCats", data)
    else:
        _check_problem_details(own_report, resp)

    if report is None:
        own_report.summary()
    return resp


# ===========================================================================
# GET sm-data -- GET /policy-data/ues/{ueId}/sm-data
# ===========================================================================


def get_sm_data(
    ue_id: str,
    version: str = "v1",
    report: TestReport | None = None,
    expect_status: int = 200,
    snssai: dict | None = None,
    dnn: str | None = None,
) -> Response:
    own_report = report or TestReport(f"get_sm_data[{version}]")

    params: dict[str, str] = {}
    if snssai is not None:
        params["snssai"] = _snssai_json(snssai)
    if dnn is not None:
        params["dnn"] = dnn

    resp = _safe_request(
        own_report,
        "GET",
        f"/nudr-dr/{version}/policy-data/ues/{ue_id}/sm-data",
        params or None,
    )
    if resp.status == -1:
        if report is None:
            own_report.summary()
        return resp

    filter_desc = ""
    if snssai is not None and dnn is not None:
        filter_desc = " filtered by SNSSAI+DNN"
    elif snssai is not None:
        filter_desc = " filtered by SNSSAI"
    elif dnn is not None:
        filter_desc = f" filtered by DNN '{dnn}'"

    own_report.check_eq(
        f"[{version}] GET sm-data for UE {ue_id}{filter_desc} returns {expect_status}",
        expect_status,
        resp.status,
    )

    data = resp.json()
    if expect_status == 200:
        own_report.check(
            "response is a JSON object", isinstance(data, dict), f"got: {data!r}"
        )
        sm_snssai_data = data.get("smPolicySnssaiData", {}) if isinstance(data, dict) else {}
        own_report.check_in(
            "response has 'smPolicySnssaiData'",
            "smPolicySnssaiData",
            data if isinstance(data, dict) else {},
        )

        non_hex_keys = [k for k in sm_snssai_data if k.startswith("{")]
        own_report.check(
            "smPolicySnssaiData keys are hex-encoded (not raw JSON)",
            not non_hex_keys,
            f"non-hex keys: {non_hex_keys}",
        )

        if snssai is not None:
            hex_key = _snssai_hex_key()
            own_report.check_in(
                f"smPolicySnssaiData contains SNSSAI hex key '{hex_key}'",
                hex_key,
                sm_snssai_data,
            )

        if dnn is not None:
            bad_dnns = [
                d
                for entry in sm_snssai_data.values()
                for d in entry.get("smPolicyDnnData", {})
                if d != dnn
            ]
            own_report.check(
                f"only DNN '{dnn}' present after DNN filter",
                not bad_dnns,
                f"unexpected DNNs: {bad_dnns}",
            )
    else:
        _check_problem_details(own_report, resp)

    if report is None:
        own_report.summary()
    return resp


# ===========================================================================
# GET ue-policy-set -- GET /policy-data/ues/{ueId}/ue-policy-set
# ===========================================================================


def get_ue_policy_set(
    ue_id: str,
    version: str = "v1",
    report: TestReport | None = None,
    expect_status: int = 200,
) -> Response:
    own_report = report or TestReport(f"get_ue_policy_set[{version}]")
    resp = _safe_request(
        own_report, "GET", f"/nudr-dr/{version}/policy-data/ues/{ue_id}/ue-policy-set"
    )
    if resp.status == -1:
        if report is None:
            own_report.summary()
        return resp

    own_report.check_eq(
        f"[{version}] GET ue-policy-set for UE {ue_id} returns {expect_status}",
        expect_status,
        resp.status,
    )

    if expect_status != 200:
        _check_problem_details(own_report, resp)

    if report is None:
        own_report.summary()
    return resp


# ===========================================================================
# Traffic Influence Data -- /application-data/influenceData [TS 29.519]
# ===========================================================================


def _influence_data_path(version: str) -> str:
    return f"/nudr-dr/{version}/application-data/influenceData"


def influence_data_query_params() -> dict:
    """The query the free5gc PCF sends while creating an SM policy."""
    return {
        "dnns": DNN,
        "snssais": json.dumps([_snssai()], separators=(",", ":")),
        "supis": f"imsi-{UE_IDS_STANDARD[0]}",
    }


def influence_data_subscription_body() -> dict:
    """A TrafficInfluSub like the one the free5gc PCF sends after the query."""
    return {
        "dnns": [DNN],
        "snssais": [_snssai()],
        "supis": [f"imsi-{UE_IDS_STANDARD[0]}"],
        "notificationUri": NOTIF_URI,
    }


def get_influence_data(
    version: str = "v1",
    report: TestReport | None = None,
    expect_status: int = 200,
) -> Response:
    own_report = report or TestReport(f"get_influence_data[{version}]")
    resp = _safe_request(
        own_report, "GET", _influence_data_path(version), influence_data_query_params()
    )
    if resp.status == -1:
        if report is None:
            own_report.summary()
        return resp

    own_report.check_eq(
        f"[{version}] GET influenceData returns {expect_status}",
        expect_status,
        resp.status,
    )
    if expect_status == 200:
        # The free5gc PCF only accepts a 200 with a JSON array here; anything
        # else leaves it dereferencing a nil response.
        own_report.check_in(
            "Content-Type is application/json",
            "application/json",
            resp.header("content-type") or "",
        )
        # No Traffic Influence Data is seeded (or stored) by the UDR.
        own_report.check_eq("response is an empty JSON array", [], resp.json())

    if report is None:
        own_report.summary()
    return resp


def create_influence_data_subscription(
    version: str = "v1",
    report: TestReport | None = None,
    body: dict | None = None,
    expect_status: int = 201,
) -> tuple[str | None, Response]:
    """POST a TrafficInfluSub. Returns (subscription_id, Response); the id is
    None unless the UDR returned a Location header."""
    own_report = report or TestReport(f"create_influence_data_subscription[{version}]")
    body = body if body is not None else influence_data_subscription_body()
    collection = _influence_data_path(version) + "/subs-to-notify"
    resp = _safe_request(own_report, "POST", collection, body=body)
    if resp.status == -1:
        if report is None:
            own_report.summary()
        return None, resp

    own_report.check_eq(
        f"[{version}] POST influenceData/subs-to-notify returns {expect_status}",
        expect_status,
        resp.status,
    )

    subscription_id = None
    if expect_status == 201:
        location = resp.header("location") or ""
        own_report.check("Location header is present", bool(location), f"got: {location!r}")
        if location:
            subscription_id = location.rstrip("/").rsplit("/", 1)[-1]
            own_report.check_eq(
                "Location points at the new subscription",
                f"{collection}/{subscription_id}",
                urllib.parse.urlparse(location).path,
            )
        data = resp.json()
        own_report.check_eq(
            "response echoes the notificationUri",
            body.get("notificationUri"),
            data.get("notificationUri") if isinstance(data, dict) else None,
        )

    if report is None:
        own_report.summary()
    return subscription_id, resp


def delete_influence_data_subscription(
    subscription_id: str,
    version: str = "v1",
    report: TestReport | None = None,
    expect_status: int = 204,
) -> Response:
    own_report = report or TestReport(f"delete_influence_data_subscription[{version}]")
    resp = _safe_request(
        own_report,
        "DELETE",
        f"{_influence_data_path(version)}/subs-to-notify/{subscription_id}",
    )
    if resp.status == -1:
        if report is None:
            own_report.summary()
        return resp

    own_report.check_eq(
        f"[{version}] DELETE influenceData/subs-to-notify/{subscription_id} "
        f"returns {expect_status}",
        expect_status,
        resp.status,
    )
    if expect_status != 204:
        _check_problem_details(own_report, resp)

    if report is None:
        own_report.summary()
    return resp


# ===========================================================================
# CONNECTIVITY
# ===========================================================================


def check_connectivity() -> bool:
    """Try a known-bad path to verify the UDR is reachable at all."""
    try:
        curl_request("GET", "/nudr-dr/v2/policy-data/ues/__probe__/am-data")
        return True
    except RuntimeError:
        return False


# ===========================================================================
# SUITE -- every scenario, across configured API versions (CI entrypoint)
# ===========================================================================


def run_suite() -> bool:
    """Runs every scenario below regardless of earlier failures, and reports
    one aggregate pass/fail count -- a single broken step doesn't hide
    problems in the rest of the suite."""
    overall = TestReport("policy_data_suite")

    print(f"Target: http://{UDR_HOST}:{UDR_PORT}  |  Versions: {', '.join(API_VERSIONS)}")
    print("Checking UDR connectivity...", end=" ")
    if not check_connectivity():
        print("UNREACHABLE")
        print(
            f"\nERROR: Cannot connect to UDR at http://{UDR_HOST}:{UDR_PORT}\n"
            f"Make sure the UDR is running and the DB is seeded with "
            f"oai_db_policy_data.sql.",
            file=sys.stderr,
        )
        return False
    print("OK\n")

    for version in API_VERSIONS:
        step = TestReport(f"am-data [{version}]")
        for ue_id in UE_IDS_STANDARD + UE_IDS_PREMIUM:
            get_am_data(ue_id, version, step)
        get_am_data(UE_ID_UNKNOWN, version, step, expect_status=404)
        overall.merge(step)
        step.summary()

        step = TestReport(f"sm-data [{version}]")
        ue_id = UE_IDS_STANDARD[0]
        get_sm_data(ue_id, version, step)
        get_sm_data(ue_id, version, step, snssai=_snssai())
        get_sm_data(ue_id, version, step, dnn=DNN)
        get_sm_data(ue_id, version, step, snssai=_snssai(), dnn=DNN)
        get_sm_data(UE_ID_UNKNOWN, version, step, expect_status=404)
        overall.merge(step)
        step.summary()

        step = TestReport(f"ue-policy-set [{version}]")
        # oai_db_policy_data.sql creates the UePolicySet table but seeds no
        # rows into it, so even a known UE has no policy set provisioned yet.
        get_ue_policy_set(UE_IDS_STANDARD[0], version, step, expect_status=404)
        get_ue_policy_set(UE_ID_UNKNOWN, version, step, expect_status=404)
        overall.merge(step)
        step.summary()

        step = TestReport(f"influence-data [{version}]")
        get_influence_data(version, step)
        subscription_id, _ = create_influence_data_subscription(version, step)
        # notificationUri is mandatory in TrafficInfluSub.
        create_influence_data_subscription(
            version, step, body={"dnns": [DNN]}, expect_status=400
        )
        if subscription_id:
            delete_influence_data_subscription(subscription_id, version, step)
            delete_influence_data_subscription(
                subscription_id, version, step, expect_status=404
            )
        else:
            step.check("POST returned a subscription id to delete", False)
        overall.merge(step)
        step.summary()

    print()
    return overall.summary()


# ===========================================================================
# CLI
# ===========================================================================


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    sub = parser.add_subparsers(dest="command", required=True)

    p_am = sub.add_parser("am-data", help="GET policy-data/ues/{ueId}/am-data")
    p_am.add_argument("ue_id")
    p_am.add_argument("--version", default="v1")
    p_am.add_argument("--expect-status", type=int, default=200)

    p_sm = sub.add_parser("sm-data", help="GET policy-data/ues/{ueId}/sm-data")
    p_sm.add_argument("ue_id")
    p_sm.add_argument("--version", default="v1")
    p_sm.add_argument("--snssai", metavar="JSON", default=None, help='e.g. \'{"sst":222,"sd":"00007b"}\'')
    p_sm.add_argument("--dnn", default=None)
    p_sm.add_argument("--expect-status", type=int, default=200)

    p_ups = sub.add_parser("ue-policy-set", help="GET policy-data/ues/{ueId}/ue-policy-set")
    p_ups.add_argument("ue_id")
    p_ups.add_argument("--version", default="v1")
    p_ups.add_argument("--expect-status", type=int, default=200)

    p_idg = sub.add_parser("influence-data", help="GET application-data/influenceData")
    p_idg.add_argument("--version", default="v1")
    p_idg.add_argument("--expect-status", type=int, default=200)

    p_ids = sub.add_parser(
        "influence-data-subscribe",
        help="POST application-data/influenceData/subs-to-notify",
    )
    p_ids.add_argument("--version", default="v1")

    p_idu = sub.add_parser(
        "influence-data-unsubscribe",
        help="DELETE application-data/influenceData/subs-to-notify/{subscriptionId}",
    )
    p_idu.add_argument("subscription_id")
    p_idu.add_argument("--version", default="v1")
    p_idu.add_argument("--expect-status", type=int, default=204)

    sub.add_parser("suite", help="run every scenario in sequence (CI entrypoint)")

    args = parser.parse_args()

    if args.command == "am-data":
        report = TestReport("get_am_data")
        get_am_data(args.ue_id, args.version, report, expect_status=args.expect_status)
        return 0 if report.summary() else 1

    if args.command == "sm-data":
        report = TestReport("get_sm_data")
        snssai = json.loads(args.snssai) if args.snssai else None
        get_sm_data(
            args.ue_id,
            args.version,
            report,
            expect_status=args.expect_status,
            snssai=snssai,
            dnn=args.dnn,
        )
        return 0 if report.summary() else 1

    if args.command == "ue-policy-set":
        report = TestReport("get_ue_policy_set")
        get_ue_policy_set(
            args.ue_id, args.version, report, expect_status=args.expect_status
        )
        return 0 if report.summary() else 1

    if args.command == "influence-data":
        report = TestReport("get_influence_data")
        get_influence_data(args.version, report, expect_status=args.expect_status)
        return 0 if report.summary() else 1

    if args.command == "influence-data-subscribe":
        report = TestReport("create_influence_data_subscription")
        subscription_id, _ = create_influence_data_subscription(args.version, report)
        if subscription_id:
            print(f"subscription_id: {subscription_id}", file=sys.stderr)
        return 0 if report.summary() else 1

    if args.command == "influence-data-unsubscribe":
        report = TestReport("delete_influence_data_subscription")
        delete_influence_data_subscription(
            args.subscription_id, args.version, report, expect_status=args.expect_status
        )
        return 0 if report.summary() else 1

    if args.command == "suite":
        return 0 if run_suite() else 1

    parser.error(f"unknown command: {args.command}")  # pragma: no cover
    return 2


if __name__ == "__main__":
    sys.exit(main())
