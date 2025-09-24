#include "Inc/qmi8658.h"
#include "Inc/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "Dev/Inc/dev_registry.h"
static const char * TAG = "QMI8658";
i2c_master_dev_handle_t i2c_master_dev_handle_QMI8658;



// 给QMI8658的寄存器写值
esp_err_t qmi8658_register_write_byte(uint8_t reg_addr, uint8_t data)
{
    uint8_t write_buf[2] = {reg_addr, data};

    return i2c_master_transmit(i2c_master_dev_handle_QMI8658, write_buf, sizeof(write_buf), 1000 / portTICK_PERIOD_MS);
}

// 读取QMI8658寄存器的值
esp_err_t qmi8658_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    uint8_t write_buf[1] = {reg_addr};
    return i2c_master_transmit_receive(i2c_master_dev_handle_QMI8658, write_buf,1, data, len, 1000 / portTICK_PERIOD_MS);
}
// 初始化qmi8658
static esp_err_t qmi8658_init(void)
{
    uint8_t id = 0; // 芯片的ID号

    i2c_device_config_t i2c_device_config = {
        .scl_speed_hz = BSP_I2C_FREQ_HZ,
        .scl_wait_us = 10,
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = QMI8658_SENSOR_ADDR,//要和主机进行通信的地址
        .flags.disable_ack_check = 0,
    };
    if(ESP_OK != i2c_master_bus_add_device(i2c_master_bus_handle,&i2c_device_config,&i2c_master_dev_handle_QMI8658))
        return ESP_FAIL; 

    qmi8658_register_read(QMI8658_WHO_AM_I, &id ,1); // 读芯片的ID号
    while (id != 0x05)  // 判断读到的ID号是否是0x05
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // 延时1秒
        qmi8658_register_read(QMI8658_WHO_AM_I, &id ,1); // 读取ID号
    }
    ESP_LOGI(TAG, "QMI8658 OK!");  // 打印信息

    qmi8658_register_write_byte(QMI8658_RESET, 0xb0);  // 复位  
    vTaskDelay(10 / portTICK_PERIOD_MS);  // 延时10ms
    qmi8658_register_write_byte(QMI8658_CTRL1, 0x40); // CTRL1 设置地址自动增加
    qmi8658_register_write_byte(QMI8658_CTRL7, 0x03); // CTRL7 允许加速度和陀螺仪
    qmi8658_register_write_byte(QMI8658_CTRL2, 0x95); // CTRL2 设置ACC 4g 250Hz
    qmi8658_register_write_byte(QMI8658_CTRL3, 0xd5); // CTRL3 设置GRY 512dps 250Hz 

    return ESP_OK;
}   

// 读取加速度和陀螺仪寄存器值
void qmi8658_Read_AccAndGry(t_sQMI8658 *p)
{
    uint8_t status, data_ready=0;
    int16_t buf[6];

    qmi8658_register_read(QMI8658_STATUS0, &status, 1); // 读状态寄存器 
    if (status & 0x03) // 判断加速度和陀螺仪数据是否可读
        data_ready = 1;
    if (data_ready == 1){  // 如果数据可读
        data_ready = 0;
        qmi8658_register_read(QMI8658_AX_L, (uint8_t *)buf, 12); // 读加速度和陀螺仪值
        p->acc_x = buf[0];
        p->acc_y = buf[1];
        p->acc_z = buf[2];
        p->gyr_x = buf[3];
        p->gyr_y = buf[4];
        p->gyr_z = buf[5];
    }
}

// 获取XYZ轴的倾角值
void qmi8658_fetch_angleFromAcc(t_sQMI8658 *p)
{
    float temp;

    qmi8658_Read_AccAndGry(p); // 读取加速度和陀螺仪的寄存器值
    // 根据寄存器值 计算倾角值 并把弧度转换成角度
    temp = (float)p->acc_x / sqrt( ((float)p->acc_y * (float)p->acc_y + (float)p->acc_z * (float)p->acc_z) );
    p->AngleX = atan(temp)*57.29578f; // 180/π=57.29578
    temp = (float)p->acc_y / sqrt( ((float)p->acc_x * (float)p->acc_x + (float)p->acc_z * (float)p->acc_z) );
    p->AngleY = atan(temp)*57.29578f; // 180/π=57.29578
    temp = sqrt( ((float)p->acc_x * (float)p->acc_x + (float)p->acc_y * (float)p->acc_y) ) / (float)p->acc_z;
    p->AngleZ = atan(temp)*57.29578f; // 180/π=57.29578
}


void register_qmi8658_devices(void)
{
    static device_descriptor_t qmi = 
    {
        .name = "QMI8658",
        .init_func = qmi8658_init,
        .deinit_func = NULL,
        .priority = 3,
        .state = DEVICE_STATE_UNINITIALIZED,
        .next = NULL,
        .type = IMU,
    };

        device_register(&qmi);

}