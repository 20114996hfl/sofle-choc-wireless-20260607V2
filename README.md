# Sofle Choc Wireless ZMK Config

This repository contains a ZMK configuration for a Sofle Choc Wireless keyboard
using ProMicroNRF52840/SuperMini controllers through ZMK's pin-compatible
`nice_nano//zmk` board definition. The configuration is pinned to ZMK commit
`6e2ef41e022d555b10f116e395832913f71717b3`.

## Hardware

- Controller: ProMicroNRF52840/SuperMini (`nice_nano//zmk` build target)
- Split transport: Bluetooth split
- Encoders: one EC11 encoder on each half
- Displays: SSD1306 128x32 OLED on each half, I2C address `0x3c`
- RGB: WS2812 underglow, 30 LEDs per half; schematic `LED` net/data is P0.08 and `CS` is P0.06

## Keymap

- `DEFAULT`: QWERTY base layer with RGB shortcuts on the left top row
- `LOWER`: symbols
- `RAISE`: function keys, arrows, navigation keys
- `ADJUST`: Bluetooth slots and RGB controls
- `FN`: media and application keys

Hold `LOWER` and `RAISE` together to activate `ADJUST`.

Encoder behavior:

- Default layers: left encoder controls volume, right encoder controls page up/down
- `ADJUST`: left encoder scrolls horizontally, right encoder scrolls vertically

## Build Targets

`build.yaml` defines six firmware artifacts:

| Artifact | Use |
| --- | --- |
| `sofle_left` | Standalone mode, left half is the BLE central |
| `sofle_right` | Standalone mode, right half is the peripheral |
| `sofle_dongle` | USB dongle mode, third nice_nano_v2 acts as BLE central and USB HID output |
| `sofle_left_peripheral` | Dongle mode left half |
| `sofle_right_peripheral` | Dongle mode right half |
| `settings_reset` | One-time reset of saved RGB state, Bluetooth bonds, and other settings |

## Flashing

The `.uf2` files are produced by pushing this repository to GitHub and
downloading the build artifacts from the Actions run.

For standalone mode:

1. To restore factory RGB defaults, flash `settings_reset.uf2` once to each half.
2. Flash `sofle_left.uf2` to the left half.
3. Flash `sofle_right.uf2` to the right half.
4. Pair the keyboard with the host over Bluetooth again.

For dongle mode:

1. Flash `sofle_dongle.uf2` to the USB dongle nice_nano_v2.
2. Flash `sofle_left_peripheral.uf2` to the left half.
3. Flash `sofle_right_peripheral.uf2` to the right half.
4. Plug the dongle into the host over USB.

If a half does not reconnect after switching between standalone and dongle
mode, reflash that half with the matching firmware artifact.

## Notes

- ZMK is pinned to commit `6e2ef41e022d555b10f116e395832913f71717b3` in `config/west.yml`.
- The GitHub Actions build workflow uses the same ZMK commit for its reusable workflow.
- ZMK Studio is enabled on the USB central builds.
- Studio locking is disabled for convenient local debugging; connect the standalone left half or dongle over USB.
- Factory RGB state is solid red, 20% brightness, enabled at boot, with a 5% minimum brightness.
- OLED I2C: SDA=P0.17 and SCL=P0.20 on the ProMicroNRF52840; the built-in Sofle I2C0 node is retained with an explicit pinctrl.
- OLED status: battery level is shown as a numeric percentage, layers are shown as 1-5, and the output widget indicates USB or BLE (including the BLE profile and connection state).
- P0.13 is forced high as a GPIO hog, keeping the SuperMini 3.3 V VCC output on for the shared OLED/RGB rail. The persistent ZMK external-power device and `EP_TOG` key are disabled.
