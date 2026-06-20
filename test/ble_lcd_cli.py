#!/usr/bin/env python3
"""Interactive BLE test CLI for the LCD1602 NUS wire protocol.

See README.md for install steps and a full command reference.
"""

import asyncio
import shlex
import sys

from bleak import BleakClient, BleakScanner
from bleak.exc import BleakError

NUS_RX_CHAR_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e"

# Opcodes from generated_project/.../lcd_protocol.h
CMD_NOP = 0x00
CMD_WRITE_LINE1 = 0x01
CMD_WRITE_LINE2 = 0x02
CMD_CLEAR = 0x03
CMD_SET_CURSOR = 0x04
CMD_SET_BACKLIGHT = 0x05
CMD_WRITE_CHAR = 0x06
CMD_WRITE_CHAR_AT = 0x07
CMD_LOAD_CUSTOM_CHAR = 0x08
CMD_LOAD_AND_WRITE = 0x09
CMD_DISPLAY_MODE = 0x0A
CMD_RUN_TEST = 0x0B

LINE_MAX_COLS = 16
LCD_COLS = 16
LCD_ROWS = 2


def frame(cmd, payload=b""):
    """Build a single [cmd][len][data...] TLV frame."""
    if len(payload) > 255:
        raise ValueError("payload too long: TLV length field is one byte (max 255)")
    return bytes([cmd, len(payload)]) + payload


def warn_if_bad_cursor(col, row):
    """aip31068l_set_cursor() silently no-ops out-of-range col/row (no error is
    reported over BLE) - the char then lands wherever the cursor already was.
    Warn here so a typo doesn't look like a mystery firmware bug."""
    if not (0 <= col < LCD_COLS and 0 <= row < LCD_ROWS):
        print(
            f"Warning: col={col}, row={row} is out of range "
            f"(valid: col 0-{LCD_COLS - 1}, row 0-{LCD_ROWS - 1}). "
            "Firmware will silently ignore the cursor move and write at the "
            "current cursor position instead."
        )


def char_code(token):
    """Accept a numeric char code ('65', '0x41'), a single literal character
    ('A'), or a quoted literal character ("'5'") to force char-not-code for
    tokens that would otherwise parse as a number (e.g. the digits 0-7,
    which int(token, 0) would read as a CGRAM slot code, not the glyph)."""
    if len(token) == 3 and token[0] == token[-1] and token[0] in "'\"":
        return ord(token[1])
    try:
        return int(token, 0)
    except ValueError:
        pass
    if len(token) == 1:
        return ord(token)
    raise ValueError(f"expected a single character or numeric code, got {token!r}")


class Session:
    def __init__(self):
        self.address = None
        self.name = None
        self.client = None

    async def ensure_connected(self):
        if self.client is not None and self.client.is_connected:
            return True
        if self.address is None:
            print("No device selected. Run 'scan' first.")
            return False
        print(f"Connecting to {self.name or self.address} ...")
        try:
            self.client = BleakClient(self.address)
            await self.client.connect()
        except BleakError as exc:
            print(f"Connect failed: {exc}")
            self.client = None
            return False
        print("Connected.")
        return True

    async def send(self, payload):
        if not await self.ensure_connected():
            return
        try:
            await self.client.write_gatt_char(NUS_RX_CHAR_UUID, payload, response=False)
        except BleakError as exc:
            print(f"Write failed: {exc}")
            return
        print(f"-> sent {len(payload)} bytes: {payload.hex()}")

    async def disconnect(self):
        if self.client is not None and self.client.is_connected:
            await self.client.disconnect()
        self.client = None


async def cmd_scan(session, _args):
    print("Scanning for 5 seconds...")
    devices = await BleakScanner.discover(timeout=5.0)
    if not devices:
        print("No BLE devices found.")
        return
    devices = sorted(devices, key=lambda d: (d.name is None, d.name or "", d.address))
    for i, d in enumerate(devices, start=1):
        print(f"  {i}) {d.name or '(unnamed)'}  [{d.address}]")
    choice = input("Select device number (blank to cancel): ").strip()
    if not choice:
        return
    try:
        dev = devices[int(choice) - 1]
    except (ValueError, IndexError):
        print("Invalid selection.")
        return
    await session.disconnect()
    session.address = dev.address
    session.name = dev.name
    print(f"Selected {dev.name or '(unnamed)'} [{dev.address}]")


async def cmd_status(session, _args):
    if session.address is None:
        print("No device selected.")
        return
    state = "connected" if (session.client and session.client.is_connected) else "not connected"
    print(f"{session.name or '(unnamed)'} [{session.address}] - {state}")


async def cmd_disconnect(session, _args):
    await session.disconnect()
    print("Disconnected.")


async def cmd_nop(session, _args):
    await session.send(frame(CMD_NOP))


async def cmd_line1(session, rest):
    await _send_line(session, rest, CMD_WRITE_LINE1)


async def cmd_line2(session, rest):
    await _send_line(session, rest, CMD_WRITE_LINE2)


async def _send_line(session, text, cmd):
    payload = text.encode("utf-8")
    if len(payload) > LINE_MAX_COLS:
        print(f"Note: {len(payload)} bytes > {LINE_MAX_COLS} cols; firmware truncates to fit.")
    await session.send(frame(cmd, payload))


