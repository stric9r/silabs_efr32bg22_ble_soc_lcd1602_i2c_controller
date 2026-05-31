/**
  ******************************************************************************
  * @file           : ble_lcd_service.h
  * @brief          : NUS RX → LCD protocol dispatch interface
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

#ifndef BLE_LCD_SERVICE_H
#define BLE_LCD_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief Handle a raw NUS RX write from the BLE stack.
 *
 * Called from sl_bt_on_event() when gattdb_nus_rx receives a write. Passes
 * the raw bytes to the LCD protocol parser.
 *
 * @param p_data  Pointer to the received payload. Must not be NULL.
 * @param len     Number of valid bytes in @p p_data.
 */
void ble_lcd_service_on_rx(uint8_t const *p_data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* BLE_LCD_SERVICE_H */
