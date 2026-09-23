# Ma Bell declares almost nothing.
#
# The index, subnet, gateway and every identifier derived from them are the
# API's to issue (ADR-0015), and this tenant has no workload to size. What is
# left is one substrate name the API does not hand out.

variable "broker_host" {
  type        = string
  default     = "mqtt.mobile.deevnet.net"
  description = <<-EOT
    The substrate's MQTT broker, which the gateway dials.

    A variable rather than a hardcoded string because it is a SUBSTRATE name,
    not Ma Bell's: moving this tenant to another site means pointing at that
    site's broker. It is not an attribute of the tenant because the API does
    not issue one today.
  EOT
}
