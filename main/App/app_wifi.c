#include "Inc/app_wifi.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "lvgl.h"

#include "esp_log.h"
#include "esp_lvgl_port.h"


#include "esp_event.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "Inc/camera.h"
#include "Dev/Inc/dev_registry.h"

static const char *TAG = "app_wifi";

lv_obj_t *wifi_scan_page = NULL;                   // wifi扫描页面
lv_obj_t *wifi_password_page = NULL;
lv_obj_t *wifi_connect_page = NULL;
lv_obj_t *wifi_main_page = NULL;
lv_obj_t *wifi_list;            
lv_obj_t *label_wifi_connect; // wifi连接页面label 
lv_obj_t *ta_pass_text;       // 密码输入文本框 textarea
lv_obj_t *roller_num;         // 数字roller
lv_obj_t *roller_letter_low;  // 小写字母roller
lv_obj_t *roller_letter_up;   // 大写字母roller
lv_obj_t *label_wifi_name;    // wifi名称label
lv_obj_t *btn_back_to_desktop;
lv_obj_t *btn_delete_label;

EventGroupHandle_t s_wifi_event_group;

esp_event_handler_instance_t instance_any_id;
esp_event_handler_instance_t instance_got_ip;

lv_image_dsc_t * main_anim_imgs[4];
#define TXT  "Wifi Scan..."   // 可换成汉字，每个元素一个字符
lv_obj_t *letter[sizeof(TXT)-1];   // 每个字母一个标签

esp_netif_t *sta_netif = NULL;
uint16_t ap_count = 0;                      //定义热点数量
uint16_t number = DEFAULT_SCAN_LIST_SIZE;   //定义默认最大扫描列表数量
static int s_retry_num = 0;


// wifi账号队列
static QueueHandle_t xQueueWifiAccount = NULL;
// 队列要传输的内容
typedef struct {
    char wifi_ssid[32];  // 获取wifi名称
    char wifi_password[64]; // 获取wifi密码         
} wifi_account_t;


//定义热点列表
////用于 Wi-Fi 扫描结果缓存：调用 esp_wifi_scan_get_ap_records() 后，
//所有扫描到的 AP 信息都会依次填入 ap_info[0] … ap_info[DEFAULT_SCAN_LIST_SIZE-1]
wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
//初始化热点列表

void app_wifi_ui_register(void)
{
    //由于把wifi_ui挂进了全局链表，那么就不能单独free了，不然会破坏链表结构，使得上下指针不连贯
    //要想free，只能先退出链表，在free
    ui_config_t* wifi_ui = malloc(sizeof(ui_config_t));
    app_ui_create(wifi_ui,"WIFI_UI",UI_WIFI,wifi_scan_page,app_wifi_ui_init,app_wifi_ui_deinit);
    app_ui_add(wifi_ui);
}

// 小写字母确认键 处理函数
static void btn_letter_low_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "Btn-Letter-Low Clicked");
        char buf[2]; // 接收roller的值
        lv_roller_get_selected_str(roller_letter_low, buf, sizeof(buf));
        lv_textarea_add_text(ta_pass_text, buf);
    }
}

// 大写字母确认键 处理函数
static void btn_letter_up_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "Btn-Letter-Up Clicked");
        char buf[2]; // 接收roller的值
        lv_roller_get_selected_str(roller_letter_up, buf, sizeof(buf));
        lv_textarea_add_text(ta_pass_text, buf);
    }
}

// 返回按钮
static void btn_back_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target_obj(e);
    if(code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "btn_back Clicked");
        if(target == btn_back_to_desktop)
        {
            lvgl_port_lock(0);
            app_ui_display_by_id(UI_DESKTOP);//切换到桌面            
            lvgl_port_unlock();
        }
        else if(target == btn_delete_label)
        {
            lvgl_port_lock(0);
            lv_obj_delete(wifi_connect_page);
            wifi_connect_page=NULL;
            lvgl_port_unlock();
        }
        else
            lv_obj_del(wifi_password_page); //删除密码输入界面
    }
}


