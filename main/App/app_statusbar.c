#include "Inc/app_statusbar.h"
#include "Inc/app_ui.h"
#include "Inc/app_desktop.h"
#include <string.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "lvgl.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// static status_bar_t status_bar;
lv_obj_t *statusbar_ui_page;
// // static lv_obj_t *main_screen;

// // // 亮度调节回调
// // static void brightness_changed_cb(lv_event_t *e) {
// //     lv_obj_t *slider = lv_event_get_target(e);
// //     int brightness = lv_slider_get_value(slider);
// //     ESP_LOGI("Main", "亮度调整为: %d%%", brightness);
    
// //     // 这里可以添加实际的亮度控制代码
// //     // 例如：设置背光PWM等
// // }

// // // 快捷按钮回调
// // static void quick_btn_cb(lv_event_t *e) {
// //     lv_obj_t *btn = lv_event_get_target(e);
// //     ESP_LOGI("Main", "快捷按钮被点击");
    
// //     // 根据按钮执行相应操作
// //     // 可以在这里添加Wi-Fi、蓝牙等功能的实现
// // }


// // // 时间更新任务
// // static void time_update_task(void *arg) {
// //     while (1) {
// //         // 获取当前时间（这里需要实际的时间获取逻辑）
// //         time_t now;
// //         struct tm timeinfo;
// //         time(&now);
// //         localtime_r(&now, &timeinfo);
        
// //         char time_str[16];
// //         strftime(time_str, sizeof(time_str), "%H:%M", &timeinfo);
        
// //         // 更新状态栏时间
// //         status_bar_set_time(&status_bar, time_str);
        
// //         vTaskDelay(pdMS_TO_TICKS(1000)); // 每秒更新一次
// //     }
// // }

// // void app_main(void) {

// //     // 初始化状态栏
// //     status_bar_init(&status_bar, main_screen);
    
// //     // 设置亮度回调
// //     status_bar_set_brightness_cb(&status_bar, brightness_changed_cb);
    
// //     // 设置初始状态
// //     status_bar_set_battery(&status_bar, 85);
// //     status_bar_set_wifi(&status_bar, true);
    
// //     // 创建时间更新任务
// //     xTaskCreate(time_update_task, "time_update", 2048, NULL, 1, NULL);
    
// //     // 主循环
// //     while (1) {
// //         lv_task_handler();
// //         vTaskDelay(pdMS_TO_TICKS(10));
// //     }
// // }

// // // 在您的事件处理函数中集成状态栏
// // void handle_touch_event(lv_event_t *e) {
// //     status_bar_handle_input(&status_bar, e);
// // }



// // 动画回调函数
// static void set_height_anim(void *obj, int32_t value) {
//     status_bar_t *sb = (status_bar_t *)obj;
//     lv_obj_set_height(sb->bar, value);
//     sb->current_height = value;
// }

// // 动画完成回调
// static void anim_ready_cb(lv_anim_t *a) {
//     status_bar_t *sb = (status_bar_t *)a->user_data;
//     sb->is_expanded = (sb->current_height == EXPANDED_HEIGHT);
    
//     // 更新内容可见性
//     if (sb->is_expanded) {
//         lv_obj_clear_flag(sb->content, LV_OBJ_FLAG_HIDDEN);
//     } else if (sb->current_height <= STATUS_BAR_HEIGHT) {
//         lv_obj_add_flag(sb->content, LV_OBJ_FLAG_HIDDEN);
//     }
// }

// // 初始化状态栏
// void ui_status_bar_init(void *) {

//     statusbar_ui_page = lv_obj_create(lv_scr_act());
//     lv_obj_add_style(statusbar_ui_page,&default_style,0);
    
//     // 创建状态栏容器
//     sb->bar = lv_obj_create(parent);
//     lv_obj_set_size(sb->bar, LV_PCT(100), STATUS_BAR_HEIGHT);
//     lv_obj_align(sb->bar, LV_ALIGN_TOP_MID, 0, 0);
    
