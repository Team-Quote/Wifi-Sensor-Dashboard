#include "GPIO_LED.h"

esp_err_t LED_Init()
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    return gpio_config(&io_conf);
}

esp_err_t LED_Set(bool on)
{
    return gpio_set_level(LED_GPIO, on ? 1 : 0);
}