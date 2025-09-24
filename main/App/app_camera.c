#include "Inc/app_camera.h"
#include "Inc/app_ui.h"
#include "Inc/dev_registry.h"
#include "string.h"
#include "stdio.h"

static const char * TAG =  "app_camera";

uint8_t show = 0;
lv_obj_t *ui_camera_page;
lv_obj_t *back_btn;

// static QueueHandle_t xQueueLVGLFrame = NULL;
lv_obj_t *camera_img_show = NULL;
lv_img_dsc_t img_dsc = 
{
    .header.w = 240,
    .header.h = 176,
    .header.cf = LV_COLOR_FORMAT_RGB565,
    .data = NULL,
    .data_size = 240*176*2,
};



static void btn_back_cb(lv_event_t * e)
{
    lv_obj_t *target = lv_event_get_target_obj(e);
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED)
    {
        if(target == back_btn)
        {
            ESP_LOGI(TAG,"back to desktop");
            xEventGroupSetBits(ui_event_group,UI_DESKTOP_BIT);
        }
    }
}

// static void task_process_lcd(void *arg)
// {
//     camera_fb_t *frame = NULL;

//     while (true)
//     {
//         if (xQueueReceive(xQueueLVGLFrame, &frame, portMAX_DELAY))
//         {
            
//            if(show)
//            {
//                 lvgl_port_lock(0);
//                 lv_draw_sw_rgb565_swap(frame->buf,frame->len);
//                 img_dsc.data = frame->buf;
//                 img_dsc.data_size = frame->len;
//                 lv_img_set_src(camera_img, &img_dsc);
//                 lv_obj_invalidate(camera_img);          
//                 lvgl_port_unlock();
//            }
//             // 释放相机帧缓冲区
//             esp_camera_fb_return(frame);
          
//         }
//     }
// }



void app_camera_ui_init(void *) 
{
    show = 1;

    lvgl_port_lock(0);
    

    ui_camera_page = lv_obj_create(lv_scr_act());
    lv_obj_add_style(ui_camera_page,&default_style,LV_PART_MAIN);
    lv_obj_set_style_bg_color(ui_camera_page,lv_color_hex(0x0),0);//背景颜色为黑色
    
   
    back_btn = lv_btn_create(ui_camera_page);
    lv_obj_align(back_btn,LV_ALIGN_TOP_LEFT,0,0);
    lv_obj_set_size(back_btn,60,40);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_TRANSP, LV_PART_MAIN); // 背景透明
    lv_obj_set_style_shadow_opa(back_btn, LV_OPA_TRANSP, LV_PART_MAIN); // 阴影透明
    lv_obj_add_flag(back_btn,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(back_btn,btn_back_cb,LV_EVENT_CLICKED,NULL);

    lv_obj_t *label_back = lv_label_create(back_btn); 
    lv_label_set_text(label_back, LV_SYMBOL_LEFT);  // 按键上显示左箭头符号
    lv_obj_set_style_text_font(label_back, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label_back, lv_color_hex(0xffffff), 0); //白色箭头
    // lv_obj_align(label_back, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_center(label_back);
 


    camera_img_show = lv_img_create(ui_camera_page);
    lv_obj_align(camera_img_show, LV_ALIGN_CENTER, 0, 0);
    
    
    lvgl_port_unlock();


}


void app_camera_ui_deinit(void *) 
{
    lvgl_port_lock(0);
    // device_deinitialize("Camera");
    show = 0;
    // if(ui_camera_page != NULL)
    // {
    //     lv_obj_delete(ui_camera_page);
    //     ui_camera_page = NULL;
    // }
    lvgl_port_unlock();
}



void app_camera_ui_register(void)
{
    ui_config_t *camera = malloc(sizeof(ui_config_t));
    app_ui_create(camera,"CAMERA",UI_CAMERA,ui_camera_page,app_camera_ui_init,app_camera_ui_deinit);
    app_ui_add(camera);
}