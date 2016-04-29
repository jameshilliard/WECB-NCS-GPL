#ifndef __ACT_INTF_STACK_H__
#define __ACT_INTF_STACK_H__

#include "tsl_common.h"

#include <time.h>
#include "ctl_mem.h"
#include "ctl_validstrings.h"
#include "tsl_strconv.h"
#include "libtr69_func.h"
#include "assert.h"


///////////////////////////////
// Interface 'Last Change'
#define CTL_GET_CURRENTTIME() \
    ({ \
        struct timespec uptime_ts; \
        clock_gettime(CLOCK_MONOTONIC, & uptime_ts); \
        uptime_ts.tv_sec; \
    })

#define UPDATE_LASTCHANGETIME( n ) \
    do { \
        (n)->X_ACTIONTEC_COM_LastChangeTime = CTL_GET_CURRENTTIME(); \
    } while(0);

#define CALC_LASTCHANGE( n ) \
    do { \
        (n)->lastChange = CTL_GET_CURRENTTIME() - (n)->X_ACTIONTEC_COM_LastChangeTime; \
        rv = TR69_RT_SUCCESS_VALUE_CHANGED; \
    } while(0);


///////////////////////////////
// Interface 'Name'
#define GET_LOWERLAYER_NAME(n, pName, size) getInterfaceName( (n)->lowerLayers, pName, size )

///////////////////////////////
// Interface 'Status'

#define UPDATE_INTF_STATUS_EX_NOCB( oid, c, n, value ) \
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
                setUnfreshInterfaceStrPara( oid, "Status", value ); \
            } \
            rv = TR69_RT_SUCCESS_VALUE_CHANGED; \
        } \
    } while(0);


#define UPDATE_INTF_STATUS_EX( oid, c, n, value, cb_func, cb_param ) \
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
                setUnfreshInterfaceStrPara( oid, "Status", value ); \
            } \
            cb_func( cb_param ); \
            rv = TR69_RT_SUCCESS_VALUE_CHANGED; \
        } \
    } while(0);

//JBB_TODO: do nothing on status in GET()
/* 'assert(true)' below is a dymmy cb function that do nothing */
//#define UPDATE_INTF_STATUS( c, value) UPDATE_INTF_STATUS_EX(c, value, assert, TSL_B_TRUE )
#define UPDATE_INTF_STATUS( c, value) 

// On interface name changed, trigger HigherLayer object.
#define checkIntfNameEx( oid, c, n, cb_func, cb_param) \
  do { \
        if( POTENTIAL_CHANGE_OF_EXISTING(n,c)) { \
            if( (!tsl_strcmp((c)->status, (n)->status)) && tsl_strcmp((c)->name, (n)->name)) { \
                cb_func( cb_param ); \
            } \
        } \
  } while(0);

#define checkIntfStatusEx( oid, c, n, keyLowerLayerOid, cb_func, cb_param) \
  do { \
    if( (n) && (n)->enable ) { \
        if( IsAllLowerLayersUp((n)->lowerLayers, keyLowerLayerOid) ) { \
            if( 0 != tsl_strcmp((n)->status, CTLVS_UP)) { \
                UPDATE_INTF_STATUS_EX( oid, c, n, CTLVS_UP, cb_func, cb_param ); \
            } \
        } else { \
            if( 0 != tsl_strcmp((n)->status, CTLVS_LOWER_LAYER_DOWN)) { \
                UPDATE_INTF_STATUS_EX( oid, c, n, CTLVS_LOWER_LAYER_DOWN, cb_func, cb_param ); \
            } \
        } \
        checkIntfNameEx(oid, c, n, cb_func, cb_param); \
    } else { \
        if( 0 != tsl_strcmp((n)->status, CTLVS_DOWN)) { \
            UPDATE_INTF_STATUS_EX( oid, c, n, CTLVS_DOWN, cb_func, cb_param ); \
        } \
    } \
  } while(0);

//JBB_TODO: do nothing on status in GET()
//#define checkIntfStatus( c, keyLowerLayerOid) checkIntfStatusEx( c, keyLowerLayerOid, assert, TSL_B_TRUE ) 
#define checkIntfStatus( c, keyLowerLayerOid)

///////////////////////////////
// LowerLayer 'Trigger' HigherLayer
#define IS_TRIGGERED_BY_LOWER_LAYER(n) \
        ( (NULL != (n)) && (n)->X_ACTIONTEC_COM_Trigger )

#define TRIGGER_HIGHER_LAYER    triggerHigherLayerObj

#define CLEAR_TRIGGER_MARK(n) \
    do { \
        if( NULL != (n) ) { \
            (n)->X_ACTIONTEC_COM_Trigger = TSL_B_FALSE; \
        } \
    } while(0)


///////////////////////////////
// Update LowerLayer/HigherLayer relationship
//#define UPDATE_HI_LO_LAYER(oid, obj)  updateInterfaceStackInst(oid, (obj)?(obj)->lowerLayers:"" )


#define ADD_INTF_STACK_INST(oid, obj) addInterfaceStackInst(oid, (obj)->lowerLayers)
#define DEL_INTF_STACK_INST(oid, obj) delInterfaceStackInst(oid, (obj)->lowerLayers)

// JBB_TODO: call UPDATE_INTF_STACK at begin of SET_FUNCTION could change IntfStack, then cannot do 'brctl rm'.
#define UPDATE_INTF_STACK(oid, curObj, newObj) \
    do { \
        if( NULL == (curObj) ) { \
            if( newObj) { \
                ADD_INTF_STACK_INST( oid, newObj ); \
            } \
        } else { \
            if( NULL == (newObj) ) { \
                DEL_INTF_STACK_INST( oid, curObj ); \
            } else if( tsl_strcmp( (curObj)->lowerLayers, (newObj)->lowerLayers)) { \
                DEL_INTF_STACK_INST( oid, curObj ); \
                ADD_INTF_STACK_INST( oid, newObj ); \
            } \
        } \
    } while(0)

tsl_rv_t addInterfaceStackInst( tsl_char_t * hiLayer, const tsl_char_t * loLayers );
tsl_rv_t delInterfaceStackInst( tsl_char_t * hiLayer, const tsl_char_t * loLayers );
tsl_rv_t delAllInterfaceStackInst();

tsl_bool IsAllLowerLayersUp( const char * lowerLayers, const char *keyLowerLayerOid );
//tsl_rv_t getNextSameDepthInstObj( /*const*/ tsl_char_t * oid, tr69_node_t ** p_node, void ** obj );
tsl_rv_t getHigherLayerOid( /*const*/ tsl_char_t * oid, tsl_char_t * hl_oid, int size );
tsl_rv_t getInterfaceName( /*const*/ tsl_char_t * oid, tsl_char_t * name, int size );
tsl_rv_t triggerHigherLayerObj( tsl_char_t *p_oid_name );
tsl_rv_t setUnfreshInterfaceStrPara( const tsl_char_t * oid, const tsl_char_t * para, const tsl_char_t * value );
#endif
