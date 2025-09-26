#include "esp_camera.h"
#include "Inc/camera.h"
#include "Inc/extend_io.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "freertos/queue.h"
#include "Inc/lcd.h"
#include "Dev/Inc/dev_registry.h"


static const char * TAG =  "camera";

// 定义lcd显示队列句柄
static QueueHandle_t xQueueLCDFrame = NULL;
camera_config_t config = {
    //LEDC 外设用来给某个引脚产生 PWM 信号
    //这里可以用来产生时钟信号给摄像头的 XCLK 引脚。
    //但是 S3 芯片用不着，因为 S3 芯片的 CAM 外设会产生 XCLK 信号
    //如果不是上s3芯片，就会在esp_camera_init->camera_probe()->CAMERA_ENABLE_OUT_CLOCK->
    //                       camera_enable_out_clock函数里面使用LEDC来产生信号
    .ledc_channel = LEDC_CHANNEL_1,  // LEDC通道选择  用于生成XCLK时钟 但是S3不用
    .ledc_timer = LEDC_TIMER_1, // LEDC timer选择  用于生成XCLK时钟 但是S3不用
    .pin_d0 = CAMERA_PIN_D0,
    .pin_d1 = CAMERA_PIN_D1,
    .pin_d2 = CAMERA_PIN_D2,
    .pin_d3 = CAMERA_PIN_D3,
    .pin_d4 = CAMERA_PIN_D4,
    .pin_d5 = CAMERA_PIN_D5,
    .pin_d6 = CAMERA_PIN_D6,
    .pin_d7 = CAMERA_PIN_D7,
    .pin_xclk = CAMERA_PIN_XCLK,
    .pin_pclk = CAMERA_PIN_PCLK,//像素时钟信号引脚
    .pin_vsync = CAMERA_PIN_VSYNC,//垂直同步信号引脚
    .pin_href = CAMERA_PIN_HREF,//水平参考信号引脚


    //摄像头控制芯片的寄存器与 esp32 之间使用 sccb 通信，sccb 本质上就是 i2c 通信
    .pin_sccb_sda = -1,   // 这里写-1 表示使用已经初始化的I2C接口，会在esp_camera_init->camera_probe()函数里面进行处理
    .pin_sccb_scl = CAMERA_PIN_SIOC,
    .sccb_i2c_port = 0,
    .pin_pwdn = CAMERA_PIN_PWDN,
    .pin_reset = CAMERA_PIN_RESET,
    .xclk_freq_hz = XCLK_FREQ_HZ,
    .pixel_format = PIXFORMAT_RGB565,//像素格式为RGB565
    .frame_size = FRAMESIZE_HQVGA,
    .jpeg_quality = 12,//JPEG 压缩质量。值越小，压缩率越高，图像质量越差；值越大，压缩率越低，图像质量越好。这里设置为 12
    .fb_count = 2,//帧缓冲区的数量。这里设置为 2，表示分配两个帧缓冲区
    .fb_location = CAMERA_FB_IN_PSRAM,//帧缓冲区的位置。这里设置为 CAMERA_FB_IN_PSRAM，表示帧缓冲区存储在外部 PSRAM 中
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,//数据抓取模式。这里设置为 CAMERA_GRAB_WHEN_EMPTY，表示只有当帧缓冲区为空时才抓取数据。

};

static esp_err_t bsp_camera_deinit(void)
{
    dvp_pwdn(1); // 关闭摄像头
    return esp_camera_deinit();
}

// 摄像头硬件初始化
static esp_err_t bsp_camera_init(void)
{
    //PWDN 引脚控制摄像头进入待机模式和工作模式，高电平进入待机模式，低电平进入工作模式
    dvp_pwdn(0); // 打开摄像头

    // camera init
    esp_err_t err = esp_camera_init(&config); // 配置上面定义的参数
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
        return err;
    }

    sensor_t *s = esp_camera_sensor_get(); // 获取摄像头型号

    if (s->id.PID == GC0308_PID) {
        s->set_hmirror(s, 1);  // 这里控制摄像头镜像 写1镜像 写0不镜像
    }

    app_camera_lcd();
    return err;
}

extern lv_obj_t *camera_img_show;
extern lv_img_dsc_t img_dsc;
extern uint8_t show;
// lcd处理任务
static void task_process_lcd(void *arg)
{
    camera_fb_t *frame = NULL;

    while (true)
    {
        if (xQueueReceive(xQueueLCDFrame, &frame, portMAX_DELAY))
        {
            // esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, frame->width, frame->height, (uint16_t *)frame->buf);
            // //释放相机的帧缓冲区，以便相机的驱动程序可以重新使用
            // esp_camera_fb_return(frame);

            if(show)
            {

                lvgl_port_lock(0);
                lv_draw_sw_rgb565_swap(frame->buf,frame->len);
                img_dsc.data = frame->buf;
                img_dsc.data_size = frame->len;
                lv_img_set_src(camera_img_show, &img_dsc);
                lv_obj_invalidate(camera_img_show);          
                lvgl_port_unlock();
            }

            esp_camera_fb_return(frame);
        }

    }
}


// 摄像头处理任务
static void task_process_camera(void *arg)
{
    while (true)
    {
        if(show)
        {
            camera_fb_t *frame = esp_camera_fb_get();
            if (frame)
                xQueueSend(xQueueLCDFrame, &frame, portMAX_DELAY);
        }
        else
            vTaskDelay(1000/portTICK_PERIOD_MS);

    }
}

// 让摄像头显示到LCD
void app_camera_lcd(void)
{
    xQueueLCDFrame = xQueueCreate(2, sizeof(camera_fb_t *));//摄像头配置了两个缓冲区，那么这里就创建一个长度为2的队列
    xTaskCreatePinnedToCore(task_process_camera, "task_process_camera", 3 * 1024, NULL, 4, NULL, 1);
    xTaskCreatePinnedToCore(task_process_lcd, "task_process_lcd", 4 * 1024, NULL, 4, NULL, 0);
}

void register_camera_device(void)
{
    static device_descriptor_t camera = 
    {
        .name = "Camera",
        .init_func = bsp_camera_init,
        .deinit_func = bsp_camera_deinit,
        .next = NULL,
        .priority = DEVICE_CAMERA_PRIORITY,
        .type = DISPLAY,
        .state = DEVICE_STATE_UNINITIALIZED
    };

    device_register(&camera);
}
