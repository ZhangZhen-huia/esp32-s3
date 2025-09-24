#pragma once

#include "math.h"
#include "driver/i2c_master.h"


// I2C配置上下文
typedef struct {
    i2c_port_t port;
    gpio_num_t sda_io;
    gpio_num_t scl_io;
    uint32_t clk_source;
    uint32_t pullup;
} i2c_config_context_t;

#define BSP_I2C_SDA           (GPIO_NUM_1)   // SDA引脚
#define BSP_I2C_SCL           (GPIO_NUM_2)   // SCL引脚

#define BSP_I2C_NUM           (0)            // I2C外设
#define BSP_I2C_FREQ_HZ       100000         // 100kHz

extern i2c_master_bus_handle_t i2c_master_bus_handle;
void register_i2c_devices(void);
