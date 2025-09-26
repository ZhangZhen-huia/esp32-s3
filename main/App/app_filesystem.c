#include "Inc/app_filesystem.h"
#include <errno.h> 
#include <dirent.h>
#include <stddef.h>
#include <time.h>
#include <sys/stat.h>   
#include <sys/types.h>  
#include "user_config.h"



const static char *TAG = "filesystem";
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"

lv_obj_t *filesystem_ui_page;
multi_dir_browser_t *multi_dir_control;
file_reader_t *file_reader;
user_data_t *user_data;

static lv_img_dsc_t file_img_dsc = 
{
    .header.w = 240,
    .header.h = 176,
    .header.cf = LV_COLOR_FORMAT_RGB565,
    .data = NULL,
    .data_size = 240*176*2,
};

static lv_image_dsc_t *sd_dsc[8] = {0};
extern lv_image_dsc_t * main_anim_imgs[4];
extern lv_image_dsc_t * desktop_anim_imgs[4];


int multi_dir_browser_new_tab(multi_dir_browser_t *browser, const char *path);
bool multi_dir_browser_open_dir(multi_dir_browser_t *browser, int tab_index, const char *path);
static void file_reader_create_ui(file_reader_t *fr, multi_dir_browser_t *browser);
static void file_reader_handle_event(lv_event_t *e, multi_dir_browser_t *browser);
bool is_text_file(const char *filename);
bool is_img_file(const char *filename);
static void file_reader_close(file_reader_t *fr);
static error_t file_reader_open_file(file_reader_t *fr, multi_dir_browser_t *browser, const char *path);
static void file_reader_display_prevpage(file_reader_t *fr);




/* 加载一张 SD 卡图片到 PSRAM */
static bool load_one_img(const char *path, uint8_t idx)
{
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        ESP_LOGE(TAG, "open %s fail", path);
        return false;
    }
    lv_image_header_t head;
    fread(&head, sizeof(head), 1, fp);
    size_t px_sz = head.w * head.h * 2;

    uint8_t *px = heap_caps_malloc(px_sz, MALLOC_CAP_SPIRAM);
    if (!px) { fclose(fp); return false; }
    fread(px, 1, px_sz, fp);
    fclose(fp);

    sd_dsc[idx] = heap_caps_malloc(sizeof(lv_image_dsc_t), MALLOC_CAP_SPIRAM);
    *sd_dsc[idx] = (lv_image_dsc_t){
        .header = head,
        .data   = px,
        .data_size = px_sz,
    };
    return true;
}


/* 一次性加载 n 张 */
bool load_all_sd_imgs(void)
{
    const char *file[8] = {
        "/sdcard/gif/img1.bin",
        "/sdcard/gif/img2.bin",
        "/sdcard/gif/img3.bin",
        "/sdcard/gif/img4.bin",
        "/sdcard/gif/img6.bin",
        "/sdcard/gif/img7.bin",
        "/sdcard/gif/img8.bin",
        "/sdcard/gif/img9.bin"
    };
    for (int i = 0; i < 8; i++) {
        if (!load_one_img(file[i], i)) return false;
        if(i<4)
            main_anim_imgs[i] = sd_dsc[i];
        else
            desktop_anim_imgs[i-4] = sd_dsc[i];

    }
    ESP_LOGI(TAG,"load finish");
    return true;
}


// 初始化样式
static void init_styles(multi_dir_browser_t *browser) {
    // 背景样式
    lv_style_init(&browser->style_bg);
    lv_style_set_bg_opa( &browser->style_bg, LV_OPA_COVER ); // 背景透明度
    lv_style_set_border_width(&browser->style_bg, 0); // 边框宽度
    lv_style_set_pad_all(&browser->style_bg, 0);  // 内间距
    lv_style_set_radius(&browser->style_bg, 0);   // 圆角半径
    lv_style_set_width(&browser->style_bg, 320);  // 宽
    lv_style_set_height(&browser->style_bg, 240); // 高
    

    //列表样式
    lv_style_init(&browser->style_list);
    lv_style_set_bg_color(&browser->style_list, lv_color_hex(0xffffff));
    #if USING_CHINESE
    lv_style_set_text_font(&browser->style_list,&font_alipuhui18);
    #else
    lv_style_set_text_font(&browser->style_list,&lv_font_montserrat_18);
    #endif
    lv_style_set_size(&browser->style_list,320,200);
    lv_style_set_border_width(&browser->style_list, 0);
    lv_style_set_radius(&browser->style_list, 0);
    lv_style_set_pad_all(&browser->style_list, 0);
    lv_style_set_pad_gap(&browser->style_list, 0);



    
    // 列表按钮样式            
    lv_style_init(&browser->style_list_btn);
    lv_style_set_bg_opa(&browser->style_list_btn, 255);
    lv_style_set_bg_color(&browser->style_list_btn,lv_color_hex(0x0));
    lv_style_set_text_color(&browser->style_list_btn,lv_color_hex(0xffffff));
    #if USING_CHINESE
    lv_style_set_text_font(&browser->style_list_btn,&font_alipuhui18);
    #else
    lv_style_set_text_font(&browser->style_list_btn,&lv_font_montserrat_18);
    #endif
    lv_style_set_border_width(&browser->style_list_btn, 0);
    lv_style_set_radius(&browser->style_list_btn, 0);
    
    // 标签样式
    lv_style_init(&browser->style_label);
    lv_style_set_text_color(&browser->style_label, lv_color_white());
    #if USING_CHINESE
    lv_style_set_text_font(&browser->style_label,&font_alipuhui18);
    #else
    lv_style_set_text_font(&browser->style_label,&lv_font_montserrat_18);
    #endif
}



