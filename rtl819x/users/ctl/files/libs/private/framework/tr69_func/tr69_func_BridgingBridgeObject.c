/*./auto/tr69_func_BridgingBridgeObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"

#include "ctl.h"
#include "tr69_cms_object.h"

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_BridgingBridgeObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.Bridging.Bridge
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_BridgingBridgeObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_BridgingBridgeObject_value(tsl_char_t *p_oid_name, _BridgingBridgeObject *p_cur_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

	return rv;
}


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_set_BridgingBridgeObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.Bridging.Bridge
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_BridgingBridgeObject_t *p_cur_data
 *	        st_BridgingBridgeObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_BridgingBridgeObject_value(tsl_char_t *p_oid_name, _BridgingBridgeObject *p_cur_data, _BridgingBridgeObject *p_new_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_CHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

#ifdef AEI_CTL_BRIDGE
    if( ENABLE_NEW_OR_ENABLE_EXISTING(p_new_data, p_cur_data)) {
        //DO_SYSTEM( "ifconfig eth0 up" );
#if 0
        DO_SYSTEM( "ifconfig wlan0 up" );
        DO_SYSTEM( "ifconfig wlan0-va0 up" );
        DO_SYSTEM( "ifconfig wlan1 up" );
        DO_SYSTEM( "ifconfig wlan1-va0 up" );

        DO_SYSTEM( "brctl addbr br0" );
        DO_SYSTEM( "brctl addif br0 eth0" );
        DO_SYSTEM( "brctl addif br0 wlan0-va0" );
        DO_SYSTEM( "brctl addif br0 wlan1-va0" );
#endif
        //set_lan_dhcpc( "br0" );
    } else if( DELETE_OR_DISABLE_EXISTING(p_new_data, p_cur_data)) {
#if 0
        DO_SYSTEM( "ifconfig eth0 down" );
        DO_SYSTEM( "ifconfig wlan0 down" );
        DO_SYSTEM( "ifconfig wlan0-va0 down" );
        DO_SYSTEM( "ifconfig wlan1 down" );
        DO_SYSTEM( "ifconfig wlan1-va0 down" );

        DO_SYSTEM( "brctl delif br0 eth0" );
        DO_SYSTEM( "brctl delif br0 wlan0-va0" );
        DO_SYSTEM( "brctl delif br0 wlan1-va0" );
        DO_SYSTEM( "brctl delbr br0" );
#endif

    }
#endif
	return rv;
}

