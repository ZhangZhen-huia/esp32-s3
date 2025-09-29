#include "Inc/key.h"

void bsp_key_init(void)
{
    gpio_config_t gpio = 
    {
        .pin_bit_mask = (1 << KEY_GPIO_NUM),
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,//下拉禁止
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };

    ESP_ERROR_CHECK(gpio_config(&gpio));
}


