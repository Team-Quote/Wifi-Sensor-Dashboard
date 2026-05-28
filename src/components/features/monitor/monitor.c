// monitor.c - Periodic Wi-Fi telemetry sampler
//
// This module answers the dashboard question: "How good is my Wi-Fi right now?"
// It does not manage the Wi-Fi connection. It only reads current information and
// stores a clean snapshot in app_state for the web server and display to use.

#include "monitor.h"

#include <inttypes.h>
#include <stdio.h>

#include "app_config.h"
#include "app_state.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_netif_ip_addr.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "signal_math.h"
#include "wifi_manager.h"

static const char *TAG = "monitor";
static TaskHandle_t s_monitor_task_handle;

static void update_ip_info(void)
{
    esp_netif_t *netif = wifi_manager_get_netif();
    if (netif == NULL) {
        return;
    }

    esp_netif_ip_info_t ip_info;
    if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
        char ip_addr[WIFI_DIAG_IPV4_LEN];
        char gateway_addr[WIFI_DIAG_IPV4_LEN];

        snprintf(ip_addr, sizeof(ip_addr), IPSTR, IP2STR(&ip_info.ip));
        snprintf(gateway_addr, sizeof(gateway_addr), IPSTR, IP2STR(&ip_info.gw));

        app_state_set_network_info(ip_addr, gateway_addr);
    }
}

static void update_signal_info(void)
{
    if (!wifi_manager_is_connected()) {
        // Do not sample RSSI while disconnected; esp_wifi_sta_get_ap_info() only
        // gives useful data after association with an AP.
        return;
    }

    wifi_ap_record_t ap_info;
    esp_err_t ret = esp_wifi_sta_get_ap_info(&ap_info);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "esp_wifi_sta_get_ap_info failed: %s", esp_err_to_name(ret));
        return;
    }

    int quality = signal_math_rssi_to_quality(ap_info.rssi);
    const char *label = signal_math_quality_to_label(quality);

    app_state_set_signal(ap_info.rssi, quality, label, ap_info.primary);
    update_ip_info();
}

static void update_uptime(void)
{
    // esp_timer_get_time() returns microseconds since boot.
    uint32_t uptime_sec = (uint32_t)(esp_timer_get_time() / 1000000ULL);
    app_state_set_uptime(uptime_sec);
}

static void print_status(void)
{
    wifi_diag_status_t status;
    if (app_state_get(&status) != ESP_OK) {
        return;
    }

    ESP_LOGI(TAG,
             "state=%s ssid=%s ip=%s gw=%s rssi=%d dBm quality=%d%% label=%s ch=%u uptime=%" PRIu32 "s reconnects=%" PRIu32 " samples=%" PRIu32,
             app_state_connection_state_to_string(status.connection_state),
             status.ssid,
             status.ip_addr,
             status.gateway_addr,
             status.rssi,
             status.quality,
             status.signal_label,
             status.channel,
             status.uptime_sec,
             status.reconnect_count,
             status.total_samples);
}

void monitor_task(void *arg)
{
    (void)arg;

    while (true) {
        update_uptime();
        update_signal_info();
        print_status();

        vTaskDelay(pdMS_TO_TICKS(APP_MONITOR_INTERVAL_MS));
    }
}

esp_err_t monitor_start(void)
{
    if (s_monitor_task_handle != NULL) {
        return ESP_OK;
    }

    BaseType_t ok = xTaskCreate(monitor_task,
                                "monitor_task",
                                4096,
                                NULL,
                                5,
                                &s_monitor_task_handle);

    ESP_RETURN_ON_FALSE(ok == pdPASS,
                        ESP_ERR_NO_MEM,
                        TAG,
                        "failed to create monitor task");

    ESP_LOGI(TAG, "Monitor task started");
    return ESP_OK;
}
