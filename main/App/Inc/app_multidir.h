#pragma once 
#include "lvgl.h"
#include <stdbool.h>
#include <stdio.h>
#include "dirent.h"
#include <time.h>
#include <sys/stat.h>   // 主要声明：stat／lstat／fstat 函数和 struct stat
#include <sys/types.h>  // 辅助类型（早期实现需要，可移植性建议保留）
#include "string.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "math.h"
#include <sys/param.h>

// 最大路径长度
#define MAX_PATH_LEN 512
#define MAX_FILENAME_LEN 256
#define MAX_TABS 5
#define MAX_HISTORY 10
#define MAX_BOOKMARKS 10


// 文件信息结构
typedef struct {
    char name[MAX_FILENAME_LEN];
    bool is_dir;
    size_t size;
    time_t modified;
} file_info_t;

// 目录历史记录
typedef struct {
    char path[MAX_PATH_LEN];
    int scroll_pos;
    int selected_index;
} dir_history_t;

// 书签结构
typedef struct {
    char name[MAX_FILENAME_LEN];
    char path[MAX_PATH_LEN];
} bookmark_t;

// 标签页结构
typedef struct {
    char current_path[MAX_PATH_LEN];//当前路径
    file_info_t *files;//文件信息
    int file_count;//文件数量
    int selected_index;
    int scroll_pos;
    
    lv_obj_t *tab;//标签页
    lv_obj_t *file_list;//文件列表
    lv_obj_t *path_label;//路径标签
    
    dir_history_t history[MAX_HISTORY];
    int history_count;
    int history_index;
} tab_t;

// 多目录浏览器上下文
typedef struct {
    tab_t tabs[MAX_TABS];//标签页结构体
    int active_tab;//当前活动的标签页
    int tab_count;//标签页数量
    
    bookmark_t bookmarks[MAX_BOOKMARKS];//书签
    int bookmark_count;//数量
    
    lv_obj_t *tabview;//标签视图
    lv_obj_t *main_screen;//主屏幕
    
    // 样式
    lv_style_t style_bg;
    lv_style_t style_list;
    lv_style_t style_list_btn;
    lv_style_t style_label;
    
    // 状态标志
    bool initialized;
    
} multi_dir_browser_t;
// 按钮标识枚举
typedef enum {
    BUTTON_NEW_TAB,
    BUTTON_BOOKMARKS,
    BUTTON_REFRESH,
    BUTTON_BOOKMARK_ITEM,
    BUTTON_FILE_ITEM,
    BUTTON_BACK
} button_type_t;


typedef struct 
{
    multi_dir_browser_t *browser;
    int index;
}user_data_t;



// 初始化多目录浏览器
void multi_dir_browser_init(multi_dir_browser_t *browser);

// 打开目录
bool multi_dir_browser_open_dir(multi_dir_browser_t *browser, int tab_index, const char *path);

// 创建新标签页
int multi_dir_browser_new_tab(multi_dir_browser_t *browser, const char *path);

// 关闭标签页
bool multi_dir_browser_close_tab(multi_dir_browser_t *browser, int tab_index);

// 刷新当前标签页
void multi_dir_browser_refresh(multi_dir_browser_t *browser);

// 处理UI事件
void multi_dir_browser_handle_event(lv_event_t *e);

// 添加书签
bool multi_dir_browser_add_bookmark(multi_dir_browser_t *browser, const char *name, const char *path);

// 显示书签菜单
void multi_dir_browser_show_bookmarks(multi_dir_browser_t *browser);

