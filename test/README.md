# BLE LCD1602 Test CLI

A small interactive CLI for exercising the BLE-to-LCD wire protocol
implemented by this firmware (see `generated_project/.../lcd_protocol.h`),
without needing a phone app. It talks to the device over the Nordic UART
Service (NUS) using [`bleak`](https://github.com/hbldh/bleak).

> Reference-only tooling: this script and the rest of `test/` are not part
> of the Simplicity Studio / GNU ARM build and are not referenced by the
> `.cproject`. They run on your PC, not the EFR32.

## Install

```
pip install bleak
```

or on Debian/Ubuntu:

```
sudo apt install python3-bleak
```

## Run

```
python3 test/ble_lcd_cli.py
```

## Quick start

```
(lcd) scan
Scanning for 5 seconds...
  1) LCD 1602 I2C Controller  [AA:BB:CC:DD:EE:FF]
Select device number (blank to cancel): 1
Selected LCD 1602 I2C Controller [AA:BB:CC:DD:EE:FF]
(lcd) line1 Hello World
Connecting to LCD 1602 I2C Controller ...
Connected.
-> sent 13 bytes: 010b48656c6c6f20576f726c64
```

The selected device address is remembered for the rest of the session, so
you only need to `scan` once. The connection itself is established lazily
(on first send) and reconnected automatically on the next command if it
drops — which `selftest` does on purpose (see below).

## Commands

| Command | Description |
|---|---|
| `scan` | Scan for nearby BLE devices and pick one by number |
| `status` | Show the selected device and connection state |
| `disconnect` | Disconnect from the current device |
| `nop` | Send a no-op frame |
| `line1 <text>` | Write text to row 0 (padded/truncated to 16 cols by firmware) |
| `line2 <text>` | Write text to row 1 |
| `clear` | Clear the display |
| `cursor <col> <row>` | Move the cursor (zero-indexed) |
| `backlight <0-255>` | Set backlight brightness |
| `putc <char\|code> [flags]` | Write one character at the current cursor |
| `putc_at <col> <row> <char\|code> [flags]` | Write one character at a position |
| `loadchar <slot> <row0> .. <row7>` | Load a custom CGRAM character (8 row bytes) |
| `load_write <col> <row> <slot> <row0>..<row7> [flags]` | Load a custom character and write it in one frame |
| `mode <disp_on 0\|1> <cursor 0\|1> <blink 0\|1>` | Set display/cursor/blink state |
| `selftest` | Run the onboard ~14 s self-test animation |
| `help` | List commands |
| `quit` / `exit` | Exit the CLI |

Numbers can be given in decimal (`128`) or hex (`0x80`). For `putc`/`putc_at`,
either a single literal character (`A`) or a numeric character code works.
`[flags]` on `putc`/`putc_at`/`load_write` defaults to `0` if omitted — see
[Flags](#flags) below for what bit0 does.

**Digits 0-7 are ambiguous on their own**, because char codes 0-7 are
reserved for CGRAM custom-character slots on this controller — they are
*not* the same as the printable glyphs `'0'`-`'7'` (those live at their
normal ASCII codes, `0x30`-`0x37`). So:

- `putc 5` sends **code 5** (CGRAM slot 5, not the digit).
- `putc 0x35` or `putc '5'` (quoted) sends the literal character `'5'`.

## Flags

`[flags]` on `putc`, `putc_at`, and `load_write` is a one-byte bitmask. Only
bit0 is currently defined by the firmware (`lcd_protocol.h`); bits 1-7 are
reserved and ignored:

| Bit | Value | Name | Effect |
|---|---|---|---|
| 0 | `0x01` | `CLEAR_PREV` | Before writing the new character, write a space over the position of the *last* character written with `set_cursor` (i.e. by `putc_at` or `load_write`), erasing it. |

Two things that aren't obvious from the bit table alone:

- Only `putc_at`/`load_write` record a "last special" position. Plain `putc`
  never updates it — but if you pass `putc <code> 1`, it *will* still clear
  whatever position was last recorded by an earlier `putc_at`/`load_write`.
- That tracked position is **not** reset by `clear`; it's only reset when a
  `selftest` run finishes. Right after boot, or before any `putc_at`/
  `load_write`, there's nothing to clear and the flag is a no-op.
- **`putc <code> 1` does not write at "wherever the cursor was."** It writes
  the space at the old tracked position, and the display's entry mode
  auto-increments one column right after every write — so your new
  character lands immediately right of the *erased* cell, not at whatever
  position you last set with `cursor`. Try this to see it:

  ```
  clear
  putc_at 5 0 A      # 'A' appears at col 5, row 0; this position is now tracked
  cursor 10 0        # move the cursor elsewhere on purpose
  putc X 1           # col 5 goes blank, and 'X' appears at col 6 - not col 10
  ```

Example — move a cursor character across row 1 without leaving a trail:

```
putc_at 0 1 '>'
putc_at 1 1 '>' 1     # erases column 0's '>' before drawing this one
putc_at 2 1 '>' 1     # erases column 1's '>' before drawing this one
```

## Examples

```
putc_at 5 1 X            write 'X' at column 5, row 1
putc_at 15 0 X            write 'X' at the last column of row 0

# Custom char: load a heart bitmap into CGRAM slot 0, then display it.
# Char codes 0-7 map directly to CGRAM slots, so "putc 0" shows slot 0.
loadchar 0 0x00 0x0A 0x1F 0x1F 0x1F 0x0E 0x04 0x00
putc_at 0 0 0

# load_write does the load + place-on-screen in a single BLE frame.
# The slot number doubles as the char code that gets written.
load_write 3 1 1 0x04 0x0E 0x1F 0x1F 0x1F 0x0E 0x04 0x00

mode 1 1 1                display on, cursor on, cursor blinking
mode 1 0 0                display on, cursor hidden (typical idle state)
mode 0 0 0                display off
```

## Notes

- The NUS RX characteristic is write-without-response only; the CLI always
  writes that way — there's no ACK from the device.
- `selftest` (`RUN_TEST`) intentionally drops the BLE connection while the
  on-device animation runs; the firmware resumes advertising when it's
  done. Just run another command afterward — the CLI reconnects using the
  remembered address.
- **`col`/`row` are validated on-device, not reported back.** Valid range is
  `col` 0-15, `row` 0-1. If you pass an out-of-range value (or swap the
  argument order — it's `col` then `row`, not `row` then `col`), the
  firmware silently skips the cursor move and writes wherever the cursor
  already was — it looks like the command did nothing, but it actually ran
  with the wrong position. The CLI now warns about this client-side for
  `cursor`, `putc_at`, and `load_write`.
