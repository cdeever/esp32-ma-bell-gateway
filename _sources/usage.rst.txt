User Guide
==========

This section provides step-by-step instructions for setting up, configuring, and using the Ma Bell Gateway. Each topic below will guide users through common tasks and troubleshooting.

Wiring Connections
------------------

Instructions for connecting legacy telephone hardware, ringer modules, power supplies, and the Ma Bell Gateway itself.

Bluetooth Syncing
-----------------

How to pair the Ma Bell Gateway with Bluetooth-enabled devices (e.g., mobile phones), including first-time setup and troubleshooting.

Setting Up WiFi
---------------

The Ma Bell Gateway requires WiFi credentials to be **provisioned before deployment**. WiFi settings are configured at build/provisioning time and stored in the device's non-volatile storage (NVS). There is no runtime user interface or provisioning mechanism for changing WiFi credentials on a running device.

Provisioning Process
^^^^^^^^^^^^^^^^^^^^

WiFi credentials must be written to the device's NVS partition before the firmware is flashed (or after flashing, before first boot).

**Prerequisites:**

- Python 3.x installed
- ESP-IDF environment configured
- USB connection to ESP32 device

**Steps:**

1. Navigate to the project's tools directory:

   .. code-block:: bash

      cd tools

2. Run the WiFi provisioning script with your network credentials:

   .. code-block:: bash

      ./provision_wifi.py "YourNetworkSSID" "YourPassword"

   Replace ``YourNetworkSSID`` and ``YourPassword`` with your actual WiFi network name and password.

3. The script will write the credentials to the ESP32's NVS partition at flash offset 0x9000.

4. Flash the firmware (if not already flashed):

   .. code-block:: bash

      idf.py flash

5. The device will automatically connect to the provisioned WiFi network on boot.

Verifying WiFi Connection
^^^^^^^^^^^^^^^^^^^^^^^^^^

After the device boots, you can verify WiFi connectivity by:

1. **Serial Monitor**: Check the ESP32's serial output for WiFi connection status:

   .. code-block:: bash

      idf.py monitor

   Look for log messages indicating successful WiFi connection and IP address assignment.

2. **Web Interface**: Once connected, the device's web interface should be accessible at the assigned IP address (displayed in serial logs).

3. **Network Status LED**: The device may provide visual feedback through status LEDs indicating WiFi connection state (if implemented).

Troubleshooting
^^^^^^^^^^^^^^^

**No WiFi Connection:**

- Verify credentials were provisioned correctly (check serial logs for "No WiFi credentials found" error)
- Ensure SSID and password are correct (case-sensitive)
- Check that the WiFi network is within range and operational
- Verify the network uses WPA2 security (WPA3 may not be supported)

**Changing WiFi Credentials:**

To update WiFi credentials, re-run the provisioning script with new credentials:

.. code-block:: bash

   cd tools
   ./provision_wifi.py "NewSSID" "NewPassword"

The device will use the new credentials on the next boot.

**Erasing Credentials:**

To completely erase NVS (including WiFi credentials):

.. code-block:: bash

   idf.py erase-flash

.. warning::
   Erasing flash will remove ALL stored data, including WiFi credentials, Bluetooth pairings, and any other configuration. You will need to re-provision WiFi credentials and re-pair Bluetooth devices.

For detailed provisioning instructions and troubleshooting, see ``WIFI_SETUP.md`` in the project root directory.

Status Screen
-------------

Overview of the system status indicators (LEDs, OLED, or web UI), including ring/call state, Bluetooth connection, and fault/warning alerts.

URL for Internal State
----------------------

How to access the Gateway’s internal state and diagnostics via a web browser (local IP address or hostname). Includes example URLs and typical state information available.

Serial Interface
----------------

Instructions for connecting to the Gateway’s serial port for debugging, setup, or recovery.  
- Description of the serial jack or header location (back panel, internal header, or both)
- Default serial parameters (baud rate, data bits, parity, stop bits)
- Common troubleshooting or recovery procedures via serial console

Log Configuration
-----------------

Instructions for viewing logs via serial console, and options for sending logs to a central server (e.g., syslog, MQTT, or cloud logging integration).

---

