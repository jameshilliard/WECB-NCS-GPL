/*./auto/tr69_func_EthernetLinkObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"

#include "act_intf_stack.h"
#include "tr69_cms_object.h"
#include "libtr69_func.h"

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_EthernetLinkObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.Ethernet.Link
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_EthernetLinkObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_EthernetLinkObject_value(tsl_char_t *p_oid_name, _EthernetLinkObject *p_cur_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

    CALC_LASTCHANGE( p_cur_data );
    rv = TR69_RT_SUCCESS_VALUE_CHANGED;
	return rv;
}


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_set_EthernetLinkObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.Ethernet.Link
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_EthernetLinkObject_t *p_cur_data
 *	        st_EthernetLinkObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_EthernetLinkObject_value(tsl_char_t *p_oid_name, _EthernetLinkObject *p_cur_data, _EthernetLinkObject *p_new_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_CHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

#ifdef AEI_CTL_BRIDGE
    UPDATE_INTF_STACK( p_oid_name, p_cur_data, p_new_data );

    if( ENABLE_NEW_OR_ENABLE_EXISTING(p_new_data, p_cur_data)) {
        // Enable
        checkIntfStatusEx( p_oid_name, p_cur_data, p_new_data, NULL,
                TRIGGER_HIGHER_LAYER, p_oid_name );
        // JBB_TODO: How to up br0 on system boot
        DO_SYSTEM( "ifconfig %s up", p_new_data->name );
    } else if( POTENTIAL_CHANGE_OF_EXISTING(p_new_data, p_cur_data)){
        if( IS_TRIGGERED_BY_LOWER_LAYER( p_new_data ) ) {
            checkIntfStatusEx( p_oid_name, p_cur_data, p_new_data, NULL,
                TRIGGER_HIGHER_LAYER, p_oid_name );
        }
    } else if( DELETE_OR_DISABLE_EXISTING(p_new_data, p_cur_data)){
        // Disable
        DO_SYSTEM( "ifconfig %s down", p_cur_data->name );
        UPDATE_INTF_STATUS_EX( p_oid_name, p_cur_data, p_new_data, CTLVS_DOWN,
                TRIGGER_HIGHER_LAYER, p_oid_name );
    }

    CLEAR_TRIGGER_MARK( p_new_data );
#endif

	return rv;
}

