Firmware
========

This section describes the firmware, control logic, and (optionally) the build environment and programming process for the Ma Bell Gateway.

- **Firmware Architecture:**  
  Outline the key software modules—Bluetooth control, ringer signaling, off-hook detection, audio handling, and system state machine.

- **Configuration and Customization:**  
  Explain how to set system parameters (e.g., ring cadence, volume levels, paired devices), including any configuration files or menu-driven setup.

- **Flashing and Updates:**  
  Provide step-by-step instructions for building and flashing the ESP32 firmware, including links to required toolchains or IDEs.

- **Diagnostics and Logging:**  
  Document built-in logging, serial debug output, or any status LEDs/indicators to assist with troubleshooting.

.. toctree::
   :maxdepth: 1
   :caption: Contents:

   architecture
   state-management
   phone-hardware
 