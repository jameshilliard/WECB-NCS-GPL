/*./auto/tr69_func_BridgingBridgePortObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"

#include "act_intf_stack.h"
#include "tr69_cms_object.h"
#include "libtr69_func.h"

#ifdef AEI_CTL_BRIDGE
#include "act_vlan_rtl.h"
#include <ctype.h>

#define CTL_RTL_VLAN
#define CTL_RTL_VLAN_DEFAULT_STATUS    (TSL_B_FALSE)

#endif

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_BridgingBridgePortObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.Bridging.Bridge.i.Port
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_BridgingBridgePortObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_BridgingBridgePortObject_value(tsl_char_t *p_oid_name, _BridgingBridgePortObject *p_cur_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

    CALC_LASTCHANGE( p_cur_data );
    rv = TR69_RT_SUCCESS_VALUE_CHANGED;
	return rv;
}

#ifdef AEI_CTL_BRIDGE

/*
 * Function: IsPortInBridge()
 *
 * Description: Whether the device belong to other BridgePort(s) within the bridge.
 *
 */
static tsl_bool_t isPortInBridge( tsl_char_t *p_oid_name,
                                     const tsl_char_t * name,
                                     const tsl_char_t * ExceptBridgePort )
{
    tsl_bool_t belong = TSL_B_FALSE;
    tsl_char_t strBridge[BUFLEN_256] = {0};
    tsl_char_t * ptr = NULL;
    tsl_rv_t rv = TSL_RV_FAIL;
    tr69_node_t *p_next_node = NULL;
    tsl_bool_t bEnable = TSL_B_FALSE;
    tsl_bool_t bMgmtPort = TSL_B_FALSE;
    tsl_char_t * pName = NULL;
    tsl_int_t type;

    snprintf( strBridge, sizeof(strBridge), "%s", p_oid_name );
    
    // Strip the '.1'
    ptr = rindex( strBridge, '.' );
    if( NULL == ptr ) {
        ctllog_error( "%s: invlaue input oid name '%s'", __FUNCTION__, p_oid_name );
        return belong;
    }
    ptr[0] = '\0';
    ctllog_notice( "isPortInBridge: '%s'", strBridge );

    do {
        tr69_get_next_node(strBridge, &p_next_node);
        if (p_next_node && p_next_node->node_type == TR69_NODE_TYPE_INSTANCE) {
            // Ignore the specified BridgePort
            if( 0 == tsl_strcmp( p_next_node->full_name, ExceptBridgePort )) {
                continue;
            }
            // Check Bridge Port Enable/Disable
            if( TSL_RV_SUC != (rv=ACCESS_LEAF( tr69_get_unfresh_leaf_data,
                            (void **) &bEnable, &type,
                            "%s.Enable", p_next_node->full_name))) {
                continue;
            }
            if( TSL_B_FALSE == bEnable )
                continue;
            // Check Bridge Port being ManagermentPort or NOT
            if( TSL_RV_SUC != (rv=ACCESS_LEAF( tr69_get_unfresh_leaf_data,
                            (void **) &bMgmtPort, &type,
                            "%s.ManagementPort", p_next_node->full_name))) {
                continue;
            }
            if( TSL_B_TRUE == bMgmtPort )
                continue;
            //If Bridge Port Name match, return TRUE
            if( TSL_RV_SUC != (rv=ACCESS_LEAF( tr69_get_unfresh_leaf_data,
                            (void **) &pName, &type,
                            "%s.Name", p_next_node->full_name))) {
                continue;
            }
            ctllog_notice( "pName: '%s'", pName );
            if( 0 == tsl_strcmp( pName, name )) {
                belong = TSL_B_TRUE;
            }
            CTLMEM_FREE_BUF_AND_NULL_PTR( pName );
            if( belong ) {
                ctllog_notice( "%s belong to bridge %s", name, p_oid_name );
                break;
            }
        }
    } while(p_next_node != NULL);

    return belong;
}

