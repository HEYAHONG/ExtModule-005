#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_task.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
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

    /*
     * esp_libc组件重新设定environ环境变量，需要将其复原。
     * 可通过hgetenv获取环境变量的项目
     */
    environ=global_environ;



    /*
     * 设定时区
     * 对于newlib而言，时区可通过environ环境变量设置，调用tzset生效
     */
    tzset();

    {
        //初始化nvs_flash
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            nvs_flash_erase();
            nvs_flash_init();
        }
    }

    /*
     * 启动hruntime
     */
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

static int cmd_ota_entry(int argc,const char *argv[])
{
    hshell_context_t * hshell_ctx=hshell_context_get_from_main_argv(argc,argv);
    if(argc<=1)
    {
        hshell_printf(hshell_ctx,"%s\t[command]:\r\n",argv[0]!=NULL?argv[0]:"ota");
        hshell_printf(hshell_ctx,"\tfactory\t\tdownload factory app to OTA partition\r\n");
        hshell_printf(hshell_ctx,"\tboot\t\tset boot partition.boot [n]\r\n");
    }
    else
    {
        if(argv[1]!=NULL && strcmp(argv[1],"factory")==0)
        {
            /*
             * 将factory分区下载至下一个OTA分区，覆盖掉原有分区内容
             */
            const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
            if(update_partition==NULL)
            {
                hshell_printf(hshell_ctx,"OTA partition is not found!\r\n");
                return -1;
            }
            const esp_partition_t *factory_partition=NULL;
            {
                esp_partition_iterator_t it=esp_partition_find(ESP_PARTITION_TYPE_APP,ESP_PARTITION_SUBTYPE_APP_FACTORY,NULL);
                if(it!=NULL)
                {
                    factory_partition=esp_partition_get(it);
                    esp_partition_iterator_release(it);
                }
            }
            if(factory_partition==NULL)
            {
                hshell_printf(hshell_ctx,"factory partition is not found!\r\n");
                return-1;
            }
            esp_ota_handle_t update_handle = 0;
            if(ESP_OK!=esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle))
            {
                hshell_printf(hshell_ctx,"esp_ota_begin error!\r\n");
                return -1;
            }
            {
                /*
                 * 复制数据
                 */
                const void *partition_bin = NULL;
                esp_partition_mmap_handle_t data_map=0;
                if(ESP_OK!=(esp_partition_mmap(factory_partition, 0, factory_partition->size, ESP_PARTITION_MMAP_DATA, &partition_bin, &data_map)))
                {

                    esp_ota_abort(update_handle);
                    hshell_printf(hshell_ctx,"esp_partition_mmap error!\r\n");
                    return -1;
                }
                if(ESP_OK!=(esp_ota_write(update_handle, (const void *)partition_bin, factory_partition->size)))
                {
                    esp_partition_munmap(data_map);
                    esp_ota_abort(update_handle);
                    hshell_printf(hshell_ctx,"esp_ota_write error!\r\n");
                    return -1;
                }
                esp_partition_munmap(data_map);
            }
            if(ESP_OK!=(esp_ota_end(update_handle)))
            {
                hshell_printf(hshell_ctx,"esp_ota_end error!\r\n");
                return -1;
            }
            if(ESP_OK!=(esp_ota_set_boot_partition(update_partition)))
            {
                hshell_printf(hshell_ctx,"esp_ota_set_boot_partition error!\r\n");
                return -1;
            }
            hshell_printf(hshell_ctx,"done! now you can reboot!\r\n");
        }
        else if(argv[1]!=NULL && strcmp(argv[1],"boot")==0)
        {
            if(argv[2]==NULL || argc <=2)
            {
                /*
                 * 显示帮助信息
                 */
                hshell_printf(hshell_ctx,"boot\t\tset boot partition.boot [n]\r\n");
                hshell_printf(hshell_ctx,"----------\r\n");
                if(NULL!=esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY, NULL))
                {
                    hshell_printf(hshell_ctx,"-1\tfactory\r\n");
                }
                for (esp_partition_subtype_t t = ESP_PARTITION_SUBTYPE_APP_OTA_0; t != ESP_PARTITION_SUBTYPE_APP_OTA_MAX; t++)
                {
                    const esp_partition_t *p = esp_partition_find_first(ESP_PARTITION_TYPE_APP, t, NULL);
                    if (p == NULL)
                    {
                        continue;
                    }
                    hshell_printf(hshell_ctx,"%d\t%s\r\n",((int)t-ESP_PARTITION_SUBTYPE_APP_OTA_0),p->label);
                }
                return 0;
            }
            int n=atoi(argv[2]);
            const esp_partition_t *next_partition=NULL;
            if(n==-1)
            {
                next_partition=esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY, NULL);
            }
            else if(n >= 0)
            {
                esp_partition_subtype_t t =  ESP_PARTITION_SUBTYPE_APP_OTA_0+n;
                if(t < ESP_PARTITION_SUBTYPE_APP_OTA_MAX)
                {
                    next_partition=esp_partition_find_first(ESP_PARTITION_TYPE_APP, t, NULL);
                }
            }
            if(next_partition==NULL)
            {
                hshell_printf(hshell_ctx,"partition is not found!");
                return -1;
            }
            hshell_printf(hshell_ctx,"partition=%s\r\n",next_partition->label);
            esp_app_desc_t app_desc;
            if(esp_ota_get_partition_description(next_partition,&app_desc)!=ESP_OK)
            {
                hshell_printf(hshell_ctx,"esp_ota_get_partition_description error!\r\n");
                return -1;
            }
            hshell_printf(hshell_ctx,"app=%s\r\n",app_desc.project_name);
            if(ESP_OK!=(esp_ota_set_boot_partition(next_partition)))
            {
                hshell_printf(hshell_ctx,"esp_ota_set_boot_partition error!\r\n");
                return -1;
            }
            hshell_printf(hshell_ctx,"done! now you can reboot!\r\n");
        }
    }
    return 0;
};
HSHELL_COMMAND_EXPORT(ota,cmd_ota_entry,ota command);


