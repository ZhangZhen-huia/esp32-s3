// #pragma GCC diagnostic push
// #pragma GCC diagnostic ignored "-Wformat-truncation"

// #include "Inc/app_multidir.h"

// const static char *TAG = "multidir";

// static multi_dir_browser_t file_browser;


// // 保存当前滚动位置
// static void save_scroll_position(multi_dir_browser_t *browser, int tab_index) {
//     if (tab_index < 0 || tab_index >= browser->tab_count) {
//         return;
//     }
    
//     tab_t *tab = &browser->tabs[tab_index];
//     if (tab->file_list) {
//         // 获取当前滚动位置
//         lv_coord_t scroll_y = lv_obj_get_scroll_y(tab->file_list);
//         tab->scroll_pos = scroll_y;
        
//         // 保存到历史记录
//         if (tab->history_index >= 0 && tab->history_index < tab->history_count) {
//             tab->history[tab->history_index].scroll_pos = scroll_y;
//         }
//     }
// }

// void scroll_timer(lv_timer_t *timer)
// {
//     user_data_t *user_data = lv_timer_get_user_data(timer);
    
//     multi_dir_browser_t *browser = user_data->browser;
//     int tab_index = (int)(intptr_t)user_data->index;
    
//     if (tab_index >= 0 && tab_index < browser->tab_count) {
//         tab_t *tab = &browser->tabs[tab_index];
//         if (tab->file_list) {
//             lv_obj_scroll_to_y(tab->file_list, tab->scroll_pos, LV_ANIM_OFF);
//         }
//     }
    
//     lv_timer_del(timer);    
// }

// // 恢复滚动位置
// static void restore_scroll_position(multi_dir_browser_t *browser, int tab_index) {
//     if (tab_index < 0 || tab_index >= browser->tab_count) {
//         return;
//     }
    
//     tab_t *tab = &browser->tabs[tab_index];
//     if (tab->file_list && tab->scroll_pos > 0) {
//         user_data_t *user_data = malloc(sizeof(user_data_t));
//         user_data->browser = browser;
//         user_data->index = (intptr_t)tab_index;
//         // 延迟一点时间确保列表已完全渲染
//         lv_timer_t *timer = lv_timer_create(scroll_timer, 100, user_data);
        
//     }
// }

// // 初始化样式
// static void init_styles(multi_dir_browser_t *browser) {
//     // 背景样式
//     lv_style_init(&browser->style_bg);
//     lv_style_set_bg_color(&browser->style_bg, lv_color_hex(0x000000));
//     lv_style_set_border_width(&browser->style_bg, 0);
//     lv_style_set_pad_all(&browser->style_bg, 0);
    
//     // 列表样式
//     lv_style_init(&browser->style_list);
//     lv_style_set_bg_color(&browser->style_list, lv_color_hex(0x222222));
//     lv_style_set_border_color(&browser->style_list, lv_color_hex(0x444444));
//     lv_style_set_border_width(&browser->style_list, 1);
//     lv_style_set_radius(&browser->style_list, 5);
    
//     // 列表按钮样式
//     lv_style_init(&browser->style_list_btn);
//     lv_style_set_bg_color(&browser->style_list_btn, lv_color_hex(0x333333));
//     lv_style_set_bg_opa(&browser->style_list_btn, LV_OPA_COVER);
//     lv_style_set_border_width(&browser->style_list_btn, 0);
//     lv_style_set_radius(&browser->style_list_btn, 3);
//     lv_style_set_pad_all(&browser->style_list_btn, 5);
    
//     // 标签样式
//     lv_style_init(&browser->style_label);
//     lv_style_set_text_color(&browser->style_label, lv_color_white());
//     lv_style_set_text_font(&browser->style_label, &lv_font_montserrat_16);
// }

// // 初始化多目录浏览器
// void multi_dir_browser_init(multi_dir_browser_t *browser) {
//     memset(browser, 0, sizeof(multi_dir_browser_t));
    
//     // 初始化样式
//     init_styles(browser);
    
//     // 创建主屏幕
//     browser->main_screen = lv_obj_create(lv_scr_act());
//     lv_obj_add_style(browser->main_screen, &browser->style_bg, 0);
//     lv_obj_set_size(browser->main_screen, LV_PCT(100), LV_PCT(100));
    
