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
 * Wire format: byte[0] = command opcode, byte[1..n] = payload.
 *
 * | Opcode | Name            | Payload           |
 * |--------|-----------------|-------------------|
 * | 0x00   | WRITE_LINE1     | UTF-8 string ≤16B |
 * | 0x01   | WRITE_LINE2     | UTF-8 string ≤16B |
 * | 0x02   | CLEAR           | none              |
 * | 0x03   | SET_CURSOR      | reserved (future) |
 * | 0x04   | WRITE_AT        | reserved (future) |
 * | 0xFF   | RAW             | reserved (future) |
 *
 * WRITE_LINE1 and WRITE_LINE2 pad the line to 16 characters with spaces,
 * clearing any previous content without requiring an explicit CLEAR first.
 * @{
 */

/**
 * @brief Protocol command opcodes.
 */
typedef enum
{
    LCD_CMD_WRITE_LINE1 = 0x00u,
    LCD_CMD_WRITE_LINE2 = 0x01u,
    LCD_CMD_CLEAR       = 0x02u,
    LCD_CMD_SET_CURSOR  = 0x03u,
    LCD_CMD_WRITE_AT    = 0x04u,
    /* 0x05 to 0xFE reserved */
    LCD_CMD_RAW         = 0xFFu,
} lcd_cmd_t;

/**
 * @brief Return codes from lcd_protocol_process().
 */
typedef enum
{
    LCD_PROTO_OK            = 0,
    LCD_PROTO_ERR_TOO_SHORT = 1,
    LCD_PROTO_ERR_UNKNOWN   = 2,
} lcd_proto_result_t;

/**
 * @brief Parse one NUS RX packet and dispatch the corresponding LCD action.
 *
 * @param p_buf  Pointer to the received byte buffer. Must not be NULL.
 * @param len    Number of valid bytes in @p p_buf.
 * @return       @ref LCD_PROTO_OK on success, otherwise an error code.
 */
lcd_proto_result_t lcd_protocol_process(uint8_t const *p_buf, uint16_t len);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* LCD_PROTOCOL_H */
