#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Drivers
#include "GPIO_LED.h"

void app_main(void)
{
    SystemInit();
    while (1) 
    {
        blink();
    }
}

void SystemInit(void)
{
    // Initialize system components here if needed
    // Example: Clock initialization, memory initialization, etc.
    ESP_ERROR_CHECK(LED_Init());
}

void blink(void)
{
    ESP_ERROR_CHECK(LED_Set(true));
    vTaskDelay(pdMS_TO_TICKS(1000));
    ESP_ERROR_CHECK(LED_Set(false));
    vTaskDelay(pdMS_TO_TICKS(1000));
}