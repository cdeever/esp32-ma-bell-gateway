#!/usr/bin/env python3
"""
WiFi Provisioning Tool for Ma Bell Gateway

Writes WiFi credentials directly to ESP32 NVS partition.
"""

import sys
import os
import tempfile
import subprocess
import argparse

def find_esp32_port():
    """Auto-detect ESP32 serial port"""
    try:
        import serial.tools.list_ports
    except ImportError:
        print("Error: pyserial not installed")
        print("Install with: pip install pyserial")
        return None

    # Common ESP32 USB-to-UART chip vendor IDs
    ESP32_USB_IDS = [
        (0x0403, None),  # FTDI
        (0x10C4, None),  # Silicon Labs CP210x
        (0x1A86, None),  # WinChipHead CH340
        (0x303A, None),  # Espressif native USB
    ]

    ports = list(serial.tools.list_ports.comports())

    # First, try to find ports matching ESP32 USB IDs
    for port in ports:
        for vid, pid in ESP32_USB_IDS:
            if port.vid == vid:
                return port.device

    # Fallback: return first available serial port
    if ports:
        return ports[0].device

    return None

def provision_wifi(ssid, password, port):
    """Provision WiFi credentials to ESP32 NVS partition"""

    print(f"Provisioning WiFi credentials...")
    print(f"  SSID: {ssid}")
    print(f"  Port: {port}")

    # Create CSV data for NVS partition
    csv_data = f"""key,type,encoding,value
wifi,namespace,,
ssid,data,string,{ssid}
pass,data,string,{password}
"""

    # Create temporary files
    csv_file = tempfile.NamedTemporaryFile(mode='w', suffix='.csv', delete=False)
    bin_file = tempfile.NamedTemporaryFile(mode='wb', suffix='.bin', delete=False)

    try:
        # Write CSV data
        csv_file.write(csv_data)
        csv_file.close()
        bin_file.close()

        # Find nvs_partition_gen.py
        idf_path = os.environ.get('IDF_PATH')
        if not idf_path:
            print("Error: IDF_PATH environment variable not set")
            print("Please run: export IDF_PATH=/path/to/esp-idf")
            return 1

        nvs_gen_script = os.path.join(idf_path, 'components', 'nvs_flash',
                                      'nvs_partition_generator', 'nvs_partition_gen.py')

        if not os.path.exists(nvs_gen_script):
            print(f"Error: Cannot find {nvs_gen_script}")
            return 1

        # Generate NVS binary
        print(f"\nGenerating NVS partition binary...")
        result = subprocess.run([
            'python', nvs_gen_script,
            'generate', csv_file.name, bin_file.name, '0x6000'
        ], capture_output=True, text=True)

        if result.returncode != 0:
            print(f"Error generating NVS binary: {result.stderr}")
            return 1

        print("✓ NVS binary generated")

        # Flash to device
        print(f"\nFlashing to device on {port}...")
        result = subprocess.run([
            'esptool.py', '--port', port,
            'write_flash', '0x9000', bin_file.name
        ], capture_output=True, text=True)

        if result.returncode != 0:
            print(f"Error flashing device: {result.stderr}")
            return 1

        print("✓ WiFi credentials flashed successfully!")
        print("\nYou can now flash and run the main firmware:")
        print(f"  idf.py -p {port} flash monitor")

        return 0

    finally:
        # Clean up temp files
        os.unlink(csv_file.name)
        os.unlink(bin_file.name)

def main():
    parser = argparse.ArgumentParser(
        description='Provision WiFi credentials to ESP32 NVS partition'
    )
    parser.add_argument('ssid', help='WiFi network SSID')
    parser.add_argument('password', help='WiFi network password')
    parser.add_argument('-p', '--port', default=None,
                       help='Serial port (auto-detect if not specified)')

    args = parser.parse_args()

    # Auto-detect port if not specified
    port = args.port
    if port is None:
        print("Auto-detecting ESP32 serial port...")
        port = find_esp32_port()
        if port is None:
            print("Error: Could not find ESP32 device")
            print("Please specify port manually with -p /dev/ttyUSB0")
            return 1
        print(f"✓ Found ESP32 on port: {port}")

    return provision_wifi(args.ssid, args.password, port)

if __name__ == '__main__':
    sys.exit(main())
