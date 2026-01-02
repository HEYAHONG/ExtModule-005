#include "sysled.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "hbox.h"

static const char *TAG = "sysled";

/*
 * sysled0:GPIO2
 */
#define SYSLED_LED0_GPIO_NUM    GPIO_NUM_2
#define SYSLED_LED0_GPIO        (1ULL << ((size_t)SYSLED_LED0_GPIO_NUM))

static uint64_t sysled0_table=0x5555555555555555;
uint64_t sysled_get_table(uint8_t index)
{
    uint64_t ret=0;
    switch(index)
    {
    case 0:
    {
        ret= sysled0_table;
    }
    break;
    default:
    {

    }
    break;
    }
    return ret;
}

void sysled_set_table(uint8_t index,uint64_t table)
{
    switch(index)
    {
    case 0:
    {
        sysled0_table=table;
    }
    break;
    default:
    {

    }
    break;
    }
}


static uint8_t sysled_index=0;
static hdefaults_tick_t last_tick=0;
static void sysled_slot(void *param,void * signal)
{
    (void)param;
    if(hdefaults_get_api_table()->tick_get()-last_tick < 100)
    {
        //未到达100ms
        return;
    }
    else
    {
        last_tick=hdefaults_get_api_table()->tick_get();
    }
    gpio_set_level(SYSLED_LED0_GPIO_NUM,((sysled0_table&(1ULL << (sysled_index)))!=0)?1:0);
    sysled_index++;
    if(sysled_index > 63)
    {
        sysled_index=0;
    }
}

static void sysled_init(void)
{
    static bool init=false;
    if(init)
    {
        return;
    }
    init = true;
    {
        //zero-initialize the config structure.
        gpio_config_t io_conf = {};
        //disable interrupt
        io_conf.intr_type = GPIO_INTR_DISABLE;
        //set as output mode
        io_conf.mode = GPIO_MODE_OUTPUT;
        //bit mask of the pins that you want to set
        io_conf.pin_bit_mask = SYSLED_LED0_GPIO;
        //disable pull-down mode
        io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
        //disable pull-up mode
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        //configure GPIO with the given settings
        gpio_config(&io_conf);
    }
}


static void  hsysled_init(const hruntime_function_t *func)
{
    sysled_init();
    ESP_LOGI(TAG,"sysled init ok!");
}
HRUNTIME_INIT_EXPORT(sysled,0,hsysled_init,NULL);

static void  hsysled_loop(const hruntime_function_t *func)
{
    sysled_slot(NULL,NULL);
}
HRUNTIME_LOOP_EXPORT(sysled,0,hsysled_loop,NULL);

static int cmd_sysled(int argc,const char *argv[])
{
    hshell_context_t * hshell_ctx=hshell_context_get_from_main_argv(argc,argv);
    if(argc == 1)
    {
        //显示sysled表
        hshell_printf(hshell_ctx,"sysled:\r\n");
        hshell_printf(hshell_ctx,"\t0\tsysled0\r\n");
    }
    if(argc == 2)
    {
        //显示sysled表
        int syslednum=atoi(argv[1]);
        uint64_t sysled_table=sysled_get_table(syslednum);
        hshell_printf(hshell_ctx,"sysled%d table=%08X%08X\r\n",syslednum,(uint32_t)(sysled_table>>32),(uint32_t)sysled_table);
    }
    if(argc >= 3)
    {
        //设置sysled表
        int syslednum=atoi(argv[1]);
        uint64_t sysled_table=strtoull(argv[2],NULL,16);
        sysled_set_table(syslednum,sysled_table);
        hshell_printf(hshell_ctx,"sysled%d table=%08X%08X\r\n",syslednum,(uint32_t)(sysled_table>>32),(uint32_t)sysled_table);
    }
    return 0;
}

HSHELL_COMMAND_EXPORT(sysled,cmd_sysled,show sysled info\nsysled [syslednum]);