//     // 设置样式
//     lv_obj_set_style_bg_color(sb->bar, lv_color_hex(0x1a1a1a), 0);
//     lv_obj_set_style_bg_opa(sb->bar, LV_OPA_90, 0);
//     lv_obj_set_style_border_width(sb->bar, 0, 0);
//     lv_obj_set_style_radius(sb->bar, 0, 0);
//     lv_obj_set_style_pad_all(sb->bar, 5, 0);
    
//     // 创建内容容器（初始隐藏）
//     sb->content = lv_obj_create(sb->bar);
//     lv_obj_set_size(sb->content, LV_PCT(100), LV_PCT(100));
//     lv_obj_align(sb->content, LV_ALIGN_TOP_MID, 0, STATUS_BAR_HEIGHT);
//     lv_obj_set_style_bg_opa(sb->content, LV_OPA_0, 0);
//     lv_obj_set_style_border_width(sb->content, 0, 0);
//     lv_obj_add_flag(sb->content, LV_OBJ_FLAG_HIDDEN);
    
//     // 创建时间显示（在状态栏顶部）
//     sb->time_label = lv_label_create(sb->bar);
//     lv_label_set_text(sb->time_label, "12:00");
//     lv_obj_set_style_text_color(sb->time_label, lv_color_white(), 0);
//     lv_obj_set_style_text_font(sb->time_label, &lv_font_montserrat_14, 0);
//     lv_obj_align(sb->time_label, LV_ALIGN_TOP_LEFT, 10, 8);
    
//     // 创建电池显示
//     sb->battery_label = lv_label_create(sb->bar);
//     lv_label_set_text(sb->battery_label, "100%");
//     lv_obj_set_style_text_color(sb->battery_label, lv_color_white(), 0);
//     lv_obj_set_style_text_font(sb->battery_label, &lv_font_montserrat_14, 0);
//     lv_obj_align(sb->battery_label, LV_ALIGN_TOP_RIGHT, -10, 8);
    
//     // 创建Wi-Fi状态
//     sb->wifi_label = lv_label_create(sb->bar);
//     lv_label_set_text(sb->wifi_label, LV_SYMBOL_WIFI);
//     lv_obj_set_style_text_color(sb->wifi_label, lv_color_white(), 0);
//     lv_obj_set_style_text_font(sb->wifi_label, &lv_font_montserrat_14, 0);
//     lv_obj_align(sb->wifi_label, LV_ALIGN_TOP_RIGHT, -50, 8);
    
//     // 创建扩展内容
//     create_expanded_content(sb);
    
//     // 初始化状态
//     sb->current_height = STATUS_BAR_HEIGHT;
//     sb->battery_level = 100;
//     sb->wifi_connected = false;
//     strcpy(sb->time_str, "12:00");
    
//     // 更新初始状态
//     status_bar_update_info(sb);
// }

// // 创建扩展内容
// static void create_expanded_content(status_bar_t *sb) {
//     // 日期显示
//     lv_obj_t *date_label = lv_label_create(sb->content);
//     lv_label_set_text(date_label, "2023年11月15日 星期三");
//     lv_obj_set_style_text_color(date_label, lv_color_white(), 0);
//     lv_obj_set_style_text_font(date_label, &lv_font_montserrat_16, 0);
//     lv_obj_align(date_label, LV_ALIGN_TOP_LEFT, 15, 15);
    
//     // 亮度调节
//     lv_obj_t *brightness_label = lv_label_create(sb->content);
//     lv_label_set_text(brightness_label, "亮度");
//     lv_obj_set_style_text_color(brightness_label, lv_color_white(), 0);
//     lv_obj_align(brightness_label, LV_ALIGN_TOP_LEFT, 15, 50);
    
