# The state store the substrate issued this tenant (ADR-0007). Not secret: the
# credentials are in .backend.env, which `make state-backend` writes and git
# ignores.
#
# The state holds every credential this tenant was issued, so it lives here
# and not in this public repository. Anyone working on this tenant from
# another machine needs three things that are not committed: .backend.env,
# deevnet-root-ca.pem and gateway.auto.tfvars.
terraform {
  backend "s3" {
    bucket           = "tf-state"
    key              = "tenants/mabell/terraform.tfstate"
    region           = "us-east-1"
    endpoints        = { s3 = "https://tfstate.mobile.deevnet.net:9000" }
    custom_ca_bundle = "deevnet-root-ca.pem"
    use_lockfile     = true

    # MinIO, not AWS.
    skip_credentials_validation = true
    skip_region_validation      = true
    skip_requesting_account_id  = true
    skip_metadata_api_check     = true
    skip_s3_checksum            = true
    use_path_style              = true
  }
}