static void show_pic_cb(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    user_data_t *user_data = lv_event_get_user_data(e);
    fclose(file_reader->file_handle);
    free((void*)file_img_dsc.data);
    file_img_dsc.data = NULL;
    if (code == LV_EVENT_CLICKED) {

        // 返回按钮
        if (target == file_reader->next_btn) {

            if(user_data->file_index == multi_dir_control->tabs[user_data->tab_index].iterator->count-1)
                user_data->file_index = 0;
            else
                user_data->file_index += 1;
        }
        // 上一页按钮
        else if (target == file_reader->prev_btn) {
            if(user_data->file_index == 0)
                user_data->file_index = multi_dir_control->tabs[user_data->tab_index].iterator->count-1;
            else
                user_data->file_index -= 1;
        }
    }
    
    if(is_img_file(multi_dir_control->tabs[user_data->tab_index].files[user_data->file_index].name))
    {
       char path[256] = {0};
       file_iterator_get_full_path_from_index(multi_dir_control->tabs[user_data->tab_index].iterator, user_data->file_index, path, sizeof(path));
       file_reader->file_handle = fopen(path, "rb");
       if (file_reader->file_handle)
       {
        ESP_LOGI(TAG, "File selected: %s", path);
        uint8_t *img_buf = heap_caps_malloc(240 * 176 * 2, MALLOC_CAP_SPIRAM);
        if (img_buf) {
           lvgl_port_lock(0);
           size_t rd = fread(img_buf, 1, 240 * 176 * 2, file_reader->file_handle);
           if (rd == 240 * 176 * 2) {
               file_img_dsc.data = img_buf;
               lv_img_set_src(file_reader->img, &file_img_dsc);
               lv_obj_invalidate(file_reader->img);
           } else {
               free(img_buf);
               file_reader_close(file_reader);
               ESP_LOGE(TAG, "Failed to read image data");
           }
           lvgl_port_unlock();
           file_reader->is_reading = true;
         }
        }
    }
        
        
    
}        
// 处理UI事件
// 修改事件处理函数，添加滚动位置保存
static void multi_dir_browser_handle_event(lv_event_t *e) {
    lv_obj_t *target = lv_event_get_target(e);
    multi_dir_browser_t *browser = lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);
    
        // 首先检查是否是文件阅读器的事件
    if (file_reader->is_reading) {
        file_reader_handle_event(e, browser);
        return;
    }

    if (code == LV_EVENT_CLICKED) {
        // 获取按钮类型
        button_type_t button_type = (button_type_t)(intptr_t)lv_obj_get_user_data(target);
        
        switch (button_type) {
            case BUTTON_NEW_TAB:
                // multi_dir_browser_new_tab(browser, NULL);
                break;
                
            case BUTTON_BOOKMARKS:
                // multi_dir_browser_show_bookmarks(browser);
                break;
                
            case BUTTON_REFRESH:
                // 保存当前滚动位置
                // save_scroll_position(browser, browser->active_tab);
                // multi_dir_browser_refresh(browser);
                break;
                
            default:
                // 处理文件列表中的按钮
                for (int i = 0; i < browser->tab_count; i++) {
                    //获取到文件所在的文件列表
                    if (lv_obj_get_parent(target) == browser->tabs[i].file_list) {
                        //获取文件索引
                        intptr_t index = (intptr_t)lv_obj_get_user_data(target);
                        
                        //返回上级目录按钮
                        if (index == -1) {

                            char *last_slash = strrchr(browser->tabs[i].current_path, '/');
                            //存在多级目录，并且不是字符串首字符（防止把根目录 / 也砍掉）
                            if (last_slash && last_slash != browser->tabs[i].current_path) {
                                
                                //通过指针操作把当前目录的最后一级目录置空
                                *last_slash = '\0';
                                //打开置空之后的目录，也就是上级目录
                                /* 退出/返回上级时 */

                                multi_dir_browser_open_dir(browser, i, browser->tabs[i].current_path);
                            }
                            //当前目录不是根目录
                            else if (strcmp(browser->tabs[i].current_path, "/sdcard") != 0) {
                                multi_dir_browser_open_dir(browser, i, "/sdcard");
                            }
                        }
                        //是文件索引
                        // 在 multi_dir_browser_handle_event 函数中修改文件打开部分
                        else if (index >= 0 && index < browser->tabs[i].iterator->count) {
                            
                            char new_path[MAX_PATH_LEN];
                            snprintf(new_path, sizeof(new_path), "%s/%s", 
                                    browser->tabs[i].current_path, 
                                    browser->tabs[i].files[index].name);
                            
                            //如果是目录
                            if (browser->tabs[i].files[index].is_dir) {
                                // 打开目录
                                multi_dir_browser_open_dir(browser, i, new_path);
                            } else {
                                ESP_LOGI(TAG, "File selected: %s", new_path);
                                
                                // 检查文件类型
                                if(is_img_file(browser->tabs[i].files[index].name)) {
                                    // 图片文件处理
                                    char path[256] = {0};
                                    file_iterator_get_full_path_from_index(browser->tabs[i].iterator, index, path, sizeof(path));
                                    file_reader->file_handle = fopen(path, "rb");
                                    user_data_t *user_data  = malloc(sizeof(user_data_t));
                                    user_data->file_index = index;
                                    user_data->tab_index = i;

                                    if (file_reader->file_handle) {
                                        file_reader_create_ui(file_reader, browser);
                                        lv_obj_add_flag(file_reader->status_label,LV_OBJ_FLAG_HIDDEN);
                                        file_reader->img = lv_img_create(file_reader->text_area);
                                        lv_obj_set_size(file_reader->img, 240, 176);
                                        lv_obj_align(file_reader->img, LV_ALIGN_CENTER, 0, 0);
                                        lv_obj_add_event_cb(file_reader->next_btn,show_pic_cb,LV_EVENT_CLICKED,user_data);
                                        lv_obj_add_event_cb(file_reader->prev_btn,show_pic_cb,LV_EVENT_CLICKED,user_data);
                                        uint8_t *img_buf = heap_caps_malloc(240 * 176 * 2, MALLOC_CAP_SPIRAM);
                                        if (img_buf) {
                                            lvgl_port_lock(0);
                                            size_t rd = fread(img_buf, 1, 240 * 176 * 2, file_reader->file_handle);
                                            if (rd == 240 * 176 * 2) {
                                                file_img_dsc.data = img_buf;
                                                lv_img_set_src(file_reader->img, &file_img_dsc);
                                                lv_obj_invalidate(file_reader->img);
                                            } else {
                                                free(img_buf);
                                                file_reader_close(file_reader);
                                                ESP_LOGE(TAG, "Failed to read image data");
                                            }
                                            lvgl_port_unlock();
                                            file_reader->is_reading = true;
                                        }
                                    }
                                }
                                // 检查是否为文本文件
                                else if (is_text_file(browser->tabs[i].files[index].name)) {
                                        file_reader->is_reading = true;
                                    // 确保阅读器UI已创建
                                    if (!file_reader->read_screen) {
                                        file_reader_create_ui(file_reader, browser);
                                        lv_obj_add_event_cb(file_reader->next_btn,multi_dir_browser_handle_event,LV_EVENT_CLICKED,NULL);
                                        lv_obj_add_event_cb(file_reader->prev_btn,multi_dir_browser_handle_event,LV_EVENT_CLICKED,NULL);
                                    }
                                    // 打开文本文件
                                    file_reader_open_file(file_reader, browser, new_path);
   
                                    
                                } else {
                                    // 不支持的文件类型
                                    if (!file_reader->read_screen) {
                                        file_reader_create_ui(file_reader, browser);
                                        lv_obj_add_flag(file_reader->status_label,LV_OBJ_FLAG_HIDDEN);
                                    }
                                    lv_textarea_set_text(file_reader->text_area, 
                                        "不支持的文件类型");
                                    file_reader->is_reading = true;
                                }
                            }
                        }
                        break;
                    }
                }
                break;
        }
        
    }
    
    // 标签页切换事件 - 保存当前标签页的滚动位置
    if (code == LV_EVENT_VALUE_CHANGED && target == browser->tabview) {
        browser->active_tab = lv_tabview_get_tab_act(browser->tabview);
    }
}

