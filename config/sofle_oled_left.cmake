# Copyright (c) 2024
# SPDX-License-Identifier: MIT

# OLED configuration for left half
# Uses 4-pin SSD1306 OLED display on I2C

if(CONFIG_ZMK_SPLIT_BLE)
  dt_nodelabel(zmk_keyboard_collector)
endif()

shield("sofle_oled_left")