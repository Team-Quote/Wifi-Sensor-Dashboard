// wifi_manager.c - Wi-Fi station manager for the ESP32 Wi-Fi Diagnostic Dashboard
//
// This file is based on the same idea as the ESP-IDF station example, but wrapped
// inside a module so main.c stays clean. The important pattern is event-driven:
// start the Wi-Fi driver, then react to WIFI_EVENT and IP_EVENT callbacks.

#include "wifi_manager.h"

#include <string.h>

#include "app_config.h"
#include "app_state.h"
#include "esp_check.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_netif_ip_addr.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

// Event group bits used by wifi_manager_wait_for_connection().
// These bits are not the main control flow of the app; they are just useful for
// tests or optional blocking waits.
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "wifi_manager";

static EventGroupHandle_t s_wifi_event_group;
static esp_netif_t *s_sta_netif;

// Number of retry attempts during the current connection attempt.
static int s_retry_num;

// True only after the station receives an IP address.
static bool s_connected;

// Used to avoid counting startup authentication retries as real dashboard drops.
// reconnect_count should mean "we had a working connection and later lost it."
static bool s_has_connected_once;

// Prevent duplicate initialization if wifi_manager_start() is accidentally called twice.
static bool s_wifi_started;

static esp_err_t validate_wifi_config(void)
{
    // The SSID field in wifi_config_t is 32 bytes plus terminator in practice.
    // app_state.h uses 33 for the same reason.
    if (strlen(APP_WIFI_SSID) == 0 || strlen(APP_WIFI_SSID) >= 33) {
        ESP_LOGE(TAG, "APP_WIFI_SSID must be 1..32 characters");
        return ESP_ERR_INVALID_ARG;
    }

    // Password can be empty for open networks, but this project currently uses a
    // WPA2-or-better threshold. For normal WPA2/WPA3 networks, use 8..63 chars.
    if (strlen(APP_WIFI_PASSWORD) >= 64) {
        ESP_LOGE(TAG, "APP_WIFI_PASSWORD must be 63 characters or fewer");
        return ESP_ERR_INVALID_ARG;
    }

    return ESP_OK;
}

static void set_ip_info_from_event(const ip_event_got_ip_t *event)
{
    char ip_addr[WIFI_DIAG_IPV4_LEN];
    char gateway_addr[WIFI_DIAG_IPV4_LEN];

    snprintf(ip_addr, sizeof(ip_addr), IPSTR, IP2STR(&event->ip_info.ip));
    snprintf(gateway_addr, sizeof(gateway_addr), IPSTR, IP2STR(&event->ip_info.gw));

    app_state_set_network_info(ip_addr, gateway_addr);
    ESP_LOGI(TAG, "Got IP address: %s", ip_addr);
}

static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data)
{
    (void)arg;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        // The Wi-Fi driver is now running. This is the correct time to request a
        // station connection.
        ESP_LOGI(TAG, "Wi-Fi station started; connecting to AP");
        app_state_set_connection(WIFI_DIAG_STATE_CONNECTING, false);
        esp_wifi_connect();
        return;
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        const wifi_event_sta_disconnected_t *event =
            (const wifi_event_sta_disconnected_t *)event_data;
        
        bool was_connected = s_connected;
        s_connected = false;
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);

        // Count only real drop episodes, not every failed retry attempt.
        if (was_connected && s_has_connected_once) {
            app_state_increment_reconnect_count();
        }

        app_state_set_connection(WIFI_DIAG_STATE_DISCONNECTED, false);

        ESP_LOGW(TAG, "Disconnected from AP, reason=%d, retry=%d/%d",
                 event ? event->reason : -1,
                 s_retry_num,
                 APP_WIFI_MAX_RETRY);

        if (s_retry_num < APP_WIFI_MAX_RETRY) {
            s_retry_num++;
            app_state_set_connection(WIFI_DIAG_STATE_CONNECTING, false);
            esp_wifi_connect();
        } else {
            ESP_LOGE(TAG, "Maximum Wi-Fi retries reached");
            app_state_set_connection(WIFI_DIAG_STATE_FAILED, false);
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        return;
    }

    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        const ip_event_got_ip_t *event = (const ip_event_got_ip_t *)event_data;

        s_retry_num = 0;
        s_connected = true;
        s_has_connected_once = true;

        app_state_set_ssid(APP_WIFI_SSID);
        app_state_set_connection(WIFI_DIAG_STATE_CONNECTED, true);
        set_ip_info_from_event(event);

        xEventGroupClearBits(s_wifi_event_group, WIFI_FAIL_BIT);
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        return;
    }
}

