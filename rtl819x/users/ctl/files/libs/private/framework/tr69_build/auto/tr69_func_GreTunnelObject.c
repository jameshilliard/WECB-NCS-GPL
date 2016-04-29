/*./auto/tr69_func_GreTunnelObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_GreTunnelObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.GRETunnel
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_GreTunnelObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_GreTunnelObject_value(tsl_char_t *p_oid_name, _GreTunnelObject *p_cur_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

	return rv;
}


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_set_GreTunnelObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.GRETunnel
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_GreTunnelObject_t *p_cur_data
 *	        st_GreTunnelObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_GreTunnelObject_value(tsl_char_t *p_oid_name, _GreTunnelObject *p_cur_data, _GreTunnelObject *p_new_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_CHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

	return rv;
}

