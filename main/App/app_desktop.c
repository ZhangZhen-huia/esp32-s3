#include "Inc/app_desktop.h"
#include "Inc/dev_registry.h"
#include "Inc/app_ui.h"
#include "esp_wifi.h"
#include "stdio.h"
#include "math.h"
#include <errno.h>
static const char *TAG = "app_desktop";

lv_obj_t *desktop_ui_page = NULL;
EventGroupHandle_t s_desktop_event_group;
// EventBits_t desk_bits;

static lv_obj_t *setting_img;
static lv_obj_t *camera_img;
static lv_obj_t *wifi_img;
static lv_obj_t *filesystem_img;
static lv_obj_t *music_img;
// 全局变量



// LV_IMG_DECLARE(img5)
// LV_IMG_DECLARE(img6)
// LV_IMG_DECLARE(img7)
// LV_IMG_DECLARE(img8)
// LV_IMG_DECLARE(img9)
LV_IMG_DECLARE(setting)
LV_IMG_DECLARE(camera)
LV_IMG_DECLARE(wifi)
LV_IMG_DECLARE(filesystem)
LV_IMG_DECLARE(music)
// static const lv_image_dsc_t * anim_imgs[] = {
//     &img5,
//     &img6,
//     &img7,
//     &img8,
//     &img9
// };

lv_image_dsc_t * desktop_anim_imgs[4];

// 定义面板尺寸
#define PANEL_WIDTH (150 * 0.8)
#define PANEL_HEIGHT (150 * 0.7)


void app_desktop_task(void * arg)
{
    


    while(1)
    {
                          
        vTaskDelay(1000/portTICK_PERIOD_MS);

    }

}









static void img_touch_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_current_target(e);

    if(code == LV_EVENT_CLICKED)
    {
        if(obj == wifi_img)
        {
            ESP_LOGI(TAG,"wifi clicked");
            xEventGroupSetBits(ui_event_group,UI_MAIN_BIT);
        }
        else if(obj == setting_img)
        {
            ESP_LOGI(TAG,"setting clicked");
            xEventGroupSetBits(ui_event_group,UI_SETTING_BIT);
        }
        else if(obj == camera_img)
        {
            ESP_LOGI(TAG,"camera clicked");
            xEventGroupSetBits(ui_event_group,UI_CAMERA_BIT);
        }
        else if(obj == filesystem_img)
        {
            ESP_LOGI(TAG,"filesystem clicked");
            xEventGroupSetBits(ui_event_group,UI_FILESYSTEM_BIT);
        }
        else if(obj == music_img)
        {
            ESP_LOGI(TAG,"music clicked");
            xEventGroupSetBits(ui_event_group,UI_MUSIC_BIT);
        }
    }
}

