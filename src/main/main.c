// main.c - Main application entry point for WiFi Sensor Dashboard
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"

#include "nvs_manager.h"
#include "app_state.h"
#include "wifi_manager.h"
#include "monitor.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "Starting ESP32 Wi-Fi Diagnostic Dashboard");

    // 1. NVS must be initialized before Wi-Fi.
    //    The ESP-IDF Wi-Fi stack stores calibration and internal Wi-Fi data in NVS.
    ESP_ERROR_CHECK(nvs_manager_init());

    // 2. Initialize the shared status model before any module tries to update it.
    //    Wi-Fi events and the monitor task both write into app_state.
    ESP_ERROR_CHECK(app_state_init());

    // 3. Start Wi-Fi station mode.
    //    SSID, password, retry count, and timeout are now owned by app_config.h
    //    and wifi_manager.c, not by main.c.
    ESP_ERROR_CHECK(wifi_manager_start());

    // 4. Start telemetry sampling.
    //    The monitor task reads RSSI/IP/channel once per configured interval and
    //    writes the latest values into app_state.
    ESP_ERROR_CHECK(monitor_start());

    // Nothing else is required here yet.
    // Later modules such as web_server_start() and display_start() will be added
    // below monitor_start(), while main.c still remains a simple startup list.
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}