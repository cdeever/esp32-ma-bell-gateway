/*
 * SPDX-FileCopyrightText: 2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#ifndef __BT_APP_HF_H__
#define __BT_APP_HF_H__

#include <stdint.h>
#include "esp_hf_client_api.h"


#define BT_HF_TAG               "BT_HF"

/**
 * @brief     callback function for HF client
 */
void bt_app_hf_client_cb(esp_hf_client_cb_event_t event, esp_hf_client_cb_param_t *param);

/**
 * @brief     Register HFP audio data callbacks (for HCI audio path)
 *
 * Registers the incoming and outgoing audio data callbacks with the HFP stack.
 * Must be called after esp_hf_client_init() and before audio connections.
 */
void bt_app_hf_register_data_callbacks(void);

#endif /* __BT_APP_HF_H__*/
