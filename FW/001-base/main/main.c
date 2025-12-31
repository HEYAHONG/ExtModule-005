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
	ESP_LOGI(TAG,"app main enter!");
	hcpprt_init();
	while(true)
	{
		hcpprt_loop();
		vTaskDelay(1);
	}
}
