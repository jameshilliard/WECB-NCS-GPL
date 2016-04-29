/*./auto/tr69_func_ManagementServerObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"
#include "ctl.h"
#include "tr69_cms_object.h"
#include "tsl_msg.h"
#include "tsl_strconv.h"
#include "ctl_mem.h"
#include "uuid.h"
#include "libtr69_func.h"

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_ManagementServerObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.ManagementServer
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_ManagementServerObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_ManagementServerObject_value(tsl_char_t *p_oid_name, _ManagementServerObject *p_cur_data)
{
    tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;

    ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

    return rv;
}


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_set_ManagementServerObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.ManagementServer
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_ManagementServerObject_t *p_cur_data
 *	        st_ManagementServerObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_ManagementServerObject_value(tsl_char_t *p_oid_name, _ManagementServerObject *p_cur_data, _ManagementServerObject *p_new_data)
{
    tsl_int_t rv = TR69_RT_SUCCESS_VALUE_CHANGED;

    ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

    if( p_new_data != NULL ) {
        /* per VZ RFP, Generate UUID for ConnectionRequestURL */
        if( IS_EMPTY_STRING( p_new_data->X_ACTIONTEC_COM_UUID )) {
            uuid_t uuid;
            char struuid[UUID_FORMAT_LEN+1] = {0};

            uuid_generate_time( uuid );
            uuid_unparse( uuid, struuid );
            CTLMEM_REPLACE_STRING( p_new_data->X_ACTIONTEC_COM_UUID, struuid );
        }
#ifdef AEI_TR69_ACS_USERNAME_SN
        if( IS_EMPTY_STRING( p_new_data->username)) {
            tsl_char_t * pSN = NULL;
            tsl_int_t type;

            /* Use serial number as ACS User name */
            if( TSL_RV_SUC != (rv=ACCESS_LEAF( tr69_get_unfresh_leaf_data,
                            (void **) &pSN, &type,
                            "%s.SerialNumber", TR69_OID_DEVICE_INFO ))) {
                ctllog_error( "Fail to get serial number" );
            } else {
                CTLMEM_REPLACE_STRING( p_new_data->username, pSN );
                CTLMEM_REPLACE_STRING( p_new_data->connectionRequestUsername, pSN );
            }
            CTLMEM_FREE_BUF_AND_NULL_PTR( pSN );
        }
#endif
    }

    /* new ManagementServerObject */
    if (p_new_data != NULL && p_cur_data == NULL) {
        if (! p_new_data->enableCWMP)  {
            DO_SYSTEM("killall tr69");
            return rv;
        }
    }    /* detect change in management server object */
    else if (p_new_data != NULL && p_cur_data != NULL)
    {
        ctllog_debug("p_new_data->URL=%s, p_cur_data->URL=%s, p_new_data->username=%s, p_cur_data->username=%s\n",
                p_new_data->URL, p_cur_data->URL, p_new_data->username, p_cur_data->username);

        if (! p_new_data->enableCWMP && p_cur_data->enableCWMP)  {

            return rv;
        }

        if(p_new_data->STUNEnable==1 && p_cur_data->STUNEnable==0){
            if( DO_SYSTEM( "stunc &" ) < 0){
                ctllog_debug("Stun Client start failed.\n");
            }else{
                ctllog_debug("Stun Client start.\n");
            }
        }

        if(p_new_data->STUNEnable==0 && p_cur_data->STUNEnable==1){
            if( DO_SYSTEM( "killall stunc" ) < 0){
                ctllog_debug("Stun Client stop failed.\n");
            }
        }

        if(p_new_data->NATDetected == 2){
            p_new_data->NATDetected = p_cur_data->NATDetected;
            ctllog_debug("send tsl_msg_event_connection_request to TR69\n");
            tsl_msg_client_send(TSL_MSG_SOCK_APP_TR69, tsl_msg_event_connection_request);

        }

        if(p_new_data->X_ACTIONTEC_COM_SetConnReqURL==0 && p_cur_data->X_ACTIONTEC_COM_SetConnReqURL==1) {
        CTLMEM_REPLACE_STRING(p_new_data->connectionRequestURL,"");
        }

        if ( tsl_strcmp(p_new_data->URL, p_cur_data->URL) ||
                tsl_strcmp(p_new_data->username, p_cur_data->username) ||
                tsl_strcmp(p_new_data->password, p_cur_data->password) ||
                tsl_strcmp(p_new_data->connectionRequestUsername, p_cur_data->connectionRequestUsername) ||
                tsl_strcmp(p_new_data->connectionRequestPassword, p_cur_data->connectionRequestPassword) ||
                tsl_strcmp(p_new_data->periodicInformTime, p_cur_data->periodicInformTime) ||
                (p_new_data->periodicInformEnable != p_cur_data->periodicInformEnable) ||
                (p_new_data->periodicInformInterval != p_cur_data->periodicInformInterval) ||
                (p_new_data->defaultActiveNotificationThrottle != p_cur_data->defaultActiveNotificationThrottle) ||
                tsl_strcmp(p_new_data->UDPConnectionRequestAddress, p_cur_data->UDPConnectionRequestAddress))
        {
            ctllog_debug("send tsl_msg_event_tr69_acs_conf_change to TR69\n");

            tsl_msg_client_send(TSL_MSG_SOCK_APP_TR69, tsl_msg_event_tr69_acs_conf_change);
        }
    }

    /* This object cannot be deleted, so no need to handle that case. */

    return rv;

}

