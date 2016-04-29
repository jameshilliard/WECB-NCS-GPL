/*./auto/tr69_func_ZeroConfExtenderObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"

#ifdef AEI_ZERO_CONF
#include "tr69_cms_object.h"
#include "act_zconf_ext.h"
#include "act_intf_stack.h"

#define SIZE_32 32
const char zero_conf_daemons[][SIZE_32] = {"zero_conf_telus", "zero_conf_vz"};
#define XML_FILE_TELUS    "/tmp/get_parameters.telus.xml"
#define UPGRADE_LOCAL_XML "/tmp/upgrade_url.xml"
#endif

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_ZeroConfExtenderObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.X_ACTIONTEC_COM_ZeroConf.Extender
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_ZeroConfExtenderObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_ZeroConfExtenderObject_value(tsl_char_t *p_oid_name, _ZeroConfExtenderObject *p_cur_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

#ifdef AEI_ZERO_CONF
    CALC_LASTCHANGE( p_cur_data );
    rv = TR69_RT_SUCCESS_VALUE_CHANGED;
#endif
	return rv;
}


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_set_ZeroConfExtenderObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.X_ACTIONTEC_COM_ZeroConf.Extender
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_ZeroConfExtenderObject_t *p_cur_data
 *	        st_ZeroConfExtenderObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_ZeroConfExtenderObject_value(tsl_char_t *p_oid_name, _ZeroConfExtenderObject *p_cur_data, _ZeroConfExtenderObject *p_new_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_CHANGED;
#ifdef AEI_ZERO_CONF
    int i = 0;
    char cmd[64] = {0};
#endif

    ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);
#ifdef AEI_ZERO_CONF
    if(p_new_data != NULL && tsl_strcmp(p_new_data->X_ACTIONTEC_COM_RouterType, ZCONF_ROUTER_TO_SNIFF)){
        for(i = 0; i < sizeof(zero_conf_daemons)/SIZE_32; ++i) {
            if(NULL == tsl_strstr(zero_conf_daemons[i], p_new_data->X_ACTIONTEC_COM_RouterType)) {
                snprintf(cmd, sizeof(cmd), "killall -9 %s", zero_conf_daemons[i]);
                system(cmd);
            }
        }

        // Delete telus sync xml.
        if(tsl_strcmp("telus", p_new_data->X_ACTIONTEC_COM_RouterType))
        {
            snprintf(cmd, sizeof(cmd), "rm -f %s %s", XML_FILE_TELUS, UPGRADE_LOCAL_XML);
            system(cmd);
        }
    }

    if( ENABLE_NEW_OR_ENABLE_EXISTING(p_new_data, p_cur_data)) {
        start_zconf_ext( p_cur_data, p_new_data );
    } else if( POTENTIAL_CHANGE_OF_EXISTING(p_new_data, p_cur_data)) {
        if( 0 != tsl_strcmp(p_cur_data->status, p_new_data->status) ) {
            UPDATE_LASTCHANGETIME( p_new_data );
        }
        if( IS_TRIGGERED_BY_LOWER_LAYER(p_new_data)) {
            ctllog_notice( "Trigger ZeroConf Extender" );
            restart_zconf_ext( p_cur_data, p_new_data );
        }
    } else if( DELETE_OR_DISABLE_EXISTING(p_new_data, p_cur_data)) {
        stop_zconf_ext( p_cur_data, p_new_data );
    }

    CLEAR_TRIGGER_MARK( p_new_data );
#endif

	return rv;
}