//     // 创建标签视图
//     browser->tabview = lv_tabview_create(browser->main_screen);
//     lv_obj_set_size(browser->tabview, LV_PCT(100), LV_PCT(100));
    
//     // 添加默认标签页
//     browser->tab_count = 1;
//     browser->active_tab = 0;
    
//     // 初始化第一个标签页
//     browser->tabs[0].tab = lv_tabview_add_tab(browser->tabview, "Home");
    
//     // 创建路径标签
//     browser->tabs[0].path_label = lv_label_create(browser->tabs[0].tab);
//     lv_obj_add_style(browser->tabs[0].path_label, &browser->style_label, 0);
//     lv_label_set_text(browser->tabs[0].path_label, "/sdcard");
//     lv_obj_align(browser->tabs[0].path_label, LV_ALIGN_TOP_LEFT, 10, 10);
    
//     // 创建文件列表
//     browser->tabs[0].file_list = lv_list_create(browser->tabs[0].tab);
//     lv_obj_add_style(browser->tabs[0].file_list, &browser->style_list, 0);
//     lv_obj_set_size(browser->tabs[0].file_list, LV_PCT(95), LV_PCT(85));
//     lv_obj_align(browser->tabs[0].file_list, LV_ALIGN_TOP_LEFT, 10, 40);
    
//     // 添加控制按钮容器
//     lv_obj_t *btn_container = lv_obj_create(browser->main_screen);
//     lv_obj_remove_style_all(btn_container);
//     lv_obj_set_size(btn_container, LV_PCT(95), 40);
//     lv_obj_align(btn_container, LV_ALIGN_BOTTOM_MID, 0, -10);
//     lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_ROW);
//     lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
//     // 添加新标签页按钮
//     lv_obj_t *new_tab_btn = lv_btn_create(btn_container);
//     lv_obj_set_size(new_tab_btn, 100, 30);
//     lv_obj_t *new_tab_label = lv_label_create(new_tab_btn);
//     lv_label_set_text(new_tab_label, "New Tab");
//     lv_obj_center(new_tab_label);
//     lv_obj_set_user_data(new_tab_btn, (void *)(intptr_t)BUTTON_NEW_TAB);
//     lv_obj_add_event_cb(new_tab_btn, multi_dir_browser_handle_event, LV_EVENT_CLICKED, browser);
    
//     // 添加书签按钮
//     lv_obj_t *bookmark_btn = lv_btn_create(btn_container);
//     lv_obj_set_size(bookmark_btn, 100, 30);
//     lv_obj_t *bookmark_label = lv_label_create(bookmark_btn);
//     lv_label_set_text(bookmark_label, "Bookmarks");
//     lv_obj_center(bookmark_label);
//     lv_obj_set_user_data(bookmark_btn, (void *)(intptr_t)BUTTON_BOOKMARKS);
//     lv_obj_add_event_cb(bookmark_btn, multi_dir_browser_handle_event, LV_EVENT_CLICKED, browser);
    
//     // 添加刷新按钮
//     lv_obj_t *refresh_btn = lv_btn_create(btn_container);
//     lv_obj_set_size(refresh_btn, 100, 30);
//     lv_obj_t *refresh_label = lv_label_create(refresh_btn);
//     lv_label_set_text(refresh_label, "Refresh");
//     lv_obj_center(refresh_label);
//     lv_obj_set_user_data(refresh_btn, (void *)(intptr_t)BUTTON_REFRESH);
//     lv_obj_add_event_cb(refresh_btn, multi_dir_browser_handle_event, LV_EVENT_CLICKED, browser);
    
//     // 初始化默认目录
//     strlcpy(browser->tabs[0].current_path, "/sdcard", sizeof(browser->tabs[0].current_path));
//     multi_dir_browser_open_dir(browser, 0, "/sdcard");
    
//     // 加载默认书签
//     multi_dir_browser_add_bookmark(browser, "Root", "/sdcard");
//     multi_dir_browser_add_bookmark(browser, "Documents", "/sdcard/Documents");
//     multi_dir_browser_add_bookmark(browser, "Pictures", "/sdcard/Pictures");
    
//     browser->initialized = true;
    
//     // 显示主屏幕
//     lv_scr_load(browser->main_screen);
// }

