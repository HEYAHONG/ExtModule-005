#include "host/ble_hs.h"
#include "host/util/util.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "host/ble_att.h"
#include "services/gatt/ble_svc_gatt.h"
#include "esp_task.h"
#include "hbox.h"

static const char *TAG = "blehshellservice";

/*
 * HShell UUID(UUID V3,命名空间为OID,名称为HShell,a3be563b-b374-3e72-98e7-ba70753dcad8
 */
static const ble_uuid128_t hshellservice_service_uuid= BLE_UUID128_INIT(0xd8,0xca,0x3d,0x75,0x70,0xba,0xe7,0x98,0x72,0x3e,0x74,0xb3,0x3b,0x56,0xbe,0xa3);

/*
* HShell 特征UUID(UUID V3,命名空间为OID,名称为HShell.IO,7580782a-4ae0-3851-90a9-9857803461e1
*/
static const ble_uuid128_t hshellservice_characteristic_uuid= BLE_UUID128_INIT(0xe1,0x61,0x34,0x80,0x57,0x98,0xa9,0x90,0x51,0x38,0xe0,0x4a,0x2a,0x78,0x80,0x75);

/*
* HShell 特征UUID描述符(UUID V3,命名空间为OID,名称为HShell.IO.Desc,b4b00b97-81a3-35a6-9a33-9b058695039b
*/
static const ble_uuid128_t hshellservice_characteristic_descriptor_uuid= BLE_UUID128_INIT(0x9b,0x03,0x95,0x86,0x05,0x9b,0x33,0x9a,0xa6,0x35,0xa3,0x81,0x97,0xb,0xb0,0xb4);

static int gatt_svc_access(uint16_t conn_handle, uint16_t attr_handle,struct ble_gatt_access_ctxt *ctxt,void *arg);

static uint16_t gatt_svr_chr_val_handle=0;
static uint8_t gatt_svr_dsc_val=0x99;

struct ble_gatt_dsc_def hshellservice_characteristic_descriptors[]=
{
    {
        .uuid = &hshellservice_characteristic_descriptor_uuid.u,
        .att_flags = BLE_ATT_F_READ,
        .access_cb = gatt_svc_access,
    },
    {
        0,
    }
};

struct ble_gatt_chr_def hshellservice_characteristics[]=
{
    {
        .uuid = &hshellservice_characteristic_uuid.u,
        .access_cb = gatt_svc_access,
        .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_NOTIFY,
        .val_handle = &gatt_svr_chr_val_handle,
        .descriptors = hshellservice_characteristic_descriptors,
    },
    {
        0,
    }
};

const struct ble_gatt_svc_def gatt_svr_svcs[] =
{
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &hshellservice_service_uuid.u,
        .characteristics = hshellservice_characteristics,
    },
    {
        0,
    },
};

static uint64_t hshell_rx_buffer[2048/sizeof(uint64_t)]= {0};
static uint64_t hshell_tx_buffer[2048/sizeof(uint64_t)]= {0};
static int gatt_svc_access(uint16_t conn_handle, uint16_t attr_handle,struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    switch (ctxt->op)
    {
    case BLE_GATT_ACCESS_OP_READ_CHR:
        if (attr_handle == gatt_svr_chr_val_handle)
        {
            hringbuf_t * buffer=hringbuf_get((uint8_t *)hshell_tx_buffer,sizeof(hshell_tx_buffer));
            if(hringbuf_get_length(buffer))
            {
                uint8_t data_buffer[CONFIG_NIMBLE_ATT_PREFERRED_MTU]={0};
                size_t  len=hringbuf_get_length(buffer);
                {
                    size_t len_to_read=sizeof(data_buffer);
                    if(len_to_read > len)
                    {
                        len_to_read=len;
                    }
                    hringbuf_output(buffer,data_buffer,len_to_read);
                    os_mbuf_append(ctxt->om,data_buffer,len_to_read);
                }
                return 0;
            }
        }
        goto unknown;
    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        if (attr_handle == gatt_svr_chr_val_handle)
        {
            hringbuf_t * buffer=hringbuf_get((uint8_t *)hshell_rx_buffer,sizeof(hshell_rx_buffer));
            uint16_t om_len = OS_MBUF_PKTLEN(ctxt->om);
            for(size_t i=0;i<om_len;i++)
            {
                uint8_t data=0;
                os_mbuf_copydata(ctxt->om,i,sizeof(data),&data);
                hringbuf_input(buffer,&data,sizeof(data));
            }
            ble_gatts_chr_updated(attr_handle);
            return 0;
        }
        goto unknown;

    case BLE_GATT_ACCESS_OP_READ_DSC:
        {
            if (ble_uuid_cmp( ctxt->dsc->uuid, &hshellservice_characteristic_descriptor_uuid.u) == 0)
            {
                int rc = os_mbuf_append(ctxt->om,&gatt_svr_dsc_val,sizeof(gatt_svr_dsc_val));
                return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
            }
        }
        goto unknown;

    case BLE_GATT_ACCESS_OP_WRITE_DSC:
        goto unknown;

    default:
        goto unknown;
    }

unknown:
    return BLE_ATT_ERR_UNLIKELY;
}

