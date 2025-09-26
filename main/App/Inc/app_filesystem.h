#pragma once

#include "Inc/app_ui.h"
#include "Inc/sdcard.h"


// 最大路径长度
#define MAX_PATH_LEN 512
#define MAX_FILENAME_LEN 256
#define MAX_TABS 5

#define FILE_READ_BUF_SIZE 337

// 文件信息结构
typedef struct {
    char *name;
    FILE *file;
    bool is_dir;
    size_t size;
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

// 目录标签页结构
typedef struct {
    file_iterator_instance_t *iterator;//文件迭代器
    char current_path[MAX_PATH_LEN];//当前目录路径
    file_info_t *files;//文件信息
    lv_obj_t *tab;//标签页
    lv_obj_t *file_list;//文件列表
    lv_obj_t *path_label;//路径标签
} tab_t;


typedef struct 
{
    lv_obj_t *main_screen;//主屏幕
    lv_obj_t *tabview;//标签主视图
    tab_t tabs[MAX_TABS];//标签页结构体,有几个显示目录的页面，可以有多个页面同时显示目录（目前多个目录页面没有什么作用），所以只有一个即tabs[0]

    int active_tab;//当前活动的标签页
    int tab_count;//标签页数量

    // 样式
    lv_style_t style_bg;
    lv_style_t style_list;
    lv_style_t style_list_btn;
    lv_style_t style_label;

 // 状态标志
    bool initialized;
}multi_dir_browser_t;


// 按钮标识枚举
typedef enum {
    BUTTON_NEW_TAB = 'a',
    BUTTON_BOOKMARKS = 'b',
    BUTTON_REFRESH = 'c',
    BUTTON_BOOKMARK_ITEM = 'd',
    BUTTON_FILE_ITEM = 'e',
    BUTTON_BACK = 'f'
} button_type_t;

// 文件阅读器结构
typedef struct {
    char file_path[MAX_PATH_LEN];//文件路径
    FILE *file_handle;//文件句柄
    int page_num;
    int now_page_num;
    // UI元素
    lv_obj_t *read_screen;//呈现文字的屏幕
    lv_obj_t *text_area;//文本区域
    lv_obj_t *status_label;
    lv_obj_t *back_btn;//返回
    lv_obj_t *prev_btn;//上一页
    lv_obj_t *next_btn;//下一页
    lv_obj_t *img;//图片
    bool is_reading;
} file_reader_t;

typedef struct 
{
    int file_index;
    int tab_index;
}user_data_t;

void app_filesystem_ui_register(void);
bool load_all_sd_imgs(void);