// // 修改打开目录函数，添加滚动位置保存和恢复
// bool multi_dir_browser_open_dir(multi_dir_browser_t *browser, int tab_index, const char *path) {
//     if (tab_index < 0 || tab_index >= browser->tab_count) {
//         return false;
//     }
    
//     tab_t *tab = &browser->tabs[tab_index];
    
//     // 保存当前滚动位置
//     save_scroll_position(browser, tab_index);
    
//     DIR *dir = opendir(path);
//     if (!dir) {
//         ESP_LOGE(TAG, "Failed to open directory: %s", path);
//         return false;
//     }
    
//     // 保存当前状态到历史记录
//     if (tab->history_count < MAX_HISTORY) {
//         strlcpy(tab->history[tab->history_count].path, tab->current_path, MAX_PATH_LEN);
//         tab->history[tab->history_count].scroll_pos = tab->scroll_pos;
//         tab->history[tab->history_count].selected_index = tab->selected_index;
//         tab->history_count++;
//         tab->history_index = tab->history_count - 1;
//     }
    
//     // 清空当前文件列表
//     if (tab->files) {
//         free(tab->files);
//         tab->files = NULL;
//     }
    
//     // 清空UI列表
//     lv_obj_clean(tab->file_list);
    
//     // 更新当前路径
//     strlcpy(tab->current_path, path, sizeof(tab->current_path));
//     lv_label_set_text(tab->path_label, path);
    
//     // 更新标签页标题
//     char *last_slash = strrchr(path, '/');
//     if (last_slash && strlen(last_slash) > 1) {
//         lv_tabview_rename_tab(browser->tabview, tab_index, last_slash + 1);
//     } else {
//         lv_tabview_rename_tab(browser->tabview, tab_index, "Root");
//     }
    
//     // 读取目录内容
//     struct dirent *entry;
//     int count = 0;
    
//     // 第一次遍历计算文件数量
//     while ((entry = readdir(dir)) != NULL) {
//         if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
//             count++;
//         }
//     }
    
//     rewinddir(dir);
    
//     // 分配内存
//     tab->files = malloc(count * sizeof(file_info_t));
//     if (!tab->files) {
//         closedir(dir);
//         return false;
//     }
    
//     // 第二次遍历填充文件信息
//     int index = 0;
//     while ((entry = readdir(dir)) != NULL) {
//         if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
//             continue;
//         }
        
//         // 构建完整路径
//         char full_path[MAX_PATH_LEN];
//         snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
//         // 获取文件信息
//         struct stat st;
//         if (stat(full_path, &st) == 0) {
//             strlcpy(tab->files[index].name, entry->d_name, sizeof(tab->files[index].name));
//             tab->files[index].is_dir = S_ISDIR(st.st_mode);
//             tab->files[index].size = st.st_size;
//             tab->files[index].modified = st.st_mtime;
//             index++;
//         }
//     }
    
//     tab->file_count = index;
//     closedir(dir);
    
//     // 添加返回上级目录按钮（如果不是根目录）
//     if (strcmp(path, "/sdcard") != 0 && strcmp(path, "/") != 0) {
//         lv_obj_t *btn = lv_list_add_btn(tab->file_list, LV_SYMBOL_LEFT, "..");
//         lv_obj_add_style(btn, &browser->style_list_btn, 0);
//         lv_obj_set_user_data(btn, (void *)(intptr_t)-1);
//         lv_obj_add_event_cb(btn, multi_dir_browser_handle_event, LV_EVENT_CLICKED, browser);
//     }
    
//     // 添加文件和目录按钮
//     for (int i = 0; i < tab->file_count; i++) {
//         const char *icon = tab->files[i].is_dir ? LV_SYMBOL_DIRECTORY : LV_SYMBOL_FILE;
//         char btn_text[128];
//         char size_text[32];
//         char time_text[32];
        
//         // 格式化文件大小
//         if (tab->files[i].is_dir) {
//             strcpy(size_text, "[DIR]");
//         } else if (tab->files[i].size < 1024) {
//             snprintf(size_text, sizeof(size_text), "%dB", tab->files[i].size);
//         } else if (tab->files[i].size < 1024 * 1024) {
//             snprintf(size_text, sizeof(size_text), "%.1fKB", tab->files[i].size / 1024.0);
//         } else {
//             snprintf(size_text, sizeof(size_text), "%.1fMB", tab->files[i].size / (1024.0 * 1024.0));
//         }
        
