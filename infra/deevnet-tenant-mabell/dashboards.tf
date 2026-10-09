# The gateway's dashboard, in this tenant's Grafana organization (ADR-0024).
#
# The gateway publishes its key events to its MQTT log topic, the substrate's
# bridge carries them into this tenant's device log partition, and the
# organization the tenant was issued already has that partition wired in as
# the data source `deevnet-logs-devices`. What is declared here is only the
# dashboard that reads it.
#
# Kept in code because the platform treats Grafana's own database as
# rebuildable: a dashboard built by clicking does not come back, and this one
# does on the next apply.
#
# The login is the tenant's own. A tenant that existed before dashboards did
# was handed its password by the operator, and its state does not hold it: set
# it as dashboard_password in a git-ignored *.auto.tfvars. Until one or the
# other is there, nothing below is created and the rest of the tenant plans
# and applies as before.

locals {
  dashboard_password = coalesce(deevnet_tenant.mabell.dashboard_password, var.dashboard_password, "unset")
  dashboards_enabled = local.dashboard_password != "unset"
}

provider "grafana" {
  url     = deevnet_tenant.mabell.dashboard_url
  auth    = "${deevnet_tenant.mabell.dashboard_username}:${local.dashboard_password}"
  ca_cert = "${path.module}/deevnet-root-ca.pem"
}

# org_id is on every resource on purpose: under a username and password the
# provider ignores its own org_id setting and sends organization 1, which this
# tenant is not a member of.
resource "grafana_folder" "gateway" {
  count  = local.dashboards_enabled ? 1 : 0
  org_id = deevnet_tenant.mabell.dashboard_org_id
  title  = "Ma Bell"
}

resource "grafana_dashboard" "gateway" {
  count       = local.dashboards_enabled ? 1 : 0
  org_id      = deevnet_tenant.mabell.dashboard_org_id
  folder      = grafana_folder.gateway[0].uid
  config_json = file("${path.module}/dashboards/gateway.json")
}
