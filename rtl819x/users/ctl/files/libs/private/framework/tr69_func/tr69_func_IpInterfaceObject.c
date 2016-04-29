/*./auto/tr69_func_IpInterfaceObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"

#include "act_intf_stack.h"
#include "tr69_cms_object.h"
#include "libtr69_func.h"
#include "act_ip.h"
//#include "ctl_mem.h"
//#include "ctl_validstrings.h"
//#include "tsl_strconv.h"

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_IpInterfaceObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.IP.Interface
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_IpInterfaceObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_IpInterfaceObject_value(tsl_char_t *p_oid_name, _IpInterfaceObject *p_cur_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

    // Wait min interval (5 sec) here, no need to query ioctl too often.
    static __time_t lastTime = 0;
    __time_t currTime = CTL_GET_CURRENTTIME();
    if( currTime - lastTime >= CTL_UPDATE_IP_INFO_MIN_INTERNAL ) {
        p_cur_data->IPv6AddressNumberOfEntries += updateIPv6AddressInstUnderIPIntf(p_oid_name);
        lastTime = currTime;
    }

    CALC_LASTCHANGE( p_cur_data );
    rv = TR69_RT_SUCCESS_VALUE_CHANGED;
	return rv;
}

void triggerAppLayer( tsl_char_t * p_oid_name )
{
    //////////////////////////////////////////////////////////////////
    // Trigger High Layer Objects:
    // DHCPv4
    // JEAN_TODO: Find DHCPv4 client, and trigger it
    tsl_bool b_trigger = TSL_B_TRUE;
    tr69_set_leaf_data( "Device.DHCPv4.Client.1.X_ACTIONTEC_COM_Trigger",
            (void *)&b_trigger, TR69_NODE_LEAF_TYPE_UINT );
}

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_set_IpInterfaceObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.IP.Interface
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_IpInterfaceObject_t *p_cur_data
 *	        st_IpInterfaceObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_IpInterfaceObject_value(tsl_char_t *p_oid_name, _IpInterfaceObject *p_cur_data, _IpInterfaceObject *p_new_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_CHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

#ifdef AEI_CTL_BRIDGE
    UPDATE_INTF_STACK( p_oid_name, p_cur_data, p_new_data );

    if( ENABLE_NEW_OR_ENABLE_EXISTING(p_new_data, p_cur_data)) {
        // Enable
        checkIntfStatusEx( p_oid_name, p_cur_data, p_new_data, NULL,
                triggerAppLayer, p_oid_name );
        //JBB_TODO: config default ip address here or by udhcpc?
    } else if( POTENTIAL_CHANGE_OF_EXISTING(p_new_data, p_cur_data)){
        if( IS_TRIGGERED_BY_LOWER_LAYER( p_new_data ) ) {
            checkIntfStatusEx( p_oid_name, p_cur_data, p_new_data, NULL,
                    triggerAppLayer, p_oid_name );
        }
    } else if( DELETE_OR_DISABLE_EXISTING(p_new_data, p_cur_data)){
        // Disable
        UPDATE_INTF_STATUS_EX( p_oid_name, p_cur_data, p_new_data, CTLVS_DOWN,
                triggerAppLayer, p_oid_name );
    }

    CLEAR_TRIGGER_MARK( p_new_data );
#endif

	return rv;
}

