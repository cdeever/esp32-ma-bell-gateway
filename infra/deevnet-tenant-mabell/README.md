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
| `deevnet_iot_wifi_key.developer` | one key for a developer's computer on `DVNTM-TD`, in trust class `tenant_dev` |
| `deevnet_iot_device.gateway` | `ma-bell-gw-01` in the registry: an identity, no credential |
| `deevnet_iot_broker_account.gateway` | the gateway's MQTT username and password, and its topics |
| `deevnet_iot_address.gateway` | the gateway's fixed address on the device network, and its name `ma-bell-gw-01.<tenant zone>` |
| `dns_cname_record.grafana` | `grafana.<tenant zone>`, this tenant's own name for its dashboard |

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

The gateway does not ship logs itself. It publishes its key events to `mabell/log/ma-bell-gw-01`, and
a substrate bridge carries them into this tenant's device log partition, where `log_store.read_token`
reads them back. `log` is a reserved first topic level, and this is what it is for.

Only events worth keeping are sent, not the serial log: the gateway starting, WiFi and the broker
connecting, the handset lifted and replaced, ringing, the mobile phone connecting, a call starting
and ending. Each is one JSON object:

```json
{"level":"info","event":"call.started","msg":"Call started","uptime_ms":812345}
```

The gateway has no clock, so the store dates each event when it arrives. Events queued while the
gateway was offline arrive together; `uptime_ms` gives their order. If the queue overflowed, the next
event carries a `dropped` count.

Two ways to read them:

| | |
|---|---|
| `tools/gateway_logs.py` | prints the events in a terminal: `--since 1d`, `-f` to follow, and a LogsQL filter such as `'event:call.*'` |
| Grafana | the dashboard **Ma Bell Gateway** in this tenant's organization, declared in `dashboards.tf`: calls, handset lifts, restarts, events by kind, WiFi signal, and the events themselves |

The dashboard needs this tenant's Grafana password. The tenant existed before dashboards did, so the
operator handed the password over and the state does not hold it: put it in a git-ignored
`*.auto.tfvars` as `dashboard_password = "..."` and apply. `terraform output dashboard` says where to
log in.

### A name for the dashboard

`grafana.mabell.mobile.deevnet.net` is an alias, in this tenant's zone, for the substrate host that
serves the dashboard (`dns.tf`). The API's own record resource only publishes addresses inside the
tenant's network, so the alias is written as a signed dynamic update with the tenant's TSIG key.
That update is accepted from the operator networks and the tenant's own network, **not from
`DVNTM-TD`**: apply a change to it from the operator network.

It is the name and nothing more. Grafana's certificate is issued for the substrate's host name, so a
browser warns about the certificate at `https://grafana.mabell.mobile.deevnet.net:3000`, and
anything that verifies certificates refuses it. Putting this name on the certificate is the
substrate's change to make. Until then Terraform and `terraform output dashboard`'s `url` keep using
the substrate's name, and the alias is listed beside it.

## Putting it on the gateway

```bash
. $IDF_PATH/export.sh
tools/provision_tenant.py            # --dry-run to build the image without touching the board
```

This reads the outputs here and writes the Wi-Fi key, the broker address, the gateway's account, its
topics and the root CA to the board's NVS partition. Nothing is compiled into the firmware. It
replaces the whole partition, so the phone has to be paired again afterwards.

## Working on it from a Mac

`tools/wifi-tenant.sh` moves a Mac between the operator network and this tenant's networks. Run
`save` and `save-home` once, then:

| | |
|---|---|
| `use mabell` | join the tenant developer network (`DVNTM-TD`) with this tenant's developer key: the API, the state store and the broker are reachable. The gateway's web page is not: the substrate admits only the operator networks to it today |
| `use mabell device` | join the device network (`DVNTM-IOT`) with the device key, to see what the gateway sees: the broker and the internet, nothing else |
| `home` | back to the operator network |

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
| `gateway.auto.tfvars` | the gateway board's MAC, and the Grafana password | you: `gateway_mac = "..."`, `dashboard_password = "..."` |

With those in place, `terraform init` reads the state from the store. Needs Terraform 1.10 or later.

The gateway's address is reserved for its board's Wi-Fi MAC, which is not committed either. Put it in
`gateway.auto.tfvars` as `gateway_mac = "..."`; the firmware prints it at boot. Needs provider 0.6 or
later (`terraform init -upgrade`). `terraform output gateway_address` shows where the gateway is.

## Careful

Replacing the Wi-Fi key or the broker account **issues new credentials**, and the gateway stops
connecting until it is reflashed. Losing the API's database does *not* do that: it restores both
from this state.
