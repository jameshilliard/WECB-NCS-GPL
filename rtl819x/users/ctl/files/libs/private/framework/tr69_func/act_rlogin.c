/*
 *      Utiltiy function for remote login
 *
 */

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"
#include "libtr69_func.h"
#include "OID.h"

#include "act_rlogin.h"
#include "ctl_validstrings.h"
#include "act_intf_stack.h"
#include "act_firewall.h"

tsl_rv_t stop_rlogin( _RemoteLoginObject *p_cur_data, _RemoteLoginObject *p_new_data )
{
    tsl_rv_t rv = TSL_RV_FAIL;

    if( 0 == DO_SYSTEM( "killall %s", SSHD_NAME ) ) {
        rv = TSL_RV_SUC;
    }
    /* CID 33333: Dereference after null check */
    FWI_DEL_ACCEPT_RULE( "tcp", p_cur_data->port );
    return rv;
}

tsl_rv_t start_rlogin( _RemoteLoginObject *p_cur_data, _RemoteLoginObject *p_new_data )
{
    tsl_rv_t rv = TSL_RV_FAIL;

    FWI_ADD_ACCEPT_RULE( "tcp", p_new_data->port );
    if( 0 == DO_SYSTEM( "%s -p %d -I %d &", SSHD_NAME, p_new_data->port, p_new_data->idleTimeout ) ) {
        rv = TSL_RV_SUC;
    }
    return rv;
}

tsl_rv_t restart_rlogin( _RemoteLoginObject *p_cur_data, _RemoteLoginObject *p_new_data )
{
    tsl_rv_t rv = TSL_RV_FAIL;

    do {
        rv = stop_rlogin( p_cur_data, p_new_data );
        sleep( 1 );
        rv = start_rlogin( p_cur_data, p_new_data );
    } while(0);
    return rv;
}

tsl_void_t triggerRemoteLogin( tsl_void_t )
{
    tsl_bool b_trigger = TSL_B_TRUE;
    tr69_set_leaf_data( TR69_OID_REMOTE_LOGIN".X_ACTIONTEC_COM_Trigger",
            (void *)&b_trigger, TR69_NODE_LEAF_TYPE_UINT );
}

tsl_bool_t rlogin_changed( _RemoteLoginObject *p_cur_data, _RemoteLoginObject *p_new_data )
{
    tsl_bool_t bChanged = TSL_B_FALSE;

    if( (p_cur_data->idleTimeout != p_new_data->idleTimeout)
            || (p_cur_data->port != p_new_data->port) ) {
        bChanged = TSL_B_TRUE;
    }
    return bChanged;
}

