/**
  ******************************************************************************
  * @file           : backlight_ctrl.c
  * @brief          : LCD backlight dim/bright control implementation
  ******************************************************************************
  * SPDX-License-Identifier: MIT
  * Copyright (C) 2026 Stric Roberts.
  ******************************************************************************
  */

#include "backlight_ctrl.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "app_assert.h"
#include "app_timer.h"
#include "pca9633dp2.h"
#include "sl_simple_button_instances.h"
#include "sl_button.h"

#define BACKLIGHT_DIM         128u
#define BACKLIGHT_FULL        255u
#define BRIGHT_TIMEOUT_MS     (60u * 1000u)

static app_timer_t backlight_timer;
static bool        b_ble_connected = false;

/**
 * @brief Drive the backlight to full white.
 */
static void backlight_set_full(void)
{
    pca9633dp2_set_white(PCA9633DP2_DEFAULT_ADDR);
}

/**
 * @brief Drive the backlight to the dim (default/idle) level.
 */
static void backlight_set_dim(void)
{
    pca9633dp2_set_rgb(PCA9633DP2_DEFAULT_ADDR,
                       BACKLIGHT_DIM, BACKLIGHT_DIM, BACKLIGHT_DIM);
}

/**
 * @brief One-shot timer callback: dim the backlight unless a BLE
 *        connection is active.
 *
 * @param p_timer  Unused; required by app_timer_callback_t.
 * @param p_data   Unused; required by app_timer_callback_t.
 */
static void on_backlight_timeout(app_timer_t *p_timer, void *p_data)
{
    (void)p_timer; (void)p_data;
    if (!b_ble_connected)
    {
        backlight_set_dim();
    }
}

/**
 * @brief Force full brightness, optionally arming an auto-dim timer.
 *
 * @param timeout_ms  Milliseconds before auto-dim fires. 0 means stay
 *                     bright with no timer armed (caller owns dimming).
 */
static void backlight_trigger_bright(uint32_t const timeout_ms)
{
    backlight_set_full();
    if (0u != timeout_ms)
    {
        app_timer_start(&backlight_timer,
                        timeout_ms,
                        on_backlight_timeout,
                        NULL,
                        false);
    }
}

void backlight_ctrl_init(void)
{
    pca9633dp2_init(PCA9633DP2_DEFAULT_ADDR);
    backlight_set_dim();
}

void backlight_ctrl_on_connection_opened(void)
{
    b_ble_connected = true;
    app_timer_stop(&backlight_timer);
    backlight_set_full();
}

void backlight_ctrl_on_connection_closed(void)
{
    b_ble_connected = false;
    /* 1-minute grace, then dim. */
    backlight_trigger_bright(BRIGHT_TIMEOUT_MS);
}

/* Overrides the weak sl_button_on_change() provided by the SDK's
 * simple_button driver. */
void sl_button_on_change(sl_button_t const *p_handle)
{
    app_assert(NULL != p_handle, "sl_button_on_change: NULL handle\n");

    if (&sl_button_btn0 == p_handle
        && SL_SIMPLE_BUTTON_PRESSED == sl_button_get_state(p_handle))
    {
        backlight_trigger_bright(BRIGHT_TIMEOUT_MS);
    }
}
