/**************************************************************************
 *	[FUNCTION NAME]:
 *	        dal_system_setting.c
 *
 *	[DESCRIPTION]:
 *	        deal with the system settings
 *          
 **************************************************************************/
#include <dirent.h>
#include <sys/utsname.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <net/if.h>
#include <net/route.h>
#include <fcntl.h>
#include <unistd.h>
#include "include/dal_api.h"

#define buf_size 1024
#define DEFAULT_KEY    "1234567890"
typedef struct rc4_key
{
    unsigned char state[256];
    unsigned char x;
    unsigned char y;
}rc4_key;

static void swap_byte(unsigned char*x,unsigned char*y)
{
    *x=*x^*y;
    *y=*x^*y;
    *x=*x^*y;
}
static void prepare_key(unsigned char *key_data_ptr, int key_data_len, rc4_key *key)
{
    unsigned char index1;
    unsigned char index2;
    unsigned char* state;
    short counter;
    state = &key->state[0];
    for(counter = 0; counter < 256; counter++)
    state[counter] = counter;
    key->x = 0;
    key->y = 0;
    index1 = 0;
    index2 = 0;
    for(counter = 0; counter < 256; counter++)
    {
        index2 = (key_data_ptr[index1] + state[counter] + index2) % 256;
        swap_byte(&state[counter], &state[index2]);
        index1 = (index1 + 1) % key_data_len;
    }
}

static void rc4(unsigned char *buffer_ptr, int buffer_len, rc4_key *key)
{
    unsigned char x;
    unsigned char y;
    unsigned char* state;
    unsigned char xorIndex;
    short counter;
    x = key->x;
    y = key->y;
    state = &key->state[0];
    for(counter = 0; counter < buffer_len; counter++)
    {
        x = (x + 1) % 256;
        y = (state[x] + y) % 256;
        swap_byte(&state[x], &state[y]);
        xorIndex = (state[x] + state[y]) % 256;
        buffer_ptr[counter] ^= state[xorIndex];
    }
    key->x = x;
    key->y = y;
}

dal_ret_t* dal_sys_get_cfg_file()
{
      FILE *fp = NULL;
//    FILE *tmpfp;
    int num = 0;
    int count = 0;
    dal_ret_t* dal_rv = NULL;
    dal_rv = calloc(1, sizeof(dal_ret_t));
    dal_rv->param_num = 3;
    struct rc4_key *s = calloc(1, sizeof(rc4_key));

    dal_rv->param[LOCAL_CONFIG_FILE] = calloc(1,CONFIG_FILE_MAX_LENGTH);
    dal_rv->param[LOCAL_CONFIG_FILE_LENGTH] = calloc(1,64);
    dal_rv->param[LOCAL_CONFIG_FILE_NAME] = calloc(1,64);
    printf("start get cfgfile\n");
    fp = fopen(CUR_CFG_XML_FILE,"rb");
//    tmpfp = fopen("/tmp/tmpcfg","wb");
    if(fp)
    {
        int i = 0;
        count = 0;
        while(1)
        {
            num = fread(dal_rv->param[LOCAL_CONFIG_FILE]+i*CONFIG_ENC_LENGTH,1,CONFIG_ENC_LENGTH,fp);
            if(num>0)
            {
                memset(s,0,sizeof(rc4_key));
                prepare_key((unsigned char *)DEFAULT_KEY,10,s);
                rc4((unsigned char *)dal_rv->param[LOCAL_CONFIG_FILE]+i*CONFIG_ENC_LENGTH,num,s);
  //              fwrite(dal_rv->param[LOCAL_CONFIG_FILE]+i*CONFIG_ENC_LENGTH,1,num,tmpfp);
                i++;
                count += num;
            }
            else
                break;
        }
        if(i>0)
            dal_rv->ret = TSL_RV_SUC;  
    }
    else
    {
        /*CID 20514: Uninitialized scalar variable (UNINIT)
        If opening the cfg file fail, return here, or the count
        has no meaning even it is initialized*/
        /*CID 18876: Dereference after null check (FORWARD_NULL)*/
        dal_rv->ret = TSL_RV_FAIL;
        free(s);
        return dal_rv;
    }

    fclose(fp);
//    fclose(tmpfp);
    free(s);
    snprintf(dal_rv->param[LOCAL_CONFIG_FILE_LENGTH],64,"%d",count);
    snprintf(dal_rv->param[LOCAL_CONFIG_FILE_NAME], 64, CUR_CFG_XML_FILE);
    return dal_rv;
}

tsl_rv_t dal_sys_load_cfg_file(dal_ret_t* param)
{	
	// TODO
	return TSL_RV_SUC;	
}

tsl_rv_t dal_sys_reboot()  
{
	char cmd[64]={0};
	int rc;

	sprintf(cmd,"/etc/reboot.sh &");
	rc = system(cmd);

	return TSL_RV_SUC;
}



