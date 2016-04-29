#include "act_ip.h"

#include "libtr69_func.h"
#include "tr69_func.h"
#include "tr69_cms_object.h"
#include "tr69_func_common.h"
#include "act_l3_local.h"
#include "tsl_strconv.h"


static tsl_rv_t _updateIPv6AddressInst( tsl_char_t * oid, L3Addr_t * ip )
{
    tsl_rv_t rv = TSL_RV_FAIL;
    tsl_int_t bEnable = 1;

    do {
        TSL_VASSERT_BR( oid );

        if( ip ) {
            if( TSL_RV_SUC != (rv=ACCESS_LEAF(tr69_set_unfresh_leaf_data,
                        (void *) CTLVS_ENABLED, TR69_NODE_LEAF_TYPE_STRING,
                        "%s.Status", oid ))) {
                break;
            }
            if( TSL_RV_SUC != (rv=ACCESS_LEAF(tr69_set_unfresh_leaf_data,
                        (void *) CTLVS_PREFERRED, TR69_NODE_LEAF_TYPE_STRING,
                        "%s.IPAddressStatus", oid ))) {
                break;
            }
            if( TSL_RV_SUC != (rv=ACCESS_LEAF(tr69_set_unfresh_leaf_data,
                        (void *) ip->ipAddrStr, TR69_NODE_LEAF_TYPE_STRING,
                        "%s.IPAddress", oid ))) {
                break;
            }
            if( TSL_RV_SUC != (rv=ACCESS_LEAF(tr69_set_unfresh_leaf_data,
                        (void *) CTLVS_AUTOCONFIGURED, TR69_NODE_LEAF_TYPE_STRING,
                        "%s.Origin", oid ))) {
                break;
            }
            if( TSL_RV_SUC != (rv=ACCESS_LEAF(tr69_set_unfresh_leaf_data,
                        (void *) &bEnable, TR69_NODE_LEAF_TYPE_BOOL,
                        "%s.Enable", oid ))) {
                break;
            }
            //Already update info for this ip address, so clear it from ips list
            ip->ipAddrStr[0] = '\0';
        } else {
            if( TSL_RV_SUC != (rv=ACCESS_LEAF(tr69_set_unfresh_leaf_data,
                        (void *) CTLVS_DISABLE, TR69_NODE_LEAF_TYPE_STRING,
                        "%s.Status", oid ))) {
                break;
            }
            if( TSL_RV_SUC != (rv=ACCESS_LEAF(tr69_set_unfresh_leaf_data,
                        (void *) CTLVS_INVALID, TR69_NODE_LEAF_TYPE_STRING,
                        "%s.IPAddressStatus", oid ))) {
                break;
            }
            bEnable = 0;
            if( TSL_RV_SUC != (rv=ACCESS_LEAF(tr69_set_unfresh_leaf_data,
                        (void *) &bEnable, TR69_NODE_LEAF_TYPE_BOOL,
                        "%s.Enable", oid ))) {
                break;
            }
        }
    } while(0);
    return rv;
}

static tsl_rv_t _getFreeIpv6AddressInst( tsl_char_t * ipIntfIPv6Oid, tsl_char_t * inst_oid, tsl_int_t size )
{
    tsl_rv_t rv = TSL_RV_FAIL;
    tr69_node_t *p_next_node = NULL;
    tsl_char_t * pValue = NULL;
    tsl_int_t type;
    tsl_bool_t bEnable = TSL_B_FALSE;
    tsl_bool_t bFreeFound = TSL_B_FALSE;

    do {
        TSL_VASSERT_BR( ipIntfIPv6Oid );
        TSL_VASSERT_BR( inst_oid );
        TSL_VASSERT_BR( size > 0 );

        do {
            tr69_get_next_node(ipIntfIPv6Oid, &p_next_node);
            if (p_next_node && p_next_node->node_type == TR69_NODE_TYPE_INSTANCE) {
                // Skip those instances whost Origin was not 'Autoconfigured'
                if( TSL_RV_SUC != (rv=ACCESS_LEAF( tr69_get_unfresh_leaf_data,
                                (void **) &pValue, &type,
                                "%s.Origin", p_next_node->full_name))) {
                    break;
                }
                if( tsl_strcmp(pValue, CTLVS_AUTOCONFIGURED) ) {
                    CTLMEM_FREE_BUF_AND_NULL_PTR( pValue );
                    continue;
                }
                CTLMEM_FREE_BUF_AND_NULL_PTR( pValue );

                // Find the latest ip info from ips list, and do update in data-model
                if( TSL_RV_SUC != (rv=ACCESS_LEAF( tr69_get_unfresh_leaf_data,
                                (void **) &bEnable, &type,
                                "%s.Enable", p_next_node->full_name))) {
                    break;
                }
                if( TSL_B_FALSE == bEnable) {
                    bFreeFound = TSL_B_TRUE;
                    snprintf( inst_oid, size, "%s", p_next_node->full_name );
                    break;
                }
            }
        } while(p_next_node != NULL);
    } while(0);

    if( (TSL_RV_SUC == rv) && (TSL_B_FALSE == bFreeFound)) {
        rv = TSL_RV_FAIL;
    }

    return rv;
}

