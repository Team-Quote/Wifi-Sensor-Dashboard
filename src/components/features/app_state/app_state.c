// app_state.c - Thread-safe shared status model implementation
//
// This module is intentionally boring and predictable. Its whole purpose is to
// centralize shared data so that web_server.c, display.c, monitor.c, and
// wifi_manager.c do not each keep their own conflicting version of Wi-Fi state.

#include "app_state.h"

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static wifi_diag_status_t s_status;
static SemaphoreHandle_t s_mutex;

static void copy_string(char *dest, size_t dest_size, const char *src)
{
    if (dest == NULL || dest_size == 0) {
        return;
    }

    if (src == NULL) {
        dest[0] = '\0';
        return;
    }

    // snprintf always terminates the destination buffer when size > 0.
    snprintf(dest, dest_size, "%s", src);
}

esp_err_t app_state_init(void)
{
    if (s_mutex == NULL) {
        s_mutex = xSemaphoreCreateMutex();
        if (s_mutex == NULL) {
            return ESP_ERR_NO_MEM;
        }
    }

    xSemaphoreTake(s_mutex, portMAX_DELAY);

    memset(&s_status, 0, sizeof(s_status));

    // Safe default values shown before Wi-Fi connects.
    s_status.connection_state = WIFI_DIAG_STATE_BOOTING;
    s_status.wifi_connected = false;
    s_status.rssi = -127;
    s_status.quality = 0;
    s_status.channel = 0;
    copy_string(s_status.signal_label, sizeof(s_status.signal_label), "Unknown");
    copy_string(s_status.ip_addr, sizeof(s_status.ip_addr), "0.0.0.0");
    copy_string(s_status.gateway_addr, sizeof(s_status.gateway_addr), "0.0.0.0");

    xSemaphoreGive(s_mutex);
    return ESP_OK;
}

esp_err_t app_state_get(wifi_diag_status_t *out_status)
{
    if (out_status == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (s_mutex == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    xSemaphoreTake(s_mutex, portMAX_DELAY);
    *out_status = s_status;
    xSemaphoreGive(s_mutex);

    return ESP_OK;
}

esp_err_t app_state_set_connection(wifi_diag_connection_state_t state, bool connected)
{
    if (s_mutex == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    xSemaphoreTake(s_mutex, portMAX_DELAY);
    s_status.connection_state = state;
    s_status.wifi_connected = connected;
    xSemaphoreGive(s_mutex);

    return ESP_OK;
}

esp_err_t app_state_set_ssid(const char *ssid)
{
    if (s_mutex == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    xSemaphoreTake(s_mutex, portMAX_DELAY);
    copy_string(s_status.ssid, sizeof(s_status.ssid), ssid);
    xSemaphoreGive(s_mutex);

    return ESP_OK;
}

esp_err_t app_state_set_network_info(const char *ip_addr, const char *gateway_addr)
{
    if (s_mutex == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    xSemaphoreTake(s_mutex, portMAX_DELAY);
    copy_string(s_status.ip_addr, sizeof(s_status.ip_addr), ip_addr);
    copy_string(s_status.gateway_addr, sizeof(s_status.gateway_addr), gateway_addr);
    xSemaphoreGive(s_mutex);

    return ESP_OK;
}

esp_err_t app_state_set_signal(int rssi, int quality, const char *label, uint8_t channel)
{
    if (s_mutex == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    xSemaphoreTake(s_mutex, portMAX_DELAY);
    s_status.rssi = rssi;
    s_status.quality = quality;
    copy_string(s_status.signal_label, sizeof(s_status.signal_label), label);
    s_status.channel = channel;
    s_status.total_samples++;
    xSemaphoreGive(s_mutex);

    return ESP_OK;
}

esp_err_t app_state_set_uptime(uint32_t uptime_sec)
{
    if (s_mutex == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    xSemaphoreTake(s_mutex, portMAX_DELAY);
    s_status.uptime_sec = uptime_sec;
    xSemaphoreGive(s_mutex);

    return ESP_OK;
}

esp_err_t app_state_increment_reconnect_count(void)
{
    if (s_mutex == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    xSemaphoreTake(s_mutex, portMAX_DELAY);
    s_status.reconnect_count++;
    xSemaphoreGive(s_mutex);

    return ESP_OK;
}

uint32_t app_state_get_reconnect_count(void)
{
    uint32_t count = 0;

    if (s_mutex == NULL) {
        return 0;
    }

    xSemaphoreTake(s_mutex, portMAX_DELAY);
    count = s_status.reconnect_count;
    xSemaphoreGive(s_mutex);

    return count;
}

const char *app_state_connection_state_to_string(wifi_diag_connection_state_t state)
{
    switch (state) {
        case WIFI_DIAG_STATE_BOOTING:
            return "Booting";
        case WIFI_DIAG_STATE_CONNECTING:
            return "Connecting";
        case WIFI_DIAG_STATE_CONNECTED:
            return "Connected";
        case WIFI_DIAG_STATE_DISCONNECTED:
            return "Disconnected";
        case WIFI_DIAG_STATE_FAILED:
            return "Failed";
        default:
            return "Unknown";
    }
}
