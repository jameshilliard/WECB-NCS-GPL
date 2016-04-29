/*
############################################################################################
### This is the dal APIs prototype file.
### File path: openwrt/backfire_10.03/package/ctl/files/libs/private/dal/inculde/dal_api.h
### Date: 09/01/2011
############################################################################################
*/
#ifndef DAL_IPV6_API_H
#define DAL_IPV6_API_H

#ifndef IF_EXT_IPV6_SITEPREFIX
#define IF_EXT_IPV6_SITEPREFIX 	15	/*To modify IPv6SitePrefix  */
#endif
#ifndef IF_EXT_IPV6_ADDR_LINKLOCAL
#define IF_EXT_IPV6_ADDR_LINKLOCAL 16	/*To modify IPv6LinkLocalAddress  */
#endif
#ifndef IF_EXT_IPV6_TRANSTYPE
#define IF_EXT_IPV6_TRANSTYPE 17 /*DualStack or 6RD*/
#endif

#define IF_EXT_IPV6_ADDR_ULA 18 /*IPv6 ULA Address*/
#define IF_EXT_IPV6_ADDR_ULA_TYPE 19 /*X_AEI_COM_IPv6ULAAddressingType, Static*/
#define IF_EXT_IPV6_STATEFULENABLE 20   /*StatefulDHCPv6Server*/
#define IF_EXT_IPV6_MIN_INTERFACEID 21 /*MinInterfaceID*/
#define IF_EXT_IPV6_MAX_INTERFACEID 22 /*MaxInterfaceID*/
#define IF_EXT_IPV6_DHCPV6_LEASETIME 23 /*DHCPv6LeaseTime*/
#define IF_EXT_IPV6_ULA_PREFIX 24 /*IPv6ULAPrefix*/
#define IF_EXT_IPV6_SUBNETNUM 25 /*X_AEI_COM_IPv6SubnetNum */
#define IF_EXT_IPV6_CURRPREFIXID 26 /*X_AEI_COM_IPv6CurrPrefixID*/
#define IF_EXT_IPV6_RALIFETIME 27 /*X_AEI_COM_IPv6RALifetime*/

dal_ret_t* dal_get_ipv6_Device_list(char * devtype);

#endif
