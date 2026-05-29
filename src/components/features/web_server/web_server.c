// web_server.c - HTTP API server for the ESP32 Wi-Fi Diagnostic Dashboard
//
// This module exposes the current app_state as JSON. It does not own Wi-Fi,
// sampling, or signal math. It only reads the shared state model and serves it
// to same-LAN clients through ESP-IDF's lightweight HTTP server.

#include "web_server.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "app_state.h"
#include "esp_http_server.h"
#include "esp_log.h"

static const char *TAG = "web_server";

static httpd_handle_t s_server = NULL;

// Small helper used by JSON serialization.
// It escapes strings so that SSIDs or labels containing quotes/backslashes do
// not break the JSON response.
static bool json_escape_string(const char *input, char *output, size_t output_size)
{
    if (output == NULL || output_size == 0) {
        return false;
    }

    output[0] = '\0';

    if (input == NULL) {
        return true;
    }

    size_t out_index = 0;

    for (size_t i = 0; input[i] != '\0'; i++) {
        unsigned char c = (unsigned char)input[i];

        const char *escape = NULL;

        switch (c) {
            case '\"':
                escape = "\\\"";
                break;
            case '\\':
                escape = "\\\\";
                break;
            case '\n':
                escape = "\\n";
                break;
            case '\r':
                escape = "\\r";
                break;
            case '\t':
                escape = "\\t";
                break;
            default:
                break;
        }

        if (escape != NULL) {
            size_t escape_len = strlen(escape);

            if (out_index + escape_len >= output_size) {
                return false;
            }

            memcpy(&output[out_index], escape, escape_len);
            out_index += escape_len;
            output[out_index] = '\0';
            continue;
        }

        // JSON does not allow raw control characters below 0x20.
        if (c < 0x20) {
            if (out_index + 6 >= output_size) {
                return false;
            }

            int written = snprintf(&output[out_index],
                                   output_size - out_index,
                                   "\\u%04x",
                                   c);

            if (written != 6) {
                return false;
            }

            out_index += 6;
            continue;
        }

        if (out_index + 1 >= output_size) {
            return false;
        }

        output[out_index++] = (char)c;
        output[out_index] = '\0';
    }

    return true;
}

int web_server_build_status_json(char *buffer,
                                 size_t buffer_size,
                                 const wifi_diag_status_t *status)
{
    if (buffer == NULL || buffer_size == 0 || status == NULL) {
        return -1;
    }

    char ssid_json[WIFI_DIAG_SSID_LEN * 6 + 1];
    char label_json[WIFI_DIAG_LABEL_LEN * 6 + 1];
    char ip_json[WIFI_DIAG_IPV4_LEN * 6 + 1];
    char gateway_json[WIFI_DIAG_IPV4_LEN * 6 + 1];
    char state_json[32];

    const char *state_text =
        app_state_connection_state_to_string(status->connection_state);

    if (!json_escape_string(status->ssid, ssid_json, sizeof(ssid_json)) ||
        !json_escape_string(status->signal_label, label_json, sizeof(label_json)) ||
        !json_escape_string(status->ip_addr, ip_json, sizeof(ip_json)) ||
        !json_escape_string(status->gateway_addr, gateway_json, sizeof(gateway_json)) ||
        !json_escape_string(state_text, state_json, sizeof(state_json))) {
        return -1;
    }

    int written = snprintf(buffer,
                           buffer_size,
                           "{"
                           "\"wifi_connected\":%s,"
                           "\"connection_state\":\"%s\","
                           "\"ssid\":\"%s\","
                           "\"rssi\":%d,"
                           "\"quality\":%d,"
                           "\"signal_label\":\"%s\","
                           "\"channel\":%u,"
                           "\"ip_addr\":\"%s\","
                           "\"gateway_addr\":\"%s\","
                           "\"uptime_sec\":%" PRIu32 ","
                           "\"reconnect_count\":%" PRIu32 ","
                           "\"total_samples\":%" PRIu32
                           "}",
                           status->wifi_connected ? "true" : "false",
                           state_json,
                           ssid_json,
                           status->rssi,
                           status->quality,
                           label_json,
                           status->channel,
                           ip_json,
                           gateway_json,
                           status->uptime_sec,
                           status->reconnect_count,
                           status->total_samples);

    if (written < 0 || (size_t)written >= buffer_size) {
        return -1;
    }

    return written;
}

static esp_err_t status_get_handler(httpd_req_t *req)
{
    wifi_diag_status_t status;

    esp_err_t ret = app_state_get(&status);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "app_state_get failed: %s", esp_err_to_name(ret));
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "app_state_get failed");
        return ret;
    }

    char json[768];
    int written = web_server_build_status_json(json, sizeof(json), &status);

    if (written < 0) {
        ESP_LOGE(TAG, "failed to build JSON response");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "JSON build failed");
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    return httpd_resp_send(req, json, written);
}

// Temporary Phase 3 root page.
// Phase 4 will replace this with the real dashboard.
static esp_err_t root_get_handler(httpd_req_t *req)
{
    static const char html[] =
        "<!doctype html>"
        "<html>"
        "<head>"
        "<meta charset=\"utf-8\">"
        "<title>Another Black Box</title>"
        "</head>"
        "<body>"
        "<h1>Another Black Box</h1>"
        "<p>Wi-Fi Diagnostic Dashboard API is running.</p>"
        "<p>Open <a href=\"/api/status\">/api/status</a> to view JSON.</p>"
        "</body>"
        "</html>";

    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");

    return httpd_resp_send(req, html, HTTPD_RESP_USE_STRLEN);
}

esp_err_t web_server_start(void)
{
    if (s_server != NULL) {
        return ESP_OK;
    }

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Keep this lightweight for the ESP32.
    // More URI handlers can be added later for /api/history and /api/scan.
    config.server_port = 80;
    config.max_uri_handlers = 4;
    config.lru_purge_enable = true;

    esp_err_t ret = httpd_start(&s_server, &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_start failed: %s", esp_err_to_name(ret));
        s_server = NULL;
        return ret;
    }

    const httpd_uri_t root_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = root_get_handler,
        .user_ctx = NULL,
    };

    const httpd_uri_t status_uri = {
        .uri = "/api/status",
        .method = HTTP_GET,
        .handler = status_get_handler,
        .user_ctx = NULL,
    };

    ret = httpd_register_uri_handler(s_server, &root_uri);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "failed to register / handler: %s", esp_err_to_name(ret));
        httpd_stop(s_server);
        s_server = NULL;
        return ret;
    }

    ret = httpd_register_uri_handler(s_server, &status_uri);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "failed to register /api/status handler: %s", esp_err_to_name(ret));
        httpd_stop(s_server);
        s_server = NULL;
        return ret;
    }

    ESP_LOGI(TAG, "HTTP server started on port %d", config.server_port);
    return ESP_OK;
}

esp_err_t web_server_stop(void)
{
    if (s_server == NULL) {
        return ESP_OK;
    }

    esp_err_t ret = httpd_stop(s_server);
    if (ret == ESP_OK) {
        s_server = NULL;
        ESP_LOGI(TAG, "HTTP server stopped");
    }

    return ret;
}