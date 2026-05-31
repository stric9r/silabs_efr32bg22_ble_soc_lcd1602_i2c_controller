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
#include "lcd_protocol.h"
#include "hd44780.h"
#include "implement/pcf8574_i2c.h"

#define LINE_MAX_COLS  16u

static void write_padded_line(uint8_t addr,
                               uint8_t const *p_data,
                               uint16_t data_len,
                               uint8_t row)
{
    uint16_t i;
    uint16_t write_len;

    (void)hd44780_set_cursor(addr, 0u, row);

    write_len = (data_len < LINE_MAX_COLS) ? data_len : LINE_MAX_COLS;

    for (i = 0u; i < write_len; i++)
    {
        hd44780_write_char(addr, (char)p_data[i]);
    }

    for (; i < LINE_MAX_COLS; i++)
    {
        hd44780_write_char(addr, ' ');
    }
}

lcd_proto_result_t lcd_protocol_process(uint8_t const *p_buf, uint16_t len)
{
    lcd_proto_result_t result;
    bool b_status;
    uint16_t i;

    b_status = (NULL != p_buf) && (0u < len);

    if (b_status)
    {
        switch ((lcd_cmd_t)p_buf[0])
        {
            case LCD_CMD_WRITE_LINE1:
                write_padded_line(PCF8574_DEFAULT_ADDR,
                                  p_buf + 1u,
                                  (uint16_t)(len - 1u),
                                  0u);
                break;

            case LCD_CMD_WRITE_LINE2:
                write_padded_line(PCF8574_DEFAULT_ADDR,
                                  p_buf + 1u,
                                  (uint16_t)(len - 1u),
                                  1u);
                break;

            case LCD_CMD_CLEAR:
                hd44780_clear(PCF8574_DEFAULT_ADDR);
                break;

            case LCD_CMD_RAW:
                for (i = 1u; i < len; i++)
                {
                    pcf8574_write(PCF8574_DEFAULT_ADDR, p_buf[i]);
                }
                break;

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
