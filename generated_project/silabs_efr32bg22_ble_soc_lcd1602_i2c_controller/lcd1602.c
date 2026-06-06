/**
  ******************************************************************************
  * @file           : lcd1602.c
  * @brief          : DFRobot Gravity LCD1602 application API
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

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include "lcd1602.h"
#include "aip31068l.h"

/* ---------------------------------------------------------------------------
 * HD44780 / AiP31068L instruction bit masks
 * ---------------------------------------------------------------------------*/
#define DISPLAY_CTRL    0x08u   /* LCD_DISPLAYCONTROL base */
#define DISP_ON_BIT     0x04u   /* D — display on */
#define CURSOR_ON_BIT   0x02u   /* C — cursor underline */
#define BLINK_ON_BIT    0x01u   /* B — cursor blink */

#define CURSOR_SHIFT    0x10u   /* LCD_CURSORSHIFT base */
#define SHIFT_DISPLAY   0x08u   /* S/C — 1=display shift, 0=cursor move */
#define SHIFT_RIGHT     0x04u   /* R/L — 1=right, 0=left */

/* ---------------------------------------------------------------------------
 * Module state
 * ---------------------------------------------------------------------------*/

static uint8_t s_addr;
static uint8_t s_display_ctrl;   /* D|C|B bits, ORed into DISPLAY_CTRL */

/* ---------------------------------------------------------------------------
 * Pre-defined custom characters
 * ---------------------------------------------------------------------------*/

const lcd1602_custom_char_t LCD1602_CHAR_BLOCK = {{
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
}};

const lcd1602_custom_char_t LCD1602_CHAR_BLANK = {{
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
}};

/* ---------------------------------------------------------------------------
 * Internal helper
 * ---------------------------------------------------------------------------*/

static void update_display_ctrl(void)
{
    aip31068l_command(s_addr, DISPLAY_CTRL | s_display_ctrl);
}

/* ---------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------------*/

int lcd1602_init(uint8_t lines, bool font_5x10)
{
    (void)lines;      /* aip31068l_init always configures 2-line, 5x8 */
    (void)font_5x10;

    s_addr         = AIP31068L_DEFAULT_ADDR;
    s_display_ctrl = DISP_ON_BIT;   /* display on, cursor off, blink off */

    return aip31068l_init(s_addr);
}

void lcd1602_clear(void)
{
    aip31068l_clear(s_addr);
}

void lcd1602_return_home(void)
{
    aip31068l_return_home(s_addr);
}

void lcd1602_display_on(bool on)
{
    if (on)
    {
        s_display_ctrl |= DISP_ON_BIT;
    }
    else
    {
        s_display_ctrl &= (uint8_t)(~DISP_ON_BIT);
    }
    update_display_ctrl();
}

void lcd1602_cursor_on(bool on)
{
    if (on)
    {
        s_display_ctrl |= CURSOR_ON_BIT;
    }
    else
    {
        s_display_ctrl &= (uint8_t)(~CURSOR_ON_BIT);
    }
    update_display_ctrl();
}

void lcd1602_cursor_blink(bool on)
{
    if (on)
    {
        s_display_ctrl |= BLINK_ON_BIT;
    }
    else
    {
        s_display_ctrl &= (uint8_t)(~BLINK_ON_BIT);
    }
    update_display_ctrl();
}

void lcd1602_display_shift_left(void)
{
    aip31068l_command(s_addr, CURSOR_SHIFT | SHIFT_DISPLAY);
}

void lcd1602_display_shift_right(void)
{
    aip31068l_command(s_addr, CURSOR_SHIFT | SHIFT_DISPLAY | SHIFT_RIGHT);
}

void lcd1602_cursor_shift_left(void)
{
    aip31068l_command(s_addr, CURSOR_SHIFT);
}

void lcd1602_cursor_shift_right(void)
{
    aip31068l_command(s_addr, CURSOR_SHIFT | SHIFT_RIGHT);
}

int lcd1602_set_cursor(uint8_t col, uint8_t row)
{
    return aip31068l_set_cursor(s_addr, col, row);
}

void lcd1602_write_char(char c)
{
    aip31068l_write_char(s_addr, c);
}

void lcd1602_write_string(char const *p_str)
{
    aip31068l_write_string(s_addr, p_str);
}

void lcd1602_load_custom_char(lcd1602_slot_t slot,
                               lcd1602_custom_char_t const *p_char)
{
    assert(NULL != p_char);

    if (NULL != p_char)
    {
        aip31068l_load_custom_char(s_addr, slot, p_char->row);
    }
}