// This function can only be called by managementPort
static void processBridgeIntf( tsl_char_t *p_oid_name, _BridgingBridgePortObject * mngrPort )
{
    tsl_char_t lowerLayers[BUFLEN_1024] = {0};
    tsl_char_t * lowlayer = NULL;
    tsl_char_t * saveptr1;
    _BridgingBridgePortObject * subPort = NULL;

    snprintf( lowerLayers, sizeof(lowerLayers), "%s", mngrPort->lowerLayers );

    lowlayer = strtok_r( lowerLayers, ",", &saveptr1 );
    while( lowlayer != NULL ) {
        if( strlen(lowlayer) != 0 ) {
            if( TSL_RV_SUC != tr69_get_obj_data( lowlayer, (void **) &subPort)) {
                ctllog_error( "Fail to get object data of '%s'",lowlayer );
            } else {
                if( mngrPort->enable && subPort->enable ) {
                    if( (!IS_EMPTY_STRING(subPort->X_ACTIONTEC_COM_OldName))
                            && tsl_strcmp(subPort->X_ACTIONTEC_COM_OldName, subPort->name)) {
                        // If sub-port name changed, delif oldname at first
                        if( ! isPortInBridge( p_oid_name, subPort->X_ACTIONTEC_COM_OldName, lowlayer)) {
                            DO_SYSTEM( "brctl delif %s %s", mngrPort->name, subPort->X_ACTIONTEC_COM_OldName );
                        }
                    }
                    // Must avoid doing 'brctl addif' too early, so verifying lowerLayer status being UP.
                    // On system bootup, maye lowerlayer vlan virtual devices have not been created.
                    // LowerLayer Object will trigger Manegemant Port later.
                    if( 0 == tsl_strcmp(subPort->status, CTLVS_UP)) {
                        DO_SYSTEM( "brctl addif %s %s", mngrPort->name, subPort->name );
                    }
                } else {
                    if( ! IS_EMPTY_STRING(subPort->name)) {
                        if( ! isPortInBridge( p_oid_name, subPort->name, lowlayer )) {
                            DO_SYSTEM( "brctl delif %s %s", mngrPort->name, subPort->name );
                        }
                    }
                }
                tr69_free_obj_data( lowlayer, (void **) &subPort);
            }
        }
        lowlayer = strtok_r( NULL, ",", &saveptr1 );
    }
}


tsl_rv_t getVlanIdOfBridgePort( /*const*/ tsl_char_t * br_port_oid, tsl_int_t * pVlanId )
{
    tsl_rv_t rv = TSL_RV_FAIL;
    tsl_char_t strVlanPort[BUFLEN_256] = {0};
    tsl_char_t *ptr = NULL;
    tsl_char_t strPara[BUFLEN_256] = {0};
    tsl_int_t type;

    do {
        TSL_VASSERT( ! IS_EMPTY_STRING(br_port_oid) );
        TSL_VASSERT(  pVlanId );

        snprintf( strVlanPort, sizeof(strVlanPort), "%s", br_port_oid );
        ptr = strstr( strVlanPort, ".Port." );
        TSL_VASSERT( ptr );
        snprintf( ptr, sizeof(strVlanPort)-(ptr-strVlanPort+1), "%s", ".VLANPort" );

        ctllog_debug( "+++ begin to scan vlanport: '%s' +++", strVlanPort );
#if 0
        // JBB_TODO: tr69_get_next_obj_by_oid() has bug.
        // This solution will loop w/o end.
        _BridgingBridgeVlanPortObject * vlanport_obj = NULL;
        tr69_oid_stack_id siid = EMPTY_INSTANCE_ID_STACK;
        int cnt = 0;

        while ( tr69_get_next_obj_by_oid(strVlanPort, &siid, (void **)&vlanport_obj) == TSL_RV_SUC) {
			ctllog_notice( "port: %s", vlanport_obj->port );
            if( 0 == tsl_strcmp(vlanport_obj->port, br_port_oid) ) {
                snprintf( strPara, sizeof(strPara), "%s.VLANID", vlanport_obj->VLAN );
                tr69_free_obj_by_oid(strVlanPort, &siid, (void **) &vlanport_obj);
                rv = tr69_get_unfresh_leaf_data( strPara, (void **) nVlanId, &type);
                if( TSL_RV_SUC != rv ) {
                    ctllog_error( "tr69_get_unfresh_leaf_data() return FAIL" );
                }
                break;
            }
            tr69_free_obj_by_oid(strVlanPort, &siid, (void **) &vlanport_obj);

            // For safety
            if(cnt ++ > 5) {
                break;
            }
		}
#else
        tr69_node_t *p_next_node = NULL;
        tsl_char_t * strValue = NULL;

        do {
            tr69_get_next_node(strVlanPort, &p_next_node);
            if (p_next_node && p_next_node->node_type == TR69_NODE_TYPE_INSTANCE)
            {
                tsl_bool found = TSL_B_FALSE;

                snprintf( strPara, sizeof(strPara), "%s.Port", p_next_node->full_name);
                if((rv = tr69_get_unfresh_leaf_data( strPara, (void **) &strValue, &type)) != TSL_RV_SUC){
                    ctllog_error( "tr69_get_unfresh_leaf_data() return FAIL" );
                    break;
                }
                if( 0 == tsl_strcmp(strValue, br_port_oid) ) {
                    found = TSL_B_TRUE;
                } else {
                    rv = TSL_RV_FAIL;
                }
                CTLMEM_FREE_BUF_AND_NULL_PTR(strValue);

                if(found) {
                    snprintf( strPara, sizeof(strPara), "%s.VLAN", p_next_node->full_name);
                    if((rv = tr69_get_unfresh_leaf_data( strPara, (void **) &strValue, &type)) != TSL_RV_SUC){
                        ctllog_error( "tr69_get_unfresh_leaf_data() return FAIL" );
                        break;
                    }
                    snprintf( strPara, sizeof(strPara), "%s.VLANID", strValue );
                    CTLMEM_FREE_BUF_AND_NULL_PTR(strValue);

                    if((rv = tr69_get_unfresh_leaf_data( strPara, (void **) pVlanId, &type)) != TSL_RV_SUC){
                        ctllog_error( "tr69_get_unfresh_leaf_data() fail" );
                        break;
                    }
                    ctllog_notice( "get VLANID = %d",  *pVlanId );
                    break;
                }
            }
        } while(p_next_node != NULL);
#endif
        ctllog_debug( "--- end of scan vlanport ---" );
    } while(0);

    return rv;
}

