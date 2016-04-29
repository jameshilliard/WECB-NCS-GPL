#ifndef TR69_FUNC_COMMON_H
#define TR69_FUNC_COMMON_H

#include "tr69_func.h"
#include "ctl.h"

tsl_int_t tr69_common_func_parse_oid(tsl_char_t *p_name, tsl_char_t oid_stack[32][128], tsl_int_t *max_stack);

tsl_int_t tr69_common_func_get_ip(tsl_char_t *ipaddr, tsl_char_t *netmask, tsl_char_t *ethname);

tsl_int_t tr69_common_func_get_link_local_address(tsl_char_t *lladdr, tsl_char_t *ifname);

char * tr69_oid_to_fullname(tr69_oid obj_oid, tr69_oid_stack_id *iidStack, char *param);

#define DEVICE_SERIAL_NUMBER_MAX_LENGTH 64
int tr69_common_func_get_serial_number(char* sn, int sn_len);

#ifdef DEBUG_CODE_REPLACE_PRATIC
#define PSEUDO_WAN_IF_NAME	"eth0"
#endif

#define MAX_OID_LEN		256
#define CHECK_RET(ret)	do{\
		if(((ret)<0)||((ret)>MAX_OID_LEN)){\
			ctllog_debug("%s,%d:create OID fail, ret:%d\n", __FUNCTION__, __LINE__,ret);\
			return -1;\
		}\
	}while(0)

// It's a reentrant version of DO_SYSTEM()
#define DO_SYSTEM_R( cmd, arg... ) \
    ({ \
        int retval; \
        snprintf( cmd, sizeof(cmd), ##arg ); \
        puts( cmd ); \
        ctllog_notice( "system run: '%s'", cmd ); \
        retval = system( cmd ); \
        if( retval != 0 ) { \
            ctllog_warn( "system( \"%s\" ) fail with (%d)", cmd, retval ); \
        } \
        retval; \
    })

extern tsl_char_t __g_cmd[BUFLEN_1024];
#define DO_SYSTEM( arg... )     DO_SYSTEM_R( __g_cmd, ##arg )


// It's a reentrant version of ACCESS_LEAF()
#define ACCESS_LEAF_R( func, data, type, strPara, arg... ) \
    ({ \
        tsl_rv_t retval; \
        snprintf( strPara, sizeof(strPara), ##arg ); \
        ctllog_debug( "ACCESS_LEAF_DATA: '%s'", strPara ); \
        retval = func( strPara, data, type ); \
        if( retval != TSL_RV_SUC ) { \
            ctllog_error( "ACCESS_LEAF( \"%s\" ) fail with (%d)", strPara, retval ); \
        } \
        retval; \
    })

extern tsl_char_t __g_strPara[BUFLEN_1024];
#define ACCESS_LEAF( func, data, type, arg... ) ACCESS_LEAF_R( func, data, type, __g_strPara, ##arg)

#endif
