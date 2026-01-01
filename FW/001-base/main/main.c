#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_task.h"
#include "hbox.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
static const char *TAG = "esp32 main";

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