tsl_char_t * generateShortName( tsl_char_t *dst, int size, const tsl_char_t * src )
{
    const tsl_char_t * pSrc = NULL;
    tsl_char_t * pDst = NULL;
    tsl_bool firstLetter = TSL_B_TRUE;

    do {
        if( (NULL==dst) || (size<=0) || (NULL==src)) {
            ctllog_warn( "Invalid Input Para!" );
            dst = NULL;
            break;
        }

        // If name is already short enough
        if( tsl_strlen(src) <= 4 ) {
            dst[size-1] = '\0';
            tsl_strncpy( dst, src, size-1 );
            break;
        }

        // wlan1-va0 -> w1-0
        pDst = dst;
        for( pSrc = src; pSrc[0] != '\0'; pSrc ++ ) {
            tsl_bool needCpy = TSL_B_TRUE;

            if( isalpha(pSrc[0]) ) {
                if( TSL_B_FALSE == firstLetter ) {
                    needCpy = TSL_B_FALSE;
                }
                firstLetter = TSL_B_FALSE;
            }
            if( needCpy ) {
                pDst[0] = pSrc[0];
                pDst ++;
                if( pDst - dst + 1 >= size )
                    break;
            }
        }
        pDst[0] = '\0';
    } while(0);
    return dst;
}

