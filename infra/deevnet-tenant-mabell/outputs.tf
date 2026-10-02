# What the gateway is flashed with, and what this tenant was issued.
#
# Every one of these is sensitive and lives only in this tenant's state. The
# API keeps its own copy of some and a hash of others, and restores what it can
# (ADR-0012 §4, §5) - but this state is the authoritative copy.

output "device_wifi" {
  description = "What the gateway associates with. Do not hardcode the SSID - it differs by site."
  sensitive   = true
  value = {
    ssid = deevnet_iot_wifi_key.devices.ssid
    psk  = deevnet_iot_wifi_key.devices.psk
    vlan = deevnet_iot_wifi_key.devices.vlan
  }
}

output "broker" {
  description = "Where the gateway dials, and what the broker's certificate is verified against."
  value = {
    host = var.broker_host
    port = 8883
    ca   = "deevnet-mobile-root-ca.pem"
  }
}

output "gateway_broker" {
  description = "The gateway's MQTT credential and the exact topics it was granted."
  sensitive   = true
  value = {
    username  = deevnet_iot_broker_account.gateway.username
    password  = deevnet_iot_broker_account.gateway.password
    publish   = deevnet_iot_broker_account.gateway.granted_publish
    subscribe = deevnet_iot_broker_account.gateway.granted_subscribe
  }
}

# The tenant's own log store credentials (ADR-0027, CHG-0020).
#
# The gateway does NOT use these: it publishes to MQTT, and the substrate's
# bridge carries those messages into this tenant's device partition. These are
# for reading its logs back, and for anything this tenant later runs that ships
# its own.
output "log_store" {
  description = "Reading this tenant's logs, and writing from anything it runs itself."
  sensitive   = true
  value = {
    endpoint      = deevnet_tenant.mabell.log_endpoint
    account_id    = deevnet_tenant.mabell.log_account_id
    select_header = deevnet_tenant.mabell.log_select_header
    ingest_token  = deevnet_tenant.mabell.log_ingest_token
    read_token    = deevnet_tenant.mabell.log_read_token
  }
}

output "api_token" {
  description = "This tenant's own API token. Every apply after the first uses it."
  sensitive   = true
  value       = deevnet_tenant.mabell.api_token
}

output "state_backend" {
  description = "The state store this tenant was issued. `make state-backend` prints it as a backend block."
  sensitive   = true
  value = {
    endpoint   = deevnet_tenant.mabell.state_endpoint
    bucket     = deevnet_tenant.mabell.state_bucket
    key        = "${deevnet_tenant.mabell.state_key_prefix}terraform.tfstate"
    access_key = deevnet_tenant.mabell.state_access_key
    secret_key = deevnet_tenant.mabell.state_secret_key
  }
}
