# A name of this tenant's own for its dashboard: grafana.<tenant zone>.
#
# The dashboard is the substrate's Grafana, reached by a substrate host name.
# This gives it a name in the tenant's zone instead, so a bookmark says whose
# dashboard it is and does not have to change if the substrate moves Grafana.
#
# It is a CNAME, and deevnet_dns_record cannot make one: that resource
# publishes addresses, and only addresses inside this tenant's own network.
# So this is written the other way a tenant may publish names: a signed
# dynamic update (RFC 2136) to the zone's update server, with the TSIG key the
# tenant was issued. The server accepts updates from the operator networks and
# from the tenant's own, not from the tenant developer network.
#
# The target is read out of dashboard_url rather than written here, so the
# name follows the dashboard if the API starts answering with another host.
#
# CAREFUL: this is the name and nothing more. Grafana's certificate is issued
# for the substrate's host name, and Grafana believes that name is its own, so
# a browser warns about the certificate at this one and Grafana's own links
# lead back to the other. Both are the substrate's to change. Until they are,
# the Grafana provider in dashboards.tf and the `dashboard` output's url stay
# on the substrate's name.

locals {
  dashboard_host = regex("^https?://([^:/]+)", deevnet_tenant.mabell.dashboard_url)[0]
}

provider "dns" {
  update {
    server        = deevnet_tenant.mabell.dns_update_server
    key_name      = "${deevnet_tenant.mabell.tsig_key_name}."
    key_algorithm = deevnet_tenant.mabell.tsig_algorithm
    key_secret    = deevnet_tenant.mabell.tsig_secret
  }
}

resource "dns_cname_record" "grafana" {
  zone  = "${deevnet_tenant.mabell.dns_zone}."
  name  = "grafana"
  cname = "${local.dashboard_host}."
  ttl   = 300
}
