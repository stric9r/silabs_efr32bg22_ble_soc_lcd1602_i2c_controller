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
#include "app_timer.h"
#include "app.h"
#include "lcd1602.h"
#include "pca9633dp2.h"
#include "lcd_protocol.h"
#include "sl_simple_button_instances.h"
#include "sl_button.h"
#include "sl_bluetooth.h"
#include "sl_bt_api.h"

#define BACKLIGHT_DIM         128u
#define BACKLIGHT_FULL        255u
#define BRIGHT_TIMEOUT_MS     (60u * 1000u)

static app_timer_t  backlight_timer;
static bool         ble_connected = false;

static void backlight_set_full(void)
{
    pca9633dp2_set_white(PCA9633DP2_DEFAULT_ADDR);
}

static void backlight_set_dim(void)
{
    pca9633dp2_set_rgb(PCA9633DP2_DEFAULT_ADDR,
                       BACKLIGHT_DIM, BACKLIGHT_DIM, BACKLIGHT_DIM);
}

static void on_backlight_timeout(app_timer_t *timer, void *data)
{
    (void)timer; (void)data;
    if (!ble_connected) {
        backlight_set_dim();
    }
}

static void backlight_trigger_bright(void)
{
    backlight_set_full();
    app_timer_start(&backlight_timer,
                    BRIGHT_TIMEOUT_MS,
                    on_backlight_timeout,
                    NULL,
                    false);
}

void app_init(void)
{
    int lcd_init_result;

    lcd_init_result = lcd1602_init(2u, false);
    app_assert(0 == lcd_init_result, "lcd1602_init failed\n");

    pca9633dp2_init(PCA9633DP2_DEFAULT_ADDR);
    backlight_set_dim();
}

void app_process_action(void)
{
    lcd_protocol_service();
}

