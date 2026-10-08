#!/usr/bin/env bash
#
# Switch this Mac's Wi-Fi between the operator network and a Deevnet tenant's
# device network, for end-to-end testing from where the tenant's devices sit.
#
# Tenants share one device SSID and differ only by key (PPSK), and macOS
# remembers one password per SSID, so the key is passed on every join. It is
# kept in the login Keychain, because the tenant's Terraform state cannot be
# reached from the device network.
#
#   wifi-tenant.sh save [tenant]   store the tenant's SSID and key in the Keychain
#                                  (run on the operator network, where the state is)
#   wifi-tenant.sh use <tenant>    join the device network with that tenant's key
#   wifi-tenant.sh home            rejoin the operator network
#   wifi-tenant.sh status          show where this Mac is now
#
# macOS only.

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
HOME_SSID="${DEEVNET_HOME_SSID:-DVNTM}"
DEFAULT_TENANT="mabell"
JOIN_TIMEOUT_SEC=20

die() { echo "Error: $*" >&2; exit 1; }

wifi_device() {
    networksetup -listallhardwareports | awk '/Hardware Port: Wi-Fi/ { getline; print $2; exit }'
}

keychain_service() { echo "deevnet-wifi-$1"; }

tenant_dir() {
    local dir="$REPO_ROOT/infra/deevnet-tenant-$1"
    [[ -d "$dir" ]] || die "no tenant directory $dir"
    echo "$dir"
}

# macOS redacts the SSID from unprivileged tools, so report the address instead
status() {
    local dev addr router
    dev="$(wifi_device)"
    addr="$(ipconfig getifaddr "$dev" 2>/dev/null || true)"
    router="$(ipconfig getoption "$dev" router 2>/dev/null || true)"
    if [[ -z "$addr" ]]; then
        echo "Wi-Fi ($dev): no address"
    else
        echo "Wi-Fi ($dev): $addr, router $router"
    fi
}

wait_for_address() {
    local dev="$1" i
    for ((i = 0; i < JOIN_TIMEOUT_SEC; i++)); do
        if ipconfig getifaddr "$dev" >/dev/null 2>&1; then
            return 0
        fi
        sleep 1
    done
    return 1
}

join() {
    local ssid="$1" key="${2:-}" dev out
    dev="$(wifi_device)"
    [[ -n "$dev" ]] || die "no Wi-Fi interface found"

    # networksetup reports failure on stdout and still exits 0
    if [[ -n "$key" ]]; then
        out="$(networksetup -setairportnetwork "$dev" "$ssid" "$key" 2>&1)"
    else
        out="$(networksetup -setairportnetwork "$dev" "$ssid" 2>&1)"
    fi
    [[ -z "$out" ]] || die "could not join $ssid: $out"

    wait_for_address "$dev" || die "joined $ssid but got no address within ${JOIN_TIMEOUT_SEC}s"
    echo "Joined $ssid"
    status
}

save() {
    local tenant="$1" dir wifi ssid key
    dir="$(tenant_dir "$tenant")"
    # The state store's credentials, as the tenant's Makefile reads them
    wifi="$(cd "$dir" && { [[ ! -f .backend.env ]] || { set -a; . ./.backend.env; set +a; }; } \
        && terraform output -json device_wifi)" \
        || die "could not read the tenant's outputs - is the state reachable from this network?"
    ssid="$(python3 -c 'import json,sys; print(json.load(sys.stdin)["ssid"])' <<<"$wifi")"
    key="$(python3 -c 'import json,sys; print(json.load(sys.stdin)["psk"])' <<<"$wifi")"
    [[ -n "$ssid" && -n "$key" ]] || die "the tenant's device_wifi output is empty"

    # -U updates an existing item; the SSID is kept as the account name
    security delete-generic-password -s "$(keychain_service "$tenant")" >/dev/null 2>&1 || true
    security add-generic-password -U -s "$(keychain_service "$tenant")" -a "$ssid" -w "$key" \
        -D "Deevnet tenant Wi-Fi key" -j "Tenant $tenant; written by tools/wifi-tenant.sh"
    echo "Saved the $tenant key for $ssid in the login Keychain"
}

use() {
    local tenant="$1" service ssid key
    service="$(keychain_service "$tenant")"
    ssid="$(security find-generic-password -s "$service" 2>/dev/null \
        | sed -n 's/^ *"acct"<blob>="\(.*\)"$/\1/p' || true)"
    [[ -n "$ssid" ]] || die "no key saved for tenant '$tenant'. On the operator network, run: $0 save $tenant"
    key="$(security find-generic-password -s "$service" -w)"
    join "$ssid" "$key"
}

case "${1:-}" in
    save)   save "${2:-$DEFAULT_TENANT}" ;;
    use)    [[ -n "${2:-}" ]] || die "usage: $0 use <tenant>"; use "$2" ;;
    home)   join "$HOME_SSID" ;;
    status) status ;;
    *)      sed -n '3,18p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; exit 2 ;;
esac
