# WiFi Provisioning Guide

The Ma Bell Gateway requires WiFi credentials to be stored in NVS (Non-Volatile Storage) before use.

## Overview

WiFi credentials are stored in the ESP32's NVS flash partition and cannot be changed from the running device. This design ensures credentials are not exposed via web interfaces or Bluetooth.

## Prerequisites

- ESP-IDF installed and configured
- esptool.py available (comes with ESP-IDF)
- USB connection to ESP32 device

## Provisioning WiFi Credentials

Use the provided provisioning tool to set your WiFi credentials.

### Automatic Port Detection (Recommended)

Simply run the script without specifying a port - it will auto-detect your ESP32:

```bash
cd tools
./provision_wifi.py "YourNetworkSSID" "YourPassword"
```

The script will automatically find and use the ESP32 device, similar to `idf.py flash`.

### Manual Port Selection

If auto-detection fails or you have multiple devices, specify the port manually:

```bash
cd tools
./provision_wifi.py "YourNetworkSSID" "YourPassword" -p /dev/ttyUSB0
```

**Common serial ports:**
- Linux: `/dev/ttyUSB0` or `/dev/ttyACM0`
- macOS: `/dev/cu.usbserial-*`
- Windows: `COM3`, `COM4`, etc.

### After Provisioning

Flash and run the main firmware:

```bash
idf.py -p /dev/ttyUSB0 flash monitor
```

The device will automatically connect to your WiFi network on boot.

## Verifying WiFi Connection

After provisioning, monitor the serial output:

```bash
idf.py -p /dev/ttyUSB0 monitor
```

Look for these log messages:
```
I (1234) WIFI: Found WiFi credentials in NVS for SSID: YourNetworkName
I (1250) WIFI: Connecting to WiFi network: YourNetworkName
I (3456) WIFI: WiFi connected, IP: 192.168.1.100
```

## Troubleshooting

### "No WiFi credentials found in NVS storage!"

**Cause:** NVS partition not programmed or erased

**Solution:** Follow provisioning steps above

### "WiFi connection failed"

**Cause:** Incorrect credentials or WiFi network unavailable

**Solutions:**
1. Verify SSID and password are correct
2. Check WiFi network is 2.4GHz (ESP32 doesn't support 5GHz)
3. Ensure network uses WPA2/WPA3 security
4. Check signal strength at device location

### Erasing Stored Credentials

To completely erase NVS (removes all stored data including Bluetooth pairings):

```bash
esptool.py --port /dev/ttyUSB0 erase_region 0x9000 0x6000
```

Then re-provision using the steps above.

## NVS Partition Layout

The ESP32 flash partitions are organized as:

| Offset | Size | Name | Description |
|--------|------|------|-------------|
| 0x1000 | 16KB | nvs | System NVS |
| 0x9000 | 24KB | nvs | Application NVS (WiFi, BT, Config) |
| ... | ... | ... | ... |

WiFi credentials are stored in the application NVS partition at offset 0x9000.

## Security Considerations

- WiFi credentials are stored unencrypted in NVS
- For production, consider enabling ESP32 flash encryption
- Credentials are not accessible via Bluetooth or web interfaces
- Physical access to device allows credential extraction

## Support

For issues or questions, see the main project README or open an issue on GitHub.
