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
#include "pca9633dp2.h"
#include "delay.h"

#define LINE_MAX_COLS  16u

/* Tracks where the last special (CGRAM) character was written for clear_prev. */
static struct {
    uint8_t col;
    uint8_t row;
    bool    valid;
} s_last_special;

/* Set by LCD_CMD_RUN_TEST; drained by lcd_protocol_service(). */
static volatile bool s_test_requested = false;

/* ---- forward declarations ---- */
static void write_padded_line(uint8_t const *p_data, uint16_t data_len, uint8_t row);
static void cmd_do_write_char(uint8_t col, uint8_t row, bool set_cursor,
                              char c, uint8_t flags);
static void lcd_run_test(void);

/* ------------------------------------------------------------------ */

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

static void cmd_do_write_char(uint8_t col, uint8_t row, bool set_cursor,
                               char c, uint8_t flags)
{
    if ((flags & LCD_PROTO_FLAGS_CLEAR_PREV) && s_last_special.valid)
    {
        (void)lcd1602_set_cursor(s_last_special.col, s_last_special.row);
        lcd1602_write_char(' ');
    }

    if (set_cursor)
    {
        (void)lcd1602_set_cursor(col, row);
        lcd1602_write_char(c);
        s_last_special.col   = col;
        s_last_special.row   = row;
        s_last_special.valid = true;
    }
    else
    {
        lcd1602_write_char(c);
    }
}

/* ------------------------------------------------------------------ */

lcd_proto_result_t lcd_protocol_process(uint8_t const *p_buf, uint16_t len)
{
    uint16_t          offset;
    uint8_t           cmd;
    uint8_t           plen;
    uint8_t const    *data;
    lcd1602_custom_char_t ch;
    uint8_t           i;
    lcd_proto_result_t result;

    if ((NULL == p_buf) || (0u == len))
    {
        return LCD_PROTO_ERR_TOO_SHORT;
    }

    result = LCD_PROTO_OK;
    offset = 0u;

    while ((uint16_t)(offset + 2u) <= len)
    {
        cmd  = p_buf[offset];
        plen = p_buf[offset + 1u];
        data = p_buf + offset + 2u;

        if ((uint16_t)(offset + 2u + plen) > len)
        {
            result = LCD_PROTO_ERR_BAD_LEN;
            break;
        }

        switch ((lcd_cmd_t)cmd)
        {
            case LCD_CMD_NOP:
                break;

            case LCD_CMD_WRITE_LINE1:
                write_padded_line(data, plen, 0u);
                break;

            case LCD_CMD_WRITE_LINE2:
                write_padded_line(data, plen, 1u);
                break;

            case LCD_CMD_CLEAR:
                lcd1602_clear();
                break;

            case LCD_CMD_SET_CURSOR:
                if (plen >= 2u)
                {
                    (void)lcd1602_set_cursor(data[0], data[1]);
                }
                break;

            case LCD_CMD_SET_BACKLIGHT:
                if (plen >= 1u)
                {
                    pca9633dp2_set_rgb(PCA9633DP2_DEFAULT_ADDR,
                                       data[0], data[0], data[0]);
                }
                break;

            case LCD_CMD_WRITE_CHAR:
                if (plen >= 2u)
                {
                    cmd_do_write_char(0u, 0u, false, (char)data[0], data[1]);
                }
                break;

            case LCD_CMD_WRITE_CHAR_AT:
                if (plen >= 4u)
                {
                    cmd_do_write_char(data[0], data[1], true, (char)data[2], data[3]);
                }
                break;

            case LCD_CMD_LOAD_CUSTOM_CHAR:
                if (plen >= 9u)
                {
                    for (i = 0u; i < 8u; i++)
                    {
                        ch.row[i] = data[1u + i];
                    }
                    lcd1602_load_custom_char(data[0], &ch);
                }
                break;

            case LCD_CMD_LOAD_AND_WRITE:
                if (plen >= 12u)
                {
                    for (i = 0u; i < 8u; i++)
                    {
                        ch.row[i] = data[3u + i];
                    }
                    lcd1602_load_custom_char(data[2], &ch);
                    cmd_do_write_char(data[0], data[1], true, (char)data[2], data[11]);
                }
                break;

            case LCD_CMD_DISPLAY_MODE:
                if (plen >= 1u)
                {
                    lcd1602_display_on   ((data[0] & 0x01u) != 0u);
                    lcd1602_cursor_on    ((data[0] & 0x02u) != 0u);
                    lcd1602_cursor_blink ((data[0] & 0x04u) != 0u);
                }
                break;

            case LCD_CMD_RUN_TEST:
                s_test_requested = true;
                break;

            default:
                break;
        }

        offset = (uint16_t)(offset + 2u + plen);
    }

    return result;
}

