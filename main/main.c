/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "lvgl.h"
#include "lv_demos.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "esp_task_wdt.h"

#include "Dev/Inc/dev_registry.h"
#include "App/Inc/app_ui.h"
#include "App/Inc/app_wifi.h"




lv_obj_t *main_animimg_page = NULL;
static const char * TAG = "main";



void app_main_ui_init(void *)
{
    


    

    

    


}


void app_main_ui_deinit(void *)
{
    lvgl_port_lock(0);
    if(main_animimg_page != NULL)
    {
        lv_obj_delete(main_animimg_page);
        main_animimg_page = NULL;
    }

    lvgl_port_unlock();
    ESP_LOGI(TAG,"deinit main ui");
}

void app_main_ui_register(void)
{
    ui_config_t *main = malloc(sizeof(ui_config_t));
    app_ui_create(main,"MAIN",UI_MAIN,main_animimg_page,app_main_ui_init,app_main_ui_deinit);
    app_ui_add(main);

}

void app_main(void)
{

    Device_Init();
    app_ui_Init();
    
   

    while (1)
    {
        
    
        vTaskDelay(10 / portTICK_PERIOD_MS);  // 延时10ms
        // qmi8658_fetch_angleFromAcc(&QMI8658);   // 获取XYZ轴的倾角
        // // 输出XYZ轴的倾角
        // ESP_LOGI(TAG, "angle_x = %.1f  angle_y = %.1f angle_z = %.1f",QMI8658.AngleX, QMI8658.AngleY, QMI8658.AngleZ);

    }
}



