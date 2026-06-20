# silabs_efr32bg22_ble_soc_lcd1602_i2c_controller

Control a **DFRobot Gravity LCD1602** display over Bluetooth Low Energy (BLE) using a
Silicon Labs EFR32BG22 SoC.

A BLE central device connects via the **Nordic UART Service (NUS)** and sends TLV-framed
commands to write text, control the cursor and backlight, load custom characters, and more.

Example of it running on [Youtube](https://www.youtube.com/shorts/DuAswd0oLW4).

## Quick links

- [Project README](generated_project/silabs_efr32bg22_ble_soc_lcd1602_i2c_controller/readme.md) —
  full hardware, driver, and build documentation
- [lcd1602.h](generated_project/silabs_efr32bg22_ble_soc_lcd1602_i2c_controller/libs/lcd1602_aip31068l_pca9633dp2_lib/inc/lcd1602.h) —
  application-level LCD API
- [aip31068l.h](generated_project/silabs_efr32bg22_ble_soc_lcd1602_i2c_controller/libs/lcd1602_aip31068l_pca9633dp2_lib/inc/aip31068l.h) —
  AiP31068L I2C-native LCD controller driver
- [pca9633dp2.h](generated_project/silabs_efr32bg22_ble_soc_lcd1602_i2c_controller/libs/lcd1602_aip31068l_pca9633dp2_lib/inc/pca9633dp2.h) —
  PCA9633DP2 RGB backlight driver
- [lcd_intfc.h](generated_project/silabs_efr32bg22_ble_soc_lcd1602_i2c_controller/libs/lcd1602_aip31068l_pca9633dp2_lib/inc/implement/lcd_intfc.h) —
  platform-agnostic bus interface (implement to port to a new target)

## Hardware

| | |
|---|---|
| MCU | EFR32BG22 (BRD4108A radio board) |
| Display | DFRobot Gravity LCD1602 (AiP31068L @ 0x3E, PCA9633DP2 @ 0x60) |
| Interface | I2C0, SDA=PD03, SCL=PD02, 100 kHz |

## BLE Protocol

Each NUS write carries one or more **TLV frames** concatenated back-to-back:

```
[cmd : u8] [len : u8] [data : len bytes]  ← one frame
[cmd : u8] [len : u8] [data : len bytes]  ← another frame, same packet
...
```

Unknown opcodes are silently skipped using `len`, so the protocol is forward-compatible.

### Command table

| Opcode | Name               | Payload (`len` bytes)                                       |
|--------|--------------------|-------------------------------------------------------------|
| `0x00` | NOP                | none (`0`) — ignored; use as a visual separator if desired  |
| `0x01` | WRITE\_LINE1       | UTF-8 text, 1–16 B; padded to 16 cols with spaces          |
| `0x02` | WRITE\_LINE2       | UTF-8 text, 1–16 B; padded to 16 cols with spaces          |
| `0x03` | CLEAR              | none (`0`) — clear display, home cursor                     |
| `0x04` | SET\_CURSOR        | `[col][row]` (`2`) — zero-indexed, col 0–15 / row 0–1      |
| `0x05` | SET\_BACKLIGHT     | `[brightness]` (`1`) — 0 = off, 255 = full white           |
| `0x06` | WRITE\_CHAR        | `[char_code][flags]` (`2`) — write at current cursor        |
| `0x07` | WRITE\_CHAR\_AT    | `[col][row][char_code][flags]` (`4`)                        |
| `0x08` | LOAD\_CUSTOM\_CHAR | `[slot][row0..row7]` (`9`) — load CGRAM slot 0–7            |
| `0x09` | LOAD\_AND\_WRITE   | `[col][row][slot][row0..row7][flags]` (`12`)                |
| `0x0A` | DISPLAY\_MODE      | `[flags]` (`1`): bit0=disp\_on, bit1=cursor, bit2=blink    |
| `0x0B` | RUN\_TEST          | none (`0`) — start ~14 s self-test sequence                 |

**`flags` byte** (commands `0x06`, `0x07`, `0x09`):
- bit 0 (`0x01`) — `CLEAR_PREV`: write a space to the DDRAM cell where the last special
  character was displayed before writing the new one. The CGRAM slot is left intact.

### Multi-command example

Clear, write "Hi" on line 1, set backlight to 50%:

```
03 00          ← CLEAR,       len=0
01 02 48 69    ← WRITE_LINE1, len=2, "Hi"
05 01 80       ← SET_BACKLIGHT, len=1, brightness=128
```

### Custom character (CGRAM) format

8 rows × 5 bits. Each `rowN` byte: bit4 = leftmost pixel, bit0 = rightmost.

EQ bar levels used by the self-test (slots 0–3):

```
Slot 0 — ¼ full:  0x00 0x00 0x00 0x00 0x00 0x00 0x1F 0x1F
Slot 1 — ½ full:  0x00 0x00 0x00 0x00 0x1F 0x1F 0x1F 0x1F
Slot 2 — ¾ full:  0x00 0x00 0x1F 0x1F 0x1F 0x1F 0x1F 0x1F
Slot 3 — full:    0x1F 0x1F 0x1F 0x1F 0x1F 0x1F 0x1F 0x1F
```

### Self-test (`0x0B RUN_TEST`)

Send `0B 00` to start the sequence. The test runs for ~14 s and exercises the full
driver stack: backlight ramp, ASCII character scan, custom EQ-bar animation, display
flash, cursor erase sweep, and more. The BLE connection will drop during the test;
the device resumes advertising immediately when it finishes.

---

## Repository layout

```
generated_project/
  silabs_efr32bg22_ble_soc_lcd1602_i2c_controller/
    app.c / app.h              BLE init and hardware setup
    lcd_protocol.c / .h        BLE NUS command parser
    ble_lcd_service.c / .h     NUS RX dispatch
    libs/
      lcd1602_aip31068l_pca9633dp2_lib/   ← LCD library submodule
        inc/
          lcd1602.h            Application API (start here)
          aip31068l.h          AiP31068L driver header
          pca9633dp2.h         PCA9633DP2 backlight driver header
          implement/
            lcd_intfc.h        Bus interface (implement to port)
            delay.h            Delay interface (implement to port)
        src/
          lcd1602.c            Application API implementation
          aip31068l.c          AiP31068L driver
          pca9633dp2.c         PCA9633DP2 driver
        examples/
          silabs/gecko_sdk/
            lcd_intfc.c        Silicon Labs I2C implementation
            delay.c            Silicon Labs delay implementation
      utility_belt/            AI/tooling helpers
doc/
  dfrobot_gravity_lcd1602.pdf  Hardware datasheet
```

### Simplicity Studio include paths and source files

After cloning, add the following to the Simplicity Studio project configuration:

**Include paths:**
- `libs/lcd1602_aip31068l_pca9633dp2_lib/inc`
- `libs/lcd1602_aip31068l_pca9633dp2_lib/inc/implement`

**Source files (compiled directly from the submodule):**
- `libs/lcd1602_aip31068l_pca9633dp2_lib/src/lcd1602.c`
- `libs/lcd1602_aip31068l_pca9633dp2_lib/src/aip31068l.c`
- `libs/lcd1602_aip31068l_pca9633dp2_lib/src/pca9633dp2.c`

**Platform glue (copy into the project root, do not reference in place):**
`libs/lcd1602_aip31068l_pca9633dp2_lib/examples/` is reference-only and must be excluded from
the build by the project's `.cproject` source filter. Copy these two files into the project
root and let Simplicity Studio compile them from there:
- `libs/lcd1602_aip31068l_pca9633dp2_lib/examples/silabs/gecko_sdk/lcd_intfc.c`
- `libs/lcd1602_aip31068l_pca9633dp2_lib/examples/silabs/gecko_sdk/delay.c`
