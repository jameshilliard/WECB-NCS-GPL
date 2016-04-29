/*./auto/tr69_func_HostsHostIpv6AddressObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_HostsHostIpv6AddressObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.Hosts.Host.i.IPv6Address
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_HostsHostIpv6AddressObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_HostsHostIpv6AddressObject_value(tsl_char_t *p_oid_name, _HostsHostIpv6AddressObject *p_cur_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

	return rv;
}


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_set_HostsHostIpv6AddressObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.Hosts.Host.i.IPv6Address
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_HostsHostIpv6AddressObject_t *p_cur_data
 *	        st_HostsHostIpv6AddressObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_HostsHostIpv6AddressObject_value(tsl_char_t *p_oid_name, _HostsHostIpv6AddressObject *p_cur_data, _HostsHostIpv6AddressObject *p_new_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_CHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

	return rv;
}