void lcd_protocol_service(void)
{
    if (s_test_requested)
    {
        s_test_requested = false;
        lcd_run_test();
    }
}

/* ================================================================== */
/*  Self-test — called from lcd_protocol_service(), never from BLE    */
/*  callback. Blocks ~14 s. BLE connection will drop; device resumes  */
/*  advertising automatically when the test completes.                */
/* ================================================================== */

static void lcd_run_test(void)
{
    /* EQ bar custom chars: 1/4, 1/2, 3/4, full — loaded into slots 0-3. */
    static const lcd1602_custom_char_t k_bar[4] = {
        {{ 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x1Fu, 0x1Fu }},
        {{ 0x00u, 0x00u, 0x00u, 0x00u, 0x1Fu, 0x1Fu, 0x1Fu, 0x1Fu }},
        {{ 0x00u, 0x00u, 0x1Fu, 0x1Fu, 0x1Fu, 0x1Fu, 0x1Fu, 0x1Fu }},
        {{ 0x1Fu, 0x1Fu, 0x1Fu, 0x1Fu, 0x1Fu, 0x1Fu, 0x1Fu, 0x1Fu }},
    };

    /* Phase-shifted wave pattern — 5 frames × 16 columns. */
    static const uint8_t k_eq_frames[5u][16u] = {
        { 0u,1u,2u,3u,3u,2u,1u,0u,0u,1u,2u,3u,3u,2u,1u,0u },
        { 1u,2u,3u,3u,2u,1u,0u,0u,1u,2u,3u,3u,2u,1u,0u,0u },
        { 2u,3u,3u,2u,1u,0u,0u,1u,2u,3u,3u,2u,1u,0u,0u,1u },
        { 3u,3u,2u,1u,0u,0u,1u,2u,3u,3u,2u,1u,0u,0u,1u,2u },
        { 3u,2u,1u,0u,0u,1u,2u,3u,3u,2u,1u,0u,0u,1u,2u,3u },
    };

    uint8_t i;
    uint8_t f;
    uint8_t col;
    char    c;
    uint8_t b;

    /* ----------------------------------------------------------------
     * Phase 1 — Boot splash (backlight ramp from dim to full, ~1.6 s)
     * ---------------------------------------------------------------- */
    pca9633dp2_set_rgb(PCA9633DP2_DEFAULT_ADDR, 80u, 80u, 80u);
    lcd1602_display_on(true);
    lcd1602_cursor_on(false);
    lcd1602_cursor_blink(false);
    lcd1602_clear();

    write_padded_line((uint8_t const *)" EFR32BG22 BIOS ", 16u, 0u);
    write_padded_line((uint8_t const *)"  v0.1 LAUNCHING", 16u, 1u);

    delay_ms(600u);

    for (b = 80u; b < 250u; b = (uint8_t)(b + 5u))
    {
        pca9633dp2_set_rgb(PCA9633DP2_DEFAULT_ADDR, b, b, b);
        delay_ms(12u);
    }
    pca9633dp2_set_white(PCA9633DP2_DEFAULT_ADDR);
    delay_ms(600u);

    /* ----------------------------------------------------------------
     * Phase 2 — Character scan, POST-style (~2 s)
     *   All printable ASCII swept across line 2 at 18 ms/char.
     * ---------------------------------------------------------------- */
    write_padded_line((uint8_t const *)"MEM SCAN:       ", 16u, 0u);

    col = 0u;
    for (c = ' '; c <= '~'; c++)
    {
        (void)lcd1602_set_cursor(col, 1u);
        lcd1602_write_char(c);
        col = (uint8_t)((col + 1u) % LINE_MAX_COLS);
        delay_ms(18u);
    }

    delay_ms(200u);

    /* ----------------------------------------------------------------
     * Phase 3 — Self-check blurb (~0.9 s)
     *   Existential crisis included at no extra charge.
     * ---------------------------------------------------------------- */
    write_padded_line((uint8_t const *)"RAM: OK  CPU: OK", 16u, 0u);
    write_padded_line((uint8_t const *)"SOUL: NOT FOUND ", 16u, 1u);
    delay_ms(900u);

    /* ----------------------------------------------------------------
     * Phase 4 — EQ animation (~2.5 s)
     *   Bars surge dramatically. Nothing is actually playing.
     * ---------------------------------------------------------------- */
    for (i = 0u; i < 4u; i++)
    {
        lcd1602_load_custom_char((lcd1602_slot_t)i, &k_bar[i]);
    }

    write_padded_line((uint8_t const *)"  DROPPIN BEATS ", 16u, 0u);

    for (f = 0u; f < 5u; f++)
    {
        (void)lcd1602_set_cursor(0u, 1u);
        for (col = 0u; col < 16u; col++)
        {
            lcd1602_write_char((char)k_eq_frames[f][col]);
        }
        delay_ms(500u);
    }

    /* ----------------------------------------------------------------
     * Phase 5 — Fatal error (~1.2 s)
     *   Classic debugging experience.
     * ---------------------------------------------------------------- */
    write_padded_line((uint8_t const *)"!!! FATAL ERROR ", 16u, 0u);
    write_padded_line((uint8_t const *)" WHO TOUCHED IT?", 16u, 1u);
    lcd1602_cursor_on(true);
    lcd1602_cursor_blink(true);
    delay_ms(1200u);

    /* ----------------------------------------------------------------
     * Phase 6 — Display flash (~1.2 s)
     *   Simulates the monitor giving up on life.
     * ---------------------------------------------------------------- */
    for (i = 0u; i < 3u; i++)
    {
        lcd1602_display_on(false);
        delay_ms(200u);
        lcd1602_display_on(true);
        delay_ms(200u);
    }

    /* ----------------------------------------------------------------
     * Phase 7 — Cursor erase sweep (~1.6 s)
     *   The blinking cursor gobbles up every character like a very
     *   slow, methodical Pac-Man who files TPS reports.
     * ---------------------------------------------------------------- */
    lcd1602_cursor_on(true);
    lcd1602_cursor_blink(true);

    for (i = 0u; i < 2u; i++)
    {
        for (col = 0u; col < LINE_MAX_COLS; col++)
        {
            (void)lcd1602_set_cursor(col, i);
            delay_ms(50u);
            lcd1602_write_char(' ');
        }
    }

    /* ----------------------------------------------------------------
     * Phase 8 — Suspense (~0.8 s)
     *   Blank screen. Blinking cursor. Infinite dread.
     * ---------------------------------------------------------------- */
    delay_ms(800u);

    /* ----------------------------------------------------------------
     * Phase 9 — Survivor screen (~2 s)
     *   You made it. Barely.
     * ---------------------------------------------------------------- */
    lcd1602_cursor_on(false);
    lcd1602_cursor_blink(false);
    pca9633dp2_set_white(PCA9633DP2_DEFAULT_ADDR);

    write_padded_line((uint8_t const *)"  TEST COMPLETE ", 16u, 0u);
    write_padded_line((uint8_t const *)"U SURVIVED.BASIC", 16u, 1u);
    delay_ms(2000u);

    /* ----------------------------------------------------------------
     * Phase 10 — Restore idle state
     * ---------------------------------------------------------------- */
    lcd1602_clear();
    write_padded_line((uint8_t const *)"BLE LCD READY   ", 16u, 0u);
    s_last_special.valid = false;
}
