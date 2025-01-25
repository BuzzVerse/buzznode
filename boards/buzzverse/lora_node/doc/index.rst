.. _node_esp32s3:

BuzzVerse LoRa ESP32-S3 Node
############################

Overview
********

The BuzzVerse LoRa ESP32-S3 Node is a custom IoT development platform centered
around the Espressif ESP32-S3 SoC, enhanced with integrated LoRa wireless
connectivity using an Ai-Thinker Ra-02 module. This board is designed to
facilitate robust, low-power deployments for remote sensing, telemetry, and data
acquisition, leveraging both Wi-Fi/Bluetooth LE (from the ESP32-S3) and LoRa’s
long-range communication capabilities.

Key features include:

- **ESP32-S3 WROOM Module**: High-performance dual-core Xtensa LX7 MCU with
  integrated Wi-Fi and Bluetooth LE.
- **LoRa Radio (Ai-Thinker Ra-02)**: Long-range, low-power RF module based on
  the SX127x family for IoT and sensor networks.
- **Onboard Power Management**: Integrated Li-Ion battery charger, battery gauge
  (BQ27441), and voltage regulators.
- **USB-C Interface**: Convenient USB-C connector for power, programming, and
  serial communication.
- **UEXT Connector**: A versatile interface providing UART, I2C, and SPI signals
  for easy peripheral expansion.
- **Optional External Antenna**: U.FL connector for external Wi-Fi/Bluetooth
  antennas.
- **Test Points & Debug Interfaces**: Easily accessible test points and ESP32-S3
  built-in USB-JTAG for debugging.
- **User Button & LEDs**: For quick interaction, status indication, and debugging.

Hardware
********

The core processing unit is the ESP32-S3 WROOM module, which integrates:

- Dual-core Xtensa® 32-bit LX7 processors.
- 2.4 GHz Wi-Fi and Bluetooth® LE connectivity.
- Ample on-chip peripherals such as UART, SPI, I2C, PWM, ADC, DAC, and USB-OTG
  support.

**Power Supply**:

- **USB-C Input**: The board can be powered and programmed via USB-C.
- **Li-Ion Battery Support**: A dedicated JST connector allows for a rechargeable
  Li-Ion battery. The on-board charger and gauge (BQ27441) monitor and manage
  battery usage.
- **3.3V Regulated Output**: A MIC5504 LDO provides a stable 3.3V supply for the
  ESP32-S3 and peripherals.
- **LM66200 Power-Path Management**: Ensures seamless switching between USB and
  battery power.

**LoRa Module (Ai-Thinker Ra-02)**:

- Based on the Semtech SX127x LoRa transceiver.
- Long-range, low-power RF communication.
- SPI interface integrated with the ESP32-S3.
- On-board antenna footprint and test point for LoRa DIO lines.

**External Interfaces**:

- **UEXT Connector**:

  The UEXT header exposes:

  +----------+----------+
  | Signal   | Function |
  +==========+==========+
  | UART RXD | UART RX  |
  +----------+----------+
  | UART TXD | UART TX  |
  +----------+----------+
  | SDA      | I2C Data |
  +----------+----------+
  | SCL      | I2C Clock|
  +----------+----------+
  | MOSI     | SPI MOSI |
  +----------+----------+
  | MISO     | SPI MISO |
  +----------+----------+
  | SCK      | SPI SCK  |
  +----------+----------+
  | CS       | SPI CS   |
  +----------+----------+

  This makes it simple to connect sensors, displays, and other peripherals.

- **USB Port**:

  The USB-C port provides:
  
  - Programming and console access through a virtual COM port.
  - Power input.
  - Integrated USB-JTAG debugging (ESP32-S3 native feature).

**Buttons and LEDs**:

- **User Button**: For user-defined input, wake-up, or boot mode selection.
- **Status LEDs**: Power indication, user-configurable LED, and LoRa/communication
  status LEDs.

Supported Features
==================

