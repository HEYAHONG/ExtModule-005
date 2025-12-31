#include "hbox_config.h"
#include "hbox.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

hdefaults_tick_t hbox_tick_get (void)
{
    return  xTaskGetTickCount()*(1000/configTICK_RATE_HZ);
}

static portMUX_TYPE g_hbox_critical=portMUX_INITIALIZER_UNLOCKED;

void hbox_enter_critical()
{
    taskENTER_CRITICAL(&g_hbox_critical);
}

void hbox_exit_critical()
{
    taskEXIT_CRITICAL(&g_hbox_critical);
}
