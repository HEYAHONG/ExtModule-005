#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "console/console.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "services/ans/ble_svc_ans.h"
#include "esp_log.h"
#include "hbox.h"

/** GATT server. */
#define GATT_SVR_SVC_ALERT_UUID               0x1811
#define GATT_SVR_CHR_SUP_NEW_ALERT_CAT_UUID   0x2A47
#define GATT_SVR_CHR_NEW_ALERT                0x2A46
#define GATT_SVR_CHR_SUP_UNR_ALERT_CAT_UUID   0x2A48
#define GATT_SVR_CHR_UNR_ALERT_STAT_UUID      0x2A45
#define GATT_SVR_CHR_ALERT_NOT_CTRL_PT        0x2A44

static const char *TAG = "blemain";

static int blemain_gap_event(struct ble_gap_event *event, void *arg);
static uint8_t own_addr_type=0;
static void blemain_advertise(void)
{
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    const char *name=NULL;
    int rc=0;

    memset(&fields, 0, sizeof fields);

    /* Advertise two flags:
     *     o Discoverability in forthcoming advertisement (general)
     *     o BLE-only (BR/EDR unsupported).
     */
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;

    fields.uuids16 = (ble_uuid16_t[])
    {
        BLE_UUID16_INIT(GATT_SVR_SVC_ALERT_UUID)
    };
    fields.num_uuids16 = 1;
    fields.uuids16_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0)
    {
        /*
         * 设置广播数据失败
         */
        return;
    }

    memset(&adv_params, 0, sizeof adv_params);
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,&adv_params, blemain_gap_event, NULL);
    if (rc != 0)
    {
        /*
         * 启动广播失败
         */
        return;
    }
}

void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg)
{
    switch (ctxt->op)
    {
    case BLE_GATT_REGISTER_OP_SVC:
    {

    }
    break;
    case BLE_GATT_REGISTER_OP_CHR:
    {

    }
    break;
    case BLE_GATT_REGISTER_OP_DSC:
    {

    }
    break;
    default:
    {

    }
    break;
    }
}

static int blemain_gap_event(struct ble_gap_event *event, void *arg)
{
    struct ble_gap_conn_desc desc;
    int rc=0;
    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT:
    {
        if (event->connect.status == 0)
        {
            rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
            if(rc==0)
            {
                /*
                 * 连接成功
                 */
            }
        }
        if (event->connect.status != 0)
        {
            blemain_advertise();
        }
    }
    break;
    case BLE_GAP_EVENT_DISCONNECT:
    {
        blemain_advertise();
    }
    break;
    case BLE_GAP_EVENT_CONN_UPDATE:
    {
        rc = ble_gap_conn_find(event->conn_update.conn_handle, &desc);
        if(rc==0)
        {
            /*
             * 状态更新
             */
        }
    }
    break;
    case BLE_GAP_EVENT_ADV_COMPLETE:
    {
        blemain_advertise();
    }
    break;
    case BLE_GAP_EVENT_NOTIFY_TX:
    {

    }
    break;
    case BLE_GAP_EVENT_SUBSCRIBE:
    {

    }
    break;
    case BLE_GAP_EVENT_MTU:
    {

    }
    break;
    }

    return 0;
}

static void blemain_on_reset(int reason)
{

}


static void blemain_on_sync(void)
{
    int rc=0;

    rc = ble_hs_util_ensure_addr(0);

    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0)
    {
        return;
    }

    /*
     * 读取地址
     */
    uint8_t addr_val[6] = {0};
    rc = ble_hs_id_copy_addr(own_addr_type, addr_val, NULL);

    /*
     * 启动广播
     */
    blemain_advertise();

}

static void blemain_host_task(void *param)
{
    nimble_port_run();

    nimble_port_freertos_deinit();
}


static void  hblemain_init(const hruntime_function_t *func)
{
    ESP_LOGI(TAG, "init!");
    if(nimble_port_init()!= ESP_OK)
    {
        ESP_LOGI(TAG,"nimble_port_init failed!");
        return;
    }
    ble_hs_cfg.reset_cb = blemain_on_reset;
    ble_hs_cfg.sync_cb = blemain_on_sync;
    ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    /*
     * 初始化服务
     */
    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_svc_ans_init();

    /*
     * 设定设备名称服务
     */
    ble_svc_gap_device_name_set("EM005PRPH");
    /*
     * 初始化任务
     */
    nimble_port_freertos_init(blemain_host_task);
}
HRUNTIME_INIT_EXPORT(blemain,0,hblemain_init,NULL);

static void  hblemain_loop(const hruntime_function_t *func)
{

}
HRUNTIME_LOOP_EXPORT(blemain,0,hblemain_loop,NULL);

