/*./auto/tr69_func_FirewallObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"

#include "tr69_cms_object.h"

#ifdef AEI_FIREWALL
#include "act_firewall.h"
#ifdef AEI_TR69
#include "tr69_internal.h"
#endif
#endif

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_FirewallObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.Firewall
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_FirewallObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_FirewallObject_value(tsl_char_t *p_oid_name, _FirewallObject *p_cur_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

	return rv;
}


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_set_FirewallObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.Firewall
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_FirewallObject_t *p_cur_data
 *	        st_FirewallObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_FirewallObject_value(tsl_char_t *p_oid_name, _FirewallObject *p_cur_data, _FirewallObject *p_new_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_CHANGED;
#ifdef AEI_FIREWALL
    static tsl_bool_t b_fw_inited = TSL_B_FALSE;
#endif
	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

#ifdef AEI_FIREWALL
    if( ENABLE_NEW_OR_ENABLE_EXISTING(p_new_data, p_cur_data)) {
        /* Init */
        if( TSL_B_FALSE == b_fw_inited ) {
            b_fw_inited = TSL_B_TRUE;
            // Cannot run 'iptables -F' here, otherwise the rules added by other module
            // would be deleted.
            DO_SYSTEM( "iptables -I INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT" ); // stateful
            DO_SYSTEM( "iptables -A INPUT -p icmp --icmp-type echo-request -j ACCEPT" ); // ping req.
            DO_SYSTEM( "iptables -A INPUT -p 2 -j ACCEPT" ); // igmp, mediaroom
            FWI_ADD_ACCEPT_RULE( "tcp", 80 );           // httpd
#ifdef AEI_CRISTAL_ACCESS
            FWI_ADD_ACCEPT_RULE( "tcp", 81 );           //nagios
#endif
#ifdef AEI_TR69
            FWI_ADD_ACCEPT_RULE( "tcp", PASSIVE_PORT ); // tr69c
#endif
        }
        /* Also need setup rules when enable/disable those modules:
         *   SSH
         * No need add rules to other modules, since of stateful firewall;
         *   DHCPC
         */

        /* Default Rules*/
        DO_SYSTEM( "iptables -P INPUT DROP" );
    } else if( POTENTIAL_CHANGE_OF_EXISTING(p_new_data, p_cur_data)) {
        // Code will not comes here!
    } else if( DELETE_OR_DISABLE_EXISTING(p_new_data, p_cur_data)) {
        /* Default Rules*/
        DO_SYSTEM( "iptables -P INPUT ACCEPT" );
    }
#endif

	return rv;
}