#ifdef CTL_RTL_VLAN
static tsl_bool_t isRtlVlanEnabledInBridge( const tsl_char_t * oid)
{
    tsl_bool_t bRtlVlanEnabledInBr = TSL_B_FALSE;
    tsl_char_t strBridge[BUFLEN_256] = {0};
    tsl_char_t strPara[BUFLEN_256] = {0};
    tsl_rv_t rv = TSL_RV_FAIL;
    tr69_node_t *p_next_node = NULL;
    tsl_bool_t bEnable = TSL_B_FALSE;
    tsl_bool_t bMgmtPort = TSL_B_FALSE;
    tsl_int_t nVlanID;
    tsl_int_t type;

    snprintf( strBridge, sizeof(strBridge), "%s.Port", oid );

    do {
        tr69_get_next_node(strBridge, &p_next_node);
        if (p_next_node && p_next_node->node_type == TR69_NODE_TYPE_INSTANCE) {
            // Check Bridge Port Enable/Disable
            snprintf( strPara, sizeof(strPara), "%s.Enable", p_next_node->full_name );
            if((rv = tr69_get_unfresh_leaf_data( strPara, (void **) &bEnable, &type)) != TSL_RV_SUC) {
                ctllog_error( "tr69_get_unfresh_leaf_data() return FAIL" );
                continue;
            }
            if( TSL_B_FALSE == bEnable )
                continue;
            // Check Bridge Port being ManagermentPort or NOT
            snprintf( strPara, sizeof(strPara), "%s.ManagementPort", p_next_node->full_name );
            if((rv = tr69_get_unfresh_leaf_data( strPara, (void **) &bMgmtPort, &type)) != TSL_RV_SUC) {
                ctllog_error( "tr69_get_unfresh_leaf_data() return FAIL" );
                continue;
            }
            if( TSL_B_TRUE == bMgmtPort )
                continue;
            //Check if VLAN configured for this Bridge Port
            if((rv = getVlanIdOfBridgePort(p_next_node->full_name, & nVlanID)) != TSL_RV_SUC) {
                continue;
            }
            if( IS_VALID_VLANID(nVlanID) ) {
                ctllog_notice( "valid VLAN ID %d", nVlanID );
                bRtlVlanEnabledInBr = TSL_B_TRUE;
                break;
            }
        }
    } while(p_next_node != NULL);

    return bRtlVlanEnabledInBr;
}
#endif


#ifdef CTL_RTL_VLAN
static tsl_bool_t bRtlVlanEnabled = CTL_RTL_VLAN_DEFAULT_STATUS;

static tsl_bool_t isRtlVlanEnabledBefore()
{
    return bRtlVlanEnabled;
}
#endif

static tsl_bool_t isRtlVlanEnabled()
{
#ifdef CTL_RTL_VLAN
    tr69_node_t *p_next_node = NULL;

    do {
        // When disable all VLAN port, keep the virtual interfaces (untag VLAN, e.g.: eth0.4094) in br0,
        // NOT change to real interface right away.
        // After reboot DUT next time, real interface (e.g.: eth0) would be used.
        if( bRtlVlanEnabled )
            break;
        // Check all non-managementPorts.
        // When VLAN being enabled on any port, RTL VLAN must be processed on all other ports.
        // e.g.: Supposed only Enable VLAN on SSID2 in br1, VLAN should also be processed on eth/SSID1 in br0.
        do {
            tr69_get_next_node(TR69_OID_BRIDGING_BRIDGE, &p_next_node);
            if (p_next_node && p_next_node->node_type == TR69_NODE_TYPE_INSTANCE) {
                if( tsl_strlen(p_next_node->full_name)-  tsl_strlen(TR69_OID_BRIDGING_BRIDGE) >= 3 ){
                    // tr69_get_next_node() will go through all sub-instances in recursion.
                    // But we only need go through sub-instance of depth 1. So ignore others useless nodes.
                    // By 2012/11/5, tr69_get_next_samedepth_node() has bug, can not be used.
                    continue;
                }
                // Even bridge disable, must check its sub-ports. Since its sub-ports could be enable.
                bRtlVlanEnabled = isRtlVlanEnabledInBridge( p_next_node->full_name );
                if( bRtlVlanEnabled ) {
                    ctllog_notice( "RTL VLAN being enable in br '%s'", p_next_node->full_name );
                    break;
                }
            }
        } while(p_next_node != NULL);
        ctllog_notice( "isRtlVlanEnabled return %d", bRtlVlanEnabled );
    }while(0);
    return bRtlVlanEnabled;
#else
    return TSL_B_FALSE;
#endif
}


