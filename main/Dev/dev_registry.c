#include "Inc/dev_registry.h"
#include <string.h>
#include "esp_log.h"
#include "Bsp/Inc/Bsp.h"
#include "Inc/app_wifi.h"
#include "Inc/app_filesystem.h"
static const char *TAG = "DeviceRegistry";

// 设备链表头指针
static device_descriptor_t* device_list_head = NULL;




// 注册设备和状态
void register_all_devices(void) {
    register_i2c_devices();
    register_extend_io_devices();
    register_Lcd_devices();  
    register_qmi8658_devices();
    register_wifi_device();
    register_sdcard_device();
    register_camera_device();
    // 添加其他设备的注册函数调用
}


void Device_Init(void)
{
    //目前没有实现这个函数
    esp_err_t err = device_registry_init();
    if (err != ESP_OK) {
        ESP_LOGE("Main", "设备注册表初始化失败");
        return;
    }

    // 注册所有设备
    register_all_devices();

    // 列出所有已注册设备
    devices_list_all();

    // 初始化所有已注册的设备（按优先级顺序）
    err = devices_initialize_all();
    if (err != ESP_OK) {
        ESP_LOGE("Main", "设备初始化失败");
        // device_registry_deinit();
        return;
    }
    load_all_sd_imgs();

}


// 初始化设备注册表
esp_err_t device_registry_init(void) {
    // 目前不需要特殊初始化，保留函数以备将来扩展
    ESP_LOGI(TAG, "设备注册表初始化完成");
    return ESP_OK;
}

// 反初始化设备注册表
esp_err_t device_registry_deinit(void) {
    // 反初始化所有设备
    esp_err_t err = devices_deinitialize_all();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "设备反初始化失败: %s", esp_err_to_name(err));
    }
    
    // 清空设备列表
    device_descriptor_t* current = device_list_head;
    while (current != NULL) {
        device_descriptor_t* next = current->next;
        // 注意：这里不释放设备描述符内存，由注册者管理
        current = next;
    }
    device_list_head = NULL;
    
    ESP_LOGI(TAG, "设备注册表反初始化完成");
    return ESP_OK;
}

