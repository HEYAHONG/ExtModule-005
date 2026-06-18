#include <nuttx/config.h>

#include <errno.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include "hbox.h"
#include "hrc.h"
#include "pthread.h"
#include "syslog.h"
#include "builtin/builtin.h"
extern "C" int main(int argc, FAR char *argv[]);
static bool g_hbox_init=false;
int main(int argc, FAR char *argv[])
{
    {
        if(g_hbox_init)
        {
            /*
             * 只允许使用一个实例
             */
            syslog(LOG_INFO,"exiting!\r\n");
            return -1;
        }
        g_hbox_init=true;
        atexit([]() {
            g_hbox_init=false;
        });
    }
    {
        const char *banner="";
        if(RCGetHandle("banner")!=NULL)
        {
            banner=(const char *)RCGetHandle("banner");
        }
        syslog(LOG_INFO,"\r\nhbox init!\r\n%s",banner);
    }
    hcpprt_init();
    while(true)
    {
        hcpprt_loop();
        hthrd_yield();
    }
    return 0;
}


/*
 * 启动nsh
 */
static void  nsh_init(const hruntime_function_t *func)
{
    syslog(LOG_INFO,"nsh init!");
    /*
     * 执行nsh
     */
    if(exec_builtin("nsh",0,NULL,NULL) < 0)
    {
        syslog(LOG_INFO,"nsh init failed!\r\n");
    }
}
HRUNTIME_INIT_EXPORT(nsh,255,nsh_init,NULL);

