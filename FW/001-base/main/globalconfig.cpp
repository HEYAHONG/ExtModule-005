#include "globalconfig.h"
#include "hbox.h"
#include "stdio.h"

static hcjson_t * global_config=NULL;
#define GLOBALCONFIG_FILE_PATH "/var/config.json"

static hcjson_t * load_global_config(void)
{
    if(global_config==NULL)
    {
        hdefaults_mutex_lock(NULL);

        if(global_config==NULL)
        {
            FILE* f = fopen(GLOBALCONFIG_FILE_PATH, "r");
            if (f != NULL)
            {
                fseek(f,0,SEEK_END);
                long file_size=ftell(f);
                if(file_size > 0)
                {
                    fseek(f,0,SEEK_SET);
                    char * buff=(char *)malloc(file_size+1);
                    memset(buff,0,file_size+1);
                    if(buff !=NULL)
                    {
                        fread(buff,1,file_size,f);
                        global_config=hcjson_parse(buff);
                        free(buff);
                    }
                }
                fclose(f);
            }
        }

        if(global_config==NULL)
        {
            global_config=hcjson_create_object();
        }


        hdefaults_mutex_unlock(NULL);
    }
    return global_config;
}

static void save_global_config(void)
{
    if(global_config!=NULL)
    {
        hdefaults_mutex_lock(NULL);

        char * global_config_string=hcjson_print(global_config);
        if(global_config_string!=NULL)
        {
            FILE* f = fopen(GLOBALCONFIG_FILE_PATH, "w");
            if (f != NULL)
            {
                fwrite(global_config_string,1,hstrlen(global_config_string),f);
                fclose(f);
            }
            hcjson_free(global_config_string);
        }
        hcjson_delete(global_config);
        global_config=NULL;

        hdefaults_mutex_unlock(NULL);
    }
}

static int globalconfig_reference_count=0;
hcjson_t * globalconfig_load(void)
{
    hdefaults_mutex_lock(NULL);
    globalconfig_reference_count++;
    hdefaults_mutex_unlock(NULL);
    return load_global_config();
}

void globalconfig_save(void)
{
    hdefaults_mutex_lock(NULL);
    globalconfig_reference_count--;
    if(globalconfig_reference_count==0)
    {
        save_global_config();
    }
    hdefaults_mutex_unlock(NULL);
}

static int cmd_globalconfig_entry(int argc,const char *argv[])
{
    hshell_context_t * hshell_ctx=hshell_context_get_from_main_argv(argc,argv);
    if(argc<=1)
    {
        hshell_printf(hshell_ctx,"%s\t[command]:\r\n",argv[0]!=NULL?argv[0]:"ota");
        hshell_printf(hshell_ctx,"\tdump\t\tdump globalconfig\r\n");
    }
    else
    {
        if(argv[1]!=NULL && strcmp(argv[1],"dump")==0)
        {
            hcjson_t * config=globalconfig_load();
            if(config!=NULL)
            {
                char *str=hcjson_print(config);
                if(str!=NULL)
                {
                    hshell_printf(hshell_ctx,"%s\r\n",str);
                    hcjson_free(str);
                }
                globalconfig_save();
            }
        }
    }
    return 0;
};
HSHELL_COMMAND_EXPORT(globalconfig,cmd_globalconfig_entry,globalconfig command);


