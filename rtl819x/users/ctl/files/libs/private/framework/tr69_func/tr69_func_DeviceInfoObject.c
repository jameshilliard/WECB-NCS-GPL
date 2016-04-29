/*./auto/tr69_func_DeviceInfoObject.c*/
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <time.h>
#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"
#include "ctl.h"
#include "tr69_cms_object.h"
#include "tsl_strconv.h"


#include "ctl_validstrings.h"
#include "libtr69_func.h"
#include "tsl_msg.h"

#define FIRSTUSE_FILE_NAME	"/mnt/rt_conf/firstuse"

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_DeviceInfoObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.DeviceInfo
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_DeviceInfoObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_DeviceInfoObject_value(tsl_char_t *p_oid_name, _DeviceInfoObject *p_cur_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;
	struct timespec ts;
	
	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

	clock_gettime(CLOCK_MONOTONIC, &ts);
	p_cur_data->upTime = ts.tv_sec;

	if (tsl_strcmp(p_cur_data->firstUseDate,"0001-01-01T00:00:00Z")==0) 	{
		FILE *fp=NULL;
		char line[128] = {0};

		if ((fp=fopen(FIRSTUSE_FILE_NAME,"r"))!=NULL) {
			if (fgets(line,sizeof(line),fp) != NULL)
			{
				CTLMEM_REPLACE_STRING(p_cur_data->firstUseDate, line);
			}

			fclose(fp);
		}
	}

	return rv;
}


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_set_DeviceInfoObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.DeviceInfo
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_DeviceInfoObject_t *p_cur_data
 *	        st_DeviceInfoObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_DeviceInfoObject_value(tsl_char_t *p_oid_name, _DeviceInfoObject *p_cur_data, _DeviceInfoObject *p_new_data)
{
	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_CHANGED;
    tsl_char_t strHostname[BUFLEN_32] = {0};
#if defined(AEI_CRISTAL_ACCESS)
    tsl_int_t snLen = 0;
#endif
    tsl_char_t cmd[BUFLEN_128] = {0};
    tsl_bool b_trigger = TSL_B_TRUE;
	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

	if (p_new_data != NULL && p_cur_data == NULL) {
		FILE *pfp = NULL;
		char pipeline[128]={0};
		char *pos = NULL;
		char sn[32]={0};

		if ((pfp = popen("flash get MP_SERIAL_NUM", "r"))!=NULL) {
			if (fgets(pipeline, sizeof(pipeline)-1, pfp)) {
				pos = strstr(pipeline,"MP_SERIAL_NUM=\"");
				if (pos)
				{
					snprintf(sn, sizeof(sn),"%s", pos+strlen("MP_SERIAL_NUM=\""));
					if (strlen(sn)>1)
						sn[strlen(sn)-2]='\0';
				}
			}
			pclose(pfp);
		}

		if (strlen(sn) > 0) {
			CTLMEM_REPLACE_STRING(p_new_data->serialNumber, sn);
#if defined(AEI_CRISTAL_ACCESS)
                snLen = tsl_strlen( sn );
                if( snLen >= 4 ) {
                    snprintf( strHostname, sizeof(strHostname), "WECB-%c%c%c%c",
                                sn[snLen-4], sn[snLen-3],
                                sn[snLen-2], sn[snLen-1] );
                } else {
                    snprintf( strHostname, sizeof(strHostname), "WECB" );
                }

                CTLMEM_REPLACE_STRING(p_new_data->X_ACTIONTEC_COM_HostName, strHostname);
                snprintf(cmd,sizeof(cmd),"hostname %s",strHostname);
                system(cmd);
#endif
		} else {
			if ((pfp = popen("flash get HW_NIC0_ADDR", "r"))!=NULL) {
				if (fgets(pipeline, sizeof(pipeline)-1, pfp)) {
					pos = strstr(pipeline,"HW_NIC0_ADDR=");
					if (pos)
					{
						snprintf(sn, sizeof(sn),"SN%s", pos+strlen("HW_NIC0_ADDR="));
						if (strlen(sn)>1)
							sn[strlen(sn)-1]='\0';
					}
				}
				pclose(pfp);
			}

			CTLMEM_REPLACE_STRING(p_new_data->serialNumber, sn);
#if defined(AEI_CRISTAL_ACCESS)
                    snLen = tsl_strlen( sn );
                    if( snLen >= 4 ) {
                        snprintf( strHostname, sizeof(strHostname), "WECB-%c%c%c%c",
                                    sn[snLen-4], sn[snLen-3],
                                    sn[snLen-2], sn[snLen-1] );
                    } else {
                        snprintf( strHostname, sizeof(strHostname), "WECB" );
                    }

                    CTLMEM_REPLACE_STRING(p_new_data->X_ACTIONTEC_COM_HostName, strHostname);
                    snprintf(cmd,sizeof(cmd),"hostname %s",strHostname);
                    system(cmd);
#endif
		}
		
		CTLMEM_REPLACE_STRING(p_new_data->softwareVersion, SOFTWARE_VERSION);

		if (tsl_strcmp(p_new_data->firstUseDate,"0001-01-01T00:00:00Z")==0) {
			FILE *fp=NULL;
			char line[128] = {0};

			if ((fp=fopen(FIRSTUSE_FILE_NAME,"r"))!=NULL) {
				if (fgets(line,sizeof(line),fp) != NULL)
				{
					CTLMEM_REPLACE_STRING(p_new_data->firstUseDate, line);
				}

				fclose(fp);
			}
		}
	}
#if defined(AEI_CRISTAL_ACCESS)
       else if(p_new_data && p_cur_data)
        {
            if(tsl_strlen(p_new_data->X_ACTIONTEC_COM_HostName) >0 
                && tsl_strcmp(p_new_data->X_ACTIONTEC_COM_HostName, p_cur_data->X_ACTIONTEC_COM_HostName))
            {
                    snprintf(cmd,sizeof(cmd),"hostname %s",p_new_data->X_ACTIONTEC_COM_HostName);
                    system(cmd);
    
                    tr69_set_leaf_data( "Device.X_ACTIONTEC_COM_MDNS.X_ACTIONTEC_COM_Trigger",
                            (void *)&b_trigger, TR69_NODE_LEAF_TYPE_BOOL);
            }
        }
#endif
	return rv;
}