static void lv_wifi_connect(void)
{
    lv_obj_del(wifi_password_page); // 删除密码输入界面

    // 创建一个面板对象
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_bg_opa( &style, LV_OPA_COVER );
    lv_style_set_border_width(&style, 0);
    lv_style_set_pad_all(&style, 0);
    lv_style_set_radius(&style, 0);  
    lv_style_set_width(&style, 320);  
    lv_style_set_height(&style, 240); 

    wifi_connect_page = lv_obj_create(lv_scr_act());
    lv_obj_add_style(wifi_connect_page, &style, 0);

    // 绘制label提示
    label_wifi_connect = lv_label_create(wifi_connect_page);
    lv_label_set_text(label_wifi_connect, "WLAN CONNECTING...");
    lv_obj_set_style_text_font(label_wifi_connect, &lv_font_montserrat_22, 0);
    lv_obj_align(label_wifi_connect, LV_ALIGN_CENTER, 0, -50);
}


// WiFi连接按钮 处理函数
static void btn_connect_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "OK Clicked");
        const char *wifi_ssid = lv_label_get_text(label_wifi_name);
        const char *wifi_password = lv_textarea_get_text(ta_pass_text);
        if(*wifi_password != '\0') // 判断是否为空字符串
        {
            wifi_account_t wifi_account;
            strcpy(wifi_account.wifi_ssid, wifi_ssid);
            strcpy(wifi_account.wifi_password, wifi_password);
            ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                    wifi_account.wifi_ssid, wifi_account.wifi_password);
            lv_wifi_connect(); // 显示wifi连接界面
            // 发送WiFi账号密码信息到队列
            xQueueSend(xQueueWifiAccount, &wifi_account, portMAX_DELAY); 
        }
    }
}


// 删除密码按钮
static void btn_del_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "Clicked");
        lv_textarea_delete_char(ta_pass_text);
    }
}


// 数字键 处理函数
static void btn_num_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "Btn-Num Clicked");
        char buf[2]; // 接收roller的值
        lv_roller_get_selected_str(roller_num, buf, sizeof(buf));
        lv_textarea_add_text(ta_pass_text, buf);
    }
}


