/*
 * SPDX-FileCopyrightText: 2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#ifndef __APP_HF_MSG_SET_H__
#define __APP_HF_MSG_SET_H__

#include "esp_err.h"
#include "esp_bt.h"

#define HF_MSG_ARGS_MAX             (5)

typedef int (* hf_cmd_handler)(int argn, char **argv);

typedef struct {
    const char *str;
    hf_cmd_handler handler;
} hf_msg_hdl_t;

/**
 * @brief Store information about a paired Bluetooth device
 * 
 * @param bd_addr Bluetooth address of the paired device
 * @param device_name Name of the paired device (optional)
 * @return ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t app_hf_store_paired_device(const esp_bd_addr_t bd_addr, const char* device_name);

/**
 * @brief Retrieve information about the last paired Bluetooth device
 * 
 * @param bd_addr Buffer to store the Bluetooth address
 * @param device_name Buffer to store the device name (optional)
 * @param name_len Length of the device name buffer
 * @return ESP_OK on success, ESP_FAIL if no device is stored
 */
esp_err_t app_hf_get_paired_device(esp_bd_addr_t bd_addr, char* device_name, size_t name_len);

/**
 * @brief Clear stored information about the paired Bluetooth device
 * 
 * @return ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t app_hf_clear_paired_device(void);

void register_hfp_hf(void);
#endif /* __APP_HF_MSG_SET_H__*/