//         // 格式化时间
//         time_t now = time(NULL);
//         time_t diff = now - tab->files[i].modified;
        
//         if (diff < 60) {
//             strcpy(time_text, "just now");
//         } else if (diff < 3600) {
//             snprintf(time_text, sizeof(time_text), "%dm ago", (int)(diff / 60));
//         } else if (diff < 86400) {
//             snprintf(time_text, sizeof(time_text), "%dh ago", (int)(diff / 3600));
//         } else {
//             snprintf(time_text, sizeof(time_text), "%dd ago", (int)(diff / 86400));
//         }
        
//         snprintf(btn_text, sizeof(btn_text), "%s %s %s %s", icon, tab->files[i].name, size_text, time_text);
        
//         lv_obj_t *btn = lv_list_add_btn(tab->file_list, NULL, btn_text);
//         lv_obj_add_style(btn, &browser->style_list_btn, 0);
//         lv_obj_set_user_data(btn, (void *)(intptr_t)i);
//         lv_obj_add_event_cb(btn, multi_dir_browser_handle_event, LV_EVENT_CLICKED, browser);
//     }
    
//     // 重置滚动位置和选择索引
//     tab->selected_index = 0;
    
//     // 恢复滚动位置（如果是返回操作）
//     if (tab->scroll_pos > 0) {
//         restore_scroll_position(browser, tab_index);
//     } else {
//         lv_obj_scroll_to_y(tab->file_list, 0, LV_ANIM_OFF);
//     }
    
//     return true;
// }

// // 修改标签页切换函数，添加滚动位置保存和恢复
// int multi_dir_browser_new_tab(multi_dir_browser_t *browser, const char *path) {
//     if (browser->tab_count >= MAX_TABS) {
//         ESP_LOGE(TAG, "Maximum number of tabs reached");
//         return -1;
//     }
    
//     // 保存当前标签页的滚动位置
//     save_scroll_position(browser, browser->active_tab);
    
//     int new_tab_index = browser->tab_count;
//     browser->tab_count++;
    
//     // 创建新标签页
//     char tab_name[20];
//     snprintf(tab_name, sizeof(tab_name), "Tab %d", new_tab_index + 1);
    
//     browser->tabs[new_tab_index].tab = lv_tabview_add_tab(browser->tabview, tab_name);
    
//     // 创建路径标签
//     browser->tabs[new_tab_index].path_label = lv_label_create(browser->tabs[new_tab_index].tab);
//     lv_obj_add_style(browser->tabs[new_tab_index].path_label, &browser->style_label, 0);
//     lv_label_set_text(browser->tabs[new_tab_index].path_label, path);
//     lv_obj_align(browser->tabs[new_tab_index].path_label, LV_ALIGN_TOP_LEFT, 10, 10);
    
//     // 创建文件列表
//     browser->tabs[new_tab_index].file_list = lv_list_create(browser->tabs[new_tab_index].tab);
//     lv_obj_add_style(browser->tabs[new_tab_index].file_list, &browser->style_list, 0);
//     lv_obj_set_size(browser->tabs[new_tab_index].file_list, LV_PCT(95), LV_PCT(85));
//     lv_obj_align(browser->tabs[new_tab_index].file_list, LV_ALIGN_TOP_LEFT, 10, 40);
    
//     // 初始化目录
//     strlcpy(browser->tabs[new_tab_index].current_path, path, sizeof(browser->tabs[new_tab_index].current_path));
//     multi_dir_browser_open_dir(browser, new_tab_index, path);
    
//     // 切换到新标签页
//     lv_tabview_set_act(browser->tabview, new_tab_index, LV_ANIM_ON);
//     browser->active_tab = new_tab_index;
    
//     return new_tab_index;
// }

// // 关闭标签页
// bool multi_dir_browser_close_tab(multi_dir_browser_t *browser, int tab_index) {
//     if (tab_index < 0 || tab_index >= browser->tab_count || browser->tab_count <= 1) {
//         return false;
//     }
    
//     // 释放资源
//     if (browser->tabs[tab_index].files) {
//         free(browser->tabs[tab_index].files);
//         browser->tabs[tab_index].files = NULL;
//     }
    
//     // 移除标签页
//     lv_obj_del(browser->tabs[tab_index].tab);
    
