#include <string.h>

#include "act_intf_stack.h"
#include "ctl.h"
#include "tsl_common.h"
#include "ctl_log.h"
#include "ctl_validstrings.h"
#include "tsl_strconv.h"
#include "libtr69_func.h"
#include "tr69_func.h"
#include "tr69_cms_object.h"
#include "tr69_func_common.h"

static tsl_rv_t _addSingleInterfaceStackInst(tsl_char_t * hiLayer,tsl_char_t * loLayer)
{
    tsl_rv_t rv = TSL_RV_FAIL;
    tsl_int_t inst_numb = 0;
    tsl_char_t strPara[BUFLEN_256] = {0};

    do {
        rv = tr69_add_instance( TR69_OID_INTERFACE_STACK, &inst_numb);

        if( rv != TSL_RV_SUC) {
            ctllog_error( "Fail to add instance of InterfaceStack!" );
            break;
        }

        snprintf( strPara, sizeof(strPara), "%s.%d.%s",
                TR69_OID_INTERFACE_STACK, inst_numb, "HigherLayer" );
        rv = tr69_set_unfresh_leaf_data( strPara, (void *) hiLayer, TR69_NODE_LEAF_TYPE_STRING );
        if( rv != TSL_RV_SUC) {
            ctllog_error( "Fail to set %s!", strPara );
            break;
        }

        snprintf( strPara, sizeof(strPara), "%s.%d.%s",
                TR69_OID_INTERFACE_STACK, inst_numb, "LowerLayer" );
        rv = tr69_set_unfresh_leaf_data( strPara, (void *) loLayer, TR69_NODE_LEAF_TYPE_STRING );
        if( rv != TSL_RV_SUC) {
            ctllog_error( "Fail to set %s!", strPara );
            break;
        }
    } while(0);
    return rv;
}

static tsl_rv_t _delSingleInterfaceStackInst(tsl_char_t * hiLayer,tsl_char_t * loLayers)
{   
    tsl_rv_t rv = TSL_RV_FAIL;
    
    // JBB_TODO: go through Interface Stack to find the matched one
    ctllog_error( "%s() NOT implemented yet.", __FUNCTION__ );
    return rv;
}

tsl_rv_t addInterfaceStackInst( tsl_char_t * hiLayer, const tsl_char_t * loLayers )
{
    tsl_rv_t rv = TSL_RV_FAIL;
    tsl_char_t strtmp[BUFLEN_1024];
    tsl_char_t *saveptr;
    tsl_char_t *loLayer = NULL;

    do {
        if( IS_EMPTY_STRING(hiLayer) ||IS_EMPTY_STRING(loLayers) ) {
            ctllog_warn( "Input empty string!" );
            break;
        }
        ctllog_debug( "add hi/los: %s/%s", hiLayer, loLayers );
        
        snprintf( strtmp, sizeof(strtmp), "%s", loLayers);
        loLayer = strtok_r( strtmp, ",", &saveptr);
        while( loLayer != NULL ) {
            ctllog_debug( "add hi/lo: %s/%s", hiLayer, loLayer );
            rv = _addSingleInterfaceStackInst( hiLayer, loLayer );
            if( rv != TSL_RV_SUC ) {
                break;
            }
            loLayer = strtok_r( NULL, ",", &saveptr);
        }
    } while(0);
    return rv;
}

tsl_rv_t delInterfaceStackInst( tsl_char_t * hiLayer, const tsl_char_t * loLayers )
{
    tsl_rv_t rv = TSL_RV_FAIL;
    tsl_char_t strtmp[BUFLEN_1024];
    tsl_char_t *saveptr;
    tsl_char_t *loLayer = NULL;

    do  {
        if( IS_EMPTY_STRING(hiLayer) ||IS_EMPTY_STRING(loLayers) ) {
            ctllog_warn( "Input empty string!" );
            break;
        }
        snprintf( strtmp, sizeof(strtmp), "%s", loLayers);
        loLayer = strtok_r( strtmp, ",", &saveptr);
        while( loLayer != NULL ) {
            rv = _delSingleInterfaceStackInst( hiLayer, loLayer );
            if( rv != TSL_RV_SUC ) {
                break;
            }
            loLayer = strtok_r( NULL, ",", &saveptr);
        }
        
    } while(0);
    return rv;
}