/*
 * shell相关变量
 */
static int hshell_putchar(int ch)
{
    if(ch>0)
    {
        hringbuf_t * buffer=hringbuf_get((uint8_t *)hshell_tx_buffer,sizeof(hshell_tx_buffer));
        {
            uint8_t data=(uint8_t)ch;
            hringbuf_input(buffer,(const uint8_t *)&data,sizeof(data));
        }
    }
    return ch;

}


static int hshell_getchar(void)
{
    int ch=EOF;
    {
        hringbuf_t * buffer=hringbuf_get((uint8_t *)hshell_rx_buffer,sizeof(hshell_rx_buffer));
        if(hringbuf_get_length(buffer))
        {
            uint8_t ch_val=0;
            hringbuf_output(buffer,&ch_val,sizeof(ch_val));
            ch=ch_val;
        }
    }
    return ch;
}

static hshell_context_t ctx;
static void hshell_init(void)
{
    memset(&ctx,0,sizeof(ctx));
    hshell_context_external_api_t api=hshell_context_default_external_api();
    api.getchar=hshell_getchar;
    api.putchar=hshell_putchar;
    hshell_external_api_set(&ctx,api);
    hshell_command_name_shortcut_set(&ctx,true);
    HSHELL_COMMANDS_REGISTER(&ctx);	//注册命令
    hshell_echo_set(&ctx,false);//关闭回显
}

static void hshell_task( void * pvParameters)
{
    while(true)
    {
        size_t cnt=0;
        while(hshell_loop(&ctx)==0)
        {
            cnt++;
        }
        if(cnt==0)
        {
            hringbuf_t * buffer=hringbuf_get((uint8_t *)hshell_tx_buffer,sizeof(hshell_tx_buffer));
            if(hringbuf_get_length(buffer))
            {
                /*
                 * 通知剩余数据
                 */
                if(gatt_svr_chr_val_handle!=0)
                {
                    ble_gatts_chr_updated(gatt_svr_chr_val_handle);
                }
                vTaskDelay(1);
            }
        }
        vTaskDelay(1);
    }
}


static void  hblehshellservice_init(const hruntime_function_t *func)
{
     if(0==ble_gatts_count_cfg(gatt_svr_svcs) && 0==ble_gatts_add_svcs(gatt_svr_svcs))
     {
         ESP_LOGI(TAG, "init!");
     }
     hshell_init();
}
HRUNTIME_INIT_EXPORT(blehshellservice,255,hblehshellservice_init,NULL);
static TaskHandle_t hshell_task_handle=NULL;
static void  hblehshellservice_loop(const hruntime_function_t *func)
{
    if(hshell_task_handle==NULL)
    {
        xTaskCreate(hshell_task,"blehshell",ESP_TASK_MAIN_STACK,NULL,ESP_TASK_MAIN_PRIO,&hshell_task_handle);
    }
}
HRUNTIME_LOOP_EXPORT(blehshellservice,255,hblehshellservice_loop,NULL);