//     // 移动后续标签页
//     for (int i = tab_index; i < browser->tab_count - 1; i++) {
//         browser->tabs[i] = browser->tabs[i + 1];
//     }
    
//     browser->tab_count--;
    
//     // 更新活动标签页
//     if (browser->active_tab >= tab_index) {
//         browser->active_tab = MAX(0, browser->active_tab - 1);
//     }
    
//     // 切换到剩余的第一个标签页
//     lv_tabview_set_act(browser->tabview, browser->active_tab, LV_ANIM_ON);
    
//     return true;
// }

// // 刷新当前标签页
// void multi_dir_browser_refresh(multi_dir_browser_t *browser) {
//     int active_tab = browser->active_tab;
//     if (active_tab >= 0 && active_tab < browser->tab_count) {
//         multi_dir_browser_open_dir(browser, active_tab, browser->tabs[active_tab].current_path);
//     }
// }

// // 添加书签
// bool multi_dir_browser_add_bookmark(multi_dir_browser_t *browser, const char *name, const char *path) {
//     if (browser->bookmark_count >= MAX_BOOKMARKS) {
//         return false;
//     }
    
//     // 检查是否已存在
//     for (int i = 0; i < browser->bookmark_count; i++) {
//         if (strcmp(browser->bookmarks[i].path, path) == 0) {
//             // 更新现有书签
//             strlcpy(browser->bookmarks[i].name, name, MAX_FILENAME_LEN);
//             return true;
//         }
//     }
    
//     // 添加新书签
//     strlcpy(browser->bookmarks[browser->bookmark_count].name, name, MAX_FILENAME_LEN);
//     strlcpy(browser->bookmarks[browser->bookmark_count].path, path, MAX_PATH_LEN);
//     browser->bookmark_count++;
    
//     return true;
// }


// static void close_menu_cb(lv_event_t * e)
// {
//     lv_obj_t * menu = lv_obj_get_parent(lv_event_get_target(e));
//     lv_obj_del(menu);
// }

// // 显示书签菜单
// void multi_dir_browser_show_bookmarks(multi_dir_browser_t *browser) {
//     // 创建书签菜单
//     lv_obj_t *bookmark_menu = lv_obj_create(lv_scr_act());
//     lv_obj_set_user_data(bookmark_menu, (void*)BUTTON_BOOKMARKS); // 设置标识
//     lv_obj_set_size(bookmark_menu, 300, 400);
//     lv_obj_center(bookmark_menu);
//     lv_obj_add_style(bookmark_menu, &browser->style_list, 0);
    
//     // 添加标题
//     lv_obj_t *title = lv_label_create(bookmark_menu);
//     lv_obj_add_style(title, &browser->style_label, 0);
//     lv_label_set_text(title, "Bookmarks");
//     lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
//     lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    
//     // 创建书签列表
//     lv_obj_t *bookmark_list = lv_list_create(bookmark_menu);
//     lv_obj_set_size(bookmark_list, LV_PCT(90), LV_PCT(70));
//     lv_obj_align(bookmark_list, LV_ALIGN_TOP_MID, 0, 50);
//     lv_obj_add_style(bookmark_list, &browser->style_list, 0);
    
//     // 添加书签项
//     for (int i = 0; i < browser->bookmark_count; i++) {
//         lv_obj_t *btn = lv_list_add_btn(bookmark_list, LV_SYMBOL_DIRECTORY, browser->bookmarks[i].name);
//         lv_obj_add_style(btn, &browser->style_list_btn, 0);
//         lv_obj_set_user_data(btn, (void *)(intptr_t)i);
//         lv_obj_add_event_cb(btn, multi_dir_browser_handle_event, LV_EVENT_CLICKED, browser);
//     }
    
//     // 添加关闭按钮
//     lv_obj_t *close_btn = lv_btn_create(bookmark_menu);
//     lv_obj_set_size(close_btn, 100, 30);
//     lv_obj_align(close_btn, LV_ALIGN_BOTTOM_MID, 0, -10);
//     lv_obj_t *close_label = lv_label_create(close_btn);
//     lv_label_set_text(close_label, "Close");
//     lv_obj_center(close_label);
//     lv_obj_add_event_cb(close_btn,close_menu_cb, LV_EVENT_CLICKED, NULL);
// }



