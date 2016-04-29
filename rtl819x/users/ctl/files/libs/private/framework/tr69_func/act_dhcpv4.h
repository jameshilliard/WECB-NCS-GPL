#ifndef __ACT_DHCPV4_H__
#define __ACT_DHCPV4_H__

#include "tr69_func.h"

void set_lan_dhcpc(char *iface, _Dhcpv4ClientObject *p_new_data);
tsl_rv_t stop_dhcpc( _Dhcpv4ClientObject *p_new_data );
tsl_void_t triggerDHCPv4Cient( tsl_void_t );

#ifdef AEI_NCS_CFG_SYNC
tsl_rv_t stop_dhcpd( );
tsl_rv_t start_dhcpd();
tsl_rv_t stop_dnsmasq( );
tsl_rv_t start_dnsmasq();
#endif

#endif
