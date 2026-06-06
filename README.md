# silabs_efr32bg22_ble_soc_lcd1602_i2c_controller

Control a **DFRobot Gravity LCD1602** display over Bluetooth Low Energy (BLE) using a
Silicon Labs EFR32BG22 SoC.

A BLE central device connects via the **Nordic UART Service (NUS)** and sends opcode-based
commands to write text to either display line, clear the screen, and more.

## Quick links

- [Project README](generated_project/silabs_efr32bg22_ble_soc_lcd1602_i2c_controller/readme.md) —
  full hardware, driver, and build documentation
- [lcd1602.h](generated_project/silabs_efr32bg22_ble_soc_lcd1602_i2c_controller/lcd1602.h) —
  application-level LCD API
- [aip31068l.h](generated_project/silabs_efr32bg22_ble_soc_lcd1602_i2c_controller/aip31068l.h) —
  AiP31068L I2C transport driver
- [pca9633dp2.h](generated_project/silabs_efr32bg22_ble_soc_lcd1602_i2c_controller/pca9633dp2.h) —
  PCA9633DP2 RGB backlight driver

## Hardware

| | |
|---|---|
| MCU | EFR32BG22 (BRD4108A radio board) |
| Display | DFRobot Gravity LCD1602 (AiP31068L @ 0x3E, PCA9633DP2 @ 0x60) |
| Interface | I2C0, SDA=PD03, SCL=PD02, 100 kHz |

## Repository layout

```
generated_project/
  silabs_efr32bg22_ble_soc_lcd1602_i2c_controller/
    app.c / app.h             BLE init and hardware setup
    lcd1602.c / lcd1602.h     Clean LCD application API
    aip31068l.c / aip31068l.h AiP31068L I2C native LCD driver
    pca9633dp2.c / pca9633dp2.h PCA9633DP2 RGB backlight driver
    lcd_protocol.c / .h       BLE NUS command parser
    ble_lcd_service.c / .h    NUS RX dispatch
    delay.c                   Microsecond delay (sl_udelay)
    libs/
      hd44780_pcf8574/        Legacy submodule (delay.h only, to be removed)
      utility_belt/           AI/tooling helpers
doc/
  dfrobot_gravity_lcd1602.pdf Hardware datasheet
```
