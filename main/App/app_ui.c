#include "Inc/app_ui.h"

#include "Inc/dev_registry.h"

static const char * TAG = "UI";
lv_style_t default_style;

//页面ui管理者
ui_manager_t ui_manager = 
{
	.current = NULL,
	.head = NULL,
	.is_animating = false
};

EventGroupHandle_t ui_event_group;

void app_main_ui_register(void);
void app_ui_task(void *arg);

void app_ui_Init(void)
{


    lv_style_init(&default_style);
    lv_style_set_bg_opa( &default_style, LV_OPA_COVER ); // 背景透明度
    lv_style_set_border_width(&default_style, 0); // 边框宽度
    lv_style_set_pad_all(&default_style, 0);  // 内间距
    lv_style_set_radius(&default_style, 0);   // 圆角半径
    lv_style_set_width(&default_style, 320);  // 宽
    lv_style_set_height(&default_style, 240); // 高


	//创建ui
	app_main_ui_register();
	app_wifi_ui_register();
	app_desktop_ui_register();
	app_statusbar_ui_register();
	app_camera_ui_register();
	app_setting_ui_register();
	app_filesystem_ui_register();
	app_music_ui_register();
	ui_event_group = xEventGroupCreate();
	xTaskCreatePinnedToCore(app_ui_task,"app_ui_task",1024*5,NULL,2,NULL,1);

	app_ui_display_by_id(UI_DESKTOP);
}



void app_ui_task(void *arg)
{

	while(1)
	{
		EventBits_t ui_bits = xEventGroupWaitBits(	ui_event_group,
													UI_MAIN_BIT|UI_WIFI_BIT|UI_DESKTOP_BIT|UI_CAMERA_BIT|UI_SETTING_BIT|UI_FILESYSTEM_BIT|UI_MUSIC_BIT,
													pdTRUE,
													pdFALSE,
													portMAX_DELAY);

		if(ui_bits & UI_MAIN_BIT)
		{
			app_ui_display_by_id(UI_MAIN);
		}
		else if(ui_bits & UI_WIFI_BIT)
		{
			device_deinitialize("music");
			device_initialize("Wifi");
			app_ui_display_by_id(UI_WIFI);
		}
		else if(ui_bits & UI_DESKTOP_BIT)
		{
			app_ui_display_by_id(UI_DESKTOP);
		}
		else if(ui_bits & UI_CAMERA_BIT)
		{
			device_deinitialize("music");
			app_ui_display_by_id(UI_CAMERA);
		}
		else if(ui_bits & UI_SETTING_BIT)
		{
			app_ui_display_by_id(UI_SETTING);
		}
		else if(ui_bits & UI_FILESYSTEM_BIT)
		{
			app_ui_display_by_id(UI_FILESYSTEM);
		}
		else if(ui_bits & UI_MUSIC_BIT)
		{
			device_deinitialize("Wifi");
			device_initialize("music");
			app_ui_display_by_id(UI_MUSIC);
		}		
	}
}



//创建ui
esp_err_t app_ui_create(ui_config_t *ui, char *name,ui_id_e id,lv_obj_t *screen,void (*init), void(*deinit))
{
	if(ui == NULL)
	{
		ESP_LOGE(TAG,"ui_config is null");
		return ESP_FAIL;
	}
	//赋值为0，便于初始化
	lv_memset(ui,0,sizeof(ui_config_t));

	ui->init_handle = init;
	ui->deinit_handle = deinit;
	ui->page_id = id;
	ui->name = name;
	ui->screen = screen;
	return ESP_OK;
}


//在链表头部添加一个页面
esp_err_t app_ui_add(ui_config_t *ui)
{
	if(ui == NULL)
	{
		ESP_LOGE(TAG,"the ui to add is null!");
		return ESP_FAIL;
	}
	//如果这是第一个画面,那就把他设置为链表头部
	if(ui_manager.head ==NULL)
	{
		ESP_LOGI(TAG,"add first ui %s",ui->name);
		ui_manager.head = ui;
		// ui_manager.current = ui;
	}	
	else
	{
		ESP_LOGI(TAG,"add ui %s to list",ui->name);
		ui->next = ui_manager.head;
		ui_manager.head = ui;
	}

	return ESP_OK;
}

