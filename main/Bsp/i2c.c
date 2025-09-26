#include "Inc/i2c.h"
#include "Dev/Inc/dev_registry.h"

i2c_master_bus_handle_t i2c_master_bus_handle;




static esp_err_t bsp_i2c_init(void)
{
    i2c_master_bus_config_t i2c_master_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = BSP_I2C_NUM,
        .scl_io_num = BSP_I2C_SCL,
        .sda_io_num = BSP_I2C_SDA,
        .flags.enable_internal_pullup = true
    };
    return i2c_new_master_bus(&i2c_master_bus_config,&i2c_master_bus_handle);
}




void register_i2c_devices(void)
{
    
// I2C设备描述符
    static device_descriptor_t i2c_master_device = {
        .name = "I2C_Master",
        .type = PROTOCOL,
        .init_func = bsp_i2c_init,
        .deinit_func = NULL,
        .priority = DEVICE_I2C_PRIORITY, // 最高优先级
        .state = DEVICE_STATE_UNINITIALIZED,
        .next = NULL
    };

    device_register(&i2c_master_device);
}
