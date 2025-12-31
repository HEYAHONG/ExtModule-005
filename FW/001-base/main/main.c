#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "hbox.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
static const char *TAG = "esp32 main";

void app_main(void)
{
    hcpprt_init();
    while(true)
    {
        hcpprt_loop();
        vTaskDelay(1);
    }
}

/*
 * 主初始化
 */
void  main_init(const hruntime_function_t *func)
{
    ESP_LOGI(TAG,"app main enter!");
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
