# UDR policy-data tests

Requires Docker (with `docker compose`), `curl` and Python 3. There are no pip dependencies.

Run from the repository root:

```bash
# 1. Build the UDR from this checkout and start it with MySQL, seeded with
#    build/scripts/oai_db_v2.sql, oai_db_policy_data.sql and
#    ci-scripts/tests/database/oai_db_policy_data_tests.sql
docker compose -f ci-scripts/tests/docker-compose.yaml up -d --build

# 2. Wait until both containers report (healthy)
docker compose -f ci-scripts/tests/docker-compose.yaml ps

# 3. Run the full suite (exits 0 if everything passes)
python3 ci-scripts/tests/test_policy_data.py suite

# 4. Tear down
docker compose -f ci-scripts/tests/docker-compose.yaml down -v
```

`--build` rebuilds the UDR image (`oai-udr:policy-data-test`) from your working tree, so the
tests always run against your local changes. To test an image you already have, set
`UDR_TEST_IMAGE` and leave out `--build`:

```bash
UDR_TEST_IMAGE=oaisoftwarealliance/oai-udr:develop \
  docker compose -f ci-scripts/tests/docker-compose.yaml up -d
```

To check a single endpoint, run one subcommand, for example
`python3 ci-scripts/tests/test_policy_data.py sm-data 208950000000040 --dnn internet`.
`--help` lists all subcommands and the environment variables (`UDR_HOST`, `UDR_PORT`, ...).
