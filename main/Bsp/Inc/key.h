#pragma once

#include "driver/gpio.h"

#define KEY_GPIO_NUM 0


typedef enum
{
    KEY_RELEASED,
    KEY_PRESSED,
    KEY_PRESSING,
    KEY_SHORT,
    KEY_LONG,
}key_state_e;