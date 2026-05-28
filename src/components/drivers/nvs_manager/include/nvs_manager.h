// nvs_manager.h - Public interface for NVS initialization
//
// Keep NVS startup isolated from main.c so main.c stays as a clean module
// startup list. Wi-Fi should not start until this function returns ESP_OK.

#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t nvs_manager_init(void);

#ifdef __cplusplus
}
#endif