tsl_rv_t delAllInterfaceStackInst()
{
    tsl_rv_t rv = TSL_RV_SUC;
    _InterfaceStackObject * ifs_obj = NULL;
    tr69_oid_stack_id siid = EMPTY_INSTANCE_ID_STACK;
    tsl_char_t strFullname [BUFLEN_1024] = {0};

    do {
        while ( tr69_get_next_obj_by_oid(TR69_OID_INTERFACE_STACK, &siid, (void **)&ifs_obj) == TSL_RV_SUC) {
            // JBB_TODO, how to do if tr69_oid_to_fullname return NULL?
            snprintf( strFullname , sizeof(strFullname ), "%s",
                    tr69_oid_to_fullname(TR69_OID_INTERFACE_STACK, &siid, NULL));
            tr69_free_obj_by_oid(TR69_OID_INTERFACE_STACK, &siid, (void **) &ifs_obj);
            if( TSL_RV_SUC != tr69_del2_instance( strFullname ) ) {
                ctllog_warn( "Fail to del instance" );
            }
		}
    } while(0);
    ctllog_debug( "del all intf stack instance" );
    return rv;
}


tsl_rv_t getHigherLayerOid( /*const*/ tsl_char_t * oid, tsl_char_t * hl_oid, int size )
{
    tsl_rv_t rv = TSL_RV_FAIL;
    //tr69_node_t * p_node = NULL;

    do {
        if( IS_EMPTY_STRING(oid) || (NULL == hl_oid ) || (size <= 0)) {
            ctllog_error( "Input invalid param" );
            break;
        }
        TSL_VASSERT( ! IS_EMPTY_STRING(oid) );
        TSL_VASSERT( NULL != hl_oid );
        TSL_VASSERT( size > 0 );
 
#if 0
        _InterfaceStackObject * ifs_obj = NULL;
        tr69_oid_stack_id siid = EMPTY_INSTANCE_ID_STACK;

        while ( tr69_get_next_obj_by_oid(TR69_OID_INTERFACE_STACK, &siid, (void **)&ifs_obj) == TSL_RV_SUC) {
			ctllog_notice( "lowerLayer: %s", ifs_obj->lowerLayer );
            if( 0 == tsl_strcmp(ifs_obj->lowerLayer, oid) ) {
                snprintf( hl_oid, size, "%s", ifs_obj->higherLayer );
                tr69_free_obj_by_oid(TR69_OID_INTERFACE_STACK, &siid, (void **) &ifs_obj);
                rv = TSL_RV_SUC;
                break;
            }
            tr69_free_obj_by_oid(TR69_OID_INTERFACE_STACK, &siid, (void **) &ifs_obj);
		}
#else
        tr69_node_t *p_next_node = NULL;
        tsl_char_t strPara[BUFLEN_256] = {0};
        tsl_int_t type;
        tsl_char_t * strValue = NULL;

        do {
            tr69_get_next_node(TR69_OID_INTERFACE_STACK, &p_next_node);
            if (p_next_node && p_next_node->node_type == TR69_NODE_TYPE_INSTANCE)
            {
                tsl_bool found = TSL_B_FALSE;

                snprintf( strPara, sizeof(strPara), "%s.LowerLayer", p_next_node->full_name);
                if((rv = tr69_get_unfresh_leaf_data( strPara, (void **) &strValue, &type)) != TSL_RV_SUC){
                    ctllog_error( "tr69_get_unfresh_leaf_data() return FAIL" );
                    break;
                }
                if( 0 == tsl_strcmp(strValue, oid) ) {
                    found = TSL_B_TRUE;
                } else {
                    rv = TSL_RV_FAIL;
                }
                CTLMEM_FREE_BUF_AND_NULL_PTR(strValue);

                if(found) {
                    snprintf( strPara, sizeof(strPara), "%s.HigherLayer", p_next_node->full_name);
                    if((rv = tr69_get_unfresh_leaf_data( strPara, (void **) &strValue, &type)) != TSL_RV_SUC){
                        ctllog_error( "tr69_get_unfresh_leaf_data() return FAIL" );
                        break;
                    }
                    snprintf( hl_oid, size, "%s", strValue );
                    CTLMEM_FREE_BUF_AND_NULL_PTR(strValue);
                    break;
                }
            }
        } while(p_next_node != NULL);

#endif
    } while(0);

    return rv;
}

tsl_rv_t getInterfaceName( /*const*/ tsl_char_t * oid, tsl_char_t * name, int size )
{
    tsl_rv_t rv = TSL_RV_FAIL;
    tsl_char_t strPara[BUFLEN_256] = {0};
    tsl_char_t * pName = NULL;
    tsl_int_t type;

    do {
        TSL_VASSERT( ! IS_EMPTY_STRING(oid) );
        TSL_VASSERT( name );
        TSL_VASSERT( size > 0 );

        snprintf(strPara, sizeof(strPara), "%s.Name", oid );
		rv = tr69_get_unfresh_leaf_data( strPara, (void **) &pName, &type);
        if( TSL_RV_SUC != rv ) {
            ctllog_error( "tr69_get_unfresh_leaf_data() return FAIL" );
        }
        snprintf( name, size, "%s", pName );
        CTLMEM_FREE_BUF_AND_NULL_PTR( pName );
    } while(0);

    return rv;
}

