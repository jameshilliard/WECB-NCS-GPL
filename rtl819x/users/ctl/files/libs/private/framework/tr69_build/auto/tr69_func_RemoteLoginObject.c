/*./auto/tr69_func_RemoteLoginObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_RemoteLoginObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.X_ACTIONTEC_COM_RemoteLogin
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_RemoteLoginObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_RemoteLoginObject_value(tsl_char_t *p_oid_name, _RemoteLoginObject *p_cur_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

	return rv;
}


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_set_RemoteLoginObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.X_ACTIONTEC_COM_RemoteLogin
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_RemoteLoginObject_t *p_cur_data
 *	        st_RemoteLoginObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_RemoteLoginObject_value(tsl_char_t *p_oid_name, _RemoteLoginObject *p_cur_data, _RemoteLoginObject *p_new_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_CHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

	return rv;
}

