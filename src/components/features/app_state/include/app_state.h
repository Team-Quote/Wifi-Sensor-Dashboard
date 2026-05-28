// app_state.h - Thread-safe shared status model
//
// The dashboard has multiple readers and writers:
// - wifi_manager writes connection state and IP assignment events.
// - monitor writes RSSI, quality, channel, and uptime.
// - web_server will read this state to generate JSON.
// - display will read this state to draw the TFT screen.
//
// Because several FreeRTOS tasks/events touch the same data, app_state.c protects
// the structure with a mutex.

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WIFI_DIAG_SSID_LEN  33
#define WIFI_DIAG_LABEL_LEN 16
#define WIFI_DIAG_IPV4_LEN  16

typedef enum {
    WIFI_DIAG_STATE_BOOTING = 0,
    WIFI_DIAG_STATE_CONNECTING,
    WIFI_DIAG_STATE_CONNECTED,
    WIFI_DIAG_STATE_DISCONNECTED,
    WIFI_DIAG_STATE_FAILED,
} wifi_diag_connection_state_t;

typedef struct {
    bool wifi_connected;
    wifi_diag_connection_state_t connection_state;

    char ssid[WIFI_DIAG_SSID_LEN];
    int rssi;
    int quality;
    char signal_label[WIFI_DIAG_LABEL_LEN];
    uint8_t channel;

    char ip_addr[WIFI_DIAG_IPV4_LEN];
    char gateway_addr[WIFI_DIAG_IPV4_LEN];

    uint32_t uptime_sec;
    uint32_t reconnect_count;
    uint32_t total_samples;
} wifi_diag_status_t;

esp_err_t app_state_init(void);
esp_err_t app_state_get(wifi_diag_status_t *out_status);

esp_err_t app_state_set_connection(wifi_diag_connection_state_t state, bool connected);
esp_err_t app_state_set_ssid(const char *ssid);
esp_err_t app_state_set_network_info(const char *ip_addr, const char *gateway_addr);
esp_err_t app_state_set_signal(int rssi, int quality, const char *label, uint8_t channel);
esp_err_t app_state_set_uptime(uint32_t uptime_sec);

esp_err_t app_state_increment_reconnect_count(void);
uint32_t app_state_get_reconnect_count(void);

const char *app_state_connection_state_to_string(wifi_diag_connection_state_t state);

#ifdef __cplusplus
}
#endif
