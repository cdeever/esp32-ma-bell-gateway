# The Ma Bell tenant.
#
# The gateway in this repository is an edge device: an ESP32 that lets a
# vintage telephone place and receive calls through a paired mobile. This
# directory is the tenant it belongs to on the Deevnet substrate - its network
# identity, its Wi-Fi key, its place in the device registry, and its MQTT
# account.
#
# It declares NO workload. There is no backend service yet: the gateway talks
# to the broker and to nothing else this tenant owns. A VM that nothing runs on
# would cost memory on the tenant hypervisor and prove nothing. One is added
# when there is something to put on it.
#
# Everything here goes through the Deevnet API (ADR-0015). This tenant holds
# one credential, its Deevnet token, and no substrate credential at all.
#
# Like EdS, and unlike the pattern in ADR-0006 where the repository *is* the
# tenant, this lives inside the application's own repository so the firmware
# and the infrastructure it depends on stay together.

terraform {
  required_version = ">= 1.5"

  required_providers {
    deevnet = {
      source  = "deevnet/deevnet"
      version = "~> 0.1"
    }
  }

  # The substrate's state store (ADR-0007). Its credentials are outputs of the
  # tenant below, so it is configured AFTER the first apply: `make
  # state-backend`, then `terraform init -migrate-state`.
  #
  # Until then the state is the local file, and that file holds every
  # credential this tenant was issued. Losing it means asking the API to
  # restore them; committing it would publish them.
  #
  # backend "s3" {
  #   bucket       = "tf-state"
  #   key          = "tenants/mabell/terraform.tfstate"
  #   region       = "us-east-1"
  #   endpoints    = { s3 = "http://tfstate.mobile.deevnet.net:9000" }
  #   use_lockfile = true
  #
  #   skip_credentials_validation = true
  #   skip_region_validation      = true
  #   skip_requesting_account_id  = true
  #   skip_metadata_api_check     = true
  #   skip_s3_checksum            = true
  #   use_path_style              = true
  # }
}

# DEEVNET_API_ENDPOINT, DEEVNET_API_TOKEN, DEEVNET_API_CACERT.
provider "deevnet" {}

resource "deevnet_tenant" "mabell" {
  name = "mabell"
}

# The Wi-Fi key the gateway associates with (ADR-0012 §3, CHG-0013).
#
# One key per tenant per trust class, not one per device. The SSID and the VLAN
# come back from the API: a tenant does not pick a VLAN, which is what keeps
# this tenant's devices on the segment the substrate intends.
#
# CAREFUL: replacing this resource issues a NEW key, and any device already
# flashed with the old one stops associating until it is reflashed. A lost API
# database does NOT do that - it restores this key from state.
resource "deevnet_iot_wifi_key" "devices" {
  tenant      = deevnet_tenant.mabell.name
  name        = "devices"
  trust_class = "iot"
}

# The gateway, in the tenant's device registry (ADR-0012 §3, CHG-0014).
#
# The entry is an identity and nothing else: no credential, no access. What it
# is for is the pairing rule below - a broker account for a device may only be
# issued against a device in trust class `iot`, the class for devices whose
# firmware its owner controls.
#
# The MAC is deliberately unset: it is a label for the owner's own inventory,
# the substrate enforces nothing with it, and recording one here would make a
# first flash wait on a registration.
resource "deevnet_iot_device" "gateway" {
  tenant      = deevnet_tenant.mabell.name
  name        = "ma-bell-gw-01"
  trust_class = "iot"
}

# The gateway's MQTT account (ADR-0012 §3, §10; CHG-0016).
#
# Topic patterns are RELATIVE to the tenant. The API writes the "mabell/"
# prefix itself, which is what confines this tenant to its own topics - so
# write "log/ma-bell-gw-01", never "mabell/log/ma-bell-gw-01". They come back
# absolute in granted_publish and granted_subscribe, which is what the broker
# enforces.
#
# Two grants, and no more than the device can currently use:
#
#   log/<device>      its log messages, which the substrate's bridge carries
#                     into this tenant's log partition (ADR-0027 §3). `log` is
#                     a reserved first level and this is what it is for.
#   phone/<device>/…  the gateway's own state: off-hook, ringing, dialling.
#                     Provisional, because no service consumes it yet. It is
#                     here so the firmware has somewhere to report to while the
#                     application side is written, and it is scoped to THIS
#                     device rather than the tenant's whole tree.
#
# It subscribes to nothing. The gateway takes no commands over MQTT today, and
# a subscription it does not use is an access it does not need.
#
# CAREFUL: replacing this resource issues a NEW password, and the gateway stops
# connecting until it is reflashed. A lost API database does NOT do that - it
# restores this account from state (ADR-0012 §5).
resource "deevnet_iot_broker_account" "gateway" {
  tenant = deevnet_tenant.mabell.name
  name   = "ma-bell-gw-01"
  device = deevnet_iot_device.gateway.name

  publish = [
    "log/ma-bell-gw-01",
    "phone/ma-bell-gw-01/state",
  ]
  subscribe = []
}