//密码输入界面
static void list_btn_cb(lv_event_t * e)
{
 // 获取点击到的WiFi名称
    const char *wifi_name=NULL;
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {
        wifi_name = lv_list_get_btn_text(wifi_list, obj);
        ESP_LOGI(TAG, "WLAN Name: %s", wifi_name);
    }

    // 创建密码输入页面，这个就是覆盖了wifi_scan_page页面
    wifi_password_page = lv_obj_create(lv_scr_act());
    lv_obj_set_size(wifi_password_page, 320, 240);
    lv_obj_set_style_border_width(wifi_password_page, 0, 0); // 设置边框宽度
    lv_obj_set_style_pad_all(wifi_password_page, 0, 0);  // 设置间隙
    lv_obj_set_style_radius(wifi_password_page, 0, 0); // 设置圆角

    // 创建返回按钮
    lv_obj_t *btn_back = lv_btn_create(wifi_password_page);
    lv_obj_align(btn_back, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_size(btn_back, 60, 40);
    lv_obj_set_style_border_width(btn_back, 0, 0); // 设置边框宽度
    lv_obj_set_style_pad_all(btn_back, 0, 0);  // 设置间隙
    lv_obj_set_style_bg_opa(btn_back, LV_OPA_TRANSP, LV_PART_MAIN); // 背景透明
    lv_obj_set_style_shadow_opa(btn_back, LV_OPA_TRANSP, LV_PART_MAIN); // 阴影透明
    lv_obj_add_event_cb(btn_back, btn_back_cb, LV_EVENT_ALL, NULL); // 添加按键处理函数

    lv_obj_t *label_back = lv_label_create(btn_back); 
    lv_label_set_text(label_back, LV_SYMBOL_LEFT);  // 按键上显示左箭头符号
    lv_obj_set_style_text_font(label_back, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label_back, lv_color_hex(0x000000), 0); 
    lv_obj_align(label_back, LV_ALIGN_TOP_LEFT, 10, 10);

    // 显示选中的wifi名称
    label_wifi_name = lv_label_create(wifi_password_page);
    // lv_obj_set_style_text_font(label_wifi_name, &font_alipuhui20, 0);
    lv_obj_set_style_text_font(label_wifi_name, &lv_font_montserrat_20, 0);
    lv_label_set_text(label_wifi_name, wifi_name);
    lv_obj_align(label_wifi_name, LV_ALIGN_TOP_MID, 0, 10);

    // 创建密码输入框
    ta_pass_text = lv_textarea_create(wifi_password_page);
    lv_obj_set_style_text_font(ta_pass_text, &lv_font_montserrat_20, 0);
    lv_textarea_set_one_line(ta_pass_text, true);  // 一行显示
    lv_textarea_set_password_mode(ta_pass_text, false); // 是否使用密码输入显示模式
    lv_textarea_set_placeholder_text(ta_pass_text, "password"); // 设置提醒词
    lv_obj_set_width(ta_pass_text, 150); // 宽度
    lv_obj_align(ta_pass_text, LV_ALIGN_TOP_LEFT, 10, 40); // 位置
    lv_obj_add_state(ta_pass_text, LV_STATE_FOCUSED); // 显示光标

    // 创建“连接按钮”
    lv_obj_t *btn_connect = lv_btn_create(wifi_password_page);
    lv_obj_align(btn_connect, LV_ALIGN_TOP_LEFT, 170, 40);
    lv_obj_set_width(btn_connect, 65); // 宽度
    lv_obj_add_event_cb(btn_connect, btn_connect_cb, LV_EVENT_ALL, NULL); // 事件处理函数

    lv_obj_t *label_ok = lv_label_create(btn_connect);
    lv_label_set_text(label_ok, "OK");
    lv_obj_set_style_text_font(label_ok, &lv_font_montserrat_20, 0);
    lv_obj_center(label_ok);

    // 创建“删除按钮”
    lv_obj_t *btn_del = lv_btn_create(wifi_password_page);
    lv_obj_align(btn_del, LV_ALIGN_TOP_LEFT, 245, 40);
    lv_obj_set_width(btn_del, 65); // 宽度
    lv_obj_add_event_cb(btn_del, btn_del_cb, LV_EVENT_ALL, NULL);  // 事件处理函数

    lv_obj_t *label_del = lv_label_create(btn_del);
    lv_label_set_text(label_del, LV_SYMBOL_BACKSPACE);
    lv_obj_set_style_text_font(label_del, &lv_font_montserrat_20, 0);
    lv_obj_center(label_del);

    // 创建roller样式
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_bg_color(&style, lv_color_black());
    lv_style_set_text_color(&style, lv_color_white());
    lv_style_set_border_width(&style, 0);
    lv_style_set_pad_all(&style, 0);
    lv_style_set_radius(&style, 0);

    // 创建"数字"roller
    const char * opts_num = "0\n1\n2\n3\n4\n5\n6\n7\n8\n9";

    roller_num = lv_roller_create(wifi_password_page);
    lv_obj_add_style(roller_num, &style, 0);
    lv_obj_set_style_bg_opa(roller_num, LV_OPA_50, LV_PART_SELECTED);

    lv_roller_set_options(roller_num, opts_num, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(roller_num, 3); // 显示3行
    lv_roller_set_selected(roller_num, 5, LV_ANIM_OFF); // 默认选择
    lv_obj_set_width(roller_num, 90);
    lv_obj_set_style_text_font(roller_num, &lv_font_montserrat_20, 0);
    lv_obj_align(roller_num, LV_ALIGN_BOTTOM_LEFT, 15, -53);
    // lv_obj_add_event_cb(roller_num, mask_event_cb, LV_EVENT_ALL, NULL); // 事件处理函数

    // 创建"数字"roller 的确认键
    lv_obj_t *btn_num_ok = lv_btn_create(wifi_password_page);
    lv_obj_align(btn_num_ok, LV_ALIGN_BOTTOM_LEFT, 15, -10); // 位置
    lv_obj_set_width(btn_num_ok, 90); // 宽度
    lv_obj_add_event_cb(btn_num_ok, btn_num_cb, LV_EVENT_ALL, NULL); // 事件处理函数

    lv_obj_t *label_num_ok = lv_label_create(btn_num_ok);
    lv_label_set_text(label_num_ok, LV_SYMBOL_OK);
    lv_obj_set_style_text_font(label_num_ok, &lv_font_montserrat_20, 0);
    lv_obj_center(label_num_ok);

    // 创建"小写字母"roller
    const char * opts_letter_low = "a\nb\nc\nd\ne\nf\ng\nh\ni\nj\nk\nl\nm\nn\no\np\nq\nr\ns\nt\nu\nv\nw\nx\ny\nz";

    roller_letter_low = lv_roller_create(wifi_password_page);
    lv_obj_add_style(roller_letter_low, &style, 0);
    lv_obj_set_style_bg_opa(roller_letter_low, LV_OPA_50, LV_PART_SELECTED); // 设置选中项的透明度
    lv_roller_set_options(roller_letter_low, opts_letter_low, LV_ROLLER_MODE_INFINITE); // 循环滚动模式
    lv_roller_set_visible_row_count(roller_letter_low, 3);
    lv_roller_set_selected(roller_letter_low, 15, LV_ANIM_OFF); // 
    lv_obj_set_width(roller_letter_low, 90);
    lv_obj_set_style_text_font(roller_letter_low, &lv_font_montserrat_20, 0);
    lv_obj_align(roller_letter_low, LV_ALIGN_BOTTOM_LEFT, 115, -53);
    // lv_obj_add_event_cb(roller_letter_low, mask_event_cb, LV_EVENT_ALL, NULL); // 事件处理函数

    // 创建"小写字母"roller的确认键
    lv_obj_t *btn_letter_low_ok = lv_btn_create(wifi_password_page);
    lv_obj_align(btn_letter_low_ok, LV_ALIGN_BOTTOM_LEFT, 115, -10);
    lv_obj_set_width(btn_letter_low_ok, 90); // 宽度
    lv_obj_add_event_cb(btn_letter_low_ok, btn_letter_low_cb, LV_EVENT_ALL, NULL); // 事件处理函数

    lv_obj_t *label_letter_low_ok = lv_label_create(btn_letter_low_ok);
    lv_label_set_text(label_letter_low_ok, LV_SYMBOL_OK);
    lv_obj_set_style_text_font(label_letter_low_ok, &lv_font_montserrat_20, 0);
    lv_obj_center(label_letter_low_ok);

    // 创建"大写字母"roller
    const char * opts_letter_up = "A\nB\nC\nD\nE\nF\nG\nH\nI\nJ\nK\nL\nM\nN\nO\nP\nQ\nR\nS\nT\nU\nV\nW\nX\nY\nZ";

    roller_letter_up = lv_roller_create(wifi_password_page);
    lv_obj_add_style(roller_letter_up, &style, 0);
    lv_obj_set_style_bg_opa(roller_letter_up, LV_OPA_50, LV_PART_SELECTED); // 设置选中项的透明度
    lv_roller_set_options(roller_letter_up, opts_letter_up, LV_ROLLER_MODE_INFINITE); // 循环滚动模式
    lv_roller_set_visible_row_count(roller_letter_up, 3);
    lv_roller_set_selected(roller_letter_up, 15, LV_ANIM_OFF);
    lv_obj_set_width(roller_letter_up, 90);
    lv_obj_set_style_text_font(roller_letter_up, &lv_font_montserrat_20, 0);
    lv_obj_align(roller_letter_up, LV_ALIGN_BOTTOM_LEFT, 215, -53);
    // lv_obj_add_event_cb(roller_letter_up, mask_event_cb, LV_EVENT_ALL, NULL); // 事件处理函数

    // 创建"大写字母"roller的确认键
    lv_obj_t *btn_letter_up_ok = lv_btn_create(wifi_password_page);
    lv_obj_align(btn_letter_up_ok, LV_ALIGN_BOTTOM_LEFT, 215, -10);
    lv_obj_set_width(btn_letter_up_ok, 90); 
    lv_obj_add_event_cb(btn_letter_up_ok, btn_letter_up_cb, LV_EVENT_ALL, NULL); // 事件处理函数

    lv_obj_t *label_letter_up_ok = lv_label_create(btn_letter_up_ok);
    lv_label_set_text(label_letter_up_ok, LV_SYMBOL_OK);
    lv_obj_set_style_text_font(label_letter_up_ok, &lv_font_montserrat_20, 0);
    lv_obj_center(label_letter_up_ok);}


// wifi处理任务
static void wifi_connect(void *arg)
{
    wifi_account_t wifi_account;

    while (true)
    {
        // 如果收到wifi账号队列消息
        if(xQueueReceive(xQueueWifiAccount, &wifi_account, portMAX_DELAY))
        {
            wifi_config_t wifi_config = {
                .sta = {
                    .threshold.authmode = WIFI_AUTH_WPA2_PSK,
                    .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
                    .sae_h2e_identifier = "",
                    },
            };
            strcpy((char *)wifi_config.sta.ssid, wifi_account.wifi_ssid);
            strcpy((char *)wifi_config.sta.password, wifi_account.wifi_password);
            ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
            ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 wifi_config.sta.ssid, wifi_config.sta.password);
            if(esp_wifi_sta_get_ap_info(ap_info) == ESP_OK)
            {
                esp_wifi_disconnect();
            }
            else
            {
                esp_wifi_connect();
            }   
            /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
            * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
            EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdTRUE,
            pdFALSE,
            portMAX_DELAY);

            /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
            * happened. */
            if (bits & WIFI_CONNECTED_BIT) {
                ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                        wifi_config.sta.ssid, wifi_config.sta.password);
                lvgl_port_lock(0);
                lv_label_set_text(label_wifi_connect, "WLAN CONNECT SUCCEED");
                lvgl_port_unlock();
                xEventGroupSetBits(ui_event_group,UI_DESKTOP_BIT);
                
            } else if (bits & WIFI_FAIL_BIT) {
                
                lvgl_port_lock(0);
                lv_label_set_text(label_wifi_connect, "WLAN CONNECT FAILED");
                lvgl_port_unlock();
                ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                        wifi_config.sta.ssid, wifi_config.sta.password);
               
            } else {
                ESP_LOGE(TAG, "UNEXPECTED EVENT");
                lvgl_port_lock(0);
                lv_label_set_text(label_wifi_connect, "WLAN CONNECT UNEXPECTED");
                lvgl_port_unlock();
                
            }
            lvgl_port_lock(0);
            btn_delete_label = lv_btn_create(wifi_connect_page);
            lv_obj_align(btn_delete_label,LV_ALIGN_BOTTOM_MID,0,-40);
            lv_obj_set_size(btn_delete_label, 100, 80);
            lv_obj_set_style_bg_opa(btn_delete_label, LV_OPA_TRANSP, LV_PART_MAIN); // 背景透明
            lv_obj_set_style_shadow_opa(btn_delete_label, LV_OPA_TRANSP, LV_PART_MAIN); // 阴影透明
            lv_obj_add_event_cb(btn_delete_label, btn_back_cb, LV_EVENT_ALL, NULL); // 添加按键处理函数

            lv_obj_t *label_back = lv_label_create(btn_delete_label); 
            lv_label_set_text(label_back, LV_SYMBOL_OK);  // 按键上显示OK符号
            lv_obj_set_style_text_font(label_back, &lv_font_montserrat_20, 0);
            lv_obj_set_style_text_color(label_back, lv_color_hex(0x000000), 0); 
            lv_obj_align(label_back, LV_ALIGN_CENTER, 0, 0);
            lvgl_port_unlock();
        }
    }
}