void btn_close_cb(lv_event_t * e)
{
    ESP_LOGI(TAG,"btn_close clicked");
    xEventGroupSetBits(ui_event_group,UI_DESKTOP_BIT);
}


// 加载文件内容
static bool file_reader_load_file(file_reader_t *fr, const char *path) {
    FILE *file = fopen(path, "r");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open file: %s, error: %s", path, strerror(errno));
        return false;
    }
    // 关闭之前打开的文件
    if (fr->file_handle) {
        fclose(fr->file_handle);
        fr->file_handle = NULL;
    }
    fr->file_handle = file;
    fr->page_num = 0;
    fr->now_page_num = 0;
    strlcpy(fr->file_path, path, sizeof(fr->file_path));
    
    //遍历计算要显示的页数
    char *buf  =malloc(FILE_READ_BUF_SIZE);
    size_t page_count = 0;
    if(fr->file_handle)
    {
        //读取固定的块然后返回
        while(fread(buf, 1, FILE_READ_BUF_SIZE, fr->file_handle))
        {
            page_count++;
        }

    }
    fr->page_num = page_count;
    fr->now_page_num = 0;
    rewind(fr->file_handle);
    ESP_LOGI(TAG, "Loaded file: %s, total pages: %d", path, fr->page_num);
    free(buf);
    return true;
}

