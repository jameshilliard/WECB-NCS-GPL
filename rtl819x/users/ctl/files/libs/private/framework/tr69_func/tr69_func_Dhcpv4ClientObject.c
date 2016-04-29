/*./auto/tr69_func_Dhcpv4ClientObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"

#include "act_dhcpv4.h"
#include "act_intf_stack.h"
#include "tr69_cms_object.h"
#include "ctl_validstrings.h"
#include "tsl_strconv.h"
#include "libtr69_func.h"

#include "ctl.h"
#include "tsl_msg.h"
#include "act_zconf_ext.h"

#ifdef AEI_CTL_BRIDGE
static tsl_char_t last_ifStatus[BUFLEN_32] = {0};
#endif

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_Dhcpv4ClientObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.DHCPv4.Client
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_Dhcpv4ClientObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_Dhcpv4ClientObject_value(tsl_char_t *p_oid_name, _Dhcpv4ClientObject *p_cur_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

	return rv;
}


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_set_Dhcpv4ClientObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.DHCPv4.Client
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_Dhcpv4ClientObject_t *p_cur_data
 *	        st_Dhcpv4ClientObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_Dhcpv4ClientObject_value(tsl_char_t *p_oid_name, _Dhcpv4ClientObject *p_cur_data, _Dhcpv4ClientObject *p_new_data)
{
    tsl_int_t rv = TR69_RT_SUCCESS_VALUE_CHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

#ifdef AEI_CTL_BRIDGE
    if( DELETE_OR_DISABLE_EXISTING(p_new_data, p_cur_data)){
        // Disable
        //ctllog_error( "NOT support DHCPv4 CLient - Disable" );
        stop_dhcpc( p_new_data );
    } else if( p_new_data != NULL && p_new_data->enable ) {
        tsl_char_t strPara[BUFLEN_256] = {0};
        tsl_char_t * c_ifEnable;
        tsl_bool   b_ifEnable;
        tsl_char_t * ifStatus = NULL;
        tsl_char_t * ifName = NULL;
        tsl_int_t type;

        do {
            snprintf(strPara, sizeof(strPara), "%s.Enable", p_new_data->interface );
            if( tr69_get_unfresh_leaf_data( strPara, (void **) &c_ifEnable, &type) < 0) {
                break;
            }
            b_ifEnable = (tsl_bool) c_ifEnable;
            if( TSL_B_FALSE == b_ifEnable ) {
                ctllog_warn( "IP interface disabled" );
                break;
            }

            snprintf(strPara, sizeof(strPara), "%s.Status", p_new_data->interface );
            if( tr69_get_unfresh_leaf_data( strPara, (void **) &ifStatus, &type) < 0) {
                break;
            }
            if( 0!=tsl_strcmp(ifStatus, CTLVS_UP )) {
                if(0 == tsl_strcmp(last_ifStatus, CTLVS_UP)) {
                    // ifStatus chenged from "Up" to other status
                    stop_dhcpc( p_new_data );
                }
                ctllog_debug( "IP interface status being '%s'", ifStatus );
                break;
            }

            snprintf(strPara, sizeof(strPara), "%s.Name", p_new_data->interface );
            if( tr69_get_unfresh_leaf_data( strPara, (void **) &ifName, &type) < 0) {
                break;
            }

            if( ENABLE_NEW_OR_ENABLE_EXISTING(p_new_data, p_cur_data)) {
                // Enable
                if( 0 == tsl_strcmp(ifStatus, CTLVS_UP)) {
                    set_lan_dhcpc( ifName, p_new_data );
                }
            } else if( POTENTIAL_CHANGE_OF_EXISTING(p_new_data, p_cur_data)) {
                // Modify
#ifdef AEI_TR69
                /* Notify tr69 client that Internet is ready */
                if (  tsl_strcmp(p_new_data->IPAddress , p_cur_data->IPAddress)!=0 &&
                        tsl_strcmp(p_new_data->IPAddress,"0.0.0.0")!=0  )
                {
                    do{
                        tsl_bool   b_NtpEnable;
                        tsl_char_t * c_NtpEnable;
                        snprintf(strPara, sizeof(strPara), "%s.X_ACTIONTEC_COM_InternetStatus", p_new_data->interface );
                        tr69_set_unfresh_leaf_data(strPara, (void *)CTLVS_UP, TR69_NODE_LEAF_TYPE_STRING);
                        if(tr69_get_unfresh_leaf_data("Device.Time.Enable",(void **) &c_NtpEnable, &type)<0){
                            ctllog_error("fail to get Ntp Enable");
                            break;
                        }
                        b_NtpEnable = (tsl_bool) c_NtpEnable;
                        if(TSL_B_FALSE == b_NtpEnable){
                            ctllog_error("Device IP connected, can access internet\n");
                            tsl_msg_client_send(TSL_MSG_SOCK_APP_TR69, tsl_msg_event_tr69_wanconnection_up);
                        }
                        else
                        {
                            tsl_bool b_trigger = TSL_B_TRUE;
                            tr69_set_leaf_data("Device.Time.X_ACTIONTEC_COM_Trigger",
                                    (void *)&b_trigger, TR69_NODE_LEAF_TYPE_UINT );
                        }
                    }while(0);
                }
