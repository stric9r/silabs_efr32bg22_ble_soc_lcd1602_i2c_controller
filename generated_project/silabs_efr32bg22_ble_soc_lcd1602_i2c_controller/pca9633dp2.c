/**
  ******************************************************************************
  * @file           : pca9633dp2.c
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
  *
  * Protocol reference: DFRobot_RGBLCD1602 C++ driver (MIT), yangfeng@dfrobot.com
  * Register layout: PCA9633 datasheet, 0xC0>>1 address variant.
  ******************************************************************************
  */

#include "pca9633dp2.h"
#include "sl_i2cspm.h"
#include "em_i2c.h"
#include "sl_i2cspm_instances.h"

/* Shares the same I2C peripheral as the AiP31068L LCD. */
#define pca9633dp2_i2c  sl_i2cspm_pcf8574

/* PCA9633 register addresses */
#define REG_MODE1   0x00u
#define REG_MODE2   0x01u
#define REG_PWM_B   0x02u   /* Blue  channel PWM */
#define REG_PWM_G   0x03u   /* Green channel PWM */
#define REG_PWM_R   0x04u   /* Red   channel PWM */
#define REG_LEDOUT  0x08u   /* LED output state  */

/* ---------------------------------------------------------------------------
 * Internal helper
 * ---------------------------------------------------------------------------*/

static void write_reg(uint8_t addr, uint8_t reg, uint8_t value)
{
    uint8_t buf[2] = { reg, value };
    I2C_TransferSeq_TypeDef seq;

    seq.addr        = (uint16_t)(addr << 1u);
    seq.flags       = I2C_FLAG_WRITE;
    seq.buf[0].data = buf;
    seq.buf[0].len  = 2u;

    (void)I2CSPM_Transfer(pca9633dp2_i2c, &seq);
}

/* ---------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------------*/

void pca9633dp2_init(uint8_t addr)
{
    write_reg(addr, REG_MODE1,  0x00u);   /* normal mode, no sleep */
    write_reg(addr, REG_LEDOUT, 0xFFu);   /* all channels: individual PWM */
    write_reg(addr, REG_MODE2,  0x20u);   /* DMBLNK=1, totem-pole output */
    pca9633dp2_set_white(addr);
}

void pca9633dp2_set_rgb(uint8_t addr, uint8_t r, uint8_t g, uint8_t b)
{
    write_reg(addr, REG_PWM_R, r);
    write_reg(addr, REG_PWM_G, g);
    write_reg(addr, REG_PWM_B, b);
}

void pca9633dp2_set_white(uint8_t addr)
{
    pca9633dp2_set_rgb(addr, 0xFFu, 0xFFu, 0xFFu);
}

void pca9633dp2_off(uint8_t addr)
{
    pca9633dp2_set_rgb(addr, 0x00u, 0x00u, 0x00u);
}