#ifdef CTL_RTL_VLAN
static tsl_rv_t TriggerVLANUntagPortsInBridge(const tsl_char_t * oid)
{
    tsl_char_t strBridge[BUFLEN_256] = {0};
    tsl_char_t strPara[BUFLEN_256] = {0};
    tsl_rv_t rv = TSL_RV_FAIL;
    tr69_node_t *p_next_node = NULL;
    tsl_bool_t bEnable = TSL_B_FALSE;
    tsl_bool_t bMgmtPort = TSL_B_FALSE;
    tsl_int_t nVlanID;
    tsl_int_t type;
    tsl_bool_t b_trigger = TSL_B_TRUE;

    snprintf( strBridge, sizeof(strBridge), "%s.Port", oid );
    do {
        tr69_get_next_node(strBridge, &p_next_node);
        if (p_next_node && p_next_node->node_type == TR69_NODE_TYPE_INSTANCE) {
            // Check Bridge Port Enable/Disable
            snprintf( strPara, sizeof(strPara), "%s.Enable", p_next_node->full_name );
            if((rv = tr69_get_unfresh_leaf_data( strPara, (void **) &bEnable, &type)) != TSL_RV_SUC) {
                ctllog_error( "tr69_get_unfresh_leaf_data() return FAIL" );
                continue;
            }
            if( TSL_B_FALSE == bEnable )
                continue;
            // Check Bridge Port being ManagermentPort or NOT
            snprintf( strPara, sizeof(strPara), "%s.ManagementPort", p_next_node->full_name );
            if((rv = tr69_get_unfresh_leaf_data( strPara, (void **) &bMgmtPort, &type)) != TSL_RV_SUC) {
                ctllog_error( "tr69_get_unfresh_leaf_data() return FAIL" );
                continue;
            }
            if( TSL_B_TRUE == bMgmtPort )
                continue;
            //Check if VLAN configured for this Bridge Port
            if((rv = getVlanIdOfBridgePort(p_next_node->full_name, & nVlanID)) == TSL_RV_SUC) {
                if( IS_VALID_VLANID(nVlanID) ) {
                    ctllog_notice( "valid VLAN ID %d", nVlanID );
                    continue;
                }
            }
            // If VLAN Untag, need trigger.
            ctllog_notice( "Trigger untag bridge port %s", p_next_node->full_name );
            snprintf( strPara, sizeof(strPara), "%s.X_ACTIONTEC_COM_Trigger", p_next_node->full_name );
            rv = tr69_set_leaf_data( strPara, (void *) &b_trigger, TR69_NODE_LEAF_TYPE_UINT );
        }
    } while(p_next_node != NULL);

    return rv;
}

static tsl_rv_t TriggerVLANUntagPorts()
{
    tsl_rv_t rv = TSL_RV_FAIL;
    tr69_node_t *p_next_node = NULL;

    // Check all non-managementPorts.
    // If it being VLAN Untag, triiger it to also create viatual interface. Since VLAN has been configtured as enable.
    do {
        tr69_get_next_node(TR69_OID_BRIDGING_BRIDGE, &p_next_node);
        if (p_next_node && p_next_node->node_type == TR69_NODE_TYPE_INSTANCE) {
            if( tsl_strlen(p_next_node->full_name)-  tsl_strlen(TR69_OID_BRIDGING_BRIDGE) >= 3 ){
                // tr69_get_next_node() will go through all sub-instances in recursion.
                // But we only need go through sub-instance of depth 1. So ignore others useless nodes.
                // By 2012/11/5, tr69_get_next_samedepth_node() has bug, can not be used.
                continue;
            }
            // Even bridge disable, must check its sub-ports. Since its sub-ports could be enable.
            rv = TriggerVLANUntagPortsInBridge( p_next_node->full_name );
            if( rv != TSL_RV_SUC ) {
                ctllog_error( "Fail to trigger ports of bridge %s", p_next_node->full_name );
                break;
            }
        }
        rv = TSL_RV_SUC;
    } while(p_next_node != NULL);

    return rv;
}
#endif