The BuzzVerse LoRa ESP32-S3 Node supports a range of Zephyr-compatible features:

+------------+------------+-------------------------------------+
| Interface  | Controller | Driver/Component                    |
+============+============+=====================================+
| Wi-Fi/BT   | on-chip    | esp32-s3 wireless stack             |
+------------+------------+-------------------------------------+
| LoRa       | external   | sx127x-based LoRa driver (SPI)      |
+------------+------------+-------------------------------------+
| UART       | on-chip    | serial port                         |
+------------+------------+-------------------------------------+
| I2C        | on-chip    | i2c                                 |
+------------+------------+-------------------------------------+
| SPI        | on-chip    | spi                                 |
+------------+------------+-------------------------------------+
| GPIO       | on-chip    | gpio                                |
+------------+------------+-------------------------------------+
| ADC        | on-chip    | adc                                 |
+------------+------------+-------------------------------------+
| Timers     | on-chip    | counter                             |
+------------+------------+-------------------------------------+
| Watchdog   | on-chip    | watchdog                            |
+------------+------------+-------------------------------------+
| TRNG       | on-chip    | entropy                             |
+------------+------------+-------------------------------------+
| PWM        | on-chip    | pwm (LEDC, MCPWM)                   |
+------------+------------+-------------------------------------+
| USB-JTAG   | on-chip    | native debugging support            |
+------------+------------+-------------------------------------+
| Battery    | external   | Battery gauge & charger monitoring  |
+------------+------------+-------------------------------------+

Connections and IOs
===================

- **UEXT Connector**: Provides UART (TX, RX), I2C (SDA, SCL), and SPI (MOSI, MISO,
  SCK, CS).
- **USB-C Connector**: For power, programming, and USB-JTAG.
- **Battery Connector**: JST connector for Li-Ion battery input.
- **Test Points (TPx)**: Various test points are available for probing signals
  during development and debugging.
- **U.FL Connector (Optional)**: For external Wi-Fi/BLE antenna.

Prerequisites
-------------

- **Espressif HAL Blobs**:

  If using Wi-Fi and Bluetooth functionalities within Zephyr, ensure that the
  Espressif binary blobs are fetched:

  .. code-block:: console

     west blobs fetch hal_espressif

  .. note::
     Run this command after :file:`west update` to ensure your environment is
     up to date.

- **LoRa Drivers**:

  For LoRa functionality, ensure you have the appropriate SX127x drivers or
  modules enabled in your project’s configuration.

Building & Flashing
*******************

By default, this board can be flashed and debugged via the integrated USB-JTAG
without any additional hardware.

1. Connect the board to your PC via USB-C.
2. Enable the required configurations in your project’s :file:`prj.conf`,
   including Wi-Fi, BT, and LoRa as needed.
3. Build the application with:

   .. zephyr-app-commands::
      :board: node_esp32s3
      :goals: build

4. Flash with:

   .. zephyr-app-commands::
      :board: node_esp32s3
      :goals: flash

After flashing, you can open a serial monitor (e.g. `west espressif monitor`) to
view console output.

Debugging
*********

The ESP32-S3 includes integrated JTAG over USB, simplifying debugging:

1. Connect the board via USB.
2. Use OpenOCD ESP32 from Espressif’s repository or supported toolchains.
3. Launch GDB to step through code, inspect memory, and set breakpoints.

For further details, refer to ESP32-S3 debugging guides on the Espressif
documentation.

References
**********

- `ESP32-S3 WROOM`_
- `Ai-Thinker Ra-02`_
- `ESP-IDF Programming Guide`_
- `Zephyr Project Documentation`_

.. _ESP32-S3 WROOM: https://www.espressif.com/en/products/socs/esp32-s3/overview
.. _Ai-Thinker Ra-02: https://docs.ai-thinker.com/en/lora/ra-02
.. _ESP-IDF Programming Guide: https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/
.. _Zephyr Project Documentation: https://docs.zephyrproject.org/latest/