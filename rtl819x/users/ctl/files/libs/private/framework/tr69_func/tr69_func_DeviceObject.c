/*./auto/tr69_func_DeviceObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"
#ifdef AEI_CTL_BRIDGE
#include "act_intf_stack.h"
#endif

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_DeviceObject_value
 *
 *	[DESCRIPTION]:
 *	        Device
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_DeviceObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_DeviceObject_value(tsl_char_t *p_oid_name, _DeviceObject *p_cur_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

	return rv;
}


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_set_DeviceObject_value
 *
 *	[DESCRIPTION]:
 *	        Device
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_DeviceObject_t *p_cur_data
 *	        st_DeviceObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_DeviceObject_value(tsl_char_t *p_oid_name, _DeviceObject *p_cur_data, _DeviceObject *p_new_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_CHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

#ifdef AEI_CTL_BRIDGE
    if( (NULL==p_cur_data) && (NULL != p_new_data) ) {
        delAllInterfaceStackInst();
        
        // There's bug in data_center.
        // when delete instance in its parent object's set function,
        // must set number of enteries manually.
        p_new_data->interfaceStackNumberOfEntries = 0;
    }
#endif

	return rv;
}

