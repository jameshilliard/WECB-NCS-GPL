/*./auto/tr69_func_TimeServerCfgObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"
#include "tr69_cms_object.h"

#include "tr69_func.h"
#include "tsl_strconv.h"
#include "ctl_tms.h"
#include "ctl.h"
#include "tsl_msg.h"
#include "act_intf_stack.h"

tsl_bool_t tr69_util_sntp_is_changed(const _TimeServerCfgObject *p_new_data, const _TimeServerCfgObject *p_cur_data)
{
    tsl_bool_t ret = TSL_B_FALSE;

    if (tsl_strcmp(p_new_data->NTPServer1, p_cur_data->NTPServer1) ||
            tsl_strcmp(p_new_data->NTPServer2, p_cur_data->NTPServer2) ||
            tsl_strcmp(p_new_data->NTPServer3, p_cur_data->NTPServer3) ||
            tsl_strcmp(p_new_data->NTPServer4, p_cur_data->NTPServer4) ||
            tsl_strcmp(p_new_data->NTPServer5, p_cur_data->NTPServer5) ||
            tsl_strcmp(p_new_data->localTimeZone, p_cur_data->localTimeZone) ||
            (p_new_data->X_ACTIONTEC_COM_ForceUpdateFrequency != p_cur_data->X_ACTIONTEC_COM_ForceUpdateFrequency) ||
            (p_new_data->X_ACTIONTEC_COM_MinUpdateFrequency != p_cur_data->X_ACTIONTEC_COM_MinUpdateFrequency)
       )

    {
        ret = TSL_B_TRUE;
    }

    return (ret);
}

tsl_void_t stop_ntp()
{
      DO_SYSTEM("killall -9 ntpclient > /dev/null 2>&1");
}

tsl_void_t start_ntp(tsl_char_t *p_oid_name, _TimeServerCfgObject *p_cur_data,_TimeServerCfgObject *p_new_data)
{
    char servers[BUFLEN_1024] = {0};
    sprintf(servers, "-p %s", p_new_data->NTPServer1);
    if(!IS_EMPTY_STRING(p_new_data->NTPServer2 ))
    {
        strcat(servers, " -p " );
        strcat(servers, p_new_data->NTPServer2);
    }
    if(!IS_EMPTY_STRING(p_new_data->NTPServer3) )
    {
        strcat(servers, " -p " );
        strcat(servers, p_new_data->NTPServer3);
    }
    if(!IS_EMPTY_STRING(p_new_data->NTPServer4) )
    {
        strcat(servers, " -p " );
        strcat(servers, p_new_data->NTPServer4);
    }
    if(!IS_EMPTY_STRING(p_new_data->NTPServer5) )
    {
        strcat(servers, " -p " );
        strcat(servers, p_new_data->NTPServer5);
    }


    CTLMEM_REPLACE_STRING(p_new_data->status, CTLVS_UNSYNCHRONIZED);

    if(p_new_data->localTimeZone != NULL && p_new_data->localTimeZone[0] != '\0' )
    {
         DO_SYSTEM("/sbin/ntpclient -n -t \"%s\" -f %d -m %d %s &", p_new_data->localTimeZone,
         p_new_data->X_ACTIONTEC_COM_ForceUpdateFrequency,p_new_data->X_ACTIONTEC_COM_MinUpdateFrequency,servers);
    }
    else
    {
         DO_SYSTEM("/sbin/ntpclient -n %s &", servers);
    }
    ctllog_debug("start ntpclient ok\n");
    }

/**************************************************************************
 *  [FUNCTION NAME]:
 *          tf69_func_get_TimeServerCfgObject_value
 *
 *  [DESCRIPTION]:
 *          Device.Time
 *
 *  [PARAMETER]:
 *          tsl_char_t *p_oid_name
 *          st_TimeServerCfgObject_t *p_cur_data
 *
 *  [RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_TimeServerCfgObject_value(tsl_char_t *p_oid_name, _TimeServerCfgObject *p_cur_data)
{
    tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;

    ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

    char dateTimeBuf[BUFLEN_64];
    ctl_timer_get_time(0, dateTimeBuf, sizeof(dateTimeBuf));

    if (p_cur_data)
    {
        CTLMEM_REPLACE_STRING(p_cur_data->currentLocalTime, dateTimeBuf);
        rv = TR69_RT_SUCCESS_VALUE_CHANGED;
    }

    return rv;
}


/**************************************************************************
 *  [FUNCTION NAME]:
 *          tf69_func_set_TimeServerCfgObject_value
 *
 *  [DESCRIPTION]:
 *          Device.Time
 *
 *  [PARAMETER]:
 *          tsl_char_t *p_oid_name
 *          st_TimeServerCfgObject_t *p_cur_data
 *          st_TimeServerCfgObject_t *p_new_data
 *
 *  [RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_TimeServerCfgObject_value(tsl_char_t *p_oid_name, _TimeServerCfgObject *p_cur_data, _TimeServerCfgObject *p_new_data)
{
    tsl_int_t rv = TR69_RT_SUCCESS_VALUE_CHANGED;
    tsl_char_t * interface_status = NULL;
    tsl_int_t type;
    tsl_int_t interface_upflag=0;
    ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);
    if((p_cur_data==NULL)&&(p_new_data!=NULL))
    {
         CTLMEM_REPLACE_STRING(p_new_data->status, CTLVS_DISABLED);
    }
    /* add and enable sntp, or enable existing sntp */
    do{
        if( tr69_get_unfresh_leaf_data("Device.IP.Interface.1.X_ACTIONTEC_COM_InternetStatus", (void **) &interface_status, &type) < 0) {
          break;
          }
          if(tsl_strcmp(interface_status,CTLVS_UP)==0)
          {
              interface_upflag=1;
          }
          CTLMEM_FREE_BUF_AND_NULL_PTR( interface_status );
    }while(0);
    if (ENABLE_NEW_OR_ENABLE_EXISTING(p_new_data, p_cur_data))
    {
        CTLMEM_REPLACE_STRING(p_new_data->status, CTLVS_UNSYNCHRONIZED);
        if(interface_upflag)
        {
            if(!IS_EMPTY_STRING(p_new_data->NTPServer1))
            {
               start_ntp(p_oid_name,p_cur_data,p_new_data);
            }
        }

    }
    /* edit existing sntp */
    else if (POTENTIAL_CHANGE_OF_EXISTING(p_new_data, p_cur_data))
    {
        if((tr69_util_sntp_is_changed(p_new_data, p_cur_data) == TSL_B_TRUE)|| (IS_TRIGGERED_BY_LOWER_LAYER(p_new_data)))
        {
            if(!IS_EMPTY_STRING(p_new_data->NTPServer1))
            {
                stop_ntp();
                start_ntp(p_oid_name, p_cur_data,  p_new_data);
            }
        }
#ifdef AEI_TR69
        else if((!tsl_strcmp(p_new_data->status,CTLVS_SYNCHRONIZED))||(!tsl_strcmp(p_new_data->status,CTLVS_ERROR_FAILEDTOSYNCHRONIZE)))
        {
            if(!tsl_strcmp(p_cur_data->status,CTLVS_UNSYNCHRONIZED)){
                ctllog_error("wan up and NTP status changed");
                tsl_msg_client_send(TSL_MSG_SOCK_APP_TR69, tsl_msg_event_tr69_wanconnection_up);
            }
        }
#endif
    }
    else if (DELETE_OR_DISABLE_EXISTING(p_new_data, p_cur_data))
    {
        stop_ntp();
        CTLMEM_REPLACE_STRING(p_new_data->status, CTLVS_DISABLED);
        ctllog_debug("stop ntpclient ok\n");
    }
    CLEAR_TRIGGER_MARK( p_new_data );
    return rv;
}

