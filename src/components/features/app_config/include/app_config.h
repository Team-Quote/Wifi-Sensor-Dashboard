// app_config.h - Project-level configuration for the Wi-Fi dashboard
//
// This file is the simple Phase 1 configuration module.
// Later, these values can be moved to ESP-IDF menuconfig/Kconfig without
// changing the rest of the project architecture.
//
// Do not commit your real Wi-Fi password to a public GitHub repository.

#pragma once

#include "esp_wifi_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Wi-Fi credentials used by wifi_manager.c.
// Replace these placeholder strings with your local test network.
#define APP_WIFI_SSID          "TP-Link_7020_MLO"
#define APP_WIFI_PASSWORD      "53204608"

// Number of reconnect attempts after a disconnect event before reporting FAILED.
#define APP_WIFI_MAX_RETRY     8

// Optional wait time used by wifi_manager_wait_for_connection().
// The app no longer treats a slow DHCP response as a startup failure, but tests
// may still call the wait helper when they need a clear pass/fail result.
#define APP_WIFI_CONNECT_TIMEOUT_MS 30000

// Minimum security mode accepted when connecting to an AP.
// WPA2 is a good default for normal home routers. WPA3 networks are still allowed
// because they are stronger than this threshold.
#define APP_WIFI_AUTHMODE_THRESHOLD WIFI_AUTH_WPA2_PSK

// Telemetry sampling period for monitor.c.
// Your requirement says RSSI should be sampled once every 1.0 second.
#define APP_MONITOR_INTERVAL_MS 1000

#ifdef __cplusplus
}
#endif