static void jump_exec(void *var, int32_t v)
{
    /* v: 0→1000→0 缓入-缓出，振幅 8 px，中心对齐 */
    int32_t y = (v - 500) * 6 / 1000;   // -4 ~ +4
    lv_obj_align((lv_obj_t *)var, LV_ALIGN_BOTTOM_MID,
                 lv_obj_get_x_aligned((lv_obj_t *)var), y-10);
}

void app_wifi_ui_init(void *)
{
    ESP_LOGI(TAG,"wifi ui");
    lvgl_port_lock(0);
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_bg_opa( &style, LV_OPA_COVER ); // 背景透明度
    lv_style_set_border_width(&style, 0); // 边框宽度
    lv_style_set_pad_all(&style, 0);  // 内间距
    lv_style_set_radius(&style, 0);   // 圆角半径
    lv_style_set_width(&style, 320);  // 宽
    lv_style_set_height(&style, 240); // 高

    wifi_main_page = lv_obj_create(lv_scr_act());
    lv_obj_add_style(wifi_main_page, &style, 0);

    //跳动字体
    for (int i = 0; i < sizeof(TXT)-1; ++i) {
        char buf[2] = {TXT[i], '\0'};
        letter[i] = lv_label_create(wifi_main_page);
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

    lv_obj_t * animimg0 = lv_animimg_create(wifi_main_page);
    
    lv_obj_set_align(animimg0,LV_ALIGN_TOP_MID);
    lv_obj_set_size(animimg0,200,200);
    lv_animimg_set_src(animimg0, (const void **)main_anim_imgs, 4);
    lv_animimg_set_duration(animimg0, 500);
    lv_animimg_set_repeat_count(animimg0,LV_ANIM_REPEAT_INFINITE);
    lvgl_port_unlock();
    lv_animimg_start(animimg0);
    //开启扫描,阻塞扫描
    esp_wifi_scan_start(NULL, true);

    // EventBits_t wifi_bits = xEventGroupWaitBits(s_wifi_event_group,
    //         WIFI_SCAN_DOWN_BIT,
    //         pdTRUE,
    //         pdFALSE,
    //         portMAX_DELAY);
    //         if(wifi_bits & WIFI_SCAN_DOWN_BIT)
    //         {
    //             xEventGroupSetBits(ui_event_group,UI_WIFI_BIT);
    //         }
    lvgl_port_lock(0);
    lv_obj_del(wifi_main_page);
    wifi_main_page = NULL;
    wifi_scan_page = lv_obj_create(lv_scr_act());
    lv_obj_add_style(wifi_scan_page, &default_style, 0);
    lv_obj_set_scrollbar_mode(wifi_scan_page, LV_SCROLLBAR_MODE_OFF); // 隐藏wifi_scan_page滚动条
   // 创建返回按钮
    btn_back_to_desktop = lv_btn_create(wifi_scan_page);
    lv_obj_align(btn_back_to_desktop, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_size(btn_back_to_desktop, 60, 40);
    lv_obj_set_style_border_width(btn_back_to_desktop, 0, 0); // 设置边框宽度
    lv_obj_set_style_pad_all(btn_back_to_desktop, 0, 0);  // 设置间隙
    lv_obj_set_style_bg_opa(btn_back_to_desktop, LV_OPA_TRANSP, LV_PART_MAIN); // 背景透明
    lv_obj_set_style_shadow_opa(btn_back_to_desktop, LV_OPA_TRANSP, LV_PART_MAIN); // 阴影透明
    lv_obj_add_event_cb(btn_back_to_desktop, btn_back_cb, LV_EVENT_ALL, NULL); // 添加按键处理函数

    lv_obj_t *label_back = lv_label_create(btn_back_to_desktop); 
    lv_label_set_text(label_back, LV_SYMBOL_LEFT);  // 按键上显示左箭头符号
    lv_obj_set_style_text_font(label_back, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label_back, lv_color_hex(0x000000), 0); 
    lv_obj_align(label_back, LV_ALIGN_TOP_LEFT, 10, 10);

    wifi_list = lv_list_create(wifi_scan_page);
    lv_obj_set_size(wifi_list, lv_pct(100), lv_pct(80));
    lv_obj_align(wifi_list,LV_ALIGN_CENTER,0,30);
    lv_obj_set_style_border_width(wifi_list, 0, 0);
    lv_obj_set_style_text_font(wifi_list,&lv_font_montserrat_22,LV_PART_MAIN);

#if USING_CHINESE
    lv_obj_set_style_text_font(wifi_list, &font_alipuhui20, 0);
#endif
    lv_obj_set_style_bg_color(wifi_list, lv_color_hex(0xffffff), 0);
    lv_obj_set_scrollbar_mode(wifi_list, LV_SCROLLBAR_MODE_OFF); // 隐藏wifi_list滚动条
    // 显示wifi信息
    lv_obj_t * btn = NULL;
    for (int i = 0; i < number; i++) {
        ESP_LOGI(TAG, "SSID \t\t%s", ap_info[i].ssid);  // 终端输出wifi名称
        ESP_LOGI(TAG, "RSSI \t\t%d", ap_info[i].rssi);  // 终端输出wifi信号质量
        // 添加wifi列表
        btn = lv_list_add_btn(wifi_list, LV_SYMBOL_WIFI, (const char *)ap_info[i].ssid); 
        lv_obj_add_event_cb(btn, list_btn_cb, LV_EVENT_CLICKED, NULL); // 添加点击回调函数
    }
        
    // 创建wifi连接任务
    xQueueWifiAccount = xQueueCreate(2, sizeof(wifi_account_t));
    xTaskCreatePinnedToCore(wifi_connect, "wifi_connect", 4 * 1024, NULL, 5, NULL, 1);  // 创建wifi连接任务

    lvgl_port_unlock();

}

void app_wifi_ui_deinit(void *)
{
    lvgl_port_lock(0);
    if(wifi_scan_page != NULL)
        {
            lv_obj_delete(wifi_scan_page);
            wifi_scan_page = NULL;
        }
    if(wifi_connect_page != NULL)
        {
            lv_obj_delete(wifi_connect_page);
            wifi_connect_page = NULL;
        }
        
    ESP_LOGI(TAG,"delete finish");
    lvgl_port_unlock();
    
}

// 清除wifi初始化内容
static esp_err_t wifiset_deinit(void)
{
    ESP_LOGW(TAG, "unregister instance=%p", instance_any_id);
    ESP_LOGW(TAG, "unregister instance=%p", instance_got_ip);
    if(instance_any_id)
    {
        ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
        instance_any_id = NULL;
    }
    if(instance_got_ip)
    {
        ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
        instance_got_ip = NULL;
    }
    esp_err_t err = esp_wifi_stop();
    if (err == ESP_ERR_WIFI_NOT_INIT) {
        return err;
    }
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(esp_wifi_deinit());
    ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(sta_netif));
    esp_netif_destroy(sta_netif);
    sta_netif = NULL;
    ESP_ERROR_CHECK(esp_event_loop_delete_default());
    
    device_descriptor_t *device_descriptor = device_find("Wifi");
    device_descriptor->state = DEVICE_STATE_UNINITIALIZED;

    return ESP_OK;
}










static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    //wifi启动成功
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {

        xEventGroupSetBits(s_wifi_event_group,WIFI_START_BIT);
        // //开始wifi连接
        // esp_wifi_connect();
    } 
    //wifi扫描完成
    else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE)
    {

        ESP_LOGI(TAG,"wifi scan down");
        //获取最近一次 Wi-Fi 扫描完成后发现的 AP 数量，该函数必须确保在wifi扫秒完成后进行调用，所以最好使用事件循环
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
        //在 Wi-Fi 扫描完成后，把扫描到的所有 AP 记录一次性拷贝到用户提供的缓冲区，并返回实际记录条数。
        //调用后，驱动内部缓存会被清空，因此该函数只能调用一次 
        //第一个参数是先输入缓冲区最大可容纳的数量，然后函数返回的时候保存了实际返回的ap数据
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number,ap_info));
        ESP_LOGI(TAG, "Total APs scanned = %u, actual AP number ap_info holds = %u",number , ap_count);
        xEventGroupSetBits(s_wifi_event_group,WIFI_SCAN_DOWN_BIT);
    }
    //wifi断开连接
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP: %d",s_retry_num);
        }
        else
        {
            ESP_LOGI(TAG,"connect to the AP fail");
            s_retry_num = 0;
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
           
         }
        }
            
    //基站连接到热点成功，但此时还没有获取到热点的ip
    else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
    {
        wifi_event_sta_connected_t *event = (wifi_event_sta_connected_t *)event_data;
        ESP_LOGI(TAG, "Connected to SSID: %s", event->ssid);
        ESP_LOGI(TAG, "BSSID: %02x:%02x:%02x:%02x:%02x:%02x",
                 event->bssid[0], event->bssid[1], event->bssid[2],
                 event->bssid[3], event->bssid[4], event->bssid[5]);
        ESP_LOGI(TAG, "Channel: %d", event->channel);
    }
    //设备获取IP成功
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        //打印设备分配到的ip地址
        ESP_LOGI(TAG, "got sta ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group,WIFI_CONNECTED_BIT);

    }
}