tsl_rv_t triggerHigherLayerObj( tsl_char_t *p_oid_name )
{
    tsl_rv_t rv = TSL_RV_FAIL;
    do {
        tsl_char_t strtmp[BUFLEN_128] = {0};
        tsl_bool b_trigger = TSL_B_TRUE;

        rv = getHigherLayerOid( p_oid_name, strtmp, sizeof(strtmp));
        if( TSL_RV_SUC != rv ) { 
            ctllog_warn( "Fail to get Higher Layer of '%s'", p_oid_name );
            break;
        }
        ctllog_debug( "Succ to get Higher Layer of '%s': '%s'", p_oid_name, strtmp );
        strncat( strtmp, ".X_ACTIONTEC_COM_Trigger", sizeof(strtmp) - tsl_strlen(strtmp) -1 );
        rv = tr69_set_leaf_data( strtmp, (void *) &b_trigger, TR69_NODE_LEAF_TYPE_UINT );
    } while(0);
    return rv;
}

/*
 * keyLowerLayerOid: If keyLowerLayer being Up, then return TRUE
 *                   If this param NULL, only return TRUE when all LowerLayer being Up
 **/
tsl_bool IsAllLowerLayersUp( const char * strlowerLayers, const char *keyLowerLayerOid )
{
    tsl_bool bAllUp = TSL_B_TRUE;
    tsl_char_t * lowlayer = NULL;
    tsl_char_t * saveptr1;
    tsl_char_t strPara[BUFLEN_256] = {0};
    tsl_char_t * status = NULL;
    tsl_int_t type;
    tsl_char_t lowerLayers[BUFLEN_1024];

	//ctllog_notice( " == IsAllLowerLayersUp == " );
    do {
        if( IS_EMPTY_STRING(strlowerLayers)) {
            ctllog_error( "Param lowerLayers is EMPTY!" );
            bAllUp = TSL_B_FALSE;
            break;
        }
        snprintf( lowerLayers, sizeof(lowerLayers), "%s", strlowerLayers );
        lowlayer = strtok_r( lowerLayers, ",", &saveptr1 );
        while( lowlayer != NULL ) {
            if( strlen(lowlayer) != 0 ) {
                if( (!IS_EMPTY_STRING(keyLowerLayerOid)) &&
						(NULL == tsl_strstr(lowlayer, keyLowerLayerOid))) {
                    // Only care about the key Lower Layer Interface status
                    ctllog_notice( "Only care about the key Lower Layer Interface status, %s", lowlayer );
                } else {
                	snprintf(strPara, sizeof(strPara), "%s.Status", lowlayer );
                    // Don't use tr69_get_unfresh_leaf_data() here, otherwise, WIFI would not be added into bridge.
                    // Since status of SSID object would not be updated until call its GET function.
	                if( tr69_get_leaf_data( strPara, (void **) &status, &type) < 0) {
	                    ctllog_error( "tr69_get_leaf_data() return FAIL" );
	                    bAllUp = TSL_B_FALSE;
	                    break;
	                }
	                if( 0 != tsl_strcmp(status, CTLVS_UP)) {
	                    ctllog_debug( "lowerLayer '%s's status being '%s'", lowlayer, status );
						CTLMEM_FREE_BUF_AND_NULL_PTR( status );
	                    bAllUp = TSL_B_FALSE;
	                    break;
	                }
					CTLMEM_FREE_BUF_AND_NULL_PTR( status );
                }
            }
            lowlayer = strtok_r( NULL, ",", &saveptr1 );
        }
    } while(0);

    return bAllUp;
}

tsl_rv_t setUnfreshInterfaceStrPara( const tsl_char_t * oid, const tsl_char_t * para, const tsl_char_t * value )
{
    tsl_rv_t rv = TSL_RV_FAIL;
    tsl_char_t strPara[BUFLEN_256] = {0};

    do {
        TSL_VASSERT( oid );
        TSL_VASSERT( value );

        snprintf( strPara, sizeof(strPara), "%s.%s", oid, para );
        rv = tr69_set_unfresh_leaf_data( strPara, (void *) value, TR69_NODE_LEAF_TYPE_STRING );
    } while(0);
    return rv;
}