//     sb->brightness_slider = lv_slider_create(sb->content);
//     lv_slider_set_value(sb->brightness_slider, 80, LV_ANIM_OFF);
//     lv_obj_set_size(sb->brightness_slider, 150, 10);
//     lv_obj_align(sb->brightness_slider, LV_ALIGN_TOP_LEFT, 70, 55);
//     lv_obj_set_style_bg_color(sb->brightness_slider, lv_color_hex(0x444444), LV_PART_MAIN);
//     lv_obj_set_style_bg_color(sb->brightness_slider, lv_color_white(), LV_PART_INDICATOR);
//     lv_obj_set_style_bg_color(sb->brightness_slider, lv_color_white(), LV_PART_KNOB);
    
//     // 快捷按钮
//     const char *btn_icons[] = {LV_SYMBOL_WIFI, LV_SYMBOL_BLUETOOTH, LV_SYMBOL_AUDIO, LV_SYMBOL_SETTINGS};
//     const char *btn_texts[] = {"Wi-Fi", "蓝牙", "声音", "设置"};
    
//     for (int i = 0; i < 4; i++) {
//         lv_obj_t *btn = lv_btn_create(sb->content);
//         lv_obj_set_size(btn, 70, 70);
//         lv_obj_set_style_bg_color(btn, lv_color_hex(0x333333), 0);
//         lv_obj_set_style_radius(btn, 10, 0);
        
//         // 计算位置
//         int x_pos = 15 + (i % 2) * 85;
//         int y_pos = 90 + (i / 2) * 85;
//         lv_obj_align(btn, LV_ALIGN_TOP_LEFT, x_pos, y_pos);
        
//         // 创建垂直布局容器
//         lv_obj_t *cont = lv_obj_create(btn);
//         lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
//         lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
//         lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
//         lv_obj_set_style_bg_opa(cont, LV_OPA_0, 0);
//         lv_obj_set_style_border_width(cont, 0, 0);
        
//         // 图标
//         lv_obj_t *icon = lv_label_create(cont);
//         lv_label_set_text(icon, btn_icons[i]);
//         lv_obj_set_style_text_color(icon, lv_color_white(), 0);
//         lv_obj_set_style_text_font(icon, &lv_font_montserrat_20, 0);
        
//         // 文本
//         lv_obj_t *label = lv_label_create(cont);
//         lv_label_set_text(label, btn_texts[i]);
//         lv_obj_set_style_text_color(label, lv_color_white(), 0);
//         lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
        
//         sb->quick_btns[i] = btn;
//     }
// }

// // 处理输入事件
// void status_bar_handle_input(status_bar_t *sb, lv_event_t *e) {
//     lv_event_code_t code = lv_event_get_code(e);
//     lv_indev_t *indev = lv_event_get_indev(e);
    
//     if (code == LV_EVENT_PRESSED) {
//         lv_point_t point;
//         lv_indev_get_point(indev, &point);
        
//         // 检查是否在状态栏区域按下
//         if (point.y <= STATUS_BAR_HEIGHT * 2) { // 扩大触摸区域以便更容易触发
//             sb->is_dragging = true;
//             sb->start_drag_y = point.y;
//         }
//     }
//     else if (code == LV_EVENT_PRESSING && sb->is_dragging) {
//         lv_point_t point;
//         lv_indev_get_point(indev, &point);
        
//         int delta_y = point.y - sb->start_drag_y;
        
//         // 限制拖动范围
//         int new_height = STATUS_BAR_HEIGHT + delta_y;
//         if (new_height < STATUS_BAR_HEIGHT) new_height = STATUS_BAR_HEIGHT;
//         if (new_height > EXPANDED_HEIGHT) new_height = EXPANDED_HEIGHT;
        
//         // 实时更新高度
//         lv_obj_set_height(sb->bar, new_height);
//         sb->current_height = new_height;
        
//         // 根据高度调整内容透明度
//         int opacity = (new_height - STATUS_BAR_HEIGHT) * 255 / (EXPANDED_HEIGHT - STATUS_BAR_HEIGHT);
//         if (opacity < 0) opacity = 0;
//         if (opacity > 255) opacity = 255;
        
