# Sofle Choc Wireless ZMK Config

This repository contains a ZMK configuration for a Sofle Choc Wireless keyboard
using ProMicroNRF52840/SuperMini controllers through ZMK's pin-compatible
`nice_nano_v2` board definition. The configuration is pinned to ZMK v0.3.

## Hardware

- Controller: ProMicroNRF52840/SuperMini (`nice_nano_v2` build target)
- Split transport: Bluetooth split
- Encoders: one EC11 encoder on each half
- Displays: SSD1306 128x32 OLED on each half, I2C address `0x3c`
- RGB: WS2812 underglow, 30 LEDs per half, data on P0.08; the separate PCB LED signal is P0.06

## Keymap

- `DEFAULT`: QWERTY base layer with RGB shortcuts on the left top row
- `LOWER`: symbols
- `RAISE`: function keys, arrows, navigation keys
- `ADJUST`: Bluetooth slots, RGB controls, external power toggle
- `FN`: media and application keys

Hold `LOWER` and `RAISE` together to activate `ADJUST`.

Encoder behavior:

- Default layers: left encoder controls volume, right encoder controls page up/down
- `ADJUST`: left encoder scrolls horizontally, right encoder scrolls vertically

## Build Targets

`build.yaml` defines five firmware artifacts:

| Artifact | Use |
| --- | --- |
| `sofle_left` | Standalone mode, left half is the BLE central |
| `sofle_right` | Standalone mode, right half is the peripheral |
| `sofle_dongle` | USB dongle mode, third nice_nano_v2 acts as BLE central and USB HID output |
| `sofle_left_peripheral` | Dongle mode left half |
| `sofle_right_peripheral` | Dongle mode right half |

## Flashing

The `.uf2` files are produced by pushing this repository to GitHub and
downloading the build artifacts from the Actions run.

For standalone mode:

1. Flash `sofle_left.uf2` to the left half.
2. Flash `sofle_right.uf2` to the right half.
3. Pair the keyboard with the host over Bluetooth.

For dongle mode:

1. Flash `sofle_dongle.uf2` to the USB dongle nice_nano_v2.
2. Flash `sofle_left_peripheral.uf2` to the left half.
3. Flash `sofle_right_peripheral.uf2` to the right half.
4. Plug the dongle into the host over USB.

If a half does not reconnect after switching between standalone and dongle
mode, reflash that half with the matching firmware artifact.

## Notes

- ZMK is pinned to commit `ff09f2d0c9f13a868c8f71d71d9348ade438e4b6` in `config/west.yml`.
- The GitHub Actions build workflow uses the same ZMK commit for its reusable workflow.
- ZMK Studio is enabled on the USB central builds.
- OLED I2C: SDA=P0.17 and SCL=P0.20 on the ProMicroNRF52840; the built-in Sofle I2C0 node is retained with an explicit pinctrl.
- RGB underglow does not toggle P0.13 external power, because the OLED shares the SuperMini VCC rail and must remain powered when RGB is off.