// 初始化桌面UI
void app_desktop_ui_init(void) {


    lvgl_port_lock(0);

    


    // 获取活动屏幕
    desktop_ui_page = lv_obj_create(lv_scr_act());
    lv_obj_add_style(desktop_ui_page, &default_style, 0);

    // 设置屏幕背景
    lv_obj_set_style_bg_color(desktop_ui_page, lv_color_hex(0x0), 0);
    lv_obj_set_style_bg_opa(desktop_ui_page, LV_OPA_COVER, 0);
    

    lv_obj_t * animimg0 = lv_animimg_create(desktop_ui_page);
    
    lv_obj_set_align(animimg0,LV_ALIGN_TOP_MID);
    lv_obj_set_size(animimg0,100,80);
    lv_animimg_set_src(animimg0,(const void**)desktop_anim_imgs, 4);
    lv_animimg_set_duration(animimg0, 500);
    lv_animimg_set_repeat_count(animimg0,LV_ANIM_REPEAT_INFINITE);
    lv_obj_set_align(animimg0,LV_ALIGN_TOP_LEFT);
    lvgl_port_unlock();
    lv_animimg_start(animimg0);


    lvgl_port_lock(0);
    // 创建容器，用于包含两个面板
    lv_obj_t *container = lv_obj_create(desktop_ui_page);
    lv_obj_remove_style_all(container);
    lv_obj_set_size(container,250,150);
    
    
    
    // 设置容器样式
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 0, 0);
    lv_obj_set_style_margin_all(container, 0, 0);
    lv_obj_align(container,LV_ALIGN_BOTTOM_MID,0,-20);


    // 创建setting
    setting_img= lv_img_create(container);
    lv_img_set_src(setting_img,&setting);//设置图片源
    lv_obj_set_size(setting_img, PANEL_WIDTH, PANEL_HEIGHT);
    lv_obj_align(setting_img, LV_ALIGN_CENTER, -PANEL_WIDTH, 0);
    lv_obj_set_style_radius(setting_img, 15, LV_PART_MAIN);
    lv_obj_add_flag(setting_img,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(setting_img,img_touch_cb,LV_EVENT_CLICKED,NULL);


    // 创建camera
    camera_img= lv_img_create(container);
    lv_img_set_src(camera_img,&camera);//设置图片源
    lv_obj_set_size(camera_img, PANEL_WIDTH, PANEL_HEIGHT);
    lv_obj_align(camera_img, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_radius(camera_img, 15, LV_PART_MAIN);
    lv_obj_add_flag(camera_img,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(camera_img,img_touch_cb,LV_EVENT_CLICKED,NULL);




    // 创建wifi
    wifi_img= lv_img_create(container);
    lv_img_set_src(wifi_img,&wifi);//设置图片源
    lv_obj_set_size(wifi_img, PANEL_WIDTH, PANEL_HEIGHT);
    lv_obj_align(wifi_img, LV_ALIGN_CENTER, PANEL_WIDTH, 0);
    lv_obj_set_style_radius(wifi_img, 15, LV_PART_MAIN);
    lv_obj_add_flag(wifi_img,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(wifi_img,img_touch_cb,LV_EVENT_CLICKED,NULL);


    //创建文件系统
    filesystem_img= lv_img_create(container);
    lv_img_set_src(filesystem_img,&filesystem);//设置图片源
    lv_obj_set_size(filesystem_img, PANEL_WIDTH, PANEL_HEIGHT);
    lv_obj_align(filesystem_img, LV_ALIGN_CENTER, PANEL_WIDTH*2, 0);
    lv_obj_set_style_radius(filesystem_img, 15, LV_PART_MAIN);
    lv_obj_add_flag(filesystem_img,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(filesystem_img,img_touch_cb,LV_EVENT_CLICKED,NULL);

        //创建文件系统
    music_img= lv_img_create(container);
    lv_img_set_src(music_img,&music);//设置图片源
    lv_obj_set_size(music_img, PANEL_WIDTH, PANEL_HEIGHT);
    lv_obj_align(music_img, LV_ALIGN_CENTER, PANEL_WIDTH*3, 0);
    lv_obj_set_style_radius(music_img, 15, LV_PART_MAIN);
    lv_obj_add_flag(music_img,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(music_img,img_touch_cb,LV_EVENT_CLICKED,NULL);
    lvgl_port_unlock();


    xTaskCreatePinnedToCore(app_desktop_task,"app_desktop_task",1024*4,NULL,4,NULL,1);
}

void app_desktop_ui_deinit(void *)
{
    lvgl_port_lock(0);
    if(desktop_ui_page != NULL)
    {
        lv_obj_delete(desktop_ui_page);
        desktop_ui_page = NULL;
    }
    lvgl_port_unlock();
    ESP_LOGI(TAG,"deinit desktop ui");
}



void app_desktop_ui_register(void)
{
    ui_config_t *desktop = malloc(sizeof(ui_config_t));
    app_ui_create(desktop,"DESKTOP",UI_DESKTOP,desktop_ui_page,app_desktop_ui_init,app_desktop_ui_deinit);
    app_ui_add(desktop);
}