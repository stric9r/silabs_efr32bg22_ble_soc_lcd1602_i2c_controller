/**
  ******************************************************************************
  * @file           : pca9633dp2.h
  * @brief          : PCA9633DP2 RGB LED backlight controller driver
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

#ifndef PCA9633DP2_H
#define PCA9633DP2_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @defgroup pca9633dp2 DFRobot LCD1602 Backlight Driver
 * @brief Driver for the backlight controller on DFRobot Gravity LCD1602 modules.
 *
 * Two hardware variants exist, identified by I2C address:
 *
 *   PCA9633DP2_ADDR_V10 (0x60) — V1.0 and RGB V1.0: NXP PCA9633DP2
 *   PCA9633DP2_ADDR_V11 (0x6B) — V1.1 (blue or RGB): different controller
 *
 * Both variants share the same public API; the implementation dispatches
 * internally based on the address. Pass the correct address constant for your
 * board revision to pca9633dp2_init() and all subsequent calls.
 *
 * Uses the sl_i2cspm_lcd1602 I2C handle, shared with the AiP31068L LCD driver.
 * @{
 */

/** @brief 7-bit I2C address for V1.0 and RGB V1.0 modules (PCA9633DP2). */
#define PCA9633DP2_ADDR_V10      0x60u

/** @brief 7-bit I2C address for V1.1 modules (blue or RGB). */
#define PCA9633DP2_ADDR_V11      0x6Bu

/** @brief Convenience alias — set to the address matching your board. */
#define PCA9633DP2_DEFAULT_ADDR  PCA9633DP2_ADDR_V11

/**
 * @brief Initialise the PCA9633DP2 and set backlight to white.
 *
 * Configures the device for individual PWM control and sets R, G, B
 * channels to full brightness (255, 255, 255).
 *
 * Safe to call even if the PCA9633DP2 is not present — a NACK on the
 * I2C bus is silently ignored.
 *
 * @param addr  7-bit I2C address (typically PCA9633DP2_DEFAULT_ADDR).
 */
void pca9633dp2_init(uint8_t addr);

/**
 * @brief Set the backlight to an arbitrary RGB colour.
 *
 * @param addr  7-bit I2C address.
 * @param r     Red channel intensity, 0–255.
 * @param g     Green channel intensity, 0–255.
 * @param b     Blue channel intensity, 0–255.
 */
void pca9633dp2_set_rgb(uint8_t addr, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Set the backlight to white (255, 255, 255).
 *
 * @param addr  7-bit I2C address.
 */
void pca9633dp2_set_white(uint8_t addr);

/**
 * @brief Turn the backlight off (0, 0, 0).
 *
 * @param addr  7-bit I2C address.
 */
void pca9633dp2_off(uint8_t addr);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* PCA9633DP2_H */
