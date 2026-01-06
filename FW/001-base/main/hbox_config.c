#include "hbox_config.h"
#include "hbox.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"

hdefaults_tick_t hbox_tick_get (void)
{
    return  xTaskGetTickCount()*(1000/configTICK_RATE_HZ);
}

static SemaphoreHandle_t GlobalLockHandle=NULL;
static StaticSemaphore_t GlobalLockBuffer;

void hbox_enter_critical()
{
    if(GlobalLockHandle==NULL)
    {
        GlobalLockHandle=xSemaphoreCreateRecursiveMutexStatic( &GlobalLockBuffer );
    }
    if(GlobalLockHandle!=NULL)
    {
        xSemaphoreTakeRecursive(GlobalLockHandle,portMAX_DELAY);
    }
}

void hbox_exit_critical()
{
    if(GlobalLockHandle!=NULL)
    {
        xSemaphoreGiveRecursive(GlobalLockHandle);
    }
}


/*
 * 当前时间
 */
time_t time(time_t *timer)
{
    hgettimeofday_timeval_t tv= {0};
    hgettimeofday(&tv,NULL);
    if(timer!=NULL)
    {
        (*timer)=tv.tv_sec;
    }
    return tv.tv_sec;
}

static int cmd_datetime_entry(int argc,const char *argv[])
{
    hshell_context_t * hshell_ctx=hshell_context_get_from_main_argv(argc,argv);
    time_t time_now=time(NULL);
    const char *TZ="";
    {
        const char *tz_str=hgetenv("TZ");
        if(tz_str!=NULL)
        {
            TZ=tz_str;
        }
    }
    hshell_printf(hshell_ctx,"%s %s",TZ,asctime(localtime(&time_now)));
    return 0;
};
HSHELL_COMMAND_EXPORT(datetime,cmd_datetime_entry,show datetime);

/*
 * 设置当前时间
 */
void hbox_set_time(time_t new_time)
{
    hgettimeofday_timeval_t tv= {0};
    tv.tv_sec=new_time;
    hsettimeofday(&tv,NULL);
}

static int cmd_set_datetime_entry(int argc,const char *argv[])
{
    hshell_context_t * hshell_ctx=hshell_context_get_from_main_argv(argc,argv);
    if(argc <=1)
    {
        hshell_printf(hshell_ctx,"set_datetime [year] [month] [day] [hour] [minute] [second]\r\n");
    }
    else
    {
        time_t time_now=time(NULL);
        struct tm time_now_struct= {0};
        localtime_r(&time_now,&time_now_struct);
        if(argc >= 2)
        {
            time_now_struct.tm_year=atoi(argv[1])-1900;
        }
        if(argc >= 3)
        {
            time_now_struct.tm_mon=atoi(argv[2])-1;
        }
        if(argc >= 4)
        {
            time_now_struct.tm_mday=atoi(argv[3]);
        }
        if(argc >= 5)
        {
            time_now_struct.tm_hour=atoi(argv[4]);
        }
        if(argc >= 6)
        {
            time_now_struct.tm_min=atoi(argv[5]);
        }
        if(argc >= 7)
        {
            time_now_struct.tm_sec=atoi(argv[6]);
        }
        time_now=mktime(&time_now_struct);
        hbox_set_time(time_now);
    }
    return 0;
};
HSHELL_COMMAND_EXPORT(set_datetime,cmd_set_datetime_entry,set datetime.);


static int cmd_reboot_entry(int argc,const char *argv[])
{
    esp_restart();
}
HSHELL_COMMAND_EXPORT(reboot,cmd_reboot_entry,reboot.);

#include "multi_heap.h"
static int cmd_free_entry(int argc,const char *argv[])
{
    hshell_context_t * hshell_ctx=hshell_context_get_from_main_argv(argc,argv);
    hshell_printf(hshell_ctx,"free:%d bytes\r\n",esp_get_free_heap_size());
    return 0;
};
HSHELL_COMMAND_EXPORT(free,cmd_free_entry,show memory info);