async def cmd_clear(session, _args):
    await session.send(frame(CMD_CLEAR))


async def cmd_cursor(session, args):
    col, row = (int(a, 0) for a in shlex.split(args))
    warn_if_bad_cursor(col, row)
    await session.send(frame(CMD_SET_CURSOR, bytes([col, row])))


async def cmd_backlight(session, args):
    (brightness,) = (int(a, 0) for a in shlex.split(args))
    await session.send(frame(CMD_SET_BACKLIGHT, bytes([brightness])))


async def cmd_putc(session, args):
    tokens = shlex.split(args, posix=False)
    code = char_code(tokens[0])
    flags = int(tokens[1], 0) if len(tokens) > 1 else 0
    await session.send(frame(CMD_WRITE_CHAR, bytes([code, flags])))


async def cmd_putc_at(session, args):
    tokens = shlex.split(args, posix=False)
    col, row = int(tokens[0], 0), int(tokens[1], 0)
    code = char_code(tokens[2])
    flags = int(tokens[3], 0) if len(tokens) > 3 else 0
    warn_if_bad_cursor(col, row)
    await session.send(frame(CMD_WRITE_CHAR_AT, bytes([col, row, code, flags])))


async def cmd_loadchar(session, args):
    tokens = [int(a, 0) for a in shlex.split(args)]
    if len(tokens) != 9:
        raise ValueError("expected: <slot> <row0> <row1> ... <row7> (9 values)")
    await session.send(frame(CMD_LOAD_CUSTOM_CHAR, bytes(tokens)))


async def cmd_load_write(session, args):
    tokens = [int(a, 0) for a in shlex.split(args)]
    if len(tokens) not in (11, 12):
        raise ValueError("expected: <col> <row> <slot> <row0>..<row7> [flags]")
    if len(tokens) == 11:
        tokens.append(0)
    warn_if_bad_cursor(tokens[0], tokens[1])
    await session.send(frame(CMD_LOAD_AND_WRITE, bytes(tokens)))


async def cmd_mode(session, args):
    disp_on, cursor_on, blink = (int(a, 0) for a in shlex.split(args))
    flags = (disp_on & 1) | ((cursor_on & 1) << 1) | ((blink & 1) << 2)
    await session.send(frame(CMD_DISPLAY_MODE, bytes([flags])))


async def cmd_selftest(session, _args):
    print("Starting self-test: device runs a ~14s animation and the BLE")
    print("connection drops by design. Issue any command afterward to")
    print("reconnect automatically (the selected device is remembered).")
    await session.send(frame(CMD_RUN_TEST))


HELP_TEXT = """
Device:
  scan                                       scan for BLE devices and pick one by number
  status                                     show selected device / connection state
  disconnect                                 disconnect from the current device

LCD protocol commands:
  nop                                        NOP - no-op frame
  line1 <text>                               WRITE_LINE1 - write text to row 0 (rest of line is the text)
  line2 <text>                               WRITE_LINE2 - write text to row 1
  clear                                      CLEAR - clear the display
  cursor <col> <row>                         SET_CURSOR (zero-indexed)
  backlight <0-255>                          SET_BACKLIGHT brightness
  putc <char|code> [flags]                   WRITE_CHAR at current cursor
  putc_at <col> <row> <char|code> [flags]    WRITE_CHAR_AT
  loadchar <slot> <row0> .. <row7>           LOAD_CUSTOM_CHAR (8 row bytes, decimal or 0xNN)
  load_write <col> <row> <slot> <row0>..<row7> [flags]   LOAD_AND_WRITE
  mode <disp_on 0|1> <cursor 0|1> <blink 0|1>   DISPLAY_MODE
  selftest                                   RUN_TEST - runs the onboard self-test animation

  help                                       show this text
  quit / exit                                quit the CLI
"""

COMMANDS = {
    "scan": cmd_scan,
    "status": cmd_status,
    "disconnect": cmd_disconnect,
    "nop": cmd_nop,
    "line1": cmd_line1,
    "line2": cmd_line2,
    "clear": cmd_clear,
    "cursor": cmd_cursor,
    "backlight": cmd_backlight,
    "putc": cmd_putc,
    "putc_at": cmd_putc_at,
    "loadchar": cmd_loadchar,
    "load_write": cmd_load_write,
    "mode": cmd_mode,
    "selftest": cmd_selftest,
}


async def main():
    session = Session()
    print("LCD1602 BLE test CLI. Type 'help' for commands, 'quit' to exit.")
    while True:
        try:
            line = input("(lcd) ").strip()
        except EOFError:
            break
        if not line:
            continue

        name, _, rest = line.partition(" ")
        name = name.lower()

        if name in ("quit", "exit"):
            break
        if name == "help":
            print(HELP_TEXT)
            continue

        handler = COMMANDS.get(name)
        if handler is None:
            print(f"Unknown command: {name!r}. Type 'help' for the command list.")
            continue

        try:
            await handler(session, rest)
        except (ValueError, IndexError) as exc:
            print(f"Error: {exc}")

    await session.disconnect()


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        sys.exit(0)