static tsl_rv_t init_RTL_Vlan()
{
    static tsl_bool inited = TSL_B_FALSE;
    tsl_rv_t rv = TSL_RV_FAIL;

    do {
        if( inited ) {
            rv = TSL_RV_SUC;
            break;
        }
        inited = TSL_B_TRUE;

        // Init for Global
        // 1)Cannot use 0 as pvid
        // 2)set wlan0-va3 and wlan1-va3's pvid to 4094
        // 3)Set all the others to 1
        // 4)VLAN range: [1, 4094]
        if( DO_SYSTEM( "echo 1 > /proc/net/vlan/vlanEnable" ))
            break;
        if( DO_SYSTEM( "echo %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d > /proc/net/vlan/pvid",
                       //eth p0-p8
                       CTL_RESERVED_VLANID, CTL_RESERVED_VLANID, CTL_RESERVED_VLANID, CTL_RESERVED_VLANID,
                       CTL_RESERVED_VLANID, CTL_RESERVED_VLANID, CTL_RESERVED_VLANID, CTL_RESERVED_VLANID,
                       CTL_RESERVED_VLANID,

                       CTL_RESERVED_VLANID, //wlan0
                       CTL_RESERVED_VLANID, //wlan0-va0
                       CTL_RESERVED_VLANID, //wlan0-va1
                       CTL_RESERVED_VLANID, //wlan0-va2
                       CTL_SSID4_VLANID, //wlan0-va3
                       CTL_RESERVED_VLANID, // wlan0-vxd

                       CTL_RESERVED_VLANID, //wlan1
                       CTL_RESERVED_VLANID, //wlan1-va0
                       CTL_RESERVED_VLANID, //wlan1-va1
                       CTL_RESERVED_VLANID, //wlan1-va2
                       CTL_SSID4_VLANID //wlan1-va3
                       ))
            break;
        rv = TSL_RV_SUC;
        ctllog_notice( "init RTL VLAN successfully" );
    } while(0);

    return rv;
}

#if 0
static tsl_rv_t setup_RTL_Vlan_group( tsl_char_t *p_oid_name, _BridgingBridgePortObject *p_new_data, tsl_int_t nVlanID )
{
    // JBB_TODO: Must sleep 1 second here, otherwise LAN fail to ping DUT
    sleep(1);
    // Init for br0
    // 10423: Eth(P0,p1), MoCA (p5), WLAN0_VA0, WLAN1_VA0
    //     0: no VLAN tag
    DO_SYSTEM( "echo %d > /proc/net/vlan/groupIndex", CTL_RESERVED_VLANID );
    DO_SYSTEM( "echo 1,10423,0,%d > /proc/net/vlan/vlanGroup", CTL_RESERVED_VLANID );

    // Init for br1
    //1st 20823: Eth(P0,p1), MoCA (p5), WLAN0_VA1, WLAN1_VA1
    //2nd 20823: with VLAN tag
    DO_SYSTEM( "echo %d > /proc/net/vlan/groupIndex", 2 );
    DO_SYSTEM( "echo 1,20823,20823,%d > /proc/net/vlan/vlanGroup", 2 );

    rv = TSL_RV_SUC;
    return rv;
}
#else

static tsl_rv_t setup_RTL_Vlan_group( tsl_char_t *p_oid_name, _BridgingBridgePortObject *p_new_data, tsl_int_t nVlanID )
{
    tsl_int_t memberMask;
    tsl_rv_t rv = TSL_RV_FAIL;

    // JBB_TODO: Must sleep 1 second here, otherwise LAN fail to ping DUT
    sleep(1);

    do {
        // Init for br1
        //1st 20823: Eth(P0,p1), MoCA (p5), WLAN0_VA1, WLAN1_VA1
        //2nd 20823: with VLAN tag

        // JBB_TODO: By now, PVID is read only, auto sync with VLAN object
        p_new_data->PVID = nVlanID;

        memberMask = get_vlan_member_mask( p_oid_name, p_new_data->lowerLayers, nVlanID);

        if( 0 == tsl_strcmp(CTLVS_ADMITONLYVLANTAGGED, p_new_data->acceptableFrameTypes)) {
            if( DO_SYSTEM( "echo %d > /proc/net/vlan/groupIndex", nVlanID ))
                break;
            if( DO_SYSTEM( "echo 1,%x,%x,%d > /proc/net/vlan/vlanGroup",
                            memberMask, memberMask, nVlanID ))
                break;
        } else {
            if( DO_SYSTEM( "echo %d > /proc/net/vlan/groupIndex", nVlanID ))
                break;
            if( DO_SYSTEM( "echo 1,%x,%x,%d > /proc/net/vlan/vlanGroup",
                            memberMask, 0, nVlanID ))
                break;
        }
        rv = TSL_RV_SUC;
    } while(0);
    return rv;
}
#endif

