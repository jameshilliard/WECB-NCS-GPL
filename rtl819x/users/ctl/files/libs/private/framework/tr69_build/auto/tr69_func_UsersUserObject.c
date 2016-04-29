/*./auto/tr69_func_UsersUserObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_UsersUserObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.Users.User
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_UsersUserObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_UsersUserObject_value(tsl_char_t *p_oid_name, _UsersUserObject *p_cur_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

	return rv;
}


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_set_UsersUserObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.Users.User
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_UsersUserObject_t *p_cur_data
 *	        st_UsersUserObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_UsersUserObject_value(tsl_char_t *p_oid_name, _UsersUserObject *p_cur_data, _UsersUserObject *p_new_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_CHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

	return rv;
}