//链接UI
esp_err_t app_ui_link(ui_pos_e pos,ui_config_t *ui_to_add,ui_id_e base_id)
{
	static ui_config_t *base_ui = NULL;
	if(ESP_OK != app_ui_find_by_id(base_id,&base_ui))
	{
		ESP_LOGE(TAG,"No id !");
		return ESP_FAIL;
	}

	if(ui_to_add == NULL || base_ui == NULL)
	{
		ESP_LOGE(TAG,"ui_pages is NULL!");
		return ESP_FAIL;
	}
	switch (pos)
	{
	case LEFT:
		base_ui->left = ui_to_add;
		ui_to_add->right = base_ui;
		ESP_LOGI(TAG,"%s is to the left of %s",ui_to_add->name,base_ui->name);
		break;

	case RIGHT:
	base_ui->right = ui_to_add;
	ui_to_add->left = base_ui;
	ESP_LOGI(TAG,"%s is to the right of %s",ui_to_add->name,base_ui->name);
	break;

	case UP:
	base_ui->up = ui_to_add;
	ui_to_add->down = base_ui;
	ESP_LOGI(TAG,"%s is above %s",ui_to_add->name,base_ui->name);
	break;

	case DOWN:
	base_ui->down = ui_to_add;
	ui_to_add->up = base_ui;
	ESP_LOGI(TAG,"%s is below %s",ui_to_add->name,base_ui->name);
	break;
	
	default:
		ESP_LOGE(TAG, "add ui error pos");
		return ESP_FAIL;
		break;
	}

	return ESP_OK;
}

//获取当前ui的id
esp_err_t app_ui_get_id(ui_id_e * cuid)
{
	if(ui_manager.current == NULL)
	{
		ESP_LOGE(TAG,"No current ui");
			return ESP_FAIL;	
	}

	*cuid = ui_manager.current->page_id;
	return ESP_OK;

}


//通过id找到ui
esp_err_t app_ui_find_by_id(ui_id_e id,ui_config_t **ui)
{
	ui_config_t * ui_temp = ui_manager.head;

	while(ui_temp)
	{
		if(ui_temp->page_id == id)
		{
			*ui = ui_temp;
			return ESP_OK;
		}
		ui_temp = ui_temp->next;
	}

	ESP_LOGW(TAG,"Can't find ui by provided id!");
	return ESP_FAIL;
}

//通过id显示ui
esp_err_t app_ui_display_by_id(ui_id_e id)
{
	ui_config_t *ui = NULL;
	
	if(ESP_OK != app_ui_find_by_id(id,&ui))
	{
		ESP_LOGE(TAG,"No id !");
		return ESP_FAIL;
	}
		

	if(ui_manager.current == NULL)
	{
		ui_manager.current = ui;
		ESP_LOGI(TAG,"display %s",ui_manager.current->name);
		ui_manager.current->init_handle(ui->user_data);
		return ESP_OK;

	}
	if(ui_manager.current != ui && ui != NULL)
	{
		if(ui_manager.current->deinit_handle)
		{
			lvgl_port_lock(0);
			ui_manager.current->deinit_handle(NULL);
			lvgl_port_unlock();
			ESP_LOGI(TAG,"deinit %s ",ui_manager.current->name);
		}

		ESP_LOGI(TAG,"current ui is %s",ui->name);
		ui_manager.current = ui;
	}
	ESP_LOGI(TAG,"display %s",ui_manager.current->name);
	ui_manager.current->init_handle(ui->user_data);
	return ESP_OK;
}

//显示当前页面的上下左右
esp_err_t app_ui_display_pos(ui_pos_e pos)
{
	ui_config_t *temp = ui_manager.current;

	switch (pos)
	{
	case LEFT:
		if(temp->left == NULL)
		{
			ESP_LOGE(TAG,"Current ui doesn't have left page!");
			return ESP_FAIL;
		}
		ui_manager.current = temp->left;
		break;

	case RIGHT:
		if(temp->right == NULL)
		{
			ESP_LOGE(TAG,"Current ui doesn't have right page!");
			return ESP_FAIL;
		}
		ui_manager.current = temp->right;
		break;

	case UP:
		if(temp->up == NULL)
		{
			ESP_LOGE(TAG,"Current ui doesn't have up page!");
			return ESP_FAIL;
		}
		ui_manager.current = temp->up;
		break;

	case DOWN:
		if(temp->down == NULL)
		{
			ESP_LOGE(TAG,"Current ui doesn't have down page!");
			return ESP_FAIL;
		}
		ui_manager.current = temp->down;						
		break;
	
	default:
		ESP_LOGE(TAG,"ERR POS!");
		break;
	}
	ui_manager.current->init_handle(ui_manager.current->user_data);
	return ESP_OK;
}