static tsl_rv_t deleteBridgePort(tsl_char_t *p_oid_name, _BridgingBridgePortObject *p_cur_data, _BridgingBridgePortObject *p_new_data)
{
    tsl_rv_t rv = TSL_RV_FAIL;

    do {
        TSL_VASSERT( p_cur_data );
        TSL_VASSERT( tsl_strlen(p_cur_data->name) > 0 );
        // For virtual interfaces, there must be "."
        TSL_VASSERT( tsl_strstr(p_cur_data->name, ".") );

#if !defined(CTL_RTL_VLAN)
        if( 0 == tsl_strcmp(CTLVS_ADMITONLYVLANTAGGED, p_cur_data->acceptableFrameTypes))
#endif
        {
            DO_SYSTEM( "ip link delete link %s", p_cur_data->name );
        }
        rv = TSL_RV_SUC;
    } while(0);
    return rv;
}

// This function can only be called by non-managementPort
static tsl_rv_t createBridgePort(tsl_char_t *p_oid_name, _BridgingBridgePortObject *p_cur_data, _BridgingBridgePortObject *p_new_data)
{
    tsl_rv_t rv = TSL_RV_FAIL;
    tsl_char_t strShortName[BUFLEN_64] = {0};
    tsl_char_t strName[BUFLEN_64] = {0};
    tsl_char_t strllName[BUFLEN_64] = {0};
    tsl_int_t nVlanID = 0;

    do {
        if((rv = GET_LOWERLAYER_NAME(p_new_data, strName, sizeof(strName)))!= TSL_RV_SUC)
            break;

        if( 0 == tsl_strcmp(CTLVS_ADMITONLYVLANTAGGED, p_new_data->acceptableFrameTypes)) {
            // If VLAN mode, gererate new name for virtual Interface
            if((rv = getVlanIdOfBridgePort(p_oid_name, & nVlanID)) != TSL_RV_SUC)
                break;
            if(TSL_B_FALSE == IS_VALID_VLANID(nVlanID))
                break;
#ifdef CTL_RTL_VLAN
            if( p_cur_data && (TSL_B_FALSE == isRtlVlanEnabledBefore())) {
                // When config VLAN from disable to enable the 1st time, code comes here.
                // If p_cur_data being NULL, means on systemup. No need to trigger other ports.
                TriggerVLANUntagPorts();
            }
#endif
        }
#ifdef CTL_RTL_VLAN
        else {
            nVlanID = CTL_RESERVED_VLANID;
        }
#endif

        if( (0 == tsl_strcmp(CTLVS_ADMITONLYVLANTAGGED, p_new_data->acceptableFrameTypes))
                || isRtlVlanEnabled())
        {
            snprintf( strllName, sizeof(strllName), "%s", strName );
            if( NULL == generateShortName(strShortName, sizeof(strShortName), strName ))
                break;
            snprintf( strName, sizeof(strName), "%s.%d", strShortName, nVlanID );
        }

        do {
            if( ENABLE_NEW_OR_ENABLE_EXISTING(p_new_data, p_cur_data)) {
                // Create new VLAN virtual interface
                if( (0 == tsl_strcmp(CTLVS_ADMITONLYVLANTAGGED, p_new_data->acceptableFrameTypes))
                        || isRtlVlanEnabled())
                {
                    if( TSL_RV_SUC != (rv=init_RTL_Vlan()))
                        break;
                    if( DO_SYSTEM( "ip link add link %s name %s type vlan id %d", strllName, strName, nVlanID )) {
                        rv = TSL_RV_FAIL;
                        break;
                    }
                    if( DO_SYSTEM( "ifconfig %s up", strName )) {
                        rv = TSL_RV_FAIL;
                        break;
                    }
                    rv = setup_RTL_Vlan_group(p_oid_name,p_new_data, nVlanID);
                }
            } else if( POTENTIAL_CHANGE_OF_EXISTING(p_new_data, p_cur_data)) {
                if( 0 == tsl_strcmp(strName, p_cur_data->name)) {
                    // No changes, just skip
                    rv = TSL_RV_SUC;
                    break;
                }
                // Delete old VLAN virtual interface
                deleteBridgePort( p_oid_name, p_cur_data, p_new_data );
                // Create new VLAN virtual interface
                if( (0 == tsl_strcmp(CTLVS_ADMITONLYVLANTAGGED, p_new_data->acceptableFrameTypes))
                        || isRtlVlanEnabled())
                {
                    if( TSL_RV_SUC != (rv=init_RTL_Vlan()))
                        break;
                    if( DO_SYSTEM( "ip link add link %s name %s type vlan id %d", strllName, strName, nVlanID )) {
                        rv = TSL_RV_FAIL;
                        break;
                    }
                    if( DO_SYSTEM( "ifconfig %s up", strName )) {
                        rv = TSL_RV_FAIL;
                        break;
                    }
                    rv = setup_RTL_Vlan_group(p_oid_name,p_new_data, nVlanID);
                }
            } else {
                // JBB_TODO
                rv = TSL_RV_SUC;
            }
        } while(0);

        if( TSL_RV_SUC != rv ) {
            strName[0] = '\0';
        }
        if( 0 != tsl_strcmp(strName, p_new_data->name)) {
            CTLMEM_REPLACE_STRING(p_new_data->X_ACTIONTEC_COM_OldName, p_new_data->name);
            setUnfreshInterfaceStrPara( p_oid_name, "X_ACTIONTEC_COM_OldName", p_new_data->name );
            CTLMEM_REPLACE_STRING(p_new_data->name, strName);
            setUnfreshInterfaceStrPara( p_oid_name, "Name", strName );
        }
    } while(0);
    return rv;
}


