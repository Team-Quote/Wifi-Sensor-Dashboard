// wifi_manager.h - Public interface for Wi-Fi station management
//
// This module owns ESP32 Wi-Fi station setup, Wi-Fi/IP event handling, retry
// behavior, and basic connection state. Other modules should not call
// esp_wifi_connect(), esp_wifi_start(), or esp_wifi_set_config() directly.

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"
#include "esp_netif.h"

#ifdef __cplusplus
extern "C" {
#endif

// Start Wi-Fi in station mode using values from app_config.h.
// Returns ESP_OK when the Wi-Fi driver has started successfully.
// Connection itself is asynchronous and is reported later through events.
esp_err_t wifi_manager_start(void);

// Optional helper for tests or debug builds.
// Blocks until the station receives an IP address, reaches FAILED state, or times out.
esp_err_t wifi_manager_wait_for_connection(uint32_t timeout_ms);

// Lightweight status helper used by monitor.c.
bool wifi_manager_is_connected(void);

// Gives monitor.c access to the station network interface so it can read IP/gateway.
esp_netif_t *wifi_manager_get_netif(void);

// Testable helper for CN-01. ESP32 station mode only uses 2.4 GHz channels here.
bool wifi_manager_channel_is_24ghz(uint8_t primary_channel);

#ifdef __cplusplus
}
#endif
