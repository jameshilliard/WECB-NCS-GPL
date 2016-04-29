/*./auto/tr69_func_UsersUserObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"
#if defined( AEI_WECB_CUSTOMER_TELUS ) && defined (AEI_GENERATER_UNIQUELY_PASSWORD)
#include "aei_rut_password.h"
#include "tr69_cms_object.h"
#include "libtr69_func.h"
#endif
#ifdef AEI_USERPWD_MD5
#include "md5.h"
#include "ctl.h"
#include "ctl_mem.h"
#include "tsl_strconv.h"
#endif

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

#if defined( AEI_WECB_CUSTOMER_TELUS ) && defined (AEI_GENERATER_UNIQUELY_PASSWORD)
       /*Coverity CID 33336: Dereference before null check (REVERSE_INULL)*/
    if(p_new_data!=NULL){
        tsl_char_t * pSN = NULL;
        tsl_int_t type;
        tsl_char_t serialnum[BUFLEN_64]={0};
        if( TSL_RV_SUC != (rv = ACCESS_LEAF( tr69_get_unfresh_leaf_data,
                        (void **) &pSN, &type,
                        "%s.SerialNumber", TR69_OID_DEVICE_INFO ))) {
            ctllog_error( "Fail to get serial number" );
        }
        else
        {
            snprintf(serialnum,sizeof(serialnum)-1,"%s",pSN);
            CTLMEM_FREE_BUF_AND_NULL_PTR( pSN );
            ctllog_debug("serialnum is %s",serialnum);
        }
        if(!tsl_strcmp(p_new_data->username,"root"))
        {
            if((p_cur_data==NULL)||((p_cur_data!=NULL)&&(p_cur_data->X_ACTIONTEC_COM_RandomRootPassword!=p_new_data->X_ACTIONTEC_COM_RandomRootPassword)))
            {
                if(p_new_data->X_ACTIONTEC_COM_RandomRootPasswordEnable)
                {
                    char rootPassword[BUFLEN_64]={0};
                    AEI_rut_getRootPassword(rootPassword,sizeof(rootPassword),p_new_data->X_ACTIONTEC_COM_RandomRootPassword,serialnum,sizeof(serialnum));
                    ctllog_debug("++++++++root password :: %s ++++++++",rootPassword);
                    CTLMEM_REPLACE_STRING( p_new_data->password, rootPassword );
                }
            }
        }
        else if(!tsl_strcmp(p_new_data->username,"admin"))
        {
            if(p_new_data!=NULL)
            {
            char adminPassword[BUFLEN_64]={0};
            if(IS_EMPTY_STRING(p_new_data->password)){
                AEI_rut_getAdminPassword(adminPassword,serialnum);
                CTLMEM_REPLACE_STRING( p_new_data->password,adminPassword );
                ctllog_debug("p_data->password=%s",p_new_data->password);
                }
            }
        }
    }
#endif

#ifdef AEI_USERPWD_MD5
    do {
        int pwdlen;
        if( NULL == p_new_data )
            break;
        pwdlen = tsl_strlen(p_new_data->password);
        // pwd cannot be empty
        if( pwdlen == 0 ) {
            if( NULL != p_cur_data ) {
                rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;
            }
            break;
        }
        // If strlen(pwd) < 32, it must not encrypted by MD5 before
        // Auto encrypted here.
        if( pwdlen < 32 ) {
            struct MD5Context md5c;
            unsigned char signature[BUFLEN_16] = {0};
            char *hexfmt = "%02X";
            int j = 0;
            char pass_buf[BUFLEN_4] = {0};
            char new_password[BUFLEN_64] = {0};

            MD5Init(&md5c);
            MD5Update(&md5c, (unsigned char *)p_new_data->password, pwdlen );
            MD5Final(signature, &md5c);

            for (j = 0; j < sizeof signature; j++) {
                sprintf(pass_buf, hexfmt, signature[j]);
                ctllog_debug("pass_buf: %s", pass_buf);
                strcat(new_password, pass_buf);
            }

            ctllog_notice("Encrypt pwd '%s' -> '%s' (MD5)",
                    p_new_data->password, new_password );
            CTLMEM_REPLACE_STRING( p_new_data->password, new_password );
        }
    } while(0);
#endif
	return rv;
}

