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

// LV_IMG_DECLARE(img1)
// LV_IMG_DECLARE(img2)
// LV_IMG_DECLARE(img3)
// LV_IMG_DECLARE(img4)


lv_obj_t *main_animimg_page = NULL;
// lv_image_dsc_t * main_anim_imgs[4] = {
//     &img1,
//     &img2,
//     &img3,
//     &img4
// };
lv_image_dsc_t * main_anim_imgs[4];
static const char * TAG = "main";
extern EventGroupHandle_t s_wifi_event_group;


#define TXT  "Wifi Scan..."   // 可换成汉字，每个元素一个字符
lv_obj_t *letter[sizeof(TXT)-1];   // 每个字母一个标签



static void jump_exec(void *var, int32_t v)
{
    /* v: 0→1000→0 缓入-缓出，振幅 8 px，中心对齐 */
    int32_t y = (v - 500) * 6 / 1000;   // -4 ~ +4
    lv_obj_align((lv_obj_t *)var, LV_ALIGN_BOTTOM_MID,
                 lv_obj_get_x_aligned((lv_obj_t *)var), y-10);
}


void app_main_ui_init(void *)
{

    lvgl_port_lock(0);
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_bg_opa( &style, LV_OPA_COVER ); // 背景透明度
    lv_style_set_border_width(&style, 0); // 边框宽度
    lv_style_set_pad_all(&style, 0);  // 内间距
    lv_style_set_radius(&style, 0);   // 圆角半径
    lv_style_set_width(&style, 320);  // 宽
    lv_style_set_height(&style, 240); // 高

    main_animimg_page = lv_obj_create(lv_scr_act());
    lv_obj_add_style(main_animimg_page, &style, 0);

    //跳动字体
    for (int i = 0; i < sizeof(TXT)-1; ++i) {
        char buf[2] = {TXT[i], '\0'};
        letter[i] = lv_label_create(main_animimg_page);
        lv_label_set_text(letter[i], buf);
        lv_obj_set_style_text_font(letter[i], &lv_font_montserrat_18, 0);
        lv_obj_align(letter[i], LV_ALIGN_BOTTOM_MID, (i - 5) * 18, 0);  // 水平排布,竖直跳动，跳动基值在回调里面写
    }

        for (int i = 0; i < sizeof(TXT)-1; ++i) {
        lv_anim_t *jump = lv_malloc(sizeof(lv_anim_t));
        lv_anim_init(jump);
        lv_anim_set_var(jump, letter[i]);
        lv_anim_set_values(jump, 0, 1000);
        lv_anim_set_duration(jump, 1200);                    // 单次 400 ms
        lv_anim_set_delay(jump, i * 140);                // 依次延迟 80 ms
        lv_anim_set_repeat_count(jump, LV_ANIM_REPEAT_INFINITE);
        lv_anim_set_exec_cb(jump, jump_exec);
        lv_anim_set_path_cb(jump, lv_anim_path_ease_in_out );
        lv_anim_start(jump);
        }

    lv_obj_t * animimg0 = lv_animimg_create(main_animimg_page);
    
    lv_obj_set_align(animimg0,LV_ALIGN_TOP_MID);
    lv_obj_set_size(animimg0,200,200);
    lv_animimg_set_src(animimg0, (const void **)main_anim_imgs, 4);
    lv_animimg_set_duration(animimg0, 500);
    lv_animimg_set_repeat_count(animimg0,LV_ANIM_REPEAT_INFINITE);
    lvgl_port_unlock();
    lv_animimg_start(animimg0);

    
    //开启扫描,阻塞扫描
    esp_wifi_scan_start(NULL, true);
    
    
    EventBits_t wifi_bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_SCAN_DOWN_BIT,
            pdTRUE,
            pdFALSE,
            portMAX_DELAY);
            if(wifi_bits & WIFI_SCAN_DOWN_BIT)
            {
                xEventGroupSetBits(ui_event_group,UI_WIFI_BIT);
            }
    


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