#endif

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_set_BridgingBridgePortObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.Bridging.Bridge.i.Port
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_BridgingBridgePortObject_t *p_cur_data
 *	        st_BridgingBridgePortObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_BridgingBridgePortObject_value(tsl_char_t *p_oid_name, _BridgingBridgePortObject *p_cur_data, _BridgingBridgePortObject *p_new_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_CHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

#ifdef AEI_CTL_BRIDGE
    UPDATE_INTF_STACK( p_oid_name, p_cur_data, p_new_data );
    if( ENABLE_NEW_OR_ENABLE_EXISTING(p_new_data, p_cur_data)) {
        // Enable
        // JBB_TODO: How to set bridge status based on eth0/wlan0/wlan1 status
        // JBB_TODO: If 'lowerLayer 'changed, need delete old intf and add new intf
        if( p_new_data->managementPort ) {
            DO_SYSTEM( "brctl addbr %s", p_new_data->name );
            DO_SYSTEM( "ifconfig %s up", p_new_data->name );
            processBridgeIntf( p_oid_name, p_new_data );
            checkIntfStatusEx( p_oid_name, p_cur_data, p_new_data, "Device.Bridging.Bridge.1.Port.2",
                    TRIGGER_HIGHER_LAYER, p_oid_name );
        } else {
            createBridgePort( p_oid_name, p_cur_data, p_new_data );
            checkIntfStatusEx( p_oid_name, p_cur_data, p_new_data, NULL,
                    TRIGGER_HIGHER_LAYER, p_oid_name );
        }
    } else if( POTENTIAL_CHANGE_OF_EXISTING(p_new_data, p_cur_data)){
        if( IS_TRIGGERED_BY_LOWER_LAYER( p_new_data ) ) {
            if( p_new_data->managementPort ) {
                processBridgeIntf( p_oid_name, p_new_data );
                checkIntfStatusEx( p_oid_name, p_cur_data, p_new_data, "Device.Bridging.Bridge.1.Port.2",
                        TRIGGER_HIGHER_LAYER, p_oid_name );
            } else {
                createBridgePort( p_oid_name, p_cur_data, p_new_data );
                checkIntfStatusEx( p_oid_name, p_cur_data, p_new_data, NULL,
                        TRIGGER_HIGHER_LAYER, p_oid_name );
            }
        }
    } else if( DELETE_OR_DISABLE_EXISTING(p_new_data, p_cur_data)){
        // Disable
        if( p_cur_data->managementPort ) {
            processBridgeIntf( p_oid_name, p_cur_data );
#ifdef JBB_DEBUG
            DO_SYSTEM( "ifconfig %s down", p_cur_data->name );
#endif
            DO_SYSTEM( "brctl delbr %s", p_cur_data->name );
        } else {
#ifdef JBB_DEBUG
            deleteBridgePort( p_oid_name, p_cur_data, p_new_data );
#endif
        }
        UPDATE_INTF_STATUS_EX( p_oid_name, p_cur_data, p_new_data, CTLVS_DOWN,
                TRIGGER_HIGHER_LAYER, p_oid_name );
    }

    CLEAR_TRIGGER_MARK( p_new_data );
#endif

	return rv;
}

