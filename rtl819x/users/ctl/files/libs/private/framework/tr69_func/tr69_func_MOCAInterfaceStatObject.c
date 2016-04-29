/*./auto/tr69_func_MOCAInterfaceStatObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"
#include "tr69_cms_object.h"
#include "act_moca.h"

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_MOCAInterfaceStatObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.MoCA.Interface.i.Stats
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_MOCAInterfaceStatObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_MOCAInterfaceStatObject_value(tsl_char_t *p_oid_name, _MOCAInterfaceStatObject *p_cur_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

	get_moca_stats(p_cur_data);

	return rv;
}


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_set_MOCAInterfaceStatObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.MoCA.Interface.i.Stats
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_MOCAInterfaceStatObject_t *p_cur_data
 *	        st_MOCAInterfaceStatObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_MOCAInterfaceStatObject_value(tsl_char_t *p_oid_name, _MOCAInterfaceStatObject *p_cur_data, _MOCAInterfaceStatObject *p_new_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_CHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

	return rv;
}

