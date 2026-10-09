#!/usr/bin/env bash
#
# Switch this Mac's Wi-Fi between the operator network and a Deevnet tenant's
# networks.
#
# A tenant is worked on from the tenant developer network (DVNTM-TD at the
# mobile site): the API, the state store and the broker are reachable from
# there. Its device network (DVNTM-IOT) is where its
# devices sit, and reaches the broker and the internet only; joining it is for
# seeing what a device sees.
#
# Tenants share each SSID and differ only by key (PPSK), and macOS remembers one
# password per SSID, so the key is passed on every join. Keys are kept in the
# login Keychain, because the tenant's Terraform state cannot be reached from
# every network.
#
#   wifi-tenant.sh save [tenant]        store the tenant's SSIDs and keys in the Keychain
#                                       (run where the tenant's state is reachable)
#   wifi-tenant.sh save-home            store the operator network's password in the
#                                       Keychain (asks for it once)
#   wifi-tenant.sh use <tenant>         join the tenant developer network with that
#                                       tenant's key
#   wifi-tenant.sh use <tenant> device  join the device network with that tenant's
#                                       device key
#   wifi-tenant.sh home                 rejoin the operator network
#   wifi-tenant.sh status               show where this Mac is now
#
# macOS only.

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
HOME_SSID="${DEEVNET_HOME_SSID:-DVNTM}"
DEFAULT_TENANT="mabell"
JOIN_TIMEOUT_SEC=20
JOIN_ATTEMPTS=4

die() { echo "Error: $*" >&2; exit 1; }

wifi_device() {
    networksetup -listallhardwareports | awk '/Hardware Port: Wi-Fi/ { getline; print $2; exit }'
}

keychain_service() { echo "deevnet-wifi-$1"; }

tenant_dir() {
    local dir="$REPO_ROOT/infra/deevnet-tenant-$1"
    [[ "$1" != "home" ]] || die "'home' is not a tenant"
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

# Ask the Wi-Fi interface to look for one network by name
scan_for() {
    WIFI_SSID="$1" osascript -l JavaScript -e '
        ObjC.import("CoreWLAN"); ObjC.import("stdlib");
        $.CWWiFiClient.sharedWiFiClient.interface.scanForNetworksWithNameError($.getenv("WIFI_SSID"), null);
    ' >/dev/null 2>&1 || sleep 3
}

join() {
    local ssid="$1" key="$2" dev out attempt
    dev="$(wifi_device)"
    [[ -n "$dev" ]] || die "no Wi-Fi interface found"

    # networksetup reports failure on stdout and still exits 0. It joins from
    # the system's last scan, which often lacks a network that is there, so
    # "could not find" is answered with a scan for that name and another try.
    for ((attempt = 1; attempt <= JOIN_ATTEMPTS; attempt++)); do
        out="$(networksetup -setairportnetwork "$dev" "$ssid" "$key" 2>&1)"
        [[ "$out" == *"Could not find network"* ]] || break
        scan_for "$ssid"
    done
    if [[ "$out" == *"Could not find network"* ]]; then
        die "this Mac cannot see $ssid from here ($JOIN_ATTEMPTS scans). Nothing was changed."
    fi
    [[ -z "$out" ]] || die "could not join $ssid: $out"

    wait_for_address "$dev" || die "joined $ssid but got no address within ${JOIN_TIMEOUT_SEC}s"
    echo "Joined $ssid"
    status
}

# macOS keeps the operator network's password in the System keychain, which
# networksetup is not given, and turning Wi-Fi off and on rejoins whichever
# remembered network macOS prefers at that moment - not reliably this one. So
# the password is kept beside the tenants' keys and the join is explicit.
save_home() {
    local key
    read -r -s -p "Password for $HOME_SSID: " key
    echo
    [[ -n "$key" ]] || die "no password entered"
    security add-generic-password -U -s "$(keychain_service home)" -a "$HOME_SSID" -w "$key" \
        -D "Deevnet operator Wi-Fi password" -j "Written by tools/wifi-tenant.sh"
    echo "Saved the password for $HOME_SSID in the login Keychain"
}

home() {
    local key
    key="$(security find-generic-password -s "$(keychain_service home)" -a "$HOME_SSID" -w 2>/dev/null || true)"
    [[ -n "$key" ]] || die "no password saved for $HOME_SSID. Run once: $0 save-home"
    join "$HOME_SSID" "$key"
}

# The tenant's Terraform output for each kind of network
output_name() {
    case "$1" in
        dev)    echo developer_wifi ;;
        device) echo device_wifi ;;
        *)      die "unknown network '$1' (expected 'device' or nothing)" ;;
    esac
}

save() {
    local tenant="$1" dir kind wifi ssid key saved=0
    dir="$(tenant_dir "$tenant")"

    for kind in dev device; do
        # The state store's credentials, as the tenant's Makefile reads them
        wifi="$(cd "$dir" && { [[ ! -f .backend.env ]] || { set -a; . ./.backend.env; set +a; }; } \
            && terraform output -json "$(output_name "$kind")" 2>/dev/null)" || {
            echo "Skipped $(output_name "$kind"): the tenant has no such output, or its state is not reachable"
            continue
        }
        ssid="$(python3 -c 'import json,sys; print(json.load(sys.stdin)["ssid"])' <<<"$wifi")"
        key="$(python3 -c 'import json,sys; print(json.load(sys.stdin)["psk"])' <<<"$wifi")"
        [[ -n "$ssid" && -n "$key" ]] || die "the tenant's $(output_name "$kind") output is empty"

        # The SSID is kept as the account name
        security delete-generic-password -s "$(keychain_service "$tenant-$kind")" >/dev/null 2>&1 || true
        security add-generic-password -U -s "$(keychain_service "$tenant-$kind")" -a "$ssid" -w "$key" \
            -D "Deevnet tenant Wi-Fi key" -j "Tenant $tenant; written by tools/wifi-tenant.sh"
        echo "Saved the $tenant key for $ssid in the login Keychain"
        saved=$((saved + 1))
    done

    # The item earlier versions of this script wrote, before there were two kinds
    security delete-generic-password -s "$(keychain_service "$tenant")" >/dev/null 2>&1 || true

    [[ "$saved" -gt 0 ]] || die "nothing saved for tenant '$tenant'"
}

use() {
    local tenant="$1" kind="${2:-dev}" service ssid key
    output_name "$kind" >/dev/null
    tenant_dir "$tenant" >/dev/null
    service="$(keychain_service "$tenant-$kind")"
    ssid="$(security find-generic-password -s "$service" 2>/dev/null \
        | sed -n 's/^ *"acct"<blob>="\(.*\)"$/\1/p' || true)"
    [[ -n "$ssid" ]] || die "no $(output_name "$kind") key saved for tenant '$tenant'. Where its state is reachable, run: $0 save $tenant"
    key="$(security find-generic-password -s "$service" -w)"
    join "$ssid" "$key"

    # Joining makes that network this Mac's first preferred network, with this
    # tenant's key remembered for it. Forget it again (the connection stays
    # up), so `home` returns to the operator network and nothing rejoins here
    # later with a stale key.
    networksetup -removepreferredwirelessnetwork "$(wifi_device)" "$ssid" >/dev/null
}

case "${1:-}" in
    save)   save "${2:-$DEFAULT_TENANT}" ;;
    save-home) save_home ;;
    use)    [[ -n "${2:-}" ]] || die "usage: $0 use <tenant> [device]"; use "$2" "${3:-dev}" ;;
    home)   home ;;
    status) status ;;
    *)      sed -n '3,28p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; exit 2 ;;
esac