static tsl_rv_t _addNewIPv6AddressInst( tsl_char_t * ipIntfIPv6Oid, tsl_char_t * inst_oid, tsl_int_t size )
{
    tsl_rv_t rv = TSL_RV_FAIL;
    tsl_int_t inst_numb = 0;

    do {
        TSL_VASSERT_BR( ipIntfIPv6Oid );
        TSL_VASSERT_BR( inst_oid );
        TSL_VASSERT_BR( size > BUFLEN_64 );

        if( TSL_RV_SUC != (rv=tr69_add_instance( ipIntfIPv6Oid, & inst_numb))) {
            ctllog_error( "Fail to add instance for '%s', return %d!", ipIntfIPv6Oid, rv );
            break;
        }

        snprintf( inst_oid, size, "%s.%d", ipIntfIPv6Oid, inst_numb );
    } while(0);
    return rv;
}


static L3Addr_t * _FindActiveIpAddress( L3Addr_t * ips, tsl_int_t size, tsl_char_t * ipaddr )
{
    L3Addr_t * ptr = ips;
    int i;

    do {
        TSL_VASSERT_BR( ptr );
        TSL_VASSERT_BR( size );
        TSL_VASSERT_BR( !IS_EMPTY_STRING(ipaddr) );

        for(i=0; i< size; i++) {
            if( 0 == tsl_strcmp( ptr->ipAddrStr, ipaddr)) {
                return ptr;
            }
            ptr ++;
        }
    } while(0);
    return NULL;
}

static tsl_rv_t _UpdateExistingIPv6AddressInfo( tsl_char_t * ipIntfIPv6Oid , L3Addr_t * ips, tsl_int_t size )
{
    tsl_rv_t rv = TSL_RV_SUC;
    tr69_node_t *p_next_node = NULL;
    tsl_char_t * pValue = NULL;
    tsl_int_t type;

    do {
        tr69_get_next_node(ipIntfIPv6Oid, &p_next_node);
        if (p_next_node && p_next_node->node_type == TR69_NODE_TYPE_INSTANCE) {
            // Skip those instances whost Origin was not 'Autoconfigured'
            if( TSL_RV_SUC != (rv=ACCESS_LEAF( tr69_get_unfresh_leaf_data,
                            (void **) &pValue, &type,
                            "%s.Origin", p_next_node->full_name))) {
                break;
            }
            if( tsl_strcmp(pValue, CTLVS_AUTOCONFIGURED) ) {
                CTLMEM_FREE_BUF_AND_NULL_PTR( pValue );
                continue;
            }
            CTLMEM_FREE_BUF_AND_NULL_PTR( pValue );

            // Find the latest ip info from ips list, and do update in data-model
            if( TSL_RV_SUC != (rv=ACCESS_LEAF( tr69_get_unfresh_leaf_data,
                            (void **) &pValue, &type,
                            "%s.IPAddress", p_next_node->full_name))) {
                break;
            }
            if( TSL_RV_SUC != (rv=_updateIPv6AddressInst(
                            p_next_node->full_name,
                            _FindActiveIpAddress(ips, size, pValue)))) {
                break;
            }
        }
    } while(p_next_node != NULL);
    CTLMEM_FREE_BUF_AND_NULL_PTR( pValue );
    return rv;
}

