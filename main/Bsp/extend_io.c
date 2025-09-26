#include "Inc/extend_io.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Inc/i2c.h"
#include "Dev/Inc/dev_registry.h"

i2c_master_dev_handle_t i2c_master_dev_handle_PCA9557;


// 读取PCA9557寄存器的值
esp_err_t pca9557_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    uint8_t write_buf[1] = {reg_addr};
    return i2c_master_transmit_receive(i2c_master_dev_handle_PCA9557,write_buf,1,data,len,1000 / portTICK_PERIOD_MS);
}

// 给PCA9557的寄存器写值
esp_err_t pca9557_register_write_byte(uint8_t reg_addr, uint8_t data)
{
    uint8_t write_buf[2] = {reg_addr, data};
    return i2c_master_transmit(i2c_master_dev_handle_PCA9557,write_buf,sizeof(write_buf),1000 / portTICK_PERIOD_MS);
}

// 初始化PCA9557 IO扩展芯片
static esp_err_t pca9557_init(void)
{
    //添加设备
    i2c_device_config_t i2c_device_config = {
        .scl_speed_hz = BSP_I2C_FREQ_HZ,
        .scl_wait_us = 10,
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = PCA9557_SENSOR_ADDR,//要和主机进行通信的地址
        .flags.disable_ack_check = 0,
    };
    if(ESP_OK != i2c_master_bus_add_device(i2c_master_bus_handle,&i2c_device_config,&i2c_master_dev_handle_PCA9557))
        return ESP_FAIL;


    // 写入控制引脚默认值 DVP_PWDN=1  PA_EN = 0  LCD_CS = 1
    pca9557_register_write_byte(PCA9557_OUTPUT_PORT, 0x05);  
    // 把PCA9557芯片的IO1 IO1 IO2设置为输出 其它引脚保持默认的输入
    pca9557_register_write_byte(PCA9557_CONFIGURATION_PORT, 0xf8); 
    
    return ESP_OK;

}


// 设置PCA9557芯片的某个IO引脚输出高低电平
esp_err_t pca9557_set_output_state(uint8_t gpio_bit, uint8_t level)
{
    uint8_t data;
    esp_err_t res = ESP_FAIL;

    pca9557_register_read(PCA9557_OUTPUT_PORT, &data, 1);
    res = pca9557_register_write_byte(PCA9557_OUTPUT_PORT, SET_BITS(data, gpio_bit, level));

    return res;
}

// 控制 PCA9557_LCD_CS 引脚输出高低电平 参数0输出低电平 参数1输出高电平 
void lcd_cs(uint8_t level)
{
    pca9557_set_output_state(LCD_CS_GPIO, level);
}

// 控制 PCA9557_PA_EN 引脚输出高低电平 参数0输出低电平 参数1输出高电平 
void pa_en(uint8_t level)
{
    pca9557_set_output_state(PA_EN_GPIO, level);
}

// 控制 PCA9557_DVP_PWDN 引脚输出高低电平 参数0输出低电平 参数1输出高电平 
void dvp_pwdn(uint8_t level)
{
    pca9557_set_output_state(DVP_PWDN_GPIO, level);
}

void register_extend_io_devices(void)
{
    
// 设备描述符
    static device_descriptor_t io_device = {
        .name = "extend_io",
        .type = IO,
        .init_func = pca9557_init,
        .deinit_func = NULL,
        .priority = DEVICE_EXTENDIO_PRIORITY, // 最高优先级
        .state = DEVICE_STATE_UNINITIALIZED,
        .next = NULL
    };

    device_register(&io_device);
}
