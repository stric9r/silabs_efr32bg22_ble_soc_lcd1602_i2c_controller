# EFR32BG22 BLE SoC — LCD1602 I2C Controller

Control a **DFRobot Gravity LCD1602** display over Bluetooth Low Energy using a Silicon Labs
EFR32BG22 SoC (BRD4108A radio board).

A BLE central device (phone, PC, test tool) connects via the **Nordic UART Service (NUS)** and
sends simple opcode-based commands to write text, clear the screen, or position the cursor.

---

## Hardware

| Component | Part |
|---|---|
| MCU | EFR32BG22C224F512IM40 (Cortex-M33, 512 kB flash, 32 kB RAM) |
| Radio board | BRD4108A |
| Display | DFRobot Gravity LCD1602 (AiP31068L controller + PCA9633DP2 RGB backlight) |
| I2C pins | SDA → PD03, SCL → PD02 (I2C0, 100 kHz) |

### DFRobot Gravity LCD1602 I2C addresses

| Device | 7-bit address | Function |
|---|---|---|
| AiP31068L | **0x3E** | LCD character controller |
| PCA9633DP2 | **0x60** | RGB backlight LED driver |

---

## Driver Architecture

```
app.c / lcd_protocol.c
        │
        └──► lcd1602.h / lcd1602.c       ← application API (no address arg)
                    │
                    └──► aip31068l.h / aip31068l.c   ← I2C transport driver
                                │
                                └──► sl_i2cspm (I2CSPM instance: pcf8574, PD02/PD03)
                    │
                    └──► pca9633dp2.h / pca9633dp2.c  ← RGB backlight driver
```

---

## BLE Protocol (NUS RX)

Bytes written to the NUS RX characteristic are parsed by `lcd_protocol_process()`:

| Byte[0] opcode | Payload | Effect |
|---|---|---|
| `0x00` | UTF-8 string (≤15 bytes) | Write to line 1, space-pad to 16 chars |
| `0x01` | UTF-8 string (≤15 bytes) | Write to line 2, space-pad to 16 chars |
| `0x02` | — | Clear display |

---

## lcd1602 API Summary

```c
int  lcd1602_init(uint8_t lines, bool font_5x10);
void lcd1602_clear(void);
void lcd1602_return_home(void);
void lcd1602_display_on(bool on);
void lcd1602_cursor_on(bool on);
void lcd1602_cursor_blink(bool on);
void lcd1602_display_shift_left(void);
void lcd1602_display_shift_right(void);
void lcd1602_cursor_shift_left(void);
void lcd1602_cursor_shift_right(void);
int  lcd1602_set_cursor(uint8_t col, uint8_t row);
void lcd1602_write_char(char c);
void lcd1602_write_string(char const *p_str);
void lcd1602_load_custom_char(lcd1602_slot_t slot,
                               lcd1602_custom_char_t const *p_char);
```

### Custom characters

```c
lcd1602_custom_char_t smiley = {{
    0b00000,
    0b01010,
    0b01010,
    0b00000,
    0b10001,
    0b01110,
    0b00000,
    0b00000,
}};

lcd1602_load_custom_char(0, &smiley);   /* load into CGRAM slot 0 */
lcd1602_set_cursor(0, 0);
lcd1602_write_char(0);                  /* display it */
```

Pre-defined characters: `LCD1602_CHAR_BLOCK` (all pixels on), `LCD1602_CHAR_BLANK` (all off).

---

## RGB Backlight

The `pca9633dp2_init()` call in `app_init()` sets the backlight to white at startup.

```c
pca9633dp2_set_rgb(PCA9633DP2_DEFAULT_ADDR, 255, 0, 0);   /* red */
pca9633dp2_set_white(PCA9633DP2_DEFAULT_ADDR);             /* white */
pca9633dp2_off(PCA9633DP2_DEFAULT_ADDR);                   /* off */
```

---

## Building

Open the project in **Simplicity Studio 5** (Gecko SDK 4.4.5). The build configuration is
`GNU ARM v12.2.1 - Default`. Build with **Project → Build Project** or via the
command-line makefile in `GNU ARM v12.2.1 - Default/`.

Flash the resulting `.hex` or `.s37` to the radio board using Simplicity Studio or
`commander`. A **Bluetooth Apploader OTA DFU** bootloader must already be on the device.

---

## Troubleshooting

### Bootloader

Series 2 SoC projects require a *Bluetooth Apploader OTA DFU* bootloader. Flash a
pre-built demo first (e.g. *SoC Thermometer*) to install the bootloader, then flash this
application over it.

### I2C not ACKing

- Verify SDA/SCL connections to PD03/PD02.
- Probe the bus at power-on with a logic analyser: expect ACK from 0x3E (LCD) and 0x60 (RGB).
- If 0x3E NACKs, the AiP31068L may need more VDD rise time — increase the delay in
  `aip31068l_init()`.
