#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Drivers
#include "GPIO_LED.h"

void app_main(void)
{
    ESP_ERROR_CHECK(LED_Init());
    while (1) {
        printf("Toggling LED\n");
        ESP_ERROR_CHECK(LED_Set(true));
        vTaskDelay(pdMS_TO_TICKS(1000));
        printf("Toggling LED\n");
        ESP_ERROR_CHECK(LED_Set(false));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
