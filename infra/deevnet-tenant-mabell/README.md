# The Ma Bell tenant

This directory is the gateway's place on the [Deevnet](https://github.com/deevnet) substrate: its
network identity, the Wi-Fi key it associates with, its entry in the device registry, and its MQTT
account.

It lives here, beside the firmware, rather than in its own repository, so the device and the
infrastructure it depends on stay together. EdS made the same choice for the same reason.

## What it declares

| | |
|---|---|
| `deevnet_tenant.mabell` | the tenant itself. Everything else derives from the index the API allocates |
| `deevnet_iot_wifi_key.devices` | one PPSK for this tenant's devices, in trust class `iot` |
| `deevnet_iot_device.gateway` | `ma-bell-gw-01` in the registry: an identity, no credential |
| `deevnet_iot_broker_account.gateway` | the gateway's MQTT username and password, and its topics |

**No workload.** There is no backend service yet, and a VM that nothing runs on would cost memory on
the tenant hypervisor and prove nothing. One is added when there is something to put on it.

## What the substrate issued

Index **3**, so this tenant's overlay is `10.20.131.0/24` and its log partitions are account 3. The
gateway's account is `mabell-ma-bell-gw-01`, granted exactly two topics:

```
mabell/log/ma-bell-gw-01          its log messages
mabell/phone/ma-bell-gw-01/state  its own state (provisional; nothing consumes it yet)
```

Note the `mabell/` prefix: this configuration declares topics **relative** to the tenant, and the API
writes the prefix itself. That is what stops one tenant granting itself another's topics.

It subscribes to nothing, because the gateway takes no commands over MQTT today.

## Logs

The gateway does not ship logs itself. It publishes them to `mabell/log/ma-bell-gw-01`, and a
substrate bridge carries them into this tenant's device log partition, where `log_store.read_token`
reads them back. `log` is a reserved first topic level, and this is what it is for.

The firmware does not do this yet: its MQTT client is present but unused, and it logs to serial
only. That is the next piece of work on the device side.

## Using it

```bash
export DEEVNET_API_TOKEN=$(terraform output -raw api_token)   # enrollment token on the first apply
make plan
make apply
```

`deevnet-root-ca.pem` and the state file are **not** committed: the state holds every credential this tenant
was issued, and it is the authoritative copy of most of them. Ask the operator for the CA.

To move the state into the substrate's store, run `make state-backend`, uncomment the backend block
in `main.tf` with what it prints, and `terraform init -migrate-state`.

## Careful

Replacing the Wi-Fi key or the broker account **issues new credentials**, and the gateway stops
connecting until it is reflashed. Losing the API's database does *not* do that: it restores both
from this state.
