# TuyaOS nRF52832 to ESP32-C6 migration status

## Implemented

- ESP32-C6 ESP-IDF project structure.
- ESP-NimBLE host initialization.
- Tuya BLE v2 transport UUID layout:
  - `0xFD50` primary service.
  - `00000001-0000-1001-8001-00805F9B07D0` write/write-without-response.
  - `00000002-0000-1001-8001-00805F9B07D0` notify.
  - `00000003-0000-1001-8001-00805F9B07D0` read.
- BLE connect, disconnect, MTU, subscription, receive and notify handling.
- Dedicated NVS partition and API for Tuya product ID, UUID and auth key.
- Factory plus dual-slot ESP-IDF OTA partition layout for 4 MB flash.

The current application echoes data written to characteristic `0001` through
characteristic `0002` after notifications are enabled. This validates the
ESP32-C6 BLE transport before the proprietary protocol engine is connected.

## External blocker for Smart Life pairing

The source nRF52832 package contains these precompiled libraries:

- `libtal_ble_secure.lib`
- `libtal_ble_mbedtls.lib`
- `libtal_xxtea.lib`
- `libtal_ble_group.lib`
- `libtal_ble_beacon_remoter.lib`

Inspection of `libtal_ble_secure.lib` shows an ARM EABI relocatable object.
ESP32-C6 uses the RISC-V architecture, so the ESP-IDF linker cannot use these
objects. The secure library provides `tuya_ble_event_process()` and the Tuya
BLE encryption/verification functions required by the Smart Life pairing
flow.

To complete protocol migration, obtain one of the following from Tuya:

1. Tuya BLE SDK 3.12.3 secure and crypto source code with an ESP-IDF license;
2. RISC-V (`riscv32-esp-elf`) builds compatible with the selected ESP-IDF;
3. an official TuyaOS ESP32-C6 vendor package.

Do not link or copy the ARM libraries into this project; they cannot execute on
ESP32-C6.

## Next integration point

When the compatible Tuya protocol package is available:

1. Add its sources/libraries as an ESP-IDF component.
2. Pass writes received by `on_tuya_rx()` to `tuya_ble_event_process()`.
3. Map the Tuya protocol send callback to `tuya_ble_transport_notify()`.
4. Feed the protocol-generated advertising payload into the transport.
5. Map Tuya storage callbacks to `tuya_storage`.
6. Replace the echo test and validate activation, DP downlink/uplink and OTA.

## Hardware verification

1. Build and flash the application.
2. Scan for `TUYA_ESP32C6` using nRF Connect.
3. Confirm service `0xFD50` and the three Tuya characteristics.
4. Enable notifications on characteristic `0002`.
5. Write 1–244 bytes to characteristic `0001`.
6. Confirm the same bytes are notified through characteristic `0002`.

This verifies the platform transport only. It does not yet prove Smart Life
activation or Tuya application protocol compatibility.
