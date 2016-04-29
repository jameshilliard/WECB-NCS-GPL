/*./auto/tr69_func_ZeroConfObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_ZeroConfObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.X_ACTIONTEC_COM_ZeroConf
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_ZeroConfObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_ZeroConfObject_value(tsl_char_t *p_oid_name, _ZeroConfObject *p_cur_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

	return rv;
}


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_set_ZeroConfObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.X_ACTIONTEC_COM_ZeroConf
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_ZeroConfObject_t *p_cur_data
 *	        st_ZeroConfObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_ZeroConfObject_value(tsl_char_t *p_oid_name, _ZeroConfObject *p_cur_data, _ZeroConfObject *p_new_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_CHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

	return rv;
}

