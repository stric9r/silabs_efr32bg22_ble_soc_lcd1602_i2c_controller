/**
  ******************************************************************************
  * @file           : lcd_protocol.h
  * @brief          : BLE-to-LCD wire protocol — command definitions and parser
  ******************************************************************************
  * SPDX-License-Identifier: MIT
  * Copyright (C) 2026 Stric Roberts.
  *
  * Permission is hereby granted, free of charge, to any person obtaining a
  * copy of this software and associated documentation files (the "Software"),
  * to deal in the Software without restriction, including without limitation
  * the rights to use, copy, modify, merge, publish, distribute, sublicense,
  * and/or sell copies of the Software, and to permit persons to whom the
  * Software is furnished to do so, subject to the following conditions:
  *
  * The above copyright notice and this permission notice shall be included in
  * all copies or substantial portions of the Software.
  *
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  * IN THE SOFTWARE.
  ******************************************************************************
  */

#ifndef LCD_PROTOCOL_H
#define LCD_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @defgroup lcd_protocol LCD Wire Protocol
 * @brief NUS RX packet parser that dispatches to the AiP31068L / LCD1602 driver.
 *
 * Wire format — TLV (Type-Length-Value):
 *
 *   Each command is a self-contained 2-byte header followed by its payload:
 *
 *       [cmd : u8] [len : u8] [data : len bytes]
 *
 *   A single NUS RX packet may contain one or more back-to-back TLV frames.
 *   Unknown opcodes are silently skipped using the len field, so new commands
 *   are additive and forward-compatible with older firmware.
 *
 * Command table:
 *
 * | Opcode | Name             | Payload (len bytes)                               |
 * |--------|------------------|---------------------------------------------------|
 * | 0x00   | NOP              | none (0) — ignored; safe separator               |
 * | 0x01   | WRITE_LINE1      | UTF-8 text, 1–16 B, padded to 16 cols           |
 * | 0x02   | WRITE_LINE2      | UTF-8 text, 1–16 B, padded to 16 cols           |
 * | 0x03   | CLEAR            | none (0)                                          |
 * | 0x04   | SET_CURSOR       | [col][row] (2) — zero-indexed                    |
 * | 0x05   | SET_BACKLIGHT    | [brightness] (1) — 0=off, 255=white              |
 * | 0x06   | WRITE_CHAR       | [char_code][flags] (2) — write at cursor         |
 * | 0x07   | WRITE_CHAR_AT    | [col][row][char_code][flags] (4)                 |
 * | 0x08   | LOAD_CUSTOM_CHAR | [slot][row0..row7] (9)                           |
 * | 0x09   | LOAD_AND_WRITE   | [col][row][slot][row0..row7][flags] (12)         |
 * | 0x0A   | DISPLAY_MODE     | [flags] (1): bit0=disp_on, bit1=cursor, bit2=blink|
 * | 0x0B   | RUN_TEST         | none (0) — schedules ~14 s self-test             |
 *
 * flags byte (commands 0x06, 0x07, 0x09):
 *   LCD_PROTO_FLAGS_CLEAR_PREV (bit0): write a space to the last DDRAM cell
 *   where a special character was displayed before writing the new one.
 * @{
 */

/**
 * @brief Protocol command opcodes.
 */
typedef enum
{
    LCD_CMD_NOP              = 0x00u,
    LCD_CMD_WRITE_LINE1      = 0x01u,
    LCD_CMD_WRITE_LINE2      = 0x02u,
    LCD_CMD_CLEAR            = 0x03u,
    LCD_CMD_SET_CURSOR       = 0x04u,
    LCD_CMD_SET_BACKLIGHT    = 0x05u,
    LCD_CMD_WRITE_CHAR       = 0x06u,
    LCD_CMD_WRITE_CHAR_AT    = 0x07u,
    LCD_CMD_LOAD_CUSTOM_CHAR = 0x08u,
    LCD_CMD_LOAD_AND_WRITE   = 0x09u,
    LCD_CMD_DISPLAY_MODE     = 0x0Au,
    LCD_CMD_RUN_TEST         = 0x0Bu,
} lcd_cmd_t;

/**
 * @brief Return codes from lcd_protocol_process().
 */
typedef enum
{
    LCD_PROTO_OK            = 0,
    LCD_PROTO_ERR_TOO_SHORT = 1,
    LCD_PROTO_ERR_UNKNOWN   = 2,
    LCD_PROTO_ERR_BAD_LEN   = 3,
} lcd_proto_result_t;

/** @brief flags bit: clear the previous special-character DDRAM cell. */
#define LCD_PROTO_FLAGS_CLEAR_PREV  0x01u

/**
 * @brief Parse one NUS RX packet and dispatch the corresponding LCD action(s).
 *
 * The packet may contain one or more back-to-back TLV frames.
 *
 * @param p_buf  Pointer to the received byte buffer. Must not be NULL.
 * @param len    Number of valid bytes in @p p_buf.
 * @return       @ref LCD_PROTO_OK on success, otherwise an error code.
 */
lcd_proto_result_t lcd_protocol_process(uint8_t const *p_buf, uint16_t len);

/**
 * @brief Service pending deferred actions (call from app_process_action()).
 *
 * Executes the self-test sequence if one was scheduled by a RUN_TEST command.
 * Blocks for ~14 s when a test is pending; the BLE connection will drop and
 * the device resumes advertising automatically when the test completes.
 */
void lcd_protocol_service(void);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* LCD_PROTOCOL_H */
