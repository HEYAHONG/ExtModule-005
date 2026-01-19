#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "esp_netif_sntp.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_check.h"
#include "sdkconfig.h"
#include "hbox.h"
#include "lwip/netif.h"

static const char *TAG = "eth";
/*
 * 以太网驱动句柄
 */
static  esp_eth_handle_t eth_handle=NULL;
/*
 * 以太网网络接口
 */
static esp_netif_t *eth_netif = NULL;


/**
 * @brief Initialize Ethernet driver with generic PHY (all IEEE 802.3 compliant PHYs)
 *
 *
 * @param[out] eth_handle_out initialized Ethernet driver handle
 * @return
 *          - ESP_OK on success
 *          - ESP_ERR_INVALID_ARG when passed invalid pointer
 *          - ESP_FAIL on any other failure
 */
static esp_err_t eth_init(esp_eth_handle_t *eth_handle_out)
{
    if (eth_handle_out == NULL)
    {
        ESP_LOGE(TAG, "invalid argument: eth_handle_out cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }

    // Init common MAC and PHY configs to default
    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();

    // Update PHY config based on board specific configuration
    phy_config.phy_addr = CONFIG_ETH_PHY_ADDR;
    phy_config.reset_gpio_num = CONFIG_ETH_PHY_RST_GPIO;
#if CONFIG_ETH_PHY_RST_TIMING_EN
    phy_config.hw_reset_assert_time_us = CONFIG_ETH_PHY_RST_ASSERT_TIME_US;
    phy_config.post_hw_reset_delay_ms = CONFIG_ETH_PHY_RST_DELAY_MS;
#endif // CONFIG_ETH_PHY_RST_TIMING_EN

    // Init vendor specific MAC config to default
    eth_esp32_emac_config_t esp32_emac_config = ETH_ESP32_EMAC_DEFAULT_CONFIG();
    // Update vendor specific MAC config based on board configuration
    esp32_emac_config.smi_gpio.mdc_num = CONFIG_ETH_MDC_GPIO;
    esp32_emac_config.smi_gpio.mdio_num = CONFIG_ETH_MDIO_GPIO;

#if CONFIG_ETH_PHY_INTERFACE_RMII
    // Configure RMII based on Kconfig when non-default configuration selected
    esp32_emac_config.interface = EMAC_DATA_INTERFACE_RMII;

    // Configure RMII clock mode and GPIO
#if CONFIG_ETH_RMII_CLK_INPUT
    esp32_emac_config.clock_config.rmii.clock_mode = EMAC_CLK_EXT_IN;
#else // CONFIG_ETH_RMII_CLK_OUTPUT
    esp32_emac_config.clock_config.rmii.clock_mode = EMAC_CLK_OUT;
#endif
    esp32_emac_config.clock_config.rmii.clock_gpio = CONFIG_ETH_RMII_CLK_GPIO;

#if CONFIG_ETH_RMII_CLK_EXT_LOOPBACK_EN
    esp32_emac_config.clock_config.rmii.clock_loopback_gpio = CONFIG_ETH_RMII_CLK_EXT_LOOPBACK_IN_GPIO;
#endif

#if SOC_EMAC_USE_MULTI_IO_MUX
    // Configure RMII datapane GPIOs
    esp32_emac_config.emac_dataif_gpio.rmii.tx_en_num = CONFIG_ETH_RMII_TX_EN_GPIO;
    esp32_emac_config.emac_dataif_gpio.rmii.txd0_num = CONFIG_ETH_RMII_TXD0_GPIO;
    esp32_emac_config.emac_dataif_gpio.rmii.txd1_num = CONFIG_ETH_RMII_TXD1_GPIO;
    esp32_emac_config.emac_dataif_gpio.rmii.crs_dv_num = CONFIG_ETH_RMII_CRS_DV_GPIO;
    esp32_emac_config.emac_dataif_gpio.rmii.rxd0_num = CONFIG_ETH_RMII_RXD0_GPIO;
    esp32_emac_config.emac_dataif_gpio.rmii.rxd1_num = CONFIG_ETH_RMII_RXD1_GPIO;
#endif // SOC_EMAC_USE_MULTI_IO_MUX
#endif // CONFIG_ETH_PHY_INTERFACE_RMII

    // Create new ESP32 Ethernet MAC instance
    esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&esp32_emac_config, &mac_config);
    if (mac == NULL)
    {
        ESP_LOGE(TAG, "create MAC instance failed");
        return ESP_FAIL;
    }

    // Create new generic PHY instance
    esp_eth_phy_t *phy = esp_eth_phy_new_generic(&phy_config);
    if (phy == NULL)
    {
        ESP_LOGE(TAG, "create PHY instance failed");
        mac->del(mac);
        return ESP_FAIL;
    }

    // Init Ethernet driver to default and install it
    esp_eth_handle_t eth_handle = NULL;
    esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
    if (esp_eth_driver_install(&config, &eth_handle) != ESP_OK)
    {
        ESP_LOGE(TAG, "Ethernet driver install failed");
        mac->del(mac);
        phy->del(phy);
        return ESP_FAIL;
    }

    *eth_handle_out = eth_handle;

    return ESP_OK;
}

/** Event handler for Ethernet events */
static void eth_event_handler(void *arg, esp_event_base_t event_base,int32_t event_id, void *event_data)
{
    uint8_t mac_addr[6] = {0};
    /* we can get the ethernet driver handle from event data */
    esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

    switch (event_id)
    {
    case ETHERNET_EVENT_CONNECTED:
        esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
        /*
         * 等待sntp
         */
        esp_netif_sntp_start();
        esp_netif_sntp_sync_wait(20);
        ESP_LOGI(TAG, "Ethernet Link Up");
        ESP_LOGI(TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x",
                 mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Ethernet Link Down");
        break;
    case ETHERNET_EVENT_START:
        ESP_LOGI(TAG, "Ethernet Started");
        break;
    case ETHERNET_EVENT_STOP:
        ESP_LOGI(TAG, "Ethernet Stopped");
        break;
    default:
        break;
    }
}

/** Event handler for IP_EVENT_ETH_GOT_IP */
static void got_ip_event_handler(void *arg, esp_event_base_t event_base,int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    const esp_netif_ip_info_t *ip_info = &event->ip_info;

    ESP_LOGI(TAG, "Ethernet Got IP Address");
    ESP_LOGI(TAG, "~~~~~~~~~~~");
    ESP_LOGI(TAG, "ETHIP:" IPSTR, IP2STR(&ip_info->ip));
    ESP_LOGI(TAG, "ETHMASK:" IPSTR, IP2STR(&ip_info->netmask));
    ESP_LOGI(TAG, "ETHGW:" IPSTR, IP2STR(&ip_info->gw));
    ESP_LOGI(TAG, "~~~~~~~~~~~");
}


static void heth_init(const hruntime_function * arg)
{
    // Initialize Ethernet driver
    ESP_ERROR_CHECK(eth_init(&eth_handle));

    // Initialize TCP/IP network interface aka the esp-netif (should be called only once in application)
    ESP_ERROR_CHECK(esp_netif_init());
    // Create default event loop that running in background
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    {
        /*
         * 设置ntp
         */
        const char *sntp_server_name[]=
        {
            "ntp.ntsc.ac.cn",
            "ntp.hyhsystem.cn",
            "0.cn.pool.ntp.org",
            "0.pool.ntp.org",
            "1.cn.pool.ntp.org",
            "1.pool.ntp.org",
            "2.cn.pool.ntp.org",
            "2.pool.ntp.org",
            "3.cn.pool.ntp.org",
            "3.pool.ntp.org",
        };
        esp_sntp_config_t sntp_config=ESP_NETIF_SNTP_DEFAULT_CONFIG(sntp_server_name[0]);
        for(size_t i=0; ((i < sizeof(sntp_server_name)/sizeof(sntp_server_name[0])) && (i < CONFIG_LWIP_SNTP_MAX_SERVERS)); i++)
        {
            sntp_config.servers[i]=sntp_server_name[i];
            sntp_config.num_of_servers=i+1;
        }

        sntp_config.sync_cb=[](struct timeval *tv)
        {
            if(tv!=NULL)
            {
                hsettimeofday_timeval_t new_tv;
                new_tv.tv_sec=tv->tv_sec;
                new_tv.tv_usec=tv->tv_usec;
                hsettimeofday(&new_tv,NULL);
            }
        };

        esp_netif_sntp_init(&sntp_config);
        /*
         * 启动sntp
         */
        esp_netif_sntp_start();
    }

    // Create instance of esp-netif for Ethernet
    // Use ESP_NETIF_DEFAULT_ETH when just one Ethernet interface is used and you don't need to modify
    // default esp-netif configuration parameters.
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
    eth_netif = esp_netif_new(&cfg);
    esp_eth_netif_glue_handle_t eth_netif_glue = esp_eth_new_netif_glue(eth_handle);
    // Attach Ethernet driver to TCP/IP stack
    ESP_ERROR_CHECK(esp_netif_attach(eth_netif, eth_netif_glue));

    // Register user defined event handlers
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event_handler, NULL));

    // Start Ethernet driver state machine
    ESP_ERROR_CHECK(esp_eth_start(eth_handle));

    /*
     * 等待以太网初始化
     */
    vTaskDelay(100);

    /*
     * 创建ipv6的linklocal地址,允许直接使用ipv6通信
     */
    esp_netif_create_ip6_linklocal(eth_netif);
}

HRUNTIME_INIT_EXPORT(eth,0,heth_init,NULL);
