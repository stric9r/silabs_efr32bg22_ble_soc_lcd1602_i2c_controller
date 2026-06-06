/**
  ******************************************************************************
  * @file           : ble_node.c
  * @brief          : BLE node — advertising management and GATT event dispatch
  ******************************************************************************
  * SPDX-License-Identifier: MIT
  * Copyright (C) 2026 Stric Roberts.
  ******************************************************************************
  */

#include "app_assert.h"
#include "sl_bluetooth.h"
#include "gatt_db.h"
#include "ble_node.h"
#include "ble_lcd_service.h"

static uint8_t advertising_set_handle = 0xFFu;

void ble_node_init(void)
{
}

void sl_bt_on_event(sl_bt_msg_t *evt)
{
    sl_status_t sc;

    switch (SL_BT_MSG_ID(evt->header))
    {
        case sl_bt_evt_system_boot_id:
            sc = sl_bt_advertiser_create_set(&advertising_set_handle);
            app_assert_status(sc);

            if (SL_STATUS_OK == sc)
            {
                sc = sl_bt_legacy_advertiser_generate_data(
                    advertising_set_handle,
                    sl_bt_advertiser_general_discoverable);
                app_assert_status(sc);
            }

            if (SL_STATUS_OK == sc)
            {
                sc = sl_bt_advertiser_set_timing(
                    advertising_set_handle,
                    160u,
                    160u,
                    0u,
                    0u);
                app_assert_status(sc);
            }

            if (SL_STATUS_OK == sc)
            {
                sc = sl_bt_legacy_advertiser_start(
                    advertising_set_handle,
                    sl_bt_legacy_advertiser_connectable);
                app_assert_status(sc);
            }
            break;

        case sl_bt_evt_connection_opened_id:
            break;

        case sl_bt_evt_connection_closed_id:
            sc = sl_bt_legacy_advertiser_generate_data(
                advertising_set_handle,
                sl_bt_advertiser_general_discoverable);
            app_assert_status(sc);

            if (SL_STATUS_OK == sc)
            {
                sc = sl_bt_legacy_advertiser_start(
                    advertising_set_handle,
                    sl_bt_legacy_advertiser_connectable);
                app_assert_status(sc);
            }
            break;

        case sl_bt_evt_gatt_server_attribute_value_id:
            if (gattdb_nus_rx == evt->data.evt_gatt_server_attribute_value.attribute)
            {
                ble_lcd_service_on_rx(
                    evt->data.evt_gatt_server_attribute_value.value.data,
                    evt->data.evt_gatt_server_attribute_value.value.len);
            }
            break;

        default:
            break;
    }
}
