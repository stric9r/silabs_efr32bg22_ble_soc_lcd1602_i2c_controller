/**
  ******************************************************************************
  * @file           : lcd_protocol.c
  * @brief          : BLE-to-LCD wire protocol parser implementation
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

#include <stdbool.h>
#include <stddef.h>
#include "lcd_protocol.h"
#include "lcd1602.h"

#define LINE_MAX_COLS  16u

static void write_padded_line(uint8_t const *p_data,
                               uint16_t data_len,
                               uint8_t row)
{
    uint16_t i;
    uint16_t write_len;

    (void)lcd1602_set_cursor(0u, row);

    write_len = (data_len < LINE_MAX_COLS) ? data_len : LINE_MAX_COLS;

    for (i = 0u; i < write_len; i++)
    {
        lcd1602_write_char((char)p_data[i]);
    }

    for (; i < LINE_MAX_COLS; i++)
    {
        lcd1602_write_char(' ');
    }
}

lcd_proto_result_t lcd_protocol_process(uint8_t const *p_buf, uint16_t len)
{
    lcd_proto_result_t result;
    bool b_status;

    b_status = (NULL != p_buf) && (0u < len);

    if (b_status)
    {
        switch ((lcd_cmd_t)p_buf[0])
        {
            case LCD_CMD_WRITE_LINE1:
                write_padded_line(p_buf + 1u,
                                  (uint16_t)(len - 1u),
                                  0u);
                break;

            case LCD_CMD_WRITE_LINE2:
                write_padded_line(p_buf + 1u,
                                  (uint16_t)(len - 1u),
                                  1u);
                break;

            case LCD_CMD_CLEAR:
                lcd1602_clear();
                break;

            case LCD_CMD_RAW:
            case LCD_CMD_SET_CURSOR:
            case LCD_CMD_WRITE_AT:
            default:
                b_status = false;
                break;
        }
    }

    if (!b_status && (NULL == p_buf || 0u == len))
    {
        result = LCD_PROTO_ERR_TOO_SHORT;
    }
    else if (!b_status)
    {
        result = LCD_PROTO_ERR_UNKNOWN;
    }
    else
    {
        result = LCD_PROTO_OK;
    }

    return result;
}
