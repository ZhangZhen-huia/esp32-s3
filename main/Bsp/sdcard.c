#include "Inc/sdcard.h"

#include "Dev/Inc/dev_registry.h"
#include "Inc/app_filesystem.h"

static const char *TAG = "sdcard";
// file_iterator_instance_t *file_iterator_sd = NULL;//创建一个文件迭代器


//SD 卡的句柄，这个结构体包含了 SD 卡的详细信息和状态，通过这个指针可以访问和操作 SD 
//例如挂载、打印信息、卸载等
sdmmc_card_t *card;






// 写文件内容 path是路径 data是内容
esp_err_t s_example_write_file(const char *path, char *data)
{
    ESP_LOGI(TAG, "Opening file %s", path);
    FILE *f = fopen(path, "w");   // 以只写方式打开路径中文件
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing"); 
        return ESP_FAIL;
    }
    fprintf(f, data); // 写入内容
    fclose(f);  // 关闭文件
    ESP_LOGI(TAG, "File written");

    return ESP_OK;
}

// 读文件内容 path是路径
esp_err_t s_example_read_file(const char *path)
{
    ESP_LOGI(TAG, "Reading file %s", path);
    FILE *f = fopen(path, "r");  // 以只读方式打开文件
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    char line[EXAMPLE_MAX_CHAR_SIZE];  // 定义一个字符串数组
    fgets(line, sizeof(line), f); // 获取文件中的内容到字符串数组
    fclose(f); // 关闭文件

    // strip newline
    char *pos = strchr(line, '\n'); // 查找字符串中的“\n”并返回其位置
    if (pos) {
        *pos = '\0'; // 把\n替换成\0
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line); // 把数组内容输出到终端

    return ESP_OK;
}



static esp_err_t sdcard_init(void)
{
    esp_err_t ret;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,   // 如果挂载不成功是否需要格式化SD卡
        .max_files = 5, // 允许同时打开的最大文件数

        //文件系统将存储空间划分为多个固定大小的块，每个块称为一个分配单元（也称为簇或块）
        //这个参数指定了每个分配单元的大小，单位是字节
        // 假设设置 allocation_unit_size 为 16KB（16 * 1024 字节），这意味着：
        // 文件系统会将存储空间划分为多个 16KB 的块
        // 当创建一个新文件并写入数据时，文件系统会分配一个或多个 16KB 的块来存储文件数据
        // 如果文件大小为 1KB，文件系统仍然会分配一个 16KB 的块来存储这个文件，其余的 15KB 会浪费掉。
        .allocation_unit_size = 16 * 1024  // 分配单元大小，（以字节为单位）
    };


    //挂载点
    const char mount_point[] = MOUNT_POINT;
    ESP_LOGI(TAG, "Initializing SD card");
    ESP_LOGI(TAG, "Using SDMMC peripheral");

    // SDMMC主机接口配置，配置一些接口函数，模式，频率
    //.slot = SDMMC_HOST_SLOT_1 表示使用插槽1,但是后面的slot_config插槽配置我们自定义了三个引脚（因为1线模式只需要这三个引脚）
    //所以这里选择插槽1还是0都可以正常使用（前提是我们硬件上只有一个插槽并且引脚与软件对应）
    sdmmc_host_t host = SDMMC_HOST_DEFAULT(); 

    //SDMMC插槽配置，控制 SD 卡的硬件连接和通信方式
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 1;  // 设置为1线SD模式
    slot_config.clk = BSP_SD_CLK; //时钟线引脚
    slot_config.cmd = BSP_SD_CMD;//命令线引脚
    slot_config.d0 = BSP_SD_D0;//数据线 0 引脚
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP; // 打开内部上拉电阻

    ESP_LOGI(TAG, "Mounting filesystem");

    // 挂载SD卡
    ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card); 

    if (ret != ESP_OK) {  // 如果没有挂载成功
        if (ret == ESP_FAIL) { // 如果挂载失败
            ESP_LOGE(TAG, "Failed to mount filesystem. ");
        } else { // 如果是其它错误 打印错误名称
            ESP_LOGE(TAG, "Failed to initialize the card (%s). ", esp_err_to_name(ret));
        }
        return ret;
    }
    ESP_LOGI(TAG, "Filesystem mounted"); // 提示挂载成功
    sdmmc_card_print_info(stdout, card); // 终端打印SD卡的一些信息
    
   


    return ret;
}

static esp_err_t sdcard_deinit(void)
{
    const char mount_point[] = MOUNT_POINT;
    //取消挂在SD卡
    return esp_vfs_fat_sdcard_unmount(mount_point, card);
    
}


void register_sdcard_device(void)
{
    static device_descriptor_t sd = 
    {
        .name = "SDcard",
        .init_func = sdcard_init,
        .deinit_func = sdcard_deinit,
        .next = NULL,
        .priority = 7,
        .state = DEVICE_STATE_UNINITIALIZED,
        .type = FILESYSTEM,
    };

   device_register(&sd);
}


void file_iterator_delete(file_iterator_instance_t *i)
{
    if (!i) return;
    for (size_t n = 0; n < i->count; ++n) free(i->list[n]);
    free(i->list);
    free(i->directory_path);
    free(i);
}