# UDR policy-data tests

Requires Docker (with `docker compose`), `curl` and Python 3. There are no pip dependencies.

Run from the repository root:

```bash
# 1. Start MySQL (seeded with build/scripts/oai_db_v2.sql + oai_db_policy_data.sql) and the UDR
docker compose -f ci-scripts/tests/docker-compose.yaml up -d

# 2. Wait until both containers report (healthy)
docker compose -f ci-scripts/tests/docker-compose.yaml ps

# 3. Run the full suite (exits 0 if everything passes)
python3 ci-scripts/tests/test_policy_data.py suite

# 4. Tear down
docker compose -f ci-scripts/tests/docker-compose.yaml down -v
```

To check a single endpoint, run one subcommand, for example
`python3 ci-scripts/tests/test_policy_data.py sm-data 208950000000031 --dnn default`.
`--help` lists all subcommands and the environment variables (`UDR_HOST`, `UDR_PORT`, ...).

To test local changes, rebuild the image the compose file uses, then recreate the UDR container:

```bash
docker build --tag oaisoftwarealliance/oai-udr:fix-ueid-v1.28-09 --file docker/Dockerfile.udr.ubuntu .
docker compose -f ci-scripts/tests/docker-compose.yaml up -d --force-recreate oai-udr
```
