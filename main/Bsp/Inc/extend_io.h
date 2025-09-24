#pragma once
#include "esp_err.h"
#include "stdint.h"

#define PCA9557_INPUT_PORT              0x00
#define PCA9557_OUTPUT_PORT             0x01
#define PCA9557_POLARITY_INVERSION_PORT 0x02
#define PCA9557_CONFIGURATION_PORT      0x03

#define LCD_CS_GPIO                 BIT(0)    // PCA9557_GPIO_NUM_1
#define PA_EN_GPIO                  BIT(1)    // PCA9557_GPIO_NUM_2
#define DVP_PWDN_GPIO               BIT(2)    // PCA9557_GPIO_NUM_3

#define PCA9557_SENSOR_ADDR             0x19        

#define SET_BITS(_m, _s, _v)  ((_v) ? (_m)|((_s)) : (_m)&~((_s)))


typedef struct 
{
    uint32_t scl_speed_hz;
    uint32_t scl_wait_us;
    uint32_t dev_addr_length;
    uint16_t device_address;//要和主机进行通信的地址
    uint32_t disable_ack_check;
}extend_io_config_context_t;


void lcd_cs(uint8_t level);
void pa_en(uint8_t level);
void dvp_pwdn(uint8_t level);
void register_extend_io_devices(void);


