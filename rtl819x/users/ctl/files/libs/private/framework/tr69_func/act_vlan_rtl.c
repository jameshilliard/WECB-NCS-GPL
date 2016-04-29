#include "act_vlan_rtl.h"
#include "ctl.h"
#include "OID.h"
#include "tsl_strconv.h"

typedef struct {
    tsl_char_t loLayerName[BUFLEN_32];
    tsl_uint_t memberMask;
    tsl_char_t brOidName[BUFLEN_128];
    tsl_int_t pvid;
} rtl_vlan_port;

static rtl_vlan_port g_vlan_list [] = {
    // Ethernet/MoCA
    { "Ethernet", MEMBER_MASK_ETH0, "", CTL_INVALID_VLANID },
    // 5G
    { "SSID.1", MEMBER_MASK(WLAN0_VA0_MASK_BIT), "", CTL_INVALID_VLANID },
    { "SSID.2", MEMBER_MASK(WLAN0_VA1_MASK_BIT), "", CTL_INVALID_VLANID },
    { "SSID.3", MEMBER_MASK(WLAN0_VA2_MASK_BIT), "", CTL_INVALID_VLANID },
    { "SSID.4", MEMBER_MASK(WLAN0_VA3_MASK_BIT), "", CTL_INVALID_VLANID },
    // 2.4G
    { "SSID.5", MEMBER_MASK(WLAN1_VA0_MASK_BIT), "", CTL_INVALID_VLANID },
    { "SSID.6", MEMBER_MASK(WLAN1_VA1_MASK_BIT), "", CTL_INVALID_VLANID },
    { "SSID.7", MEMBER_MASK(WLAN1_VA2_MASK_BIT), "", CTL_INVALID_VLANID },
    { "SSID.8", MEMBER_MASK(WLAN1_VA3_MASK_BIT), "", CTL_INVALID_VLANID },
    { "", 0, "", 0 }
};


tsl_rv_t set_vlan_id( tsl_char_t * p_oid_name, tsl_char_t * loLayer, tsl_int_t nVlanID )
{
    tsl_rv_t rv = TSL_RV_FAIL;
    rtl_vlan_port * p = &g_vlan_list[0];

    do {
        // Set vlan ID
        while( '\0' != p->loLayerName[0] ) {
            if( tsl_strstr(loLayer,p->loLayerName)) {
                snprintf( p->brOidName, sizeof(TR69_OID_BRIDGING_BRIDGE_PORT), "%s", p_oid_name );
                p->pvid = nVlanID;

                //ctllog_notice( "found oid = %s, pvid = %d", p->brOidName, p->pvid );
                rv = TSL_RV_SUC;
                break;
            }
            p++;
        }
        if( TSL_RV_FAIL == rv) {
            ctllog_warn( "Unknow loLayer: '%s'", loLayer );
            break;
        }
    } while(0);
    return rv;
}

tsl_rv_t clear_vlan_id( tsl_char_t * p_oid_name, tsl_char_t * loLayer)
{
    return set_vlan_id( p_oid_name, loLayer, CTL_INVALID_VLANID );
}


tsl_int_t get_vlan_member_mask( tsl_char_t * p_oid_name, tsl_char_t * loLayer, tsl_int_t nVlanID)
{
    rtl_vlan_port * p = &g_vlan_list[0];
    tsl_int_t memberMask = 0;
    
    do {
        if( TSL_RV_SUC != set_vlan_id( p_oid_name, loLayer, nVlanID)) {
            break;
        }
        //ctllog_notice( "g_vlan_list = 0x%x", (unsigned int)p );
        // Search vlan ID
        while( '\0' != p->loLayerName[0] ) {
            //ctllog_notice( ">>> %s, %d, %s, %d", p->loLayerName, p->memberMask, p->brOidName, p->pvid );
            if( (p->pvid == nVlanID) && (tsl_strlen(p->brOidName)>0) && tsl_strstr(p_oid_name,p->brOidName)) {
                memberMask |= p->memberMask;
                //ctllog_notice( ">>> memberMask: %x (%x)", memberMask,    p->memberMask);
            }
            p ++;
        }
        // If called by WIFI, no matter which SSID, must set bits for Ethernet
        memberMask |= MEMBER_MASK_ETH0;
    } while(0);
    return memberMask;
}

