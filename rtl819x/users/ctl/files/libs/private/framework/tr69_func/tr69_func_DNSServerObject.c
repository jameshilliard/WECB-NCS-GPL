/*./auto/tr69_func_DNSServerObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"
#include "act_dhcpv4.h"
#include "tr69_cms_object.h"

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_DNSServerObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.X_ACTIONTEC_COM_DNSServer
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_DNSServerObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_DNSServerObject_value(tsl_char_t *p_oid_name, _DNSServerObject *p_cur_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

	return rv;
}


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_set_DNSServerObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.X_ACTIONTEC_COM_DNSServer
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_DNSServerObject_t *p_cur_data
 *	        st_DNSServerObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_DNSServerObject_value(tsl_char_t *p_oid_name, _DNSServerObject *p_cur_data, _DNSServerObject *p_new_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_CHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);
#ifdef AEI_NCS_CFG_SYNC
ctldbg_module(CTL_MODULE_NCS_CFG_SYNC,"begin");
 if (p_new_data && p_new_data->enable)
{
    start_dnsmasq();
}
 else if (DISABLE_EXISTING(p_new_data, p_cur_data))
{
    stop_dnsmasq();
}
ctldbg_module(CTL_MODULE_NCS_CFG_SYNC,"end");
#endif
	return rv;
}

