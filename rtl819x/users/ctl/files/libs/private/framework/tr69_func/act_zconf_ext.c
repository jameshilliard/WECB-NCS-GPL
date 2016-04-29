/*
 *      Utiltiy function for zero conf of Extender
 *
 */

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"
#include "libtr69_func.h"
#include "OID.h"

#include "act_zconf_ext.h"
#include "ctl_validstrings.h"
#include "act_intf_stack.h"

int is_apmib_ready(void);


#ifdef AEI_ZERO_CONF
tsl_rv_t stop_zconf_ext( _ZeroConfExtenderObject *p_cur_data, _ZeroConfExtenderObject *p_new_data )
{
    tsl_rv_t rv = TSL_RV_FAIL;

    if( 0 == DO_SYSTEM( "killall %s", ZCONF_EXT_DAEMON_NAME ) ) {
        rv = TSL_RV_SUC;
    }
    UPDATE_ZCONF_EXT_STATUS( p_cur_data, p_new_data, CTLVS_DISCONNECTED );
    return rv;
}

tsl_rv_t start_zconf_ext( _ZeroConfExtenderObject *p_cur_data, _ZeroConfExtenderObject *p_new_data )
{
    tsl_rv_t rv = TSL_RV_FAIL;

    do {
        if( !is_apmib_ready()){
            break;
        }
        if( 0 == DO_SYSTEM( "%s &", ZCONF_EXT_LAUNCH_SCRIPT ) ) {
            rv = TSL_RV_SUC;
        }
    } while(0);
    
    if( TSL_RV_SUC == rv ) {
        UPDATE_ZCONF_EXT_STATUS( p_cur_data, p_new_data, CTLVS_CONNECTING );
    } else {
        UPDATE_ZCONF_EXT_STATUS( p_cur_data, p_new_data, CTLVS_DISCONNECTED );
    }
    return rv;
}

tsl_rv_t restart_zconf_ext( _ZeroConfExtenderObject *p_cur_data, _ZeroConfExtenderObject *p_new_data )
{
    tsl_rv_t rv = TSL_RV_FAIL;

    do {
        rv = stop_zconf_ext( p_cur_data, p_new_data );
        sleep( 1 );
        rv = start_zconf_ext( p_cur_data, p_new_data );
    } while(0);
    return rv;
}

tsl_void_t triggerZconfExt( tsl_void_t )
{
    tsl_bool b_trigger = TSL_B_TRUE;
    tr69_set_leaf_data( TR69_OID_ZERO_CONF_EXTENDER".X_ACTIONTEC_COM_Trigger",
            (void *)&b_trigger, TR69_NODE_LEAF_TYPE_UINT );
}

#endif

