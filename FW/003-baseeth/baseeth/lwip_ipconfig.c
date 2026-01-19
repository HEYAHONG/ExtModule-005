#include "hbox.h"
#include "lwip/init.h"
#include "lwip/timeouts.h"
#include "netif/ethernet.h"
#include "netif/etharp.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "lwip/dhcp6.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/ethip6.h"
#include "lwip/apps/sntp.h"

static void netif_flags_to_string(uint32_t flags,char *flags_string,size_t max_length)
{
    if(flags_string==NULL || max_length == 0)
    {
        return;
    }
    flags_string[0]='\0';
    if((flags&NETIF_FLAG_LINK_UP)!=0)
    {
        const char * str="RUNNING";
        if(strlen(str)+strlen(flags_string) < max_length)
        {
            strcat(flags_string,str);
        }
    }
    if((flags&NETIF_FLAG_UP)!=0)
    {
        const char * str="UP";
        if(strlen(str)+strlen(flags_string) < (max_length-(strlen(flags_string)>0?1:0)))
        {
            if(strlen(flags_string)>0)
            {
                strcat(flags_string,",");
            }
            strcat(flags_string,str);
        }
    }
    if((flags&NETIF_FLAG_BROADCAST)!=0)
    {
        const char * str="BROADCAST";
        if(strlen(str)+strlen(flags_string) < (max_length-(strlen(flags_string)>0?1:0)))
        {
            if(strlen(flags_string)>0)
            {
                strcat(flags_string,",");
            }
            strcat(flags_string,str);
        }
    }
    if((flags&NETIF_FLAG_ETHARP)!=0)
    {
        const char * str="ETHARP";
        if(strlen(str)+strlen(flags_string) < (max_length-(strlen(flags_string)>0?1:0)))
        {
            if(strlen(flags_string)>0)
            {
                strcat(flags_string,",");
            }
            strcat(flags_string,str);
        }
    }
    if((flags&NETIF_FLAG_ETHERNET)!=0)
    {
        const char * str="ETHERNET";
        if(strlen(str)+strlen(flags_string) < (max_length-(strlen(flags_string)>0?1:0)))
        {
            if(strlen(flags_string)>0)
            {
                strcat(flags_string,",");
            }
            strcat(flags_string,str);
        }
    }
    if((flags&NETIF_FLAG_IGMP)!=0)
    {
        const char * str="IGMP";
        if(strlen(str)+strlen(flags_string) < (max_length-(strlen(flags_string)>0?1:0)))
        {
            if(strlen(flags_string)>0)
            {
                strcat(flags_string,",");
            }
            strcat(flags_string,str);
        }
    }
    if((flags&NETIF_FLAG_MLD6)!=0)
    {
        const char * str="MLD6";
        if(strlen(str)+strlen(flags_string) < (max_length-(strlen(flags_string)>0?1:0)))
        {
            if(strlen(flags_string)>0)
            {
                strcat(flags_string,",");
            }
            strcat(flags_string,str);
        }
    }
}

