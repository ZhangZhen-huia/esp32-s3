#pragma once

#include "stdio.h"
#include "esp_err.h"
#include "esp_log.h"


// 设备优先级定义 (0=最高, 255=最低)
#define DEVICE_PRIORITY_MIN 0
#define DEVICE_PRIORITY_MAX 255

#define DEVICE_I2C_PRIORITY      0
#define DEVICE_EXTENDIO_PRIORITY DEVICE_I2C_PRIORITY+1
#define DEVICEE_LCD_PRIORITY     DEVICE_EXTENDIO_PRIORITY+1
#define DEVICE_WIFI_PRIORITY     DEVICEE_LCD_PRIORITY+1
#define DEVICE_CAMERA_PRIORITY   DEVICE_WIFI_PRIORITY+1
#define DEVICEE_SD_PRIORITY      DEVICE_CAMERA_PRIORITY+1
#define DEVICEE_MUSIC_PRIORITY   DEVICEE_SD_PRIORITY+1
#define DEVICE_IMU_PRIORITY      255



#define TYPE_LIST(_)          \
    _(PROTOCOL)   _(IO)        \
    _(DISPLAY)  _(WIFI)     \
    _(IMU) _(FILESYSTEM)        


typedef enum {
#define GEN_ENUM(e) e,
    TYPE_LIST(GEN_ENUM)
} device_type_t;

static const char *type[] = {
#define GEN_STR(e) #e,
    TYPE_LIST(GEN_STR)
};




// 设备状态枚举
typedef enum {
    DEVICE_STATE_UNINITIALIZED = 0,
    DEVICE_STATE_INITIALIZED,
    DEVICE_STATE_ERROR,
    DEVICE_STATE_PENDING
} device_state_t;

// 设备初始化/反初始化函数类型
typedef esp_err_t (*device_init_func_t)(void);
typedef esp_err_t (*device_deinit_func_t)(void);

// 设备描述符结构
typedef struct device_descriptor {
    const char* name;                // 设备名称
    device_type_t type;              // 设备类型
    device_init_func_t init_func;    // 初始化函数
    device_deinit_func_t deinit_func;// 反初始化函数
    uint8_t priority;                // 初始化优先级
    device_state_t state;            // 设备状态
    struct device_descriptor* next;  // 下一个设备指针
} device_descriptor_t;

// 设备注册表API
esp_err_t device_registry_init(void);
esp_err_t device_registry_deinit(void);
esp_err_t device_register(device_descriptor_t* device);
esp_err_t device_unregister(const char* name);
esp_err_t devices_initialize_all(void);
esp_err_t devices_deinitialize_all(void);
esp_err_t device_initialize(const char* name);
esp_err_t device_deinitialize(const char* name);
device_descriptor_t* device_find(const char* name);
device_descriptor_t* device_find_by_type(device_type_t type);
void devices_list_all(void);

void Device_Init(void);