#endif
                /* Notify mDNS that Internet is ready */
                if (  tsl_strcmp(p_new_data->IPAddress , p_cur_data->IPAddress)!=0 &&
                        tsl_strcmp(p_new_data->IPAddress,"0.0.0.0")!=0  )
                {
                            tsl_bool b_trigger = TSL_B_TRUE;
                            tr69_set_leaf_data("Device.X_ACTIONTEC_COM_MDNS.X_ACTIONTEC_COM_Trigger",
                                    (void *)&b_trigger, TR69_NODE_LEAF_TYPE_UINT );

                            b_trigger = TSL_B_TRUE;
                            tr69_set_leaf_data( TR69_OID_NETBIOS".X_ACTIONTEC_COM_Trigger",
                                    (void *)&b_trigger, TR69_NODE_LEAF_TYPE_UINT );

#ifdef AEI_ZERO_CONF
                            b_trigger = TSL_B_TRUE;
                            tr69_set_unfresh_leaf_data(TR69_OID_ZERO_CONF_EXTENDER".X_ACTIONTEC_COM_RouterType",
                                    (void *)ZCONF_ROUTER_TO_SNIFF, TR69_NODE_LEAF_TYPE_STRING);
                            tr69_set_leaf_data(TR69_OID_ZERO_CONF_EXTENDER".X_ACTIONTEC_COM_Trigger",
                                    (void *)&b_trigger, TR69_NODE_LEAF_TYPE_UINT);
#endif
                }
                if( IS_TRIGGERED_BY_LOWER_LAYER(p_new_data)) {
                    // relaunch dhcpc
                    stop_dhcpc( p_new_data );
                    set_lan_dhcpc( ifName, p_new_data );
                    break;
                }
                if(0 == tsl_strcmp(ifStatus, last_ifStatus)) {
                    ctllog_debug( "IP interface was Up, some param of DHCPv4 CLient have been modified" );
                    break;
                }
                // JBB_TODO: wait for bridge initialization.
                // Because br0 entering forwarding state always need some time especially booting up.
                //sleep(10); // sleep 10 secs in data_center maybe block it. Moved to udhcpc

                // ifStatus changed to "Up"
                if( 0 == tsl_strcmp(ifStatus, CTLVS_UP)) {
                    set_lan_dhcpc( ifName, p_new_data );
                }
            }
        } while(0);
        snprintf( last_ifStatus, sizeof(last_ifStatus), "%s", ifStatus );

        CTLMEM_FREE_BUF_AND_NULL_PTR( ifName );
        CTLMEM_FREE_BUF_AND_NULL_PTR( ifStatus );
    }
    
    CLEAR_TRIGGER_MARK( p_new_data );
#endif
	return rv;
}

