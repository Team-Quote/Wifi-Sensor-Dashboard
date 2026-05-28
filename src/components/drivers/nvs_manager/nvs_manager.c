// nvs_manager.c - Non-Volatile Storage startup helper
//
// Why this module exists:
// ESP-IDF Wi-Fi uses NVS internally for calibration data and saved Wi-Fi data.
// Instead of placing this startup boilerplate in main.c, this module gives the
// application one clean call: nvs_manager_init().

#include "nvs_manager.h"

#include "esp_log.h"
#include "nvs_flash.h"

static const char *TAG = "nvs_manager";

esp_err_t nvs_manager_init(void)
{
    // First attempt: initialize the default NVS partition.
    esp_err_t ret = nvs_flash_init();

    // These two errors are common after partition-table changes, OTA layout
    // changes, or flashing a project over an old NVS layout. The standard ESP-IDF
    // recovery pattern is: erase NVS, then try initialization again.
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS needs erase, erasing and retrying");

        esp_err_t erase_ret = nvs_flash_erase();
        if (erase_ret != ESP_OK) {
            ESP_LOGE(TAG, "nvs_flash_erase failed: %s", esp_err_to_name(erase_ret));
            return erase_ret;
        }

        ret = nvs_flash_init();
    }

    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "NVS initialized");
    } else {
        ESP_LOGE(TAG, "NVS init failed: %s", esp_err_to_name(ret));
    }

    return ret;
}
