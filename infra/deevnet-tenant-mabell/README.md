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
| `deevnet_iot_address.gateway` | the gateway's fixed address on the device network, and its name `ma-bell-gw-01.<tenant zone>` |

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

The firmware does not do this yet: it logs to serial only. It does connect to the broker and
publish its state, retained, to `mabell/phone/ma-bell-gw-01/state`, with a last will that marks it
offline.

## Putting it on the gateway

```bash
. $IDF_PATH/export.sh
tools/provision_tenant.py            # --dry-run to build the image without touching the board
```

This reads the outputs here and writes the Wi-Fi key, the broker address, the gateway's account, its
topics and the root CA to the board's NVS partition. Nothing is compiled into the firmware. It
replaces the whole partition, so the phone has to be paired again afterwards.

To test from where the gateway sits, `tools/wifi-tenant.sh` moves a Mac onto the device network with
this tenant's key and back: `save` once on the operator network, then `use mabell` and `home`.

## Using it

```bash
export DEEVNET_API_TOKEN=$(terraform output -raw api_token)   # enrollment token on the first apply
make plan
make apply
```

The state is in the substrate's state store (`backend.tf`), not in this repository: it holds every
credential this tenant was issued, and it is the authoritative copy of most of them.

Three files are **not** committed and have to be put in this directory on each machine:

| File | What it is | Where it comes from |
|---|---|---|
| `.backend.env` | the state store's credentials | a machine that already has the state: `make state-backend` writes it |
| `deevnet-root-ca.pem` | the site's root CA; public | the operator, or the tenant downloads site |
| `gateway.auto.tfvars` | the gateway board's MAC | you: `gateway_mac = "..."` |

With those in place, `terraform init` reads the state from the store. Needs Terraform 1.10 or later.

The gateway's address is reserved for its board's Wi-Fi MAC, which is not committed either. Put it in
`gateway.auto.tfvars` as `gateway_mac = "..."`; the firmware prints it at boot. Needs provider 0.6 or
later (`terraform init -upgrade`). `terraform output gateway_address` shows where the gateway is.

## Careful

Replacing the Wi-Fi key or the broker account **issues new credentials**, and the gateway stops
connecting until it is reflashed. Losing the API's database does *not* do that: it restores both
from this state.