//         if (new_height > STATUS_BAR_HEIGHT + 20) {
//             lv_obj_clear_flag(sb->content, LV_OBJ_FLAG_HIDDEN);
//             lv_obj_set_style_bg_opa(sb->content, opacity, 0);
//         } else {
//             lv_obj_add_flag(sb->content, LV_OBJ_FLAG_HIDDEN);
//         }
//     }
//     else if (code == LV_EVENT_RELEASED && sb->is_dragging) {
//         sb->is_dragging = false;
        
//         // 根据最终位置决定展开或收起
//         if (sb->current_height > EXPANDED_HEIGHT * 0.6) {
//             status_bar_expand(sb, true);
//         } else {
//             status_bar_expand(sb, false);
//         }
//     }
// }

// // 展开/收起状态栏
// void status_bar_expand(status_bar_t *sb, bool expand) {
//     if (sb->is_dragging) return;
    
//     int target_height = expand ? EXPANDED_HEIGHT : STATUS_BAR_HEIGHT;
    
//     // 停止当前动画
//     lv_anim_del(sb, set_height_anim);
    
//     // 创建新动画
//     lv_anim_init(&sb->anim);
//     lv_anim_set_var(&sb->anim, sb);
//     lv_anim_set_exec_cb(&sb->anim, set_height_anim);
//     lv_anim_set_values(&sb->anim, sb->current_height, target_height);
//     lv_anim_set_time(&sb->anim, 300);
//     lv_anim_set_path_cb(&sb->anim, lv_anim_path_ease_out);
//     lv_anim_set_ready_cb(&sb->anim, anim_ready_cb);
//     lv_anim_set_user_data(&sb->anim, sb);
//     lv_anim_start(&sb->anim);
// }

// // 更新状态信息
// void status_bar_update_info(status_bar_t *sb) {
//     // 更新时间显示（这里需要实际的时间获取逻辑）
//     lv_label_set_text(sb->time_label, sb->time_str);
    
//     // 更新电池显示
//     char battery_text[16];
//     snprintf(battery_text, sizeof(battery_text), "%d%%", sb->battery_level);
//     lv_label_set_text(sb->battery_label, battery_text);
    
//     // 更新Wi-Fi状态
//     lv_label_set_text(sb->wifi_label, sb->wifi_connected ? LV_SYMBOL_WIFI : LV_SYMBOL_WARNING);
//     lv_obj_set_style_text_color(sb->wifi_label, 
//         sb->wifi_connected ? lv_color_white() : lv_color_hex(0xff5555), 0);
// }

// // 设置亮度回调函数
// void status_bar_set_brightness_cb(status_bar_t *sb, lv_event_cb_t cb) {
//     if (sb->brightness_slider && cb) {
//         lv_obj_add_event_cb(sb->brightness_slider, cb, LV_EVENT_VALUE_CHANGED, NULL);
//     }
// }

// // 设置时间
// void status_bar_set_time(status_bar_t *sb, const char *time_str) {
//     if (time_str) {
//         strlcpy(sb->time_str, time_str, sizeof(sb->time_str));
//         status_bar_update_info(sb);
//     }
// }

// // 设置电池电量
// void status_bar_set_battery(status_bar_t *sb, int level) {
//     sb->battery_level = level;
//     if (sb->battery_level < 0) sb->battery_level = 0;
//     if (sb->battery_level > 100) sb->battery_level = 100;
//     status_bar_update_info(sb);
// }

// // 设置Wi-Fi状态
// void status_bar_set_wifi(status_bar_t *sb, bool connected) {
//     sb->wifi_connected = connected;
//     status_bar_update_info(sb);
// }

void app_statusbar_ui_init(void *)
{

}

void app_statusbar_ui_deinit(void *)
{

}


void app_statusbar_ui_register(void)
{
    ui_config_t *statusbar = malloc(sizeof(ui_config_t));
    app_ui_create(statusbar,"STATUSBAR",UI_STATUSBAR,statusbar_ui_page,app_statusbar_ui_init,app_statusbar_ui_deinit);
    app_ui_add(statusbar);
    app_ui_link(UP,statusbar,UI_DESKTOP);
}



