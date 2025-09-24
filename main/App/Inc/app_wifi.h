#pragma once

#include "app_ui.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_netif.h"


// wifi事件
#define WIFI_CONNECTED_BIT    BIT0
#define WIFI_FAIL_BIT         BIT1
#define WIFI_START_BIT        BIT2
#define WIFI_SCAN_DOWN_BIT    BIT3

// wifi最大重连次数
#define EXAMPLE_ESP_MAXIMUM_RETRY  3
//wifi扫描列表最大数目
#define DEFAULT_SCAN_LIST_SIZE 5


void register_wifi_device(void);

void app_wifi_ui_register(void);
void app_wifi_ui_init(void *);
void app_wifi_ui_deinit(void *);
