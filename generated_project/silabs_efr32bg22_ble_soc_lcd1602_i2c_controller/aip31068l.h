/**
  ******************************************************************************
  * @file           : aip31068l.h
  * @brief          : AiP31068L I2C-native LCD controller driver
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

#ifndef AIP31068L_H
#define AIP31068L_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @defgroup aip31068l AiP31068L LCD Driver
 * @brief Driver for the AiP31068L I2C-native character LCD controller.
 *
 * The AiP31068L is HD44780-compatible in command set but communicates over
 * a native I2C interface. Each transaction is two bytes:
 *   - Control byte: 0x80 for commands (Co=1, RS=0), 0x40 for data (Co=0, RS=1)
 *   - Payload byte: command or character data
 *
 * Hardware: DFRobot Gravity LCD1602 (AiP31068L at 0x3E, PCA9633DP2 at 0x60)
 * @{
 */

/** @brief Default 7-bit I2C address of the AiP31068L. */
#define AIP31068L_DEFAULT_ADDR  0x3Eu

/** @brief Number of visible character columns. */
#define AIP31068L_COLS  16u

/** @brief Number of visible character rows. */
#define AIP31068L_ROWS  2u

/** @brief Number of user-definable CGRAM character slots (0–7). */
#define AIP31068L_CGRAM_SLOTS  8u

/** @brief Number of pixel rows in one CGRAM character (5×8 font). */
#define AIP31068L_CGRAM_ROWS  8u

/**
 * @brief Execute the AiP31068L power-on initialisation sequence.
 *
 * Follows the DFRobot reference implementation: 50 ms VCC stabilisation,
 * three Function Set commands (8-bit, 2-line, 5×8), then Display ON,
 * Clear, and Entry Mode Set.
 *
 * Call once after power-on before any other function.
 *
 * @param addr  7-bit I2C address of the AiP31068L.
 * @return      0 on success.
 */
int aip31068l_init(uint8_t addr);

/**
 * @brief Send a raw command byte to the AiP31068L.
 *
 * Sends control byte 0x80 (command register) followed by @p cmd.
 * Useful for lcd1602.c state-management functions that compose commands
 * by ORing flag bits without a full re-init.
 *
 * @param addr  7-bit I2C address.
 * @param cmd   HD44780-compatible instruction byte.
 */
void aip31068l_command(uint8_t addr, uint8_t cmd);

/**
 * @brief Clear all characters and move the cursor to (0, 0).
 *
 * @param addr  7-bit I2C address.
 */
void aip31068l_clear(uint8_t addr);

/**
 * @brief Return the cursor to position (0, 0) without clearing DDRAM.
 *
 * @param addr  7-bit I2C address.
 */
void aip31068l_return_home(uint8_t addr);

/**
 * @brief Move the cursor to a specific column and row.
 *
 * Coordinates are zero-indexed: (0, 0) is the top-left character.
 *
 * @param addr  7-bit I2C address.
 * @param col   Column, 0 to AIP31068L_COLS - 1.
 * @param row   Row, 0 to AIP31068L_ROWS - 1.
 * @return      0 on success, -1 if col or row is out of range.
 */
int aip31068l_set_cursor(uint8_t addr, uint8_t col, uint8_t row);

/**
 * @brief Write one character at the current cursor position.
 *
 * @param addr  7-bit I2C address.
 * @param c     Character to display.
 */
void aip31068l_write_char(uint8_t addr, char c);

/**
 * @brief Write a null-terminated string at the current cursor position.
 *
 * @param addr   7-bit I2C address.
 * @param p_str  Null-terminated string. Must not be NULL.
 */
void aip31068l_write_string(uint8_t addr, char const *p_str);

/**
 * @brief Load a custom character into a CGRAM slot.
 *
 * Writes 8 row-bytes as a single 9-byte I2C stream transaction
 * (0x40 control byte + 8 data bytes) for efficiency.
 *
 * @param addr    7-bit I2C address.
 * @param slot    CGRAM slot, 0 to AIP31068L_CGRAM_SLOTS - 1.
 * @param p_rows  Pointer to 8 bytes: one per pixel row, 5 bits wide (bit4=left).
 */
void aip31068l_load_custom_char(uint8_t addr, uint8_t slot, uint8_t const *p_rows);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* AIP31068L_H */
