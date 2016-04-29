/*./auto/tr69_func_NETBIOSObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"

#include "act_intf_stack.h"
#include "tr69_cms_object.h"
#include "ctl_validstrings.h"
#include "tsl_strconv.h"
#include "libtr69_func.h"

#include "ctl.h"

#ifdef AEI_FIREWALL
#include "act_firewall.h"
#endif

void stopNetBIOS()
{
    system("killall nmbd");
#ifdef AEI_FIREWALL
    //system("iptables -D INPUT -i br0 -p udp --dport 137 -j ACCEPT");
    FWI_DEL_ACCEPT_RULE( "udp", 137 );
#endif
}

void startNetBIOS()
{
#ifdef AEI_FIREWALL
    //system("iptables -A INPUT -i br0 -p udp --dport 137 -j ACCEPT");
    FWI_ADD_ACCEPT_RULE( "udp", 137 );
#endif
    DO_SYSTEM( "nmbd -D" );
}

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_NETBIOSObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.X_ACTIONTEC_COM_NETBIOS
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_NETBIOSObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_NETBIOSObject_value(tsl_char_t *p_oid_name, _NETBIOSObject *p_cur_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

	return rv;
}


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_set_NETBIOSObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.X_ACTIONTEC_COM_NETBIOS
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_NETBIOSObject_t *p_cur_data
 *	        st_NETBIOSObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_NETBIOSObject_value(tsl_char_t *p_oid_name, _NETBIOSObject *p_cur_data, _NETBIOSObject *p_new_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_CHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

    if(ENABLE_NEW_OR_ENABLE_EXISTING(p_new_data,p_cur_data)) {
        startNetBIOS();
    } else if (POTENTIAL_CHANGE_OF_EXISTING(p_new_data,p_cur_data)) {
        if( IS_TRIGGERED_BY_LOWER_LAYER(p_new_data)) {
            stopNetBIOS();
            startNetBIOS();
        }
    } else if (DISABLE_EXISTING(p_new_data,p_cur_data) ) {
        stopNetBIOS();
    }

    CLEAR_TRIGGER_MARK( p_new_data );

	return rv;
}

