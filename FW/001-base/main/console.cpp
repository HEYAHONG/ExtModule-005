#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hbox.h"

static const char *TAG="console";

#define CONSOLE_UART_PORT_NUM         UART_NUM_0
#define CONSOLE_UART_PORT_PIN_TXD     GPIO_NUM_1
#define CONSOLE_UART_PORT_PIN_RXD     GPIO_NUM_3
#define CONSOLE_UART_PORT_PIN_CTS     UART_PIN_NO_CHANGE
#define CONSOLE_UART_PORT_PIN_RTS     UART_PIN_NO_CHANGE


static void hconsole_driver_install(void)
{
    //安装驱动
    if(!uart_is_driver_installed(CONSOLE_UART_PORT_NUM))
    {
        uart_driver_install(CONSOLE_UART_PORT_NUM,2048,2048,0,NULL,ESP_INTR_FLAG_IRAM);
        esp_log_set_vprintf(hvprintf);
        uart_config_t uart_config;
        memset(&uart_config,0,sizeof(uart_config));
        {
            uart_config.baud_rate = 115200;
            uart_config.data_bits = UART_DATA_8_BITS;
            uart_config.parity    = UART_PARITY_DISABLE;
            uart_config.stop_bits = UART_STOP_BITS_1;
            uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
            uart_config.source_clk = UART_SCLK_DEFAULT;
        }
        uart_param_config(CONSOLE_UART_PORT_NUM, &uart_config);
        uart_set_pin(CONSOLE_UART_PORT_NUM, CONSOLE_UART_PORT_PIN_TXD, CONSOLE_UART_PORT_PIN_RXD, CONSOLE_UART_PORT_PIN_RTS, CONSOLE_UART_PORT_PIN_CTS);
    }
}

static void  hconsole_init(const hruntime_function_t *func)
{
    hshell_command_name_shortcut_set(NULL,true);
    HSHELL_COMMANDS_REGISTER(NULL);
    ESP_LOGI(TAG,"init!");
}

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

int HPUTCHAR(int ch)
{
    if(uart_is_driver_installed(CONSOLE_UART_PORT_NUM))
    {
        uint8_t data=ch;
        if(sizeof(data)==uart_write_bytes(CONSOLE_UART_PORT_NUM,&data,sizeof(data)))
        {
            return ch;
        }
    }
    else
    {
        hconsole_driver_install();
    }
    return -1;
}

int HGETCHAR(void)
{
    if(uart_is_driver_installed(CONSOLE_UART_PORT_NUM))
    {
        uint8_t data=0;
        if(sizeof(data)==uart_read_bytes(CONSOLE_UART_PORT_NUM,&data,sizeof(data),0))
        {
            return data;
        }
    }
    else
    {
        hconsole_driver_install();
    }
    return -1;
}

#ifdef __cplusplus
}
#endif // __cplusplus

HRUNTIME_INIT_EXPORT(console,0xFFFF,hconsole_init,NULL);

static void hshell_task( void * pvParameters)
{
    while(true)
    {
        while(hshell_loop(NULL)==0);
        vTaskDelay(1);
    }
}

static TaskHandle_t hshell_task_handle=NULL;

static void  hconsole_loop(const hruntime_function_t *func)
{
    if(hdefaults_tick_get() > 3000)
    {
        /*
         * 启动时间大于一定时间后运行
         */
        if(hshell_task_handle==NULL)
        {
            xTaskCreate(hshell_task,"hshell",ESP_TASK_MAIN_STACK+4096,NULL,ESP_TASK_MAIN_PRIO,&hshell_task_handle);
        }
    }
}
HRUNTIME_LOOP_EXPORT(console,0xFFFF,hconsole_loop,NULL);


static int cmd_ps_entry(int argc,const char *argv[])
{
    hshell_context_t * hshell_ctx=hshell_context_get_from_main_argv(argc,argv);
    char strbuff[4096];
    strbuff[sizeof(strbuff)-1]='\0';
    vTaskList(strbuff);
    hshell_printf(hshell_ctx,"%s\r\n",strbuff);
    return 0;
};
HSHELL_COMMAND_EXPORT(ps,cmd_ps_entry,show process info);
