#include "hbox_config.h"
#include "hbox.h"
#include "esp_log.h"
#include "esp_system.h"
static const char *TAG = "watchdog";

static void sys_reset()
{
    esp_restart();
    //若复位未实现，使用死循环通过硬件看门狗复位
    while(true)
    {

    }
}

static void hw_feed()
{

}

/*
 * 初始化
 */
static void  watchdog_init(const hruntime_function_t *func)
{
    ESP_LOGI(TAG,"init!");
	//设置好看门狗
    hwatchdog_set_hardware_dog_feed(hw_feed);
    hwatchdog_setup_software_dog(sys_reset,hdefaults_tick_get);
}
HRUNTIME_INIT_EXPORT(watchdog,0,watchdog_init,NULL);

/*
 * 循环
 */
static void  watchdog_loop(const hruntime_function_t *func)
{

}
HRUNTIME_LOOP_EXPORT(watchdog,0,watchdog_loop,NULL);

