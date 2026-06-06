/**
  ******************************************************************************
  * @file           : app.c
  * @brief          : Hardware initialisation
  ******************************************************************************
  * SPDX-License-Identifier: MIT
  * Copyright (C) 2026 Stric Roberts.
  ******************************************************************************
  */

#include "em_common.h"
#include "app_assert.h"
#include "app.h"
#include "lcd1602.h"
#include "pca9633dp2.h"
#include "lcd_protocol.h"

void app_init(void)
{
    int lcd_init_result;

    lcd_init_result = lcd1602_init(2u, false);
    app_assert(0 == lcd_init_result, "lcd1602_init failed\n");

    pca9633dp2_init(PCA9633DP2_DEFAULT_ADDR);
}

void app_process_action(void)
{
    lcd_protocol_service();
}