esp_err_t wifi_manager_start(void)
{
    if (s_wifi_started) {
        return ESP_OK;
    }

    ESP_RETURN_ON_ERROR(validate_wifi_config(), TAG, "invalid Wi-Fi configuration");

    s_retry_num = 0;
    s_connected = false;
    s_has_connected_once = false;

    if (s_wifi_event_group == NULL) {
        s_wifi_event_group = xEventGroupCreate();
        ESP_RETURN_ON_FALSE(s_wifi_event_group != NULL,
                            ESP_ERR_NO_MEM,
                            TAG,
                            "failed to create Wi-Fi event group");
    }

    xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);

    // Initializes the TCP/IP network interface layer used by Wi-Fi.
    ESP_RETURN_ON_ERROR(esp_netif_init(), TAG, "esp_netif_init failed");

    // The default event loop receives Wi-Fi and IP events. If another component
    // already created it, ESP_ERR_INVALID_STATE is acceptable.
    esp_err_t ret = esp_event_loop_create_default();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "esp_event_loop_create_default failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Creates the default station network interface used by DHCP/IP functions.
    s_sta_netif = esp_netif_create_default_wifi_sta();
    ESP_RETURN_ON_FALSE(s_sta_netif != NULL,
                        ESP_FAIL,
                        TAG,
                        "failed to create default Wi-Fi STA netif");

    wifi_init_config_t init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_RETURN_ON_ERROR(esp_wifi_init(&init_config), TAG, "esp_wifi_init failed");

    // Register one handler for all Wi-Fi events and one handler for IP assignment.
    ESP_RETURN_ON_ERROR(esp_event_handler_instance_register(WIFI_EVENT,
                                                            ESP_EVENT_ANY_ID,
                                                            &wifi_event_handler,
                                                            NULL,
                                                            NULL),
                        TAG,
                        "failed to register Wi-Fi event handler");

    ESP_RETURN_ON_ERROR(esp_event_handler_instance_register(IP_EVENT,
                                                            IP_EVENT_STA_GOT_IP,
                                                            &wifi_event_handler,
                                                            NULL,
                                                            NULL),
                        TAG,
                        "failed to register IP event handler");

    wifi_config_t wifi_config = {0};

    // Copy strings into the ESP-IDF Wi-Fi config struct. Do not store pointers here.
    snprintf((char *)wifi_config.sta.ssid,
             sizeof(wifi_config.sta.ssid),
             "%s",
             APP_WIFI_SSID);
    snprintf((char *)wifi_config.sta.password,
             sizeof(wifi_config.sta.password),
             "%s",
             APP_WIFI_PASSWORD);

    wifi_config.sta.threshold.authmode = APP_WIFI_AUTHMODE_THRESHOLD;

    // Update visible state before the asynchronous event callbacks begin.
    app_state_set_ssid(APP_WIFI_SSID);
    app_state_set_connection(WIFI_DIAG_STATE_CONNECTING, false);

    ESP_RETURN_ON_ERROR(esp_wifi_set_mode(WIFI_MODE_STA), TAG, "esp_wifi_set_mode failed");
    ESP_RETURN_ON_ERROR(esp_wifi_set_config(WIFI_IF_STA, &wifi_config), TAG, "esp_wifi_set_config failed");
    ESP_RETURN_ON_ERROR(esp_wifi_start(), TAG, "esp_wifi_start failed");

    s_wifi_started = true;
    ESP_LOGI(TAG, "Wi-Fi manager started in station mode");

    // Important design choice:
    // Return after the driver starts instead of blocking main.c until DHCP finishes.
    // This avoids false startup failures when the AP is slow to authenticate or assign IP.
    return ESP_OK;
}

esp_err_t wifi_manager_wait_for_connection(uint32_t timeout_ms)
{
    ESP_RETURN_ON_FALSE(s_wifi_event_group != NULL,
                        ESP_ERR_INVALID_STATE,
                        TAG,
                        "Wi-Fi manager has not been started");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           pdMS_TO_TICKS(timeout_ms));

    if (bits & WIFI_CONNECTED_BIT) {
        return ESP_OK;
    }

    if (bits & WIFI_FAIL_BIT) {
        return ESP_FAIL;
    }

    return ESP_ERR_TIMEOUT;
}

bool wifi_manager_is_connected(void)
{
    return s_connected;
}

esp_netif_t *wifi_manager_get_netif(void)
{
    return s_sta_netif;
}

bool wifi_manager_channel_is_24ghz(uint8_t primary_channel)
{
    // 2.4 GHz Wi-Fi channels are commonly 1..14 globally.
    // In the United States, channels 1..11 are normally used, but 12..14 may
    // appear in other regulatory domains, so the generic helper accepts 1..14.
    return primary_channel >= 1 && primary_channel <= 14;
}