static esp_err_t wifi_sta_init(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    

    //初始化tcp/ip底层代码
    ESP_ERROR_CHECK(esp_netif_init());

    //创建默认事件循环
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    //创建wifi客户端接口
    sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);
    //初始化wifi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
     ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 注册事件处理函数（回调函数）到默认事件循环
    // 事件源（如WIFI_EVENT、IP_EVENT），事件ID，事件处理函数，传入处理函数的参数，回调处理函数的注册句柄
    // 同一个处理函数可以多次注册为不同的事件源
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    assert(instance_any_id);
    assert(instance_got_ip);
    ESP_LOGW(TAG, "register instance=%p", instance_any_id);
    ESP_LOGW(TAG, "register instance=%p", instance_got_ip);




    //设置wifi模式，配置
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_LOGI(TAG, "Max AP number ap_info can hold = %u", number);

    s_wifi_event_group = xEventGroupCreate();
    memset(ap_info, 0, sizeof(ap_info));
    //开启wifi
    esp_wifi_start();


    return ESP_OK;

}

void register_wifi_device(void)
{
    static device_descriptor_t wifi = 
    {
        .name = "Wifi",
        .init_func = wifi_sta_init,
        .deinit_func = wifiset_deinit,
        .next = NULL,
        .priority = DEVICE_WIFI_PRIORITY,
        .state = DEVICE_STATE_PENDING,
        .type = WIFI,
    };

   device_register(&wifi);
}