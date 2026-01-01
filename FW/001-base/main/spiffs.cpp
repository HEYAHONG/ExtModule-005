#include "hbox_config.h"
#include "hbox.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_spiffs.h"
static const char *TAG = "spiffs";


/*
 * 初始化
 */
static void  spiffs_init(const hruntime_function_t *func)
{
    esp_vfs_spiffs_conf_t conf =
    {
        .base_path = "/var",
        .partition_label = NULL,
        .max_files = 64,
        .format_if_mount_failed = true
    };
    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    ESP_LOGI(TAG,"init,ret=%d!",ret);
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(TAG, "Partition size: total: %d bytes, used: %d bytes", total, used);
    }
}
HRUNTIME_INIT_EXPORT(spiffs,0,spiffs_init,NULL);

/*
 * 循环
 */
static void  spiffs_loop(const hruntime_function_t *func)
{

}
HRUNTIME_LOOP_EXPORT(spiffs,0,spiffs_loop,NULL);

