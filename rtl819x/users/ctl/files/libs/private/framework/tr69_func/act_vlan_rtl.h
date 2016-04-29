#ifndef __ACT_VLAN_RTL_H__
#define __ACT_VLAN_RTL_H__

#include "tsl_common.h"

#define CTL_INVALID_VLANID      (-1)
#define CTL_MIN_VLANID          (1)
#define CTL_MAX_VLANID          (4093)
// JBB_TODO: Auto Select a used VLANID for br0
#define CTL_RESERVED_VLANID     (4094) //4095
#define CTL_SSID4_VLANID        (4093)

#define IS_VALID_VLANID(vid)     ((((vid)>=CTL_MIN_VLANID)&&((vid)<=CTL_MAX_VLANID))?TSL_B_TRUE:TSL_B_FALSE)


#define ETH_P0_MASK_BIT         0
#define ETH_P1_MASK_BIT         1
#define ETH_P2_MASK_BIT         2
#define ETH_P3_MASK_BIT         3
#define ETH_P4_MASK_BIT         4
#define ETH_P5_MASK_BIT         5
#define ETH_P6_MASK_BIT         6
#define ETH_P7_MASK_BIT         7
#define ETH_P8_MASK_BIT         8
#define WLAN0_MASK_BIT          9
#define WLAN0_VA0_MASK_BIT      10
#define WLAN0_VA1_MASK_BIT      11
#define WLAN0_VA2_MASK_BIT      12
#define WLAN0_VA3_MASK_BIT      13
#define WLAN0_VXD_MASK_BIT      14
#define WLAN1_MASK_BIT          15
#define WLAN1_VA0_MASK_BIT      16
#define WLAN1_VA1_MASK_BIT      17
#define WLAN1_VA2_MASK_BIT      18
#define WLAN1_VA3_MASK_BIT      19
#define WLAN1_VXD_MASK_BIT      20


#define MEMBER_MASK(port)   (1<<port)
#define MEMBER_MASK_ETH0    ( MEMBER_MASK(ETH_P0_MASK_BIT) \
                            | MEMBER_MASK(ETH_P1_MASK_BIT) \
                            | MEMBER_MASK(ETH_P5_MASK_BIT))

tsl_rv_t set_vlan_id( tsl_char_t * p_oid_name, tsl_char_t * loLayer, tsl_int_t nVlanID );
tsl_rv_t clear_vlan_id( tsl_char_t * p_oid_name, tsl_char_t * loLayer);
tsl_int_t get_vlan_member_mask( tsl_char_t * p_oid_name, tsl_char_t * loLayer, tsl_int_t nVlanID);

#endif

