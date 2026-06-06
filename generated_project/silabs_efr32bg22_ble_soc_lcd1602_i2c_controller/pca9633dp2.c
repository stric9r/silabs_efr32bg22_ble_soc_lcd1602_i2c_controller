/**
  ******************************************************************************
  * @file           : pca9633dp2.c
  * @brief          : DFRobot LCD1602 backlight controller driver
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
  *
  * Two hardware variants are supported, selected by the I2C address passed in:
  *
  *   0x60 — V1.0 / RGB V1.0: PCA9633DP2
  *           PWM: B=0x02, G=0x03, R=0x04   LEDOUT=0x08
  *
  *   0x6B — V1.1 (blue or RGB): different controller, incompatible register map
  *           PWM: B=0x04, G=0x05, R=0x06   enable latch at 0x07
  ******************************************************************************
  */

#include "pca9633dp2.h"
#include "sl_i2cspm.h"
#include "em_i2c.h"
#include "sl_i2cspm_instances.h"

/* Shares the sl_i2cspm_lcd1602 bus with the AiP31068L LCD controller. */
#define pca9633dp2_i2c  sl_i2cspm_lcd1602

/* PCA9633 register addresses (0x60 variant) */
#define PCA_REG_MODE1   0x00u
#define PCA_REG_MODE2   0x01u
#define PCA_REG_PWM_B   0x02u
#define PCA_REG_PWM_G   0x03u
#define PCA_REG_PWM_R   0x04u
#define PCA_REG_LEDOUT  0x08u

/* V1.1 controller register addresses (0x6B variant) */
#define V11_REG_PWM_B   0x04u
#define V11_REG_PWM_G   0x05u
#define V11_REG_PWM_R   0x06u
#define V11_REG_LATCH   0x07u   /* write 0xFF after every colour update */

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
    if (addr == PCA9633DP2_ADDR_V11) {
        write_reg(addr, 0x2Fu, 0x00u);
        write_reg(addr, 0x00u, 0x20u);
        write_reg(addr, 0x01u, 0x00u);
        write_reg(addr, 0x02u, 0x01u);
        write_reg(addr, 0x03u, 0x04u);
    } else {
        /* PCA9633: individual PWM mode (LEDOUT bits=10), no group gate */
        write_reg(addr, PCA_REG_MODE1,  0x00u);
        write_reg(addr, PCA_REG_MODE2,  0x00u);
        write_reg(addr, PCA_REG_LEDOUT, 0xAAu);
    }
    pca9633dp2_set_white(addr);
}

void pca9633dp2_set_rgb(uint8_t addr, uint8_t r, uint8_t g, uint8_t b)
{
    if (addr == PCA9633DP2_ADDR_V11) {
        write_reg(addr, V11_REG_PWM_B, b);
        write_reg(addr, V11_REG_PWM_G, g);
        write_reg(addr, V11_REG_PWM_R, r);
        write_reg(addr, V11_REG_LATCH, 0xFFu);
    } else {
        write_reg(addr, PCA_REG_PWM_R, r);
        write_reg(addr, PCA_REG_PWM_G, g);
        write_reg(addr, PCA_REG_PWM_B, b);
    }
}

void pca9633dp2_set_white(uint8_t addr)
{
    pca9633dp2_set_rgb(addr, 0xFFu, 0xFFu, 0xFFu);
}

void pca9633dp2_off(uint8_t addr)
{
    pca9633dp2_set_rgb(addr, 0x00u, 0x00u, 0x00u);
}
