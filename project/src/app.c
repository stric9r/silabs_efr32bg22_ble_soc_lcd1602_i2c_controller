/**
  ******************************************************************************
  * @file           : app.c
  * @brief          : BLE application — event glue and hardware initialisation
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

#include "em_common.h"
#include "app_assert.h"
#include "sl_bluetooth.h"
#include "gatt_db.h"
#include "app.h"
#include "implement/pcf8574_i2c.h"
#include "implement/delay.h"
#include "hd44780.h"
#include "ble_lcd_service.h"

static uint8_t advertising_set_handle = 0xFFu;

void app_init(void)
{
    int lcd_init_result;

    pcf8574_init();
    delay_init();

    lcd_init_result = hd44780_init(PCF8574_DEFAULT_ADDR);
    app_assert(0 == lcd_init_result, "hd44780_init failed\n");
}

void app_process_action(void)
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
