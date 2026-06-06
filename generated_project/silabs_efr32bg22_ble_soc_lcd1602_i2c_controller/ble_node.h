/**
  ******************************************************************************
  * @file           : ble_node.h
  * @brief          : BLE node — advertising and event dispatch interface
  ******************************************************************************
  * SPDX-License-Identifier: MIT
  * Copyright (C) 2026 Stric Roberts.
  ******************************************************************************
  */

#ifndef BLE_NODE_H
#define BLE_NODE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialise BLE node state.
 *
 * Currently a no-op stub. Reserved for future per-boot initialisation that
 * must run before the BLE stack raises sl_bt_evt_system_boot_id.
 */
void ble_node_init(void);

#ifdef __cplusplus
}
#endif

#endif /* BLE_NODE_H */
