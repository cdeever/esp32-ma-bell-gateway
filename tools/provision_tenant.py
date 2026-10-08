#!/usr/bin/env python3
"""
Tenant Provisioning Tool for Ma Bell Gateway

Writes what the Deevnet tenant issued for the gateway to the ESP32 NVS
partition: the WiFi key it associates with, and the MQTT broker address,
account, CA and topics it publishes to.

The values are read from the tenant's Terraform outputs
(infra/deevnet-tenant-mabell), so nothing is typed or hardcoded.

NOTE: this replaces the whole NVS partition, like provision_wifi.py. The
Bluetooth pairing is stored there too, so the phone has to be paired again.
"""

import argparse
import csv
import json
import os
import subprocess
import sys
import tempfile

from provision_wifi import find_esp32_port

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DEFAULT_TENANT_DIR = os.path.join(REPO_ROOT, 'infra', 'deevnet-tenant-mabell')

NVS_OFFSET = '0x9000'
NVS_SIZE = '0x6000'

# Limits enforced by the firmware (main/config/wifi_config.h, mqtt_config.h),
# counting the terminator the firmware's buffers need
LIMITS = {
    'wifi ssid': 31,
    'wifi psk': 63,
    'broker host': 63,
    'broker username': 63,
    'broker password': 127,
    'state topic': 95,
    'log topic': 95,
}
MAX_CA_BYTES = 3999


def backend_env(tenant_dir):
    """The environment plus the state store's credentials from .backend.env, as the Makefile does"""
    env = dict(os.environ)
    path = os.path.join(tenant_dir, '.backend.env')
    if os.path.exists(path):
        with open(path) as f:
            for line in f:
                name, sep, value = line.strip().partition('=')
                if sep and not name.startswith('#'):
                    env[name] = value
    return env


def read_outputs(tenant_dir, outputs_json):
    """Return the tenant's Terraform outputs as {name: value}"""
    if outputs_json:
        with (sys.stdin if outputs_json == '-' else open(outputs_json)) as f:
            raw = json.load(f)
    else:
        result = subprocess.run(['terraform', 'output', '-json'], cwd=tenant_dir,
                                capture_output=True, text=True, env=backend_env(tenant_dir))
        if result.returncode != 0:
            raise RuntimeError(f"terraform output failed in {tenant_dir}:\n{result.stderr.strip()}")
        raw = json.loads(result.stdout)

    if not raw:
        raise RuntimeError("the tenant has no outputs - has it been applied, and is the state reachable?")
    return {name: entry['value'] for name, entry in raw.items()}


def pick_topic(topics, marker):
    """The one granted topic containing marker, e.g. '/log/'"""
    matches = [t for t in topics if marker in t]
    if len(matches) != 1:
        raise RuntimeError(f"expected exactly one granted topic containing '{marker}', found {matches}")
    return matches[0]


def build_settings(outputs, tenant_dir, ca_path):
    """Collect and validate everything that goes into NVS"""
    for name in ('device_wifi', 'broker', 'gateway_broker'):
        if name not in outputs:
            raise RuntimeError(f"tenant output '{name}' is missing")

    wifi = outputs['device_wifi']
    broker = outputs['broker']
    account = outputs['gateway_broker']

    ca_path = ca_path or os.path.join(tenant_dir, broker['ca'])
    if not os.path.exists(ca_path):
        raise RuntimeError(f"broker CA not found: {ca_path}")
    with open(ca_path) as f:
        ca_pem = f.read()
    if 'BEGIN CERTIFICATE' not in ca_pem:
        raise RuntimeError(f"{ca_path} is not a PEM certificate")
    if len(ca_pem.encode()) > MAX_CA_BYTES:
        raise RuntimeError(f"{ca_path} is {len(ca_pem.encode())} bytes; NVS holds at most {MAX_CA_BYTES}")

    settings = {
        'wifi ssid': wifi['ssid'],
        'wifi psk': wifi['psk'],
        'broker host': broker['host'],
        'broker port': int(broker['port']),
        'broker username': account['username'],
        'broker password': account['password'],
        'state topic': pick_topic(account['publish'], '/phone/'),
        'log topic': pick_topic(account['publish'], '/log/'),
        'ca path': os.path.abspath(ca_path),
    }

    for name, limit in LIMITS.items():
        length = len(settings[name].encode())
        if length == 0:
            raise RuntimeError(f"{name} is empty")
        if length > limit:
            raise RuntimeError(f"{name} is {length} bytes; the firmware holds at most {limit}")

    return settings