// 显示当前页的内容
static void file_reader_display_nextpage(file_reader_t *fr) {
    if (fr->page_num == 0) {
        lv_textarea_set_text(fr->text_area, "file is empty or read failed");
        ESP_LOGW(TAG, "No lines to display");
        return;
    }
    fr->now_page_num ++;
    long pos = ftell(fr->file_handle);      // 当前位置
    ESP_LOGI(TAG,"读取之前位置 %ld",pos);    

    char *buf  =malloc(FILE_READ_BUF_SIZE+4);
    if(fr->file_handle)
    {
        if (fread(buf, 1, FILE_READ_BUF_SIZE, fr->file_handle) != NULL) {
            // 判断最后一个utf编码是否读取完整
            //UTF8规定，首字节的高两位是字符的开始，并且携带长度信息
            //而&0xc0就是把高两位取出来，如果最高2位是10，那说明是UTF字符的后续字节，那就是字符不完整
            if((buf[FILE_READ_BUF_SIZE - 1] & 0xc0) == 0x80) {
                // 不完整, 重新读取
                int i = FILE_READ_BUF_SIZE;
                while(fread(buf + i, 1, 1, fr->file_handle) == 1) {
                    //如果最后一个字节是首字节，表示上一个字符读取完整
                    if((buf[i] & 0xc0) != 0x80) {
                        // 读取到了一个完整的utf编码
                        buf[i] = 0;
                        fseek(fr->file_handle, -1, SEEK_CUR); // 回退一个字节
                        break;
                    }
                    i++;
                }
            }
        }    
    }
    //设置一页
    lv_textarea_set_text(fr->text_area, buf);
    lv_textarea_set_cursor_pos(fr->text_area, 0);
    lv_obj_scroll_to_y(fr->text_area, 0, LV_ANIM_OFF);

    char status_text[50];
    pos = ftell(fr->file_handle);      // 当前位置
    ESP_LOGI(TAG,"读取之后位置 %ld",pos);    

    snprintf(status_text, sizeof(status_text), "%d/%d\npage",fr->now_page_num, fr->page_num);
    lv_label_set_text(fr->status_label, status_text);
    
    // 更新按钮状态
    lv_obj_clear_state(fr->prev_btn, LV_STATE_DISABLED);
    lv_obj_clear_flag(fr->prev_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_state(fr->next_btn, LV_STATE_DISABLED);
    lv_obj_clear_flag(fr->next_btn, LV_OBJ_FLAG_HIDDEN);
    if (fr->now_page_num <= 1) {
        lv_obj_add_state(fr->prev_btn, LV_STATE_DISABLED);
        lv_obj_add_flag(fr->prev_btn,LV_OBJ_FLAG_HIDDEN);
    }
    
    if (fr->now_page_num >= fr->page_num) {
        lv_obj_add_state(fr->next_btn, LV_STATE_DISABLED);
        lv_obj_add_flag(fr->next_btn,LV_OBJ_FLAG_HIDDEN);
    }
}

static void file_reader_display_prevpage(file_reader_t *fr) {
    if (fr->page_num == 0) {
        lv_textarea_set_text(fr->text_area, "file is empty or read failed");
        ESP_LOGW(TAG, "No lines to display");
        return;
    }
    fr->now_page_num--;
    char *buf  =malloc(FILE_READ_BUF_SIZE+4);
    long pos = ftell(fr->file_handle);      // 当前位置
    ESP_LOGI(TAG,"当前位置 %ld",pos);
    if((pos - 2*FILE_READ_BUF_SIZE) <0 )
        fseek(fr->file_handle,0, SEEK_SET); 
    else 
        {
            fseek(fr->file_handle,pos - 2*FILE_READ_BUF_SIZE, SEEK_SET); // 前移 N
            ESP_LOGI(TAG,"前移动到 %ld 位置",pos - 2*FILE_READ_BUF_SIZE);
        }
      

    if(fr->file_handle)
    {
        if (fread(buf, 1, FILE_READ_BUF_SIZE, fr->file_handle) != NULL) {
            // 判断最后一个utf编码是否读取完整
            if((buf[FILE_READ_BUF_SIZE - 1] & 0xc0) == 0x80) {
                // 不完整, 重新读取(UTF-8 的第一个字节的高位为1, 说明这个字节是一个utf编码的开始)
                int i = FILE_READ_BUF_SIZE;
                while(fread(buf + i, 1, 1, fr->file_handle) == 1) {
                    if((buf[i] & 0xc0) != 0x80) {
                        // 读取到了一个完整的utf编码
                        buf[i] = 0;
                        fseek(fr->file_handle, -1, SEEK_CUR); // 回退一个字节
                        break;
                    }
                    i++;
                }
            }
        }    
    }
    //设置一页
    lv_textarea_set_text(fr->text_area, buf);
    lv_textarea_set_cursor_pos(fr->text_area, 0);
    lv_obj_scroll_to_y(fr->text_area, 0, LV_ANIM_OFF);

    char status_text[50];

    snprintf(status_text, sizeof(status_text), "%d/%d\npage",fr->now_page_num, fr->page_num);
    lv_label_set_text(fr->status_label, status_text);
    pos = ftell(fr->file_handle);      // 当前位置
    ESP_LOGI(TAG,"读取之后位置 %ld",pos);

    // 更新按钮状态
    lv_obj_clear_state(fr->prev_btn, LV_STATE_DISABLED);
    lv_obj_clear_flag(fr->prev_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_state(fr->next_btn, LV_STATE_DISABLED);
    lv_obj_clear_flag(fr->next_btn, LV_OBJ_FLAG_HIDDEN);
    if (fr->now_page_num <= 1) {
        lv_obj_add_state(fr->prev_btn, LV_STATE_DISABLED);
        lv_obj_add_flag(fr->prev_btn,LV_OBJ_FLAG_HIDDEN);
    }
    
    if (fr->now_page_num >= fr->page_num) {
        lv_obj_add_state(fr->next_btn, LV_STATE_DISABLED);
        lv_obj_add_flag(fr->next_btn,LV_OBJ_FLAG_HIDDEN);
    }
}


// 创建文件阅读界面
static void file_reader_create_ui(file_reader_t *fr, multi_dir_browser_t *browser) {
    
    // 创建阅读界面
    fr->read_screen = lv_obj_create(browser->main_screen);
    lv_obj_add_style(fr->read_screen,&default_style,0);
    lv_obj_set_style_bg_color(fr->read_screen, lv_color_hex(0x0), 0);
    lv_obj_set_size(fr->read_screen, LV_PCT(100), LV_PCT(100));
    
    // 创建底部栏
    lv_obj_t *top_bar = lv_obj_create(fr->read_screen);
    lv_obj_set_size(top_bar, LV_PCT(100), 40);
    lv_obj_align(top_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(top_bar, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_border_width(top_bar, 0, 0);
    lv_obj_set_scrollbar_mode(top_bar,LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_border_width(top_bar,0,0);

    // 创建返回按钮
    fr->back_btn = lv_btn_create(top_bar);
    lv_obj_set_size(fr->back_btn, 80, 30);
    lv_obj_set_style_bg_opa(fr->back_btn,LV_OPA_0,0);
    lv_obj_set_style_shadow_opa(fr->back_btn,LV_OPA_0,0);
    lv_obj_set_style_border_opa(fr->back_btn,LV_OPA_0,0);
    lv_obj_align(fr->back_btn, LV_ALIGN_CENTER,0, 0);
    lv_obj_add_event_cb(fr->back_btn,multi_dir_browser_handle_event,LV_EVENT_CLICKED,NULL);

    lv_obj_t *back_label = lv_label_create(fr->back_btn);
    lv_label_set_text(back_label,"Back");
    lv_obj_set_align(back_label,LV_ALIGN_CENTER);
    lv_obj_set_style_text_font(back_label,&lv_font_montserrat_20,0);
    lv_obj_set_style_text_color(back_label, lv_color_black(), 0);
    
        
    // 创建上一页按钮
    fr->prev_btn = lv_btn_create(top_bar);
    lv_obj_set_size(fr->prev_btn, 80, 30);
    lv_obj_set_style_bg_opa(fr->prev_btn,LV_OPA_0,0);
    lv_obj_set_style_shadow_opa(fr->prev_btn,LV_OPA_0,0);
    lv_obj_set_style_border_opa(fr->prev_btn,LV_OPA_0,0);
    lv_obj_align(fr->prev_btn, LV_ALIGN_LEFT_MID,0, 0);
    // lv_obj_add_event_cb(fr->prev_btn,multi_dir_browser_handle_event,LV_EVENT_CLICKED,NULL);

    lv_obj_t *prev_label = lv_label_create(fr->prev_btn);
    lv_label_set_text(prev_label,"Prev");
    lv_obj_set_style_text_font(prev_label,&lv_font_montserrat_20,0);
    lv_obj_set_style_text_color(prev_label, lv_color_black(), 0);
    lv_obj_set_align(prev_label,LV_ALIGN_CENTER);
   
    // 创建下一页按钮
    fr->next_btn = lv_btn_create(top_bar);
    lv_obj_set_size(fr->next_btn, 80, 30);
    lv_obj_set_style_bg_opa(fr->next_btn,LV_OPA_0,0);
    lv_obj_set_style_shadow_opa(fr->next_btn,LV_OPA_0,0);
    lv_obj_set_style_border_opa(fr->next_btn,LV_OPA_0,0);
    lv_obj_align(fr->next_btn, LV_ALIGN_RIGHT_MID,0, 0);
    // lv_obj_add_event_cb(fr->next_btn,multi_dir_browser_handle_event,LV_EVENT_CLICKED,NULL);

    lv_obj_t *next_label = lv_label_create(fr->next_btn);
    lv_label_set_text(next_label,"Next");
    lv_obj_set_style_text_font(next_label,&lv_font_montserrat_20,0);
    lv_obj_set_style_text_color(next_label, lv_color_black(), 0);
    lv_obj_set_align(next_label,LV_ALIGN_CENTER);

    // 创建状态标签
    fr->status_label = lv_label_create(top_bar);
    lv_obj_set_style_text_color(fr->status_label, lv_color_hex(0xf80000), 0);
    lv_obj_set_style_text_font(fr->status_label, &lv_font_montserrat_14, 0);
    lv_obj_align(fr->status_label, LV_ALIGN_CENTER, 60, 0);

    // // 创建文本区域
    fr->text_area = lv_textarea_create(fr->read_screen);
    lv_obj_set_size(fr->text_area, 320, 195);
    lv_obj_align(fr->text_area, LV_ALIGN_BOTTOM_MID,0,-45);
    lv_textarea_set_text(fr->text_area, "");
    lv_obj_set_style_text_color(fr->text_area, lv_color_black(), 0);
    lv_obj_set_style_border_width(fr->text_area,0,0);
    lv_obj_set_style_bg_color(fr->text_area, lv_color_hex(0xffffff), 0);
    lv_textarea_set_align(fr->text_area, LV_TEXT_ALIGN_LEFT);
    // lv_textarea_set_accepted_chars(fr->text_area, NULL);//字符限制，限制为第二个输入的参数，如果为NULL就不限制
    // lv_textarea_set_one_line(fr->text_area, false);//设置为多行输入模式，true单行，即按回车就发送，false 多行，回车是换行
    lv_textarea_set_password_mode(fr->text_area, false);
    lv_textarea_set_cursor_click_pos(fr->text_area, false);
    lv_obj_clear_flag(fr->text_area, LV_OBJ_FLAG_CLICKABLE);
    lv_textarea_set_cursor_pos(fr->text_area, 0);
    lv_obj_scroll_to_y(fr->text_area, 0, LV_ANIM_OFF);
    lv_obj_set_scrollbar_mode(fr->text_area,LV_SCROLLBAR_MODE_OFF);
    // 设置文本区域的字体
    static lv_style_t text_style;
    lv_style_init(&text_style);
    #if USING_CHINESE
    lv_style_set_text_font(&text_style,&font_alipuhui18);
    #else
    lv_style_set_text_font(&text_style,&lv_font_montserrat_18);
    #endif
    // lv_style_set_text_line_space(&text_style, 4);//设置行间距，单行模式无效
    lv_obj_add_style(fr->text_area, &text_style, 0);

        

}

// 打开文件阅读器
static error_t file_reader_open_file(file_reader_t *fr, multi_dir_browser_t *browser, const char *path) {
    
    ESP_LOGI(TAG, "Attempting to open file: %s", path);
    if (!file_reader_load_file(fr, path)) {
        // 显示错误信息
        lv_textarea_set_text(fr->text_area, "open failed !");
        ESP_LOGE(TAG, "Failed to load file: %s", path);
        return ESP_FAIL;
    }
    // 显示一页
    file_reader_display_nextpage(fr);
    ESP_LOGI(TAG, "File reader opened successfully: %s", path);
    return ESP_OK;
}

//文件阅读器关闭函数
static void file_reader_close(file_reader_t *fr) {
    ESP_LOGI(TAG, "Closing file reader");
    
    if (fr->file_handle) {
        fclose(fr->file_handle);
        fr->file_handle = NULL;
    }
    
    // 清理图片数据
    if (file_img_dsc.data) {
        free((void*)file_img_dsc.data);
        file_img_dsc.data = NULL;
    }
    if(user_data)
    {
        free(user_data);
        user_data = NULL;
    }
    fr->now_page_num = 0;
    fr->page_num = 0;
    fr->is_reading = false;
    fr->file_path[0] = '\0';
}

//处理文件阅读器事件
static void file_reader_handle_event(lv_event_t *e, multi_dir_browser_t *browser) {
    lv_obj_t *target = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED) {
        // 返回按钮
        if (target == file_reader->back_btn) {

            lv_obj_delete(file_reader->read_screen);
            file_reader->read_screen = NULL;
            file_reader_close(file_reader);
            return;
        }
        
        // 上一页按钮
        if (target == file_reader->prev_btn) {
            if (file_reader->now_page_num > 0) {
                file_reader_display_prevpage(file_reader);
            }
            return;
        }
        
        // 下一页按钮
        if (target == file_reader->next_btn) {
            if (file_reader->now_page_num < file_reader->page_num) {
                file_reader_display_nextpage(file_reader);
            }
            return;
        }
    }
}

// 检查文件是否为可读的文本文件
bool is_text_file(const char *filename) {
    const char *text_extensions[] = {
        ".txt", ".c", ".h", ".cpp", ".hpp", ".md", 
        ".log", ".ini", ".conf", ".json", ".xml", ".html",
        ".css", ".js", ".py", ".java", ".php", ".sh",
        ".bat", ".cfg", ".yml", ".yaml", ".md", ".rst",
        ".bin",
        NULL
    };
    
    const char *dot = strrchr(filename, '.');
    if (!dot) return false;
    
    for (int i = 0; text_extensions[i] != NULL; i++) {
        if (strcasecmp(dot, text_extensions[i]) == 0) {
            ESP_LOGI(TAG,"open %s file",text_extensions[i]);
            return true;
        }
    }
    return false;
}

bool is_img_file(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if (!dot) return false;
        if (strcasecmp(dot, ".pic") == 0) {
            return true;
        }
    return false;
}

void app_filesystem_ui_init(void)
{
    file_reader = heap_caps_calloc(1, sizeof(file_reader_t),MALLOC_CAP_DEFAULT);
    memset(file_reader, 0, sizeof(file_reader_t));
    
    
    multi_dir_control = heap_caps_calloc(1, sizeof(multi_dir_browser_t),MALLOC_CAP_DEFAULT);
    lvgl_port_lock(0);
    init_styles(multi_dir_control);
    filesystem_ui_page = lv_obj_create(lv_scr_act());
    lv_obj_add_style(filesystem_ui_page, &default_style, LV_PART_MAIN);
    // 设置屏幕背景
    lv_obj_set_style_bg_color(filesystem_ui_page, lv_color_hex(0x0), 0);
    lv_obj_set_style_bg_opa(filesystem_ui_page, LV_OPA_COVER, 0);

    multi_dir_control->main_screen = lv_obj_create(filesystem_ui_page);
    lv_obj_add_style(multi_dir_control->main_screen,&multi_dir_control->style_bg,0);
    lv_obj_set_style_bg_color(multi_dir_control->main_screen,lv_color_hex(0x0),0);
    lv_obj_set_scrollbar_mode(multi_dir_control->main_screen, LV_SCROLLBAR_MODE_OFF); // 隐藏setting_list滚动条


    //创建标签视图
    multi_dir_control->tabview = lv_tabview_create(multi_dir_control->main_screen);
    lv_obj_set_size(multi_dir_control->tabview, 320, 240);
    lv_tabview_set_tab_bar_size(multi_dir_control->tabview,40);
    lv_obj_set_scrollbar_mode(multi_dir_control->tabview, LV_SCROLLBAR_MODE_OFF); // 隐藏setting_list滚动条

    // 添加默认标签页
    multi_dir_control->tab_count = 1;
    multi_dir_control->active_tab = 0;

    // 初始化第一个目录标签页
    multi_dir_control->tabs[0].tab = lv_tabview_add_tab(multi_dir_control->tabview, "Home");
    lv_obj_set_scrollbar_mode(multi_dir_control->tabs[0].tab, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_size(multi_dir_control->tabs[0].tab,320,240);
    lv_obj_set_style_pad_all(multi_dir_control->tabs[0].tab,0,0);
    lv_obj_set_style_pad_gap(multi_dir_control->tabs[0].tab, 0, 0);

    // 创建第一个目录路径标签
    multi_dir_control->tabs[0].path_label = lv_label_create(multi_dir_control->tabs[0].tab);
    lv_obj_add_style(multi_dir_control->tabs[0].path_label, &multi_dir_control->style_label, 0);
    lv_label_set_text(multi_dir_control->tabs[0].path_label, "/sdcard");
    lv_obj_align(multi_dir_control->tabs[0].path_label, LV_ALIGN_TOP_LEFT, 0, 0);
    
    // 创建文件列表
    multi_dir_control->tabs[0].file_list = lv_list_create(multi_dir_control->tabs[0].tab);
    lv_obj_add_style(multi_dir_control->tabs[0].file_list,&multi_dir_control->style_list,0);
    lv_obj_set_size(multi_dir_control->tabs[0].file_list, LV_PCT(100), LV_PCT(100));
    lv_obj_align(multi_dir_control->tabs[0].file_list, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_scrollbar_mode(multi_dir_control->tabs[0].file_list, LV_SCROLLBAR_MODE_OFF); // 隐藏setting_list滚动条
    lv_obj_set_style_pad_right(multi_dir_control->tabs[0].file_list, 0, LV_PART_SCROLLBAR);   /* 右侧预留 */
    lv_obj_set_style_pad_left(multi_dir_control->tabs[0].file_list, 0, LV_PART_SCROLLBAR);    /* 左侧预留 */
    


    //返回桌面按钮
    lv_obj_t * btn_bar = lv_tabview_get_tab_bar(multi_dir_control->tabview);
    lv_obj_set_style_bg_opa(btn_bar, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_bar, 0, 0);
    lv_obj_set_style_shadow_width(btn_bar, 0, 0);
    lv_obj_t * close_btn = lv_btn_create(btn_bar);
    lv_obj_set_size(close_btn, 40, 40);
    lv_obj_align(close_btn, LV_DIR_RIGHT, 0, 0);
    lv_obj_set_style_bg_opa(close_btn, LV_OPA_TRANSP, 0);     
    lv_obj_set_style_border_width(close_btn, 0, 0);          
    lv_obj_set_style_shadow_width(close_btn, 0, 0);         
    lv_obj_add_event_cb(close_btn, btn_close_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * lbl = lv_label_create(close_btn);
    lv_label_set_text(lbl, LV_SYMBOL_CLOSE);
    lv_obj_set_style_text_font(lbl,&lv_font_montserrat_18,0);
    lv_obj_set_align(lbl,LV_ALIGN_CENTER);
    lv_obj_set_style_text_color(lbl, lv_color_black(), 0);   /* 纯黑 */
    lv_obj_center(lbl);
    

    // 添加控制按钮容器
    lv_obj_t *btn_container = lv_obj_create(multi_dir_control->main_screen);
    lv_obj_remove_style_all(btn_container);
    lv_obj_set_size(btn_container, LV_PCT(95), 40);
    lv_obj_align(btn_container, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    

    // 初始化默认目录
    strlcpy(multi_dir_control->tabs[0].current_path, "/sdcard", sizeof(multi_dir_control->tabs[0].current_path));

    multi_dir_browser_open_dir(multi_dir_control, 0, "/sdcard");
   
    multi_dir_control->initialized = true;

    lvgl_port_unlock();
}


// 修改打开目录函数，添加滚动位置保存和恢复
bool multi_dir_browser_open_dir(multi_dir_browser_t *browser, int tab_index, const char *path) {
    if (tab_index < 0 || tab_index >= browser->tab_count) {
        return false;
    }

    tab_t *tab = &browser->tabs[tab_index];

    //每次进来之前先free上一次的iterator
    file_iterator_delete(tab->iterator);
    tab->iterator = file_iterator_new(path);
    


    // 清空当前文件列表
    if (tab->files) {
        free(tab->files);
        tab->files = NULL;
    }
    
    // 清空UI列表
    lv_obj_clean(tab->file_list);

    tab->files = malloc(tab->iterator->count * sizeof(file_info_t));
    
    if (!tab->files) {
        ESP_LOGI(TAG,"Line 454 malloc failed");
        return false;
    }
    // 更新当前路径
    strlcpy(tab->current_path, path, sizeof(tab->current_path));
    lv_label_set_text(tab->path_label, path);


       
    
    
    // 更新标签页标题
    char *last_slash = strrchr(path, '/');
    if (last_slash && strlen(last_slash) > 1) {
        lv_tabview_rename_tab(browser->tabview, tab_index, last_slash + 1);
    } else {
        lv_tabview_rename_tab(browser->tabview, tab_index, "Root");
    }
    char full_path[MAX_PATH_LEN];
    for(size_t i = 0; i<tab->iterator->count; i++)
    {
        tab->files[i].name = file_iterator_get_name_from_index(tab->iterator, i);
        if ((strcasecmp(tab->files[i].name, ".") == 0) || (strcasecmp(tab->files[i].name, "..") == 0))
        {
            tab->files[i].is_dir = true;
            continue;
        }
        
        snprintf(full_path, sizeof(full_path), "%s/%s", path, tab->files[i].name);
        
        // 获取文件信息
        struct stat st;
        if (stat(full_path, &st) == 0) {

            tab->files[i].is_dir = S_ISDIR(st.st_mode);
            tab->files[i].size = st.st_size;
        }
    }

    // 添加返回上级目录按钮（如果不是根目录）
    if (strcmp(path, "/sdcard") != 0 && strcmp(path, "/") != 0) {
        
        lv_obj_t *btn = lv_list_add_btn(tab->file_list, LV_SYMBOL_LEFT, "..");
        lv_obj_set_style_bg_opa(btn, 255,0);
        lv_obj_set_style_bg_color(btn,lv_color_hex(0x0),0);
        lv_obj_set_style_text_color(btn,lv_color_hex(0xffffff),0);
        lv_obj_set_style_border_width(btn,0,0);
        lv_obj_set_style_radius(btn,0,0);
        lv_obj_set_style_text_font(btn,&lv_font_montserrat_18,0);

        lv_obj_set_user_data(btn, (void *)(intptr_t)-1);
        lv_obj_add_event_cb(btn, multi_dir_browser_handle_event, LV_EVENT_CLICKED, browser);
    }

    
    // 添加文件和目录按钮
    for (int i = 0; i < tab->iterator->count; i++) {
        if ((strcasecmp(tab->files[i].name, "system Volume Information") == 0) || (strcasecmp(tab->files[i].name, "FOUND.000") == 0)) continue;
        const char *icon = tab->files[i].is_dir ? "\uf07b" : "\uf15b";
        char btn_text[128];
        char size_text[32];

        if(!tab->files[i].is_dir)
        {
            if (tab->files[i].size < 1024) {
                snprintf(size_text, sizeof(size_text), "%dB", tab->files[i].size);
            } else if (tab->files[i].size < 1024 * 1024) {
                snprintf(size_text, sizeof(size_text), "%.1fKB", tab->files[i].size / 1024.0);
            } else {
                snprintf(size_text, sizeof(size_text), "%.1fMB", tab->files[i].size / (1024.0 * 1024.0));
            }
        }
        
        
        
        snprintf(btn_text, sizeof(btn_text), " %s %s", tab->files[i].name, size_text);
        lv_obj_t *btn = lv_list_add_btn(tab->file_list, icon, btn_text);
        lv_obj_set_size(btn,LV_PCT(100),40);
        lv_obj_add_style(btn, &browser->style_list_btn, 0);
        lv_obj_set_user_data(btn, (void *)(intptr_t)i);
        lv_obj_add_event_cb(btn, multi_dir_browser_handle_event, LV_EVENT_CLICKED, browser);
    }
    
    return true;
}


// 初始化桌面UI
void app_filesystem_ui_deinit(void) 
{
    lvgl_port_lock(0);
    free(multi_dir_control);
    free(file_reader);
    if(filesystem_ui_page != NULL)
    {
        lv_obj_delete(filesystem_ui_page);
        filesystem_ui_page = NULL;
    }
    lvgl_port_unlock();
}


void app_filesystem_ui_register(void)
{
    ui_config_t *filesystem = malloc(sizeof(ui_config_t));
    app_ui_create(filesystem,"FILESYSTEM",UI_FILESYSTEM,filesystem_ui_page,app_filesystem_ui_init,app_filesystem_ui_deinit);
    app_ui_add(filesystem);
}


#pragma GCC diagnostic pop