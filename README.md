# Sofle ZMK Firmware

Keyboard mapping can also be adjusted here:

https://nickcoutsos.github.io/keymap-editor/

## Build And Flash Flow

1. Push changes to this repository. GitHub Actions runs and builds firmware files.
2. Download the firmware artifact, open it and copy the `sofle_left` firmware file.
3. Connect the left keyboard to notebook with USB-C.
4. Double-press the small reset button on the keyboard.
5. A new folder appears on the notebook, open it and paste the `sofle_left` firmware file into it.
6. Flashing completes.

Done.

Note: if you switch between standalone and dongle mode and the right half won't pair with the new central, re-flashing the right half's firmware is the reliable fix.

## USB Dongle Flash Flow

The dongle is a third nice_nano_v2 that plugs into the host over USB. It acts as the BLE central + USB HID output; both halves become BLE peripherals that talk only to the dongle. This is more responsive than direct host BLE on a Bluetooth slot on the host.

Artifacts for this mode: `sofle_dongle`, `sofle_left_peripheral`, `sofle_right_peripheral`.

1. Push changes. Wait for GitHub Actions to finish.
2. Download the firmware artifact.
3. Flash the **dongle** nice_nano:
   - Plug the dongle into the notebook via USB-C.
   - Double-press reset on the dongle. A bootloader folder appears.
   - Drop `sofle_dongle.uf2` into it.
4. Flash the **left half** of the keyboard:
   - Plug the left half in via USB-C.
   - Double-press reset. Drop `sofle_left_peripheral.uf2` into the bootloader folder.
5. Flash the **right half** of the keyboard:
   - Plug the right half in via USB-C.
   - Double-press reset. Drop `sofle_right_peripheral.uf2` into the bootloader folder.
6. Unplug halves, power them on, plug the dongle into the host. The halves auto-pair to the dongle.