def write_nvs_csv(settings, path):
    rows = [
        ('key', 'type', 'encoding', 'value'),
        ('wifi', 'namespace', '', ''),
        ('ssid', 'data', 'string', settings['wifi ssid']),
        ('pass', 'data', 'string', settings['wifi psk']),
        ('mqtt', 'namespace', '', ''),
        ('host', 'data', 'string', settings['broker host']),
        ('port', 'data', 'u32', settings['broker port']),
        ('user', 'data', 'string', settings['broker username']),
        ('pass', 'data', 'string', settings['broker password']),
        ('state_topic', 'data', 'string', settings['state topic']),
        ('log_topic', 'data', 'string', settings['log topic']),
        ('ca', 'file', 'string', settings['ca path']),
    ]
    with open(path, 'w', newline='') as f:
        csv.writer(f).writerows(rows)


def generate_nvs_image(csv_path, bin_path):
    idf_path = os.environ.get('IDF_PATH')
    if not idf_path:
        raise RuntimeError("IDF_PATH is not set. Run: . $IDF_PATH/export.sh")

    nvs_gen_script = os.path.join(idf_path, 'components', 'nvs_flash',
                                  'nvs_partition_generator', 'nvs_partition_gen.py')
    if not os.path.exists(nvs_gen_script):
        raise RuntimeError(f"cannot find {nvs_gen_script}")

    result = subprocess.run([sys.executable, nvs_gen_script, 'generate', csv_path, bin_path, NVS_SIZE],
                            capture_output=True, text=True)
    if result.returncode != 0:
        raise RuntimeError(f"generating the NVS image failed:\n{result.stderr.strip() or result.stdout.strip()}")


def main():
    parser = argparse.ArgumentParser(
        description="Provision the gateway with its Deevnet tenant's WiFi key and MQTT account"
    )
    parser.add_argument('-p', '--port', default=None,
                        help='Serial port (auto-detect if not specified)')
    parser.add_argument('--tenant-dir', default=DEFAULT_TENANT_DIR,
                        help='Tenant Terraform directory (default: infra/deevnet-tenant-mabell)')
    parser.add_argument('--outputs-json', default=None, metavar='FILE',
                        help="Read `terraform output -json` from FILE ('-' for stdin) instead of running terraform")
    parser.add_argument('--ca', default=None,
                        help="Broker CA in PEM (default: the file the tenant's broker output names)")
    parser.add_argument('--dry-run', action='store_true',
                        help='Build the NVS image and stop, without touching the device')
    parser.add_argument('-y', '--yes', action='store_true',
                        help='Do not ask before replacing the NVS partition')
    args = parser.parse_args()

    try:
        settings = build_settings(read_outputs(args.tenant_dir, args.outputs_json), args.tenant_dir, args.ca)
    except (RuntimeError, OSError, KeyError, ValueError) as e:
        print(f"Error: {e}")
        return 1

    print("Provisioning from the tenant's outputs (secrets not shown):")
    print(f"  WiFi SSID:    {settings['wifi ssid']}")
    print(f"  Broker:       {settings['broker host']}:{settings['broker port']}")
    print(f"  Account:      {settings['broker username']}")
    print(f"  State topic:  {settings['state topic']}")
    print(f"  Log topic:    {settings['log topic']}")
    print(f"  Broker CA:    {settings['ca path']}")

    # The CSV holds the WiFi key and the broker password: keep it private and short-lived
    with tempfile.TemporaryDirectory() as tmp:
        csv_path = os.path.join(tmp, 'nvs.csv')
        bin_path = os.path.join(tmp, 'nvs.bin')

        try:
            write_nvs_csv(settings, csv_path)
            generate_nvs_image(csv_path, bin_path)
        except RuntimeError as e:
            print(f"Error: {e}")
            return 1
        print("✓ NVS image generated")

        if args.dry_run:
            print("Dry run: device not touched")
            return 0

        port = args.port
        if port is None:
            print("Auto-detecting ESP32 serial port...")
            port = find_esp32_port()
            if port is None:
                print("Error: Could not find ESP32 device")
                print("Please specify port manually with -p /dev/ttyUSB0")
                return 1
            print(f"✓ Found ESP32 on port: {port}")

        if not args.yes:
            print(f"\nThis replaces the whole NVS partition on {port}.")
            print("The Bluetooth pairing is stored there: the phone will have to be paired again.")
            if input("Continue? [y/N] ").strip().lower() != 'y':
                print("Aborted")
                return 1

        print(f"\nFlashing to device on {port}...")
        result = subprocess.run([sys.executable, '-m', 'esptool', '--port', port,
                                 'write_flash', NVS_OFFSET, bin_path],
                                capture_output=True, text=True)
        if result.returncode != 0:
            print(f"Error flashing device: {result.stderr.strip() or result.stdout.strip()}")
            return 1

    print("✓ Tenant settings flashed successfully!")
    print("\nYou can now flash and run the main firmware:")
    print(f"  idf.py -p {port} flash monitor")
    return 0


if __name__ == '__main__':
    sys.exit(main())
