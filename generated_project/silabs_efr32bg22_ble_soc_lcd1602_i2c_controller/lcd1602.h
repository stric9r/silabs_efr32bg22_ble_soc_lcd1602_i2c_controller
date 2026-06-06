/**
  ******************************************************************************
  * @file           : lcd1602.h
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

#ifndef LCD1602_H
#define LCD1602_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup lcd1602 LCD1602 Application API
 * @brief High-level driver for the DFRobot Gravity LCD1602 module.
 *
 * Wraps the AiP31068L I2C-native LCD controller. The module stores the
 * I2C address and display state internally after lcd1602_init(), so
 * callers never pass an address.
 *
 * Custom character workflow:
 *   1. Fill an lcd1602_custom_char_t with binary literals (bit4=leftmost pixel).
 *   2. Call lcd1602_load_custom_char() to write it to a CGRAM slot (0–7).
 *   3. Call lcd1602_write_char() with the slot number (0x00–0x07) to display it.
 * @{
 */

/** @brief Number of visible character columns. */
#define LCD1602_COLS  16u

/** @brief Number of visible character rows. */
#define LCD1602_ROWS  2u

/** @brief Number of user-definable CGRAM slots (0–7). */
#define LCD1602_CGRAM_SLOTS  8u

/**
 * @brief Custom character pixel map.
 *
 * Each of the 8 rows is a 5-bit mask: bit4 = leftmost pixel, bit0 = rightmost.
 * Use C binary literals to see the pattern visually:
 *
 * @code
 * lcd1602_custom_char_t smiley = {{
 *     0b00000,
 *     0b01010,
 *     0b01010,
 *     0b00000,
 *     0b10001,
 *     0b01110,
 *     0b00000,
 *     0b00000,
 * }};
 * @endcode
 */
typedef struct {
    uint8_t row[8];
} lcd1602_custom_char_t;

/** @brief CGRAM slot number (0–7). */
typedef uint8_t lcd1602_slot_t;

/**
 * @brief Pre-defined character: all pixels on (solid block).
 *
 * Load into a CGRAM slot with lcd1602_load_custom_char(), then write with
 * lcd1602_write_char() passing the slot index as the character code.
 */
extern const lcd1602_custom_char_t LCD1602_CHAR_BLOCK;

/**
 * @brief Pre-defined character: all pixels off (blank).
 */
extern const lcd1602_custom_char_t LCD1602_CHAR_BLANK;

/**
 * @brief Initialise the LCD1602 module.
 *
 * Executes the AiP31068L power-on sequence and configures display geometry.
 * Call once after power-on before any other lcd1602 function.
 *
 * @param lines    Number of display lines: 1 or 2.
 * @param font_5x10  Pass true for 5×10 dot font (1-line displays only);
 *                   false for standard 5×8.
 * @return         0 on success.
 */
int lcd1602_init(uint8_t lines, bool font_5x10);

/**
 * @brief Clear all characters and move the cursor to (0, 0).
 */
void lcd1602_clear(void);

/**
 * @brief Return the cursor to position (0, 0) without clearing the display.
 */
void lcd1602_return_home(void);

/**
 * @brief Turn the display on or off.
 *
 * Turning the display off does not clear DDRAM; the content is preserved
 * and reappears when the display is turned back on.
 *
 * @param on  true = display on, false = display off.
 */
void lcd1602_display_on(bool on);

/**
 * @brief Show or hide the underline cursor.
 *
 * @param on  true = cursor visible, false = cursor hidden.
 */
void lcd1602_cursor_on(bool on);

/**
 * @brief Enable or disable cursor blinking.
 *
 * @param on  true = cursor blinks, false = cursor static.
 */
void lcd1602_cursor_blink(bool on);

/**
 * @brief Shift the entire display content one position to the left.
 *
 * The cursor position in DDRAM is unaffected.
 */
void lcd1602_display_shift_left(void);

/**
 * @brief Shift the entire display content one position to the right.
 */
void lcd1602_display_shift_right(void);

/**
 * @brief Move the cursor one position to the left.
 */
void lcd1602_cursor_shift_left(void);

/**
 * @brief Move the cursor one position to the right.
 */
void lcd1602_cursor_shift_right(void);

/**
 * @brief Move the cursor to a specific column and row.
 *
 * Coordinates are zero-indexed: (0, 0) is the top-left character.
 *
 * @param col  Column, 0 to LCD1602_COLS - 1.
 * @param row  Row, 0 to LCD1602_ROWS - 1.
 * @return     0 on success, -1 if col or row is out of range.
 */
int lcd1602_set_cursor(uint8_t col, uint8_t row);

/**
 * @brief Write one character at the current cursor position.
 *
 * Pass a CGRAM slot index (0x00–0x07) to display a custom character.
 *
 * @param c  Character code or CGRAM slot index.
 */
void lcd1602_write_char(char c);

/**
 * @brief Write a null-terminated string at the current cursor position.
 *
 * Does not wrap or clip at column 16; use lcd1602_set_cursor() to
 * position explicitly before writing.
 *
 * @param p_str  Null-terminated string. Must not be NULL.
 */
void lcd1602_write_string(char const *p_str);

/**
 * @brief Load a custom character into a CGRAM slot.
 *
 * Writes the pixel data to the AiP31068L CGRAM. The slot can then be
 * displayed by calling lcd1602_write_char() with the slot index as the char.
 *
 * @param slot    CGRAM slot, 0 to LCD1602_CGRAM_SLOTS - 1.
 * @param p_char  Pointer to the character definition. Must not be NULL.
 */
void lcd1602_load_custom_char(lcd1602_slot_t slot,
                               lcd1602_custom_char_t const *p_char);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* LCD1602_H */
