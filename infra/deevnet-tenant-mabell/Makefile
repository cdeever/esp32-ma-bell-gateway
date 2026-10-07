# Ma Bell tenant.
#
# This tenant holds one credential: its Deevnet API token. No vault, no Proxmox
# credential, no index. The substrate issues the rest through the API
# (ADR-0015), and it lands in this tenant's Terraform state.
.ONESHELL:
SHELL := /usr/bin/bash
.SHELLFLAGS := -euo pipefail -c

# The state store's credentials (backend.tf). Written by `make state-backend`,
# or copied from a machine that already has it; never committed.
-include .backend.env
export AWS_ACCESS_KEY_ID AWS_SECRET_ACCESS_KEY

export DEEVNET_API_ENDPOINT ?= https://api.mobile.deevnet.net:8080
export DEEVNET_API_CACERT   ?= $(CURDIR)/deevnet-root-ca.pem

TF_APPROVE := $(if $(AUTO),-auto-approve,)

.PHONY: help require-token init plan apply destroy state-backend fmt validate

help:
	@echo "mabell - tenant (inside the ma-bell gateway repository)"
	@echo
	@echo "  init           terraform init"
	@echo "  plan           terraform plan"
	@echo "  apply          terraform apply    (AUTO=1 to skip approval)"
	@echo "  destroy        terraform destroy  (AUTO=1 to skip approval)"
	@echo "  state-backend  write .backend.env, the state store's credentials, from the state"
	@echo "  validate       fmt check + validate"
	@echo
	@echo "Needs DEEVNET_API_TOKEN: the enrollment token on the first apply,"
	@echo "then the api_token output. Endpoint: $(DEEVNET_API_ENDPOINT)"

require-token:
	@if [[ -z "$${DEEVNET_API_TOKEN:-}" ]]; then
		echo "DEEVNET_API_TOKEN is not set." >&2
		echo "  First apply:  the enrollment token the substrate issued at admission." >&2
		echo "  After that:   terraform output -raw api_token" >&2
		exit 2
	fi

init:
	terraform init

plan: require-token
	terraform plan

apply: require-token
	terraform apply $(TF_APPROVE)

destroy: require-token
	terraform destroy $(TF_APPROVE)

# Writes the state store's credentials where terraform's s3 backend reads them.
# Needs the state, so it runs on a machine that already has it: this is how the
# file is made to carry to another one. Nothing is printed.
state-backend:
	@umask 077; terraform output -json state_backend | python3 -c '
	import json, sys
	d = json.load(sys.stdin)
	print("AWS_ACCESS_KEY_ID=%s" % d["access_key"])
	print("AWS_SECRET_ACCESS_KEY=%s" % d["secret_key"])
	' > .backend.env
	@echo "wrote .backend.env (secret, not committed): the key to this tenant's state"

fmt:
	terraform fmt -recursive

validate:
	terraform fmt -check -recursive
	terraform validate