// // 处理UI事件
// // 修改事件处理函数，添加滚动位置保存
// void multi_dir_browser_handle_event(lv_event_t *e) {
//     lv_obj_t *target = lv_event_get_target(e);
//     multi_dir_browser_t *browser = lv_event_get_user_data(e);
//     lv_event_code_t code = lv_event_get_code(e);
    
//     if (code == LV_EVENT_CLICKED) {
//         // 获取按钮类型
//         button_type_t button_type = (button_type_t)(intptr_t)lv_obj_get_user_data(target);
        
//         switch (button_type) {
//             case BUTTON_NEW_TAB:
//                 multi_dir_browser_new_tab(browser, "/sdcard");
//                 break;
                
//             case BUTTON_BOOKMARKS:
//                 multi_dir_browser_show_bookmarks(browser);
//                 break;
                
//             case BUTTON_REFRESH:
//                 // 保存当前滚动位置
//                 save_scroll_position(browser, browser->active_tab);
//                 multi_dir_browser_refresh(browser);
//                 break;
                
//             default:
//                 // 处理文件列表中的按钮
//                 for (int i = 0; i < browser->tab_count; i++) {
//                     if (lv_obj_get_parent(target) == browser->tabs[i].file_list) {
//                         intptr_t index = (intptr_t)lv_obj_get_user_data(target);
                        
//                         if (index == -1) {
//                             // 上级目录按钮 - 保存当前滚动位置
//                             save_scroll_position(browser, i);
                            
//                             char *last_slash = strrchr(browser->tabs[i].current_path, '/');
//                             if (last_slash && last_slash != browser->tabs[i].current_path) {
//                                 *last_slash = '\0';
//                                 multi_dir_browser_open_dir(browser, i, browser->tabs[i].current_path);
//                             } else if (strcmp(browser->tabs[i].current_path, "/sdcard") != 0) {
//                                 multi_dir_browser_open_dir(browser, i, "/sdcard");
//                             }
//                         } else if (index >= 0 && index < browser->tabs[i].file_count) {
//                             // 文件或目录按钮 - 保存当前滚动位置
//                             save_scroll_position(browser, i);
                            
//                             char new_path[MAX_PATH_LEN];
//                             snprintf(new_path, sizeof(new_path), "%s/%s", 
//                                      browser->tabs[i].current_path, 
//                                      browser->tabs[i].files[index].name);
                            
//                             if (browser->tabs[i].files[index].is_dir) {
//                                 // 打开目录
//                                 multi_dir_browser_open_dir(browser, i, new_path);
//                             } else {
//                                 // 处理文件选择
//                                 ESP_LOGI(TAG, "File selected: %s", new_path);
//                                 // 这里可以添加文件处理逻辑，例如打开文件阅读器
//                             }
//                         }
//                         break;
//                     }
//                 }
//                 break;
//         }
        
//         // 书签菜单项处理
//         lv_obj_t *parent = lv_obj_get_parent(target);
//         if (parent && lv_obj_check_type(parent, &lv_list_class)) {
//             lv_obj_t *grandparent = lv_obj_get_parent(parent);
//             if (grandparent && lv_obj_get_user_data(grandparent) == (void*)BUTTON_BOOKMARKS) {
//                 // 书签菜单中的按钮 - 保存当前滚动位置
//                 save_scroll_position(browser, browser->active_tab);
                
//                 int bookmark_index = (intptr_t)lv_obj_get_user_data(target);
//                 if (bookmark_index >= 0 && bookmark_index < browser->bookmark_count) {
//                     multi_dir_browser_open_dir(browser, browser->active_tab, 
//                                               browser->bookmarks[bookmark_index].path);
//                     lv_obj_del(grandparent); // 关闭书签菜单
//                 }
//             }
//         }
//     }
    
//     // 标签页切换事件 - 保存当前标签页的滚动位置
//     if (code == LV_EVENT_VALUE_CHANGED && target == browser->tabview) {
//         int old_tab = browser->active_tab;
//         browser->active_tab = lv_tabview_get_tab_act(browser->tabview);
        
//         // 保存旧标签页的滚动位置
//         if (old_tab >= 0 && old_tab < browser->tab_count) {
//             save_scroll_position(browser, old_tab);
//         }
        
//         // 恢复新标签页的滚动位置
//         restore_scroll_position(browser, browser->active_tab);
//     }
// }

// #pragma GCC diagnostic pop