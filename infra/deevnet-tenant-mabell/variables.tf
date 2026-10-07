# Ma Bell declares almost nothing.
#
# The index, subnet, gateway and every identifier derived from them are the
# API's to issue (ADR-0015), and this tenant has no workload to size. What is
# left is one substrate name the API does not hand out, and one fact about the
# hardware that only its owner knows.

variable "gateway_mac" {
  type        = string
  description = <<-EOT
    The Wi-Fi station MAC of the ESP32 that is the gateway, in any usual
    spelling. Its fixed address is reserved for it.

    No default, on purpose: it identifies one board, and this repository is
    public. Set it in a *.auto.tfvars file, which is ignored by git. The
    firmware prints it at boot.
  EOT
}

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
