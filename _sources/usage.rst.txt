User Guide
==========

This section provides step-by-step instructions for setting up, configuring, and using the Ma Bell Gateway. Each topic below will guide users through common tasks and troubleshooting.

Wiring Connections
------------------

Instructions for connecting legacy telephone hardware, ringer modules, power supplies, and the Ma Bell Gateway itself.

Bluetooth Syncing
-----------------

The Gateway reaches the telephone network through your mobile phone, over Bluetooth. To the phone it looks like a hands-free car kit named **MA BELL**. You pair the two once; after that the Gateway keeps the connection up by itself.

**In short:** the phone stays connected for as long as it is in range. If the connection is lost, because the phone or the Gateway restarted or the phone was carried away, the Gateway reconnects by itself. If you disconnect MA BELL from the phone's own Bluetooth settings, the Gateway takes that as intended and leaves the phone alone.

Pairing a Phone
^^^^^^^^^^^^^^^

Pairing is done from the phone. The Gateway has nothing to press: it is visible to nearby phones whenever it is powered.

1. Power the Gateway and wait a few seconds for it to start.
2. On the phone, open the Bluetooth settings and look for new devices.
3. Choose **MA BELL** from the list.
4. Accept the pairing request if the phone shows one. If it asks for a PIN, enter ``0000``.
5. If the phone asks whether MA BELL may be used for calls, or for access to contacts and call history, allow calls. The Gateway does not need the contacts.

The phone then lists MA BELL as connected for calls. Lift the handset and you should hear dial tone.

The Gateway remembers **one** phone: the one that paired most recently. To move to a different phone, disconnect MA BELL on the old phone, or switch that phone's Bluetooth off, and pair the new phone as above.

Staying Connected
^^^^^^^^^^^^^^^^^

Bluetooth can end a connection for several reasons, and the Gateway treats them differently. The rule it follows is simple: a connection that was **lost** is restored, and a connection that was **ended on purpose** is left ended.

.. list-table::
   :header-rows: 1
   :widths: 34 66

   * - What happened
     - What the Gateway does
   * - The Gateway was restarted or lost power
     - Connects to the paired phone as soon as it has started, usually within ten seconds.
   * - The phone was restarted
     - Keeps trying, and reconnects shortly after the phone's Bluetooth is running again.
   * - The phone went out of range and came back
     - Keeps trying, and reconnects once the phone is back in range.
   * - Bluetooth was switched off on the phone, then on again
     - Reconnects once Bluetooth is back on.
   * - You tapped **Disconnect** on MA BELL in the phone's Bluetooth settings
     - Does **not** reconnect. See below for how to bring it back.
   * - You removed (unpaired, "forgot") MA BELL on the phone
     - Cannot reconnect. Pair the phone again.

**How hard it tries.** After losing the phone the Gateway tries to reconnect every 10 seconds for five minutes. That covers a phone restarting, or a trip to the mailbox. After five minutes it tries once a minute, for as long as it takes, so a phone that comes home in the evening is reconnected within a minute of arriving. Neither the phone nor the Gateway needs to be touched.

**After a deliberate disconnect.** Once you disconnect MA BELL from the phone, the Gateway stops calling the phone back, so that the disconnect stays in force while the phone is sitting next to it. Any one of these brings the connection back:

- Tap **MA BELL** in the phone's Bluetooth settings to connect it again.
- Restart the Gateway by switching its power off and on. It always connects to the paired phone when it starts.
- Take the phone away, or switch its Bluetooth off, or restart it. The Gateway notices that the phone has gone, and from then on treats it like any phone that went out of range: it reconnects when the phone returns.

A deliberate disconnect therefore lasts only while the phone stays nearby with its Bluetooth on. It is not a permanent setting. To keep a phone from connecting at all, remove MA BELL from that phone's paired devices.

**Calls in progress.** If the connection drops during a call, the call is not ended. It carries on at the mobile phone, as it does when a car kit is switched off.

Troubleshooting
^^^^^^^^^^^^^^^

**The phone does not reconnect.**

- Check that Bluetooth is on at the phone and that MA BELL is still in its list of paired devices.
- If MA BELL is listed but not connected, tap it. You may have disconnected it earlier; see *After a deliberate disconnect* above.
- If the phone has been away for more than five minutes, allow up to a minute after it returns.
- Restart the Gateway. It connects to the paired phone when it starts.

**MA BELL does not appear when the phone looks for new devices.**

- Check that the Gateway has power and has had a few seconds to start.
- Check that MA BELL is not already in the phone's list of paired devices.
- Another phone may be connected to the Gateway. Disconnect that phone first.

**The phone lists MA BELL but refuses to connect, or asks to pair again.**

The two no longer agree on the pairing. This happens after the Gateway's stored settings are erased or re-provisioned (see *Setting Up WiFi* below), which removes its record of the phone. Remove MA BELL from the phone's paired devices, then pair again.

**Seeing what happened.** The Gateway records each Bluetooth change in its event log, which is the quickest way to tell a lost connection from a deliberate one:

.. list-table::
   :header-rows: 1
   :widths: 28 72

   * - Event
     - Meaning
   * - ``bt.connected``
     - The phone connected. The event names the phone.
   * - ``bt.disconnected``
     - The connection ended, for any reason.
   * - ``bt.link_lost``
     - The connection was lost rather than ended on purpose. The Gateway is trying to reconnect.
   * - ``bt.reconnect_held``
     - The phone ended the connection on purpose. The Gateway is not trying to reconnect.
   * - ``bt.reconnect_resumed``
     - A phone that had disconnected on purpose went out of reach. The Gateway will reconnect when it returns.

Read the events with ``tools/gateway_logs.py``, or on the Gateway's dashboard.

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