// Return Value: new created Instance number
static tsl_uint_t _InsertNewIPv6AddressInfo( tsl_char_t * ipIntfIPv6Oid , L3Addr_t * ips, tsl_int_t size )
{
    int i;
    int add_cnt = 0;
    tsl_char_t fullname[BUFLEN_256] = {0};

    do {
        TSL_VASSERT_BR( ipIntfIPv6Oid );
        TSL_VASSERT_BR( ips );

        for(i=0; i<size; i++) {
            // Skip those Empty cell, which has already been updated into data-model.
            if( IS_EMPTY_STRING(ips[i].ipAddrStr))
                continue;

            // Insert new ip info into data-model
            if( TSL_RV_SUC != _getFreeIpv6AddressInst( ipIntfIPv6Oid, fullname, sizeof(fullname))) {
                if( TSL_RV_SUC != _addNewIPv6AddressInst(ipIntfIPv6Oid, fullname, sizeof(fullname))) {
                    continue;
                }
                add_cnt ++;
            }
            _updateIPv6AddressInst( fullname, &ips[i] );
        }
    } while(0);
    return add_cnt;
}


tsl_uint_t updateIPv6AddressInstUnderIPIntf( tsl_char_t * ipIntfOid )
{
    L3Addr_t ips[CTL_QUERY_IP_INFO_MAX_NUMB];
    // Before fix cannot login Motive issue, bypass IPv6 Info.
    // Only keep ULA for QA verify function.
    //L3lo_filter filter = { "br0", AF_INET6, IPV6_LLA | IPV6_ULA | IPV6_GUA };
    L3lo_filter filter = { "br0", AF_INET6, IPV6_ULA };
    tsl_int_t cnt = 0;
    tsl_char_t ipIntfIPv6Oid[BUFLEN_256] = {0};
    tsl_uint_t add_cnt = 0;

    do {
        //cnt = getL3LocalList( ips, 10, NULL );
        /* NULL means default filter: ("br0", AF_INET6, IPV6_LLA | IPV6_ULA | IPV6_GUA) */
        cnt = getL3LocalList( ips, CTL_QUERY_IP_INFO_MAX_NUMB, & filter );

        ctllog_debug( "ipaddr (totally %d):\n", cnt );
        snprintf( ipIntfIPv6Oid, sizeof(ipIntfIPv6Oid), "%s.IPv6Address", ipIntfOid );

        if( TSL_RV_SUC != _UpdateExistingIPv6AddressInfo( ipIntfIPv6Oid, ips, cnt ))
            break;
        add_cnt = _InsertNewIPv6AddressInfo( ipIntfIPv6Oid, ips, cnt );
    }while(0);
    return add_cnt;
}

/*
tsl_rv_t delAllIPv6AddressInstUnderIPIntf( tsl_char_t * oid )
{
    tsl_rv_t rv = TSL_RV_SUC;
    _InterfaceStackObject * ifs_obj = NULL;
    tr69_oid_stack_id siid = EMPTY_INSTANCE_ID_STACK;
    tsl_char_t strFullname [BUFLEN_1024] = {0};
    tsl_char_t strIPv6Oid[BUFLEN_1024] = {0};

    do {
        ctllog_notice( "++ del all ipv6 address instance under %s ++", oid );

        snprintf( strIPv6Oid, sizeof(strIPv6Oid), "%s.IPv6Address", oid );
        while ( tr69_get_next_obj_by_oid(strIPv6Oid, &siid, (void **)&ifs_obj) == TSL_RV_SUC) {
            // JBB_TODO, how to do if tr69_oid_to_fullname return NULL?
            snprintf( strFullname , sizeof(strFullname ), "%s",
                    tr69_oid_to_fullname(strIPv6Oid, &siid, NULL));
            tr69_free_obj_by_oid(strIPv6Oid, &siid, (void **) &ifs_obj);
            if( TSL_RV_SUC != tr69_del2_instance( strFullname ) ) {
                ctllog_warn( "Fail to del instance" );
            }
		}
    } while(0);
    ctllog_notice( "del all ipv6 address instance under %s", oid );
    return rv;
}
*/

