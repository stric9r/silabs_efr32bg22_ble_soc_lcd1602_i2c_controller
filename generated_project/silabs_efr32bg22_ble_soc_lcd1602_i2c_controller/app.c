/**
  ******************************************************************************
  * @file           : app.c
  * @brief          : Hardware initialisation
  ******************************************************************************
  * SPDX-License-Identifier: MIT
  * Copyright (C) 2026 Stric Roberts.
  ******************************************************************************
  */

#include "app.h"
#include "em_common.h"
#include "app_assert.h"
#include "lcd_intfc.h"
#include "lcd1602.h"
#include "lcd_protocol.h"
#include "backlight_ctrl.h"

void app_init(void)
{
    /* Must run before lcd1602_init(): brings up the I2C bus the LCD
     * driver writes to. Self-sufficient — does not depend on Studio's
     * generated sl_driver_init() having run first. */
    lcd_intfc_init();

    int const lcd_init_result = lcd1602_init(2u, false);
    app_assert(0 == lcd_init_result, "lcd1602_init failed\n");

    backlight_ctrl_init();
}

void app_process_action(void)
{
    lcd_protocol_service();
}
