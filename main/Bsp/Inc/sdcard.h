#pragma once

#include "file_iterator.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include <stdio.h>
#include <string.h>

#define BSP_SD_CLK          (47)
#define BSP_SD_CMD          (48)
#define BSP_SD_D0           (21)


//挂载点路径，表示 SD 卡的文件系统将被挂载到 /sdcard 目录下
//必须以/开头，表示这是一个绝对路径
//不能包含空格，引号等特殊字符
//不能嵌套，如/mnt/sdcard 或 /sdcard/mnt 是不推荐的
#define MOUNT_POINT              "/sdcard" 


#define EXAMPLE_MAX_CHAR_SIZE    64 //最大字符大小，用于定义缓冲区大小

void file_iterator_delete(file_iterator_instance_t *i);
void register_sdcard_device(void);

