#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"

#include "esp_log.h"

#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_netif.h"

#include "esp_event.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "Inc/wifi.h"
#include "Inc/camera.h"
#include "Dev/Inc/dev_registry.h"

static const char *TAG = "wifi station";

static int s_retry_num = 0;


static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    //wifi启动成功
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        //开始wifi连接
        esp_wifi_connect();
    } 
    //wifi断开连接
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP: %d",s_retry_num);
        }
        else
            ESP_LOGI(TAG,"connect to the AP fail");
    }
    //基站连接到热点成功，但此时还没有获取到热点的ip
    else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
    {
        wifi_event_sta_connected_t *event = (wifi_event_sta_connected_t *)event_data;
        ESP_LOGI(TAG, "Connected to SSID: %s", event->ssid);
        ESP_LOGI(TAG, "BSSID: %02x:%02x:%02x:%02x:%02x:%02x",
                 event->bssid[0], event->bssid[1], event->bssid[2],
                 event->bssid[3], event->bssid[4], event->bssid[5]);
        ESP_LOGI(TAG, "Channel: %d", event->channel);
    }
    //设备获取IP成功
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        //打印设备分配到的ip地址
        ESP_LOGI(TAG, "got sta ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;

    }
}


static esp_err_t wifi_sta_init(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    

    //初始化tcp/ip底层代码
    ESP_ERROR_CHECK(esp_netif_init());

    //创建默认事件循环
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    //创建wifi客户端接口
    esp_netif_create_default_wifi_sta();

    //初始化wifi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
     ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    //注册事件处理函数（回调函数）到默认事件循环
    //事件源（如WIFI_EVENT、IP_EVENT），事件ID，事件处理函数，传入处理函数的参数，回调处理函数的注册句柄
    //同一个处理函数可以多次注册为不同的事件源
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    //配置wifi
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,  //目标wifi ssid
            .password = EXAMPLE_ESP_WIFI_PASS,//目标wifi密码
        },
    };
    //设置wifi模式，配置
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );

    //开启wifi
    return esp_wifi_start();

}

// void register_wifi_device(void)
// {
//     static device_descriptor_t wifi = 
//     {
//         .name = "Wifi",
//         .init_func = wifi_sta_init,
//         .deinit_func = NULL,
//         .next = NULL,
//         .priority = 6,
//         .state = DEVICE_STATE_PENDING,
//         .type = WIFI,
//     };

//    device_register(&wifi);
// }