/*./auto/tr69_func_EthernetInterfaceObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"

#include "act_eth.h"
#include "act_intf_stack.h"
#include "tr69_cms_object.h"
#include "libtr69_func.h"

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_EthernetInterfaceObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.Ethernet.Interface
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_EthernetInterfaceObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_EthernetInterfaceObject_value(tsl_char_t *p_oid_name, _EthernetInterfaceObject *p_cur_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

    // Wait min interval (5 sec) here, no need to query ioctl too often.
    static __time_t lastTime = 0;
    __time_t currTime = CTL_GET_CURRENTTIME();
    if( currTime - lastTime >= CTL_UPDATE_ETH_INFO_MIN_INTERNAL ) {
        tsl_char_t mac[17+1] = {0};

        if( TSL_RV_SUC == GetEthMACAddr( p_cur_data->name, mac, sizeof(mac))) {
            if( tsl_strcmp( p_cur_data->MACAddress, mac)) {
                CTLMEM_REPLACE_STRING(p_cur_data->MACAddress, mac);
            }
        }
        lastTime = currTime;
    }

    CALC_LASTCHANGE( p_cur_data );
    rv = TR69_RT_SUCCESS_VALUE_CHANGED;
	return rv;
}


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_set_EthernetInterfaceObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.Ethernet.Interface
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_EthernetInterfaceObject_t *p_cur_data
 *	        st_EthernetInterfaceObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_EthernetInterfaceObject_value(tsl_char_t *p_oid_name, _EthernetInterfaceObject *p_cur_data, _EthernetInterfaceObject *p_new_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_CHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

#ifdef AEI_CTL_BRIDGE
    //tsl_char_t str_MACAddr[BUFLEN_16] = {0};

    UPDATE_INTF_STACK( p_oid_name, p_cur_data, p_new_data );

    if( ENABLE_NEW_OR_ENABLE_EXISTING(p_new_data, p_cur_data)) {
        // Enable
        tsl_rv_t rv = TSL_RV_FAIL;
        do {
#if 0
            // Configure MAC address
            if( TSL_RV_SUC != GetEthMACAddr( str_MACAddr, sizeof(str_MACAddr))) {
                ctllog_error("Fail to get Eth MAC address" );
                break;
            }
            if( DO_SYSTEM( "ifconfig %s hw ether %s", p_new_data->name, str_MACAddr)) {
                ctllog_error("Fail to configure Eth MAC on %s", p_new_data->name );
                break;
            }
#endif
            // Bringup
            if( DO_SYSTEM( "ifconfig %s up", p_new_data->name)) {
                ctllog_error("Fail to bringup %s", p_new_data->name );
                break;
            }
            rv = TSL_RV_SUC;
        } while(0);
        if( TSL_RV_SUC != rv ) {
            UPDATE_INTF_STATUS_EX( p_oid_name, p_cur_data, p_new_data, CTLVS_DOWN,
                    TRIGGER_HIGHER_LAYER, p_oid_name );
        } else {
            UPDATE_INTF_STATUS_EX( p_oid_name, p_cur_data, p_new_data, CTLVS_UP,
                    TRIGGER_HIGHER_LAYER, p_oid_name );
        }
    } else if( POTENTIAL_CHANGE_OF_EXISTING(p_new_data, p_cur_data)){
        // JBB_TODO: e.g.: change MAC IP address
    } else if( DELETE_OR_DISABLE_EXISTING(p_new_data, p_cur_data)){
        // Disable
        if( DO_SYSTEM( "ifconfig %s down", p_new_data->name )) {
            UPDATE_INTF_STATUS_EX( p_oid_name, p_cur_data, p_new_data, CTLVS_UNKNOWN,
                    TRIGGER_HIGHER_LAYER, p_oid_name );
        } else {
            UPDATE_INTF_STATUS_EX( p_oid_name, p_cur_data, p_new_data, CTLVS_DOWN,
                    TRIGGER_HIGHER_LAYER, p_oid_name );
        }
    }
#endif
	return rv;
}