// 注册设备到链表
esp_err_t device_register(device_descriptor_t* device) {
    if (device == NULL) {
        ESP_LOGE(TAG, "设备注册失败: 设备指针为NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (device->name == NULL) {
        ESP_LOGE(TAG, "设备注册失败: 设备名称为NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (device->init_func == NULL) {
        ESP_LOGE(TAG, "设备注册失败 %s: 初始化函数为NULL", device->name);
        return ESP_ERR_INVALID_ARG;
    }
    
    if (device->priority < DEVICE_PRIORITY_MIN || device->priority > DEVICE_PRIORITY_MAX) {
        ESP_LOGE(TAG, "设备注册失败 %s: 优先级 %d 无效", device->name, device->priority);
        return ESP_ERR_INVALID_ARG;
    }
    
    // 检查设备是否已注册
    if (device_find(device->name) != NULL) {
        ESP_LOGE(TAG, "设备注册失败 %s: 设备已注册", device->name);
        return ESP_ERR_INVALID_STATE;
    }
    
    
    // 将设备添加到链表头部
    device->next = device_list_head;
    device_list_head = device;
    
    ESP_LOGI(TAG, "设备已注册: %s (类型: %d, 优先级: %d)", 
             device->name, device->type, device->priority);
    return ESP_OK;
}

// 从链表取消注册设备
esp_err_t device_unregister(const char* name) {
    if (name == NULL) {
        ESP_LOGE(TAG, "设备取消注册失败: 名称为NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    device_descriptor_t* current = device_list_head;
    device_descriptor_t* prev = NULL;
    
    // 查找要删除的设备
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            // 找到设备，先从链表中移除
            if (prev == NULL) {
                // 删除的是头节点
                device_list_head = current->next;
            } else {
                prev->next = current->next;
            }
            
            // 如果设备已初始化，先反初始化
            if (current->state == DEVICE_STATE_INITIALIZED && current->deinit_func != NULL) {
                esp_err_t err = current->deinit_func();
                if (err != ESP_OK) {
                    ESP_LOGE(TAG, "设备 %s 反初始化失败: %s", name, esp_err_to_name(err));
                }
            }
            
            ESP_LOGI(TAG, "设备已取消注册: %s", name);
            return ESP_OK;
        }
        
        prev = current;
        current = current->next;
    }
    
    ESP_LOGE(TAG, "设备取消注册失败: %s 未找到", name);
    return ESP_ERR_NOT_FOUND;
}

// 查找设备
device_descriptor_t* device_find(const char* name) {
    if (name == NULL) {
        return NULL;
    }
    
    device_descriptor_t* current = device_list_head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

// 按类型查找设备
device_descriptor_t* device_find_by_type(device_type_t type) {
    device_descriptor_t* current = device_list_head;
    while (current != NULL) {
        if (current->type == type) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

// 初始化单个设备
esp_err_t device_initialize(const char* name) {
    device_descriptor_t* device = device_find(name);
    if (device == NULL) {
        ESP_LOGE(TAG, "设备初始化失败: %s 未找到", name);
        return ESP_ERR_NOT_FOUND;
    }
    
    if (device->state == DEVICE_STATE_INITIALIZED) {
        ESP_LOGW(TAG, "设备 %s 已经初始化", name);
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "正在初始化设备: %s", name);
    esp_err_t err = device->init_func();
    if (err != ESP_OK) {
        device->state = DEVICE_STATE_ERROR;
        ESP_LOGE(TAG, "设备 %s 初始化失败: %s", name, esp_err_to_name(err));
        return err;
    }
    
    device->state = DEVICE_STATE_INITIALIZED;
    ESP_LOGI(TAG, "设备 %s 初始化成功", name);
    return ESP_OK;
}

// 反初始化单个设备
esp_err_t device_deinitialize(const char* name) {
    device_descriptor_t* device = device_find(name);
    if (device == NULL) {
        ESP_LOGE(TAG, "设备反初始化失败: %s 未找到", name);
        return ESP_ERR_NOT_FOUND;
    }
    
    if (device->state != DEVICE_STATE_INITIALIZED) {
        ESP_LOGW(TAG, "设备 %s 未初始化", name);
        return ESP_OK;
    }
    
    if (device->deinit_func == NULL) {
        ESP_LOGW(TAG, "设备 %s 没有反初始化函数", name);
        device->state = DEVICE_STATE_UNINITIALIZED;
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "正在反初始化设备: %s", name);
    esp_err_t err = device->deinit_func();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "设备 %s 反初始化失败: %s", name, esp_err_to_name(err));
        return err;
    }
    
    device->state = DEVICE_STATE_UNINITIALIZED;
    ESP_LOGI(TAG, "设备 %s 反初始化成功", name);
    return ESP_OK;
}

// 设备比较函数（用于排序）
static int device_compare(const void* a, const void* b) {
    const device_descriptor_t* device_a = *(const device_descriptor_t**)a;
    const device_descriptor_t* device_b = *(const device_descriptor_t**)b;
    
    // 首先按优先级比较
    if (device_a->priority != device_b->priority) {
        return device_a->priority - device_b->priority;
    }
    
    // 优先级相同，按名称比较（确保稳定排序）
    return strcmp(device_a->name, device_b->name);
}

// 初始化所有设备
esp_err_t devices_initialize_all(void) {
    // 计算设备数量
    int device_count = 0;
    device_descriptor_t* current = device_list_head;
    while (current != NULL) {
        device_count++;
        current = current->next;
    }
    
    if (device_count == 0) {
        ESP_LOGW(TAG, "没有注册任何设备");
        return ESP_OK;
    }
    
    // 创建设备指针数组
    device_descriptor_t** devices = malloc(device_count * sizeof(device_descriptor_t*));
    if (devices == NULL) {
        ESP_LOGE(TAG, "内存分配失败");
        return ESP_ERR_NO_MEM;
    }
    
    // 填充设备指针数组
    current = device_list_head;
    for (int i = 0; i < device_count; i++) {
        devices[i] = current;
        current = current->next;
    }
    
    // 按优先级排序设备
    qsort(devices, device_count, sizeof(device_descriptor_t*), device_compare);
    
    ESP_LOGI(TAG, "开始初始化 %d 个设备...", device_count);
    
    // 按顺序初始化设备
    for (int i = 0; i < device_count; i++) {
        device_descriptor_t* device = devices[i];
        
        if (device->state == DEVICE_STATE_INITIALIZED) {
            ESP_LOGW(TAG, "设备 %s 已经初始化，跳过", device->name);
            continue;
        }
        if(device->state == DEVICE_STATE_PENDING){
            ESP_LOGI(TAG, "设备 %s 处于等待条件状态，跳过", device->name);
            continue;
        }
        ESP_LOGI(TAG, "正在初始化设备: %s (优先级: %d)", device->name, device->priority);
        esp_err_t err = device->init_func();
        if (err != ESP_OK) {
            device->state = DEVICE_STATE_ERROR;
            ESP_LOGE(TAG, "设备 %s 初始化失败: %s", device->name, esp_err_to_name(err));
            free(devices);
            return err;
        }
        
        device->state = DEVICE_STATE_INITIALIZED;
        ESP_LOGI(TAG, "设备 %s 初始化成功", device->name);
    }
    
    free(devices);
    ESP_LOGI(TAG, "所有处于可初始化状态的设备已初始化成功");
    return ESP_OK;
}

// 反初始化所有设备
esp_err_t devices_deinitialize_all(void) {
    // 计算设备数量
    int device_count = 0;
    device_descriptor_t* current = device_list_head;
    while (current != NULL) {
        if (current->state == DEVICE_STATE_INITIALIZED) {
            device_count++;
        }
        current = current->next;
    }
    
    if (device_count == 0) {
        ESP_LOGW(TAG, "没有已初始化的设备");
        return ESP_OK;
    }
    
    // 创建设备指针数组
    device_descriptor_t** devices = malloc(device_count * sizeof(device_descriptor_t*));
    if (devices == NULL) {
        ESP_LOGE(TAG, "内存分配失败");
        return ESP_ERR_NO_MEM;
    }
    
    // 填充已初始化的设备指针数组
    int index = 0;
    current = device_list_head;
    while (current != NULL) {
        if (current->state == DEVICE_STATE_INITIALIZED) {
            devices[index++] = current;
        }
        current = current->next;
    }
    
    // 按优先级逆序排序设备
    qsort(devices, device_count, sizeof(device_descriptor_t*), device_compare);
    
    ESP_LOGI(TAG, "开始反初始化 %d 个设备...", device_count);
    
    // 按逆序反初始化设备
    for (int i = device_count - 1; i >= 0; i--) {
        device_descriptor_t* device = devices[i];
        
        if (device->deinit_func == NULL) {
            ESP_LOGW(TAG, "设备 %s 没有反初始化函数，跳过", device->name);
            device->state = DEVICE_STATE_UNINITIALIZED;
            continue;
        }
        
        ESP_LOGI(TAG, "正在反初始化设备: %s", device->name);
        esp_err_t err = device->deinit_func();
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "设备 %s 反初始化失败: %s", device->name, esp_err_to_name(err));
            free(devices);
            return err;
        }
        
        device->state = DEVICE_STATE_UNINITIALIZED;
        ESP_LOGI(TAG, "设备 %s 反初始化成功", device->name);
    }
    
    free(devices);
    ESP_LOGI(TAG, "所有设备反初始化完成");
    return ESP_OK;
}

// 列出所有设备
void devices_list_all(void) {
    int count = 0;
    device_descriptor_t* current = device_list_head;
    
    ESP_LOGI(TAG, "已注册设备列表:");
    
    while (current != NULL) {
        const char* state_str;
        switch (current->state) {
            case DEVICE_STATE_UNINITIALIZED: state_str = "未初始化"; break;
            case DEVICE_STATE_INITIALIZED: state_str = "已初始化"; break;
            case DEVICE_STATE_ERROR: state_str = "错误"; break;
            case DEVICE_STATE_PENDING: state_str = "等待条件"; break;
            default: state_str = "未知";
        }
        
        ESP_LOGI(TAG, "  %s (类型: %s, 优先级: %d, 状态: %s)", 
                 current->name, type[current->type], current->priority, state_str);
        
        count++;
        current = current->next;
    }
    
    if (count == 0) {
        ESP_LOGI(TAG, "  没有注册任何设备");
    } else {
        ESP_LOGI(TAG, "总共 %d 个设备", count);
    }
}