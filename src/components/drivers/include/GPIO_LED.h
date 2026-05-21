#pragma once

#include <stdbool.h>
#include "esp_err.h"
#include "driver/gpio.h"

#define LED_GPIO GPIO_NUM_2


esp_err_t LED_Init(void);
esp_err_t LED_Set(bool on);