static int cmd_ifconfig_entry(int argc,const char *argv[])
{
    hshell_context_t * hshell_ctx=hshell_context_get_from_main_argv(argc,argv);
    if(argc <= 1)
    {
        hshell_printf(hshell_ctx,"usage: ifconfig interface options\r\n");
        for(uint8_t index=1; index < 254; index++)
        {
            struct netif *interface=netif_get_by_index(index);
            if(interface!=NULL)
            {
                {
                    char ifname[NETIF_NAMESIZE]= {0};
                    netif_index_to_name(index,ifname);
                    char flags_string[96]= {0};
                    netif_flags_to_string(interface->flags,flags_string,sizeof(flags_string)-1);
#if LWIP_IPV6 && LWIP_ND6_ALLOW_RA_UPDATES
                    hshell_printf(hshell_ctx,"%s: flags=%d<%s> mtu %d mtu6 %d\r\n",ifname,(int)interface->flags,flags_string,(int)interface->mtu,(int)interface->mtu6);
#else
                    hshell_printf(hshell_ctx,"%s: flags=%d<%s> mtu %d\r\n",ifname,(int)interface->flags,flags_string,(int)interface->mtu);
#endif
                }
#if             LWIP_IPV4
                {
                    char ip_str[32]= {0};
                    ipaddr_ntoa_r(&interface->ip_addr,ip_str,sizeof(ip_str));
                    char gw_str[32]= {0};
                    ipaddr_ntoa_r(&interface->gw,gw_str,sizeof(gw_str));
                    char netmask_str[32]= {0};
                    ipaddr_ntoa_r(&interface->netmask,netmask_str,sizeof(netmask_str));
                    hshell_printf(hshell_ctx,"\tinet %s netmask %s gateway %s\r\n",ip_str,netmask_str,gw_str);
                }
#endif
#if             LWIP_IPV6
                {
                    for(size_t i=0; i<LWIP_IPV6_NUM_ADDRESSES; i++)
                    {
                        if(ip6_addr_isvalid(netif_ip6_addr_state(interface,i)))
                        {
                            {
                                char addr_str[96]= {0};
                                ipaddr_ntoa_r(&interface->ip6_addr[i],addr_str,sizeof(addr_str));
                                hshell_printf(hshell_ctx,"\tinet6 %s \r\n",addr_str);
                            }
                        }
                    }
                }
#endif
                hshell_printf(hshell_ctx,"\tether %02X:%02X:%02X:%02X:%02X:%02X \r\n",(int)interface->hwaddr[0],(int)interface->hwaddr[1],(int)interface->hwaddr[2],(int)interface->hwaddr[3],(int)interface->hwaddr[4],(int)interface->hwaddr[5]);
            }
        }
    }
    else
    {
        {
            struct netif *interface=netif_find(argv[1]);
            if(interface!=NULL)
            {
                {
                    char ifname[NETIF_NAMESIZE]= {0};
                    netif_index_to_name(netif_get_index(interface),ifname);
                    char flags_string[96]= {0};
                    netif_flags_to_string(interface->flags,flags_string,sizeof(flags_string)-1);
#if LWIP_IPV6 && LWIP_ND6_ALLOW_RA_UPDATES
                    hshell_printf(hshell_ctx,"%s: flags=%d<%s> mtu %d mtu6 %d\r\n",ifname,(int)interface->flags,flags_string,(int)interface->mtu,(int)interface->mtu6);
#else
                    hshell_printf(hshell_ctx,"%s: flags=%d<%s> mtu %d\r\n",ifname,(int)interface->flags,flags_string,(int)interface->mtu);
#endif
                }
#if             LWIP_IPV4
                {
                    char ip_str[32]= {0};
                    ipaddr_ntoa_r(&interface->ip_addr,ip_str,sizeof(ip_str));
                    char gw_str[32]= {0};
                    ipaddr_ntoa_r(&interface->gw,gw_str,sizeof(gw_str));
                    char netmask_str[32]= {0};
                    ipaddr_ntoa_r(&interface->netmask,netmask_str,sizeof(netmask_str));
                    hshell_printf(hshell_ctx,"\tinet %s netmask %s gateway %s\r\n",ip_str,netmask_str,gw_str);
                }
#endif
#if             LWIP_IPV6
                {
                    for(size_t i=0; i<LWIP_IPV6_NUM_ADDRESSES; i++)
                    {
                        if(ip6_addr_isvalid(netif_ip6_addr_state(interface,i)))
                        {
                            {
                                char addr_str[96]= {0};
                                ipaddr_ntoa_r(&interface->ip6_addr[i],addr_str,sizeof(addr_str));
                                hshell_printf(hshell_ctx,"\tinet6 %s \r\n",addr_str);
                            }
                        }
                    }
                }
#endif
                hshell_printf(hshell_ctx,"\tether %02X:%02X:%02X:%02X:%02X:%02X \r\n",(int)interface->hwaddr[0],(int)interface->hwaddr[1],(int)interface->hwaddr[2],(int)interface->hwaddr[3],(int)interface->hwaddr[4],(int)interface->hwaddr[5]);
            }
            else
            {
                hshell_printf(hshell_ctx,"No such device %s\r\n",argv[1]);
            }
        }

    }
    return 0;
};
HSHELL_COMMAND_EXPORT(ifconfig,cmd_ifconfig_entry,configure a network interface);
