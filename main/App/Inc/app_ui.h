#pragma once

#include "lvgl.h"
#include "esp_log.h"
#include "esp_err.h"
#include "stdio.h"
#include "lvgl.h"
#include "lv_demos.h"
#include "esp_lvgl_port.h"
#include "freertos/FreeRTOS.h"

#include "Inc/Bsp.h"

#include "Inc/app_wifi.h"
#include "Inc/app_desktop.h"
#include "Inc/app_statusbar.h"
#include "Inc/app_camera.h"
#include "Inc/app_setting.h"
#include "Inc/app_filesystem.h"


#define UI_WIFI_BIT        BIT0
#define UI_SETTING_BIT     BIT1
#define UI_CAMERA_BIT      BIT2
#define UI_DESKTOP_BIT     BIT3
#define UI_MAIN_BIT        BIT4
#define UI_FILESYSTEM_BIT  BIT5


typedef enum
{
    UI_MAIN,
    UI_WIFI,
    UI_CAMERA,
    UI_DESKTOP,
    UI_STATUSBAR,
    UI_SETTING,
    UI_FILESYSTEM
}ui_id_e;

typedef enum{
    LEFT,
    RIGHT,
    DOWN,
    UP
}ui_pos_e;

// //定义函数指针
// typedef void(*page_init_handle)(lv_obj_t* root);

typedef struct ui_config_t
{
	ui_id_e page_id;
	lv_obj_t* screen;
    char *name;
    void (*init_handle) (void *);
	void (*deinit_handle) (void *);
    struct ui_config_t *left;
    struct ui_config_t *right;
    struct ui_config_t *up;
    struct ui_config_t *down;
    struct ui_config_t *next;
    void * user_data;
}ui_config_t;


typedef struct 
{
   ui_config_t *head;
   ui_config_t *current;
   bool is_animating;
}ui_manager_t;



extern ui_manager_t ui_manager;
extern EventGroupHandle_t ui_event_group;
extern lv_style_t default_style;



extern lv_obj_t *desktop_ui_page;
extern lv_obj_t *setting_ui_page;
extern lv_obj_t *wifi_scan_page;     
extern lv_obj_t *ui_camera_page;        
extern lv_obj_t *filesystem_ui_page;


extern file_iterator_instance_t *file_iterator_sd;


esp_err_t app_ui_create(ui_config_t *ui, char *name,ui_id_e id,lv_obj_t *screen,void (*init), void(*deinit));
esp_err_t app_ui_add(ui_config_t *ui);
esp_err_t app_ui_link(ui_pos_e pos,ui_config_t *ui_to_add,ui_id_e base_id);
esp_err_t app_ui_get_id(ui_id_e * cuid);
esp_err_t app_ui_find_by_id(ui_id_e id,ui_config_t **ui);
esp_err_t app_ui_display_pos(ui_pos_e pos);
esp_err_t app_ui_display_by_id(ui_id_e id);

void app_ui_Init(void);
/*
#define UI_REGISTER(ui,name,id,init,deinit)\
do{\
	lv_memset(ui,0,sizeof(ui_config_t));\
	ui->init_handle = init;\
	ui->deinit_handle = deinit;\
	ui->page_id = id;\
	ui->name = name;\
	if(ui_manager.head ==NULL){\
        ui_manager.head = ui;\
    }\
    ui_add(ui);\
}while (0);
*/