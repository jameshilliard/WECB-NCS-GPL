/*./auto/tr69_func_RemoteLoginObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"

#include "tr69_cms_object.h"
#include "act_rlogin.h"
#include "act_intf_stack.h"
#include <unistd.h>
#include <crypt.h>
#include "base64.h"
#if defined( AEI_WECB_CUSTOMER_NCS ) && defined (AEI_GENERATER_UNIQUELY_PASSWORD)
#include "aei_rut_password.h"
#include "tr69_cms_object.h"
#include "libtr69_func.h"
#include "ctl_validstrings.h"
#endif
#define AEI_CRYPT_MD5_SALT   "$1$SjA/wZxa$"

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
    /* Doing crypt and base64 encoding on password */
    do {
        tsl_int_t pwdlen;
        tsl_char_t * testcrypt = NULL;
        tsl_char_t base64str[BUFLEN_256] = {0};
        tsl_int_t base64len = sizeof(base64str);

        if( NULL == p_cur_data )
            break;
        if( NULL == p_new_data )
            break;
        if( 0 == tsl_strcmp(p_cur_data->password, p_new_data->password) ) {
            break;
        }

        // Only happen on password changed
        pwdlen = tsl_strlen(p_new_data->password);
        // pwd cannot be empty
        if( pwdlen == 0 ) {
            rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;
            break;
        }
        testcrypt = crypt( p_new_data->password, AEI_CRYPT_MD5_SALT );
        if( base64Encode( testcrypt, strlen(testcrypt), base64str, & base64len ) <0 ) {
            rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;
            break;
        }
        ctllog_debug( "Encrypt pwd '%s' -> crypt '%s' -> base64 '%s'",
                p_new_data->password, testcrypt, base64str );
        CTLMEM_REPLACE_STRING( p_new_data->password, base64str );
    } while(0);

    if( ENABLE_NEW_OR_ENABLE_EXISTING(p_new_data, p_cur_data)) {
        start_rlogin( p_cur_data, p_new_data );
    } else if( POTENTIAL_CHANGE_OF_EXISTING(p_new_data, p_cur_data)) {
        if( IS_TRIGGERED_BY_LOWER_LAYER(p_new_data) || rlogin_changed(p_cur_data, p_new_data)) {
            restart_rlogin( p_cur_data, p_new_data );
        }
    } else if( DELETE_OR_DISABLE_EXISTING(p_new_data, p_cur_data)) {
        stop_rlogin( p_cur_data, p_new_data );
    }

    CLEAR_TRIGGER_MARK( p_new_data );
	return rv;
}

