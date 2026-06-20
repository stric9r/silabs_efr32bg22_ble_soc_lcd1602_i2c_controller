/**
  ******************************************************************************
  * @file           : backlight_ctrl.h
  * @brief          : LCD backlight dim/bright control
  ******************************************************************************
  * SPDX-License-Identifier: MIT
  * Copyright (C) 2026 Stric Roberts.
  ******************************************************************************
  */

#ifndef BACKLIGHT_CTRL_H
#define BACKLIGHT_CTRL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialise backlight state. Starts dim.
 *
 * Call once from app_init().
 */
void backlight_ctrl_init(void);

/**
 * @brief Notify the backlight controller that a BLE connection opened.
 *
 * Cancels any pending dim timer and forces full brightness for the
 * duration of the connection. Call from sl_bt_on_event() on
 * sl_bt_evt_connection_opened_id.
 */
void backlight_ctrl_on_connection_opened(void);

/**
 * @brief Notify the backlight controller that a BLE connection closed.
 *
 * Forces full brightness, then starts the 1-minute grace timer before
 * dimming. Call from sl_bt_on_event() on sl_bt_evt_connection_closed_id.
 */
void backlight_ctrl_on_connection_closed(void);

#ifdef __cplusplus
}
#endif

#endif /* BACKLIGHT_CTRL_H */
