#include "Inc/app_setting.h"
#include "Inc/app_ui.h"
#include "stdio.h"
#include "math.h"

static const char *TAG = "app_setting";

lv_obj_t *setting_ui_page = NULL;
lv_obj_t *setting_list = NULL;
static lv_obj_t *btn_back_to_desktop = NULL;
static lv_obj_t * btn_brigntness;

lv_obj_t *adjust_page;
void app_setting_task(void * arg)
{
    


    while(1)
    {


        
        // desk_bits = xEventGroupWaitBits(s_desktop_event_group,
        //                                 UI_WIFI_BIT|UI_SETTING_BIT|UI_CAMERA_BIT,
        //                                 pdTRUE,
        //                                 pdFALSE,
        //                                 portMAX_DELAY);          

        // if(desk_bits & UI_WIFI_BIT) 
        // {
        //     app_ui_display_by_id(UI_MAIN);
        // }                               
        vTaskDelay(10/portTICK_PERIOD_MS);

    }

}



extern int bsp_display_brightness_get(void);
extern esp_err_t bsp_display_brightness_set(int brightness_percent);

static void brightness_cb(lv_event_t * e)
{

    lv_obj_t *target = lv_event_get_target_obj(e);

    bsp_display_brightness_set(lv_slider_get_value(target));
    
}

static void back_to_settinglist_cb(lv_event_t * e)
{
    lv_obj_delete(adjust_page);   
}

static void list_btn_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target_obj(e);

    adjust_page = lv_obj_create(setting_ui_page);
    lv_obj_set_size(adjust_page,320,240);
    lv_obj_center(adjust_page);
    lv_obj_set_style_bg_color(adjust_page,lv_color_white(),LV_PART_MAIN);

    lv_obj_t *btn_back_to_setting_list = lv_btn_create(adjust_page);
    lv_obj_set_size(btn_back_to_setting_list,40,30);
    lv_obj_align(btn_back_to_setting_list,LV_ALIGN_TOP_LEFT,0,0);
    lv_obj_set_style_bg_opa(btn_back_to_setting_list,LV_OPA_0,LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(btn_back_to_setting_list, LV_OPA_TRANSP, LV_PART_MAIN); // 阴影透明
    lv_obj_add_event_cb(btn_back_to_setting_list,back_to_settinglist_cb,LV_EVENT_CLICKED,NULL);

    lv_obj_t *label_back_to_setting_list = lv_label_create(btn_back_to_setting_list);
    lv_obj_align(label_back_to_setting_list,LV_ALIGN_CENTER,0,0);
    lv_label_set_text(label_back_to_setting_list,LV_SYMBOL_LEFT);
    lv_obj_set_style_text_font(label_back_to_setting_list, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label_back_to_setting_list, lv_color_hex(0x0), 0); //黑色箭头

    if(code == LV_EVENT_CLICKED)
    {
        if(target == btn_back_to_desktop)
        {
            xEventGroupSetBits(ui_event_group,UI_DESKTOP_BIT);
        }
        else if(target == btn_brigntness)
        {
            lv_obj_t *slider = lv_slider_create(adjust_page);
            lv_obj_set_style_bg_opa(slider,LV_OPA_0,LV_PART_MAIN);
            lv_obj_set_style_bg_color(slider,lv_color_hex(0x0),LV_PART_KNOB);
            lv_obj_center(slider);
            lv_obj_set_size(slider,280,10);
            lv_slider_set_value(slider,bsp_display_brightness_get(),LV_ANIM_OFF);
            lv_slider_set_range(slider,0,100);
            lv_obj_add_event_cb(slider,brightness_cb,LV_EVENT_VALUE_CHANGED,NULL);
        }

    }
}


// 初始化桌面UI
void app_setting_ui_init(void) {


    lvgl_port_lock(0);


    // 获取活动屏幕
    setting_ui_page = lv_obj_create(lv_scr_act());
    lv_obj_add_style(setting_ui_page, &default_style, LV_PART_MAIN);

    // 设置屏幕背景
    lv_obj_set_style_bg_color(setting_ui_page, lv_color_hex(0x0), 0);
    lv_obj_set_style_bg_opa(setting_ui_page, LV_OPA_COVER, 0);

    setting_list = lv_list_create(setting_ui_page);
    lv_obj_set_size(setting_list,320,200);
    lv_obj_align(setting_list,LV_ALIGN_BOTTOM_MID,0,0);
    lv_obj_set_style_text_font(setting_list,&lv_font_montserrat_22,LV_PART_MAIN);
    lv_obj_set_style_bg_color(setting_list, lv_color_hex(0xffffff),0);
    lv_obj_set_scrollbar_mode(setting_list, LV_SCROLLBAR_MODE_OFF); // 隐藏setting_list滚动条

    btn_back_to_desktop = lv_btn_create(setting_ui_page);
    lv_obj_set_size(btn_back_to_desktop,40,30);
    lv_obj_align(btn_back_to_desktop,LV_ALIGN_TOP_LEFT,0,0);
    lv_obj_set_style_bg_opa(btn_back_to_desktop,LV_OPA_TRANSP,LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(btn_back_to_desktop, LV_OPA_TRANSP, LV_PART_MAIN); // 阴影透明
    lv_obj_add_event_cb(btn_back_to_desktop,list_btn_cb,LV_EVENT_CLICKED,NULL);

    lv_obj_t *label_back_to_desktop = lv_label_create(btn_back_to_desktop);
    lv_obj_align(label_back_to_desktop,LV_ALIGN_CENTER,0,0);
    lv_label_set_text(label_back_to_desktop,LV_SYMBOL_LEFT);
    lv_obj_set_style_text_font(label_back_to_desktop, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label_back_to_desktop, lv_color_hex(0xffffff), 0); //白色箭头

    btn_brigntness = lv_list_add_button(setting_list,NULL,"Brightness");
    lv_obj_add_event_cb(btn_brigntness, list_btn_cb, LV_EVENT_CLICKED, NULL); // 添加点击回调函数



    



    

   

    lvgl_port_unlock();



    xTaskCreatePinnedToCore(app_setting_task,"app_setting_task",1024*4,NULL,4,NULL,1);
}

void app_setting_ui_deinit(void *)
{
    lvgl_port_lock(0);
    if(setting_ui_page != NULL)
    {
        lv_obj_delete(setting_ui_page);
        setting_ui_page = NULL;
    }
    lvgl_port_unlock();
    ESP_LOGI(TAG,"deinit setting ui");
}



void app_setting_ui_register(void)
{
    ui_config_t *setting = malloc(sizeof(ui_config_t));
    app_ui_create(setting,"SETTING",UI_SETTING,setting_ui_page,app_setting_ui_init,app_setting_ui_deinit);
    app_ui_add(setting);
}


