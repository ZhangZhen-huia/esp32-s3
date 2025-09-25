#pragma once


#include "lvgl.h"
#include <stdbool.h>

// 状态栏高度
#define STATUS_BAR_HEIGHT 30
#define EXPANDED_HEIGHT 200

// 状态栏上下文
typedef struct {
    lv_obj_t *bar;              // 状态栏容器
    lv_obj_t *content;          // 内容容器
    lv_obj_t *time_label;       // 时间显示
    lv_obj_t *battery_label;    // 电量显示
    lv_obj_t *wifi_label;       // Wi-Fi状态
    lv_obj_t *brightness_slider; // 亮度调节
    lv_obj_t *quick_btns[4];    // 快捷按钮
    
    // 状态变量
    bool is_expanded;
    bool is_dragging;
    int start_drag_y;
    int current_height;
    lv_anim_t anim;
    
    // 系统信息
    int battery_level;
    bool wifi_connected;
    char time_str[16];
} status_bar_t;

// // 初始化状态栏
// void status_bar_init(status_bar_t *sb, lv_obj_t *parent);

// // 处理输入事件
// void status_bar_handle_input(status_bar_t *sb, lv_event_t *e);

// // 更新状态信息
// void status_bar_update_info(status_bar_t *sb);

// // 展开/收起状态栏
// void status_bar_expand(status_bar_t *sb, bool expand);

// // 设置亮度回调函数
// void status_bar_set_brightness_cb(status_bar_t *sb, lv_event_cb_t cb);


void app_statusbar_ui_register(void);