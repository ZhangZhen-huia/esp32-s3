#include "Inc/app_statusbar.h"
#include "Inc/app_ui.h"
#include "Inc/app_desktop.h"

lv_obj_t *statusbar_ui_page;

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



