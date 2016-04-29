#ifndef __ACT_ZCONF_EXT_H
#define __ACT_ZCONF_EXT_H

#ifdef AEI_ZERO_CONF

#define ZCONF_EXT_LAUNCH_SCRIPT  "/etc/zconf.sh"
#define ZCONF_EXT_DAEMON_NAME    "zero_conf_vz"
#define ZCONF_ROUTER_TO_SNIFF "ToSniff"

#define UPDATE_ZCONF_EXT_STATUS( c, n, value ) \
    do { \
        tsl_bool b_chg = TSL_B_FALSE; \
        if( c ) { \
            if( 0!=tsl_strcmp((c)->status,value) ) { \
                b_chg = TSL_B_TRUE; \
            } \
        } else if( n ){ \
            if( 0!=tsl_strcmp((n)->status,value) ) { \
                b_chg = TSL_B_TRUE; \
            } \
        } \
        if( b_chg ) { \
            if( n ) { \
                UPDATE_LASTCHANGETIME( n ); \
                CTLMEM_REPLACE_STRING( (n)->status, value ); \
            } \
        } \
    } while(0);

tsl_rv_t stop_zconf_ext( _ZeroConfExtenderObject *p_cur_data, _ZeroConfExtenderObject *p_new_data );
tsl_rv_t start_zconf_ext( _ZeroConfExtenderObject *p_cur_data, _ZeroConfExtenderObject *p_new_data );
tsl_rv_t restart_zconf_ext( _ZeroConfExtenderObject *p_cur_data, _ZeroConfExtenderObject *p_new_data );
tsl_void_t triggerZconfExt( tsl_void_t );

#endif

#endif

