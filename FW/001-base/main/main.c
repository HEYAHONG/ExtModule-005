#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_task.h"
#include "esp_ota_ops.h"
#include "hbox.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
static const char *TAG = "esp32 main";

/*
 * 全局环境变量
 */
static char *global_environ[]=
{
    /*
     * 语言设置
     */
    (char *)"LANG=zh_CN.UTF-8",
    /*
     * 时区设置(调用tzset生效,设置完成后，C库的相关时间函数的时区即生效)
     */
    (char *)"TZ=CST-8:00:00",
    NULL
};

/*
 * 设定环境变量指针
 */
char **environ=global_environ;



void vApplicationIdleHook( void )
{
    /*
     * 空闲钩子喂狗
     */
    if(hwatchdog_is_valid())
    {
        HWATCHDOG_FEED();
    }
}

static void hruntime_task( void * pvParameters)
{
    hcpprt_init();
    while(true)
    {
        size_t count=50;
        while(count--)
        {
            hcpprt_loop();
        }
        vTaskDelay(1);
    }
}

void app_main(void)
{
    if(xTaskCreate(hruntime_task,"hruntime",ESP_TASK_MAIN_STACK,NULL,ESP_TASK_MAIN_PRIO,NULL)!=pdPASS)
    {
        ESP_LOGI(TAG,"start hruntime failed!");
    }
}

/*
 * 主初始化
 */
void  main_init(const hruntime_function_t *func)
{
    ESP_LOGI(TAG,"app main enter!");
    {
        FILE* f = fopen("/var/banner", "r");
        if (f != NULL)
        {
            fseek(f,0,SEEK_END);
            long file_size=ftell(f);
            if(file_size > 0)
            {
                fseek(f,0,SEEK_SET);
                char * buff=(char *)malloc(file_size+1);
                memset(buff,0,file_size+1);
                if(buff !=NULL)
                {
                    fread(buff,1,file_size,f);
                    ESP_LOGI(TAG,"\r\n%s",buff);
                    free(buff);
                }
            }
            fclose(f);
        }
    }

    ESP_LOGI(TAG,"ota partition count=%d",(int)esp_ota_get_app_partition_count());
    const esp_partition_t* running_partition=esp_ota_get_running_partition();
    if(running_partition!=NULL)
    {
        ESP_LOGI(TAG,"running partition=%s",running_partition->label);
    }
    esp_app_desc_t running_app_desc;
    if(esp_ota_get_partition_description(running_partition,&running_app_desc)==ESP_OK)
    {
        ESP_LOGI(TAG,"running app=%s",running_app_desc.project_name);
    }

    //关闭hruntime中的喂狗，准备由空闲任务喂狗
    hruntime_loop_enable_softwatchdog(false);
}
HRUNTIME_INIT_EXPORT(main,255,main_init,NULL);
HRUNTIME_SYMBOL_EXPORT(main_init);

/*
 * 主循环
 */
void  main_loop(const hruntime_function_t *func)
{

}
HRUNTIME_LOOP_EXPORT(main,255,main_loop,NULL);
HRUNTIME_SYMBOL_EXPORT(main_loop);
