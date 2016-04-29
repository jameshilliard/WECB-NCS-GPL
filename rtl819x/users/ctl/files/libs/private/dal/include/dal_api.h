/*
############################################################################################
### This is the dal APIs prototype file.
### File path: openwrt/backfire_10.03/package/ctl/files/libs/private/dal/inculde/dal_api.h
### Date: 07/29/2011
############################################################################################
*/

#ifndef DAL_API_H
#define DAL_API_H

#include <dirent.h>
#include <sys/utsname.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <net/if.h>
#include <net/route.h>
#include <fcntl.h>
#include <unistd.h>
#include "ctl.h"
#include "tsl_common.h"
#include "ctl_log.h"
#include "tr69_cms_object.h"
#include "OID.h"
#include "tsl_strconv.h"
#include "dal_comm.h"


typedef enum {
	DAL_NONE = 0,
	DAL_LAN ,
   	DAL_WAN,
} if_type;


#define DAL_TRUE "true"
#define DAL_FALSE "false"


//parameter name
#define KEY_IF_NAME				"X_ACTIONTEC_COM_X_IfName"
#define KEY_IF_MOCA_NAME		"IfName"
#define KEY_IF_ALIAS			"X_ACTIONTEC_IfName"


#define VAL_ENABLE				"Enable"
#define VAL_DISABLE				"Disable"
#define VAL_ADDRTYPE_DHCP		"DHCP"
#define VAL_ADDRTYPE_STATIC	"Static"
#define VAL_PPPOE				"PPPoE"
#define VAL_TYPE_LAN			"LANInterface"
#define VAL_TYPE_WAN			"WANInterface"
#define VAL_DNSTYPE_DHCP		"DHCP"
#define VAL_DNSTYPE_STATIC		"STATIC"
#define VAL_DHCP_RELAY 			"Rely"
#define VAL_STATUS_CONNECTED	"Connected"
#define VAL_ENABLED				"Enabled"
#define VAL_DISABLED			"Disabled"
#define VAL_ACTIVE				"Active"
#define VAL_INACTIVE			"Inactive"
#define VAL_ROUTE_MODE_NAT      "NAT"

#define VAL_ROUTE_MODE_ROUTE     "Route"
#define VAL_CONNECTED      "Connected"

#define MAX_VAL_SIZE				512
#define MAX_TOKEN_COUNT			10
#define MAX_TOKEN_LENGTH			90

#define DAL_MAX_PID_LEN 			256

#define OP_ADD						10
#define OP_MODIFY					11
#define OP_DEL						12

enum{
 	DEV_TYPE_BR = 0,
	DEV_TYPE_ETH_LAN,
	DEV_TYPE_MOCA_LAN,
	DEV_TYPE_WIFI_LAN,
	DEV_TYPE_ETH_WAN,
	DEV_TYPE_PPP_WAN,
	DEV_TYPE_LAST,
}dev_type;


int init_dal_ret_t(dal_ret_t* t);

/*************************************************************/
tsl_u32_t dal_get_pid(tsl_char_t *name,tsl_int_t len);
/*************************************************************/

void dal_wan_to_bridge(char *bridgeName, char *wanName);
void dal_bridge_to_wan(char *bridgeName, char *wanName);

char* dal_bridge_add(void);
tsl_rv_t dal_bridge_del(char *br_name);

void dal_bridge_set_enable(char *br_name, tsl_bool_t enable);
tsl_rv_t dal_bridge_add_port(char *br_name, char *port_name);
tsl_rv_t dal_bridge_del_port(char*br_name, char *port_name);
dal_ret_t* dal_get_available_device_list(void);

void dal_bridge_port_unregister(char *oid);
/*
char* type: LANInterface/WANInterface
*/
void dal_bridge_port_register(char *oid, char* type);
void dal_bridge_port_set_enable(char *oid, char *status);

/*************************************************************/

/* Get the LAN Device list
    e.g., ret->param[1]: br0
          ret->param[2]: wifi0
          ret->param[3]: NULL */
dal_ret_t* dal_get_LAN_Device_list(void);

/* Get the WAN Device list
    e.g., ret->param[1]: eth1
          ret->param[2]: gpon0
          ret->param[3]: NULL */
dal_ret_t* dal_get_WAN_Device_list(void);

/* Get the Bridge list
    e.g., ret->param[1]: br0
          ret->param[2]: br1
          ret->param[3]: NULL */
dal_ret_t* dal_get_bridge_Device_list(void);

/* Get the Physical list
    e.g., ret->param[1]: eth0
          ret->param[2]: eth1
          ret->param[3]: ath1
          ret->param[4]: ath2 */
dal_ret_t* dal_get_physical_Device_list(void);

/* Get the Port list of given Bridge
    e.g., ret->param[1]: eth0
          ret->param[2]: wifi0
          ret->param[3]: clink0
          ret->param[4]: NULL */
dal_ret_t* dal_get_bridge_port_list(char *br_name);

/* Get the Bridge List which enslave current port
    e.g., ret->param[1]: br0
          ret->param[2]: br1
          ret->param[4]: NULL */
dal_ret_t* dal_get_enslaved_bridge_by_port(char *port_name);

/* Get the ssid list
    e.g., ret->param[0]: name of ssid1
          ret->param[1]: ssid of ssid1
          ret->param[2]: name of ssid2
          ret->param[3]: ssid of ssid2
ret->param_num: 4
*/
dal_ret_t* dal_wifi_get_ssid_list(char *wifi_name);

/* Get Device List Based on OID */
tsl_rv_t dal_get_if_name_list_by_oid(tr69_oid oid,  dal_ret_t *dal_ret);
/*************************************************************/

/*
create, add and delete vlan interface
intf_name: interface name
    e.g., eth0, eth1, ath0,
type: the type of vlan 
    e.g., tag or untag
*/
tsl_rv_t dal_create_vlan_interface(char *intf_name, char *vlan_id,  char* type);
tsl_rv_t dal_vlan_del_inteface(char *intf_name);
char *dal_get_brNameByVlanid(char *vlanid);
tsl_rv_t dal_vlan_del_vlan(char *vlan_id);
dal_ret_t* dal_get_vlan_list();
dal_ret_t* dal_get_vlan_port_list(char *vlan_id);

/* Get Interface IP(Layer 3) settings, e.g. ret->param[IF_IP_ADDRESS] save the current IP */

#define IF_ENABLED 1	    			/* "Enabled"/"Disabled" */
#define IF_NAME 2		    		/* Description Name */
#define IF_CONN_STATUS 3		/* ConnectionStatus :"Unconfigured","Connecting","Connected","PendingDisconnect","Disconnecting",“Disconnected" */
#define IF_ADDRESS_TYPE 4		/* "NONE", "DHCP", "STATIC" */
#define IF_DEP_DEV_LIST 5		/* Dependency Device List, "br0", "eth0",... */
#define IF_CONN_TYPE 6			/* "Bridge", "Ethernet", "Wireless", "MoCA", "GPON"...*/
#define IF_STATUS_TYPE 7		/* "LAN", "WAN", "DMZ */
#define IF_IP_ADDRESS 8			/* IPv4/IPv6 Address, NULL means no IP available */
#define IF_NET_MASK 9			/* Netmask */
#define IF_DEF_GW 10			/* Default Gateway IP Address */
#define IF_EXT_IP_ADDRESS 11	/* External IP Address List xxx.xxx.xxx.xxx:xxx.xxx.xxx.xxx*/
#define IF_MTU_SIZE 12
#define IF_MAC_ADDR 13     	 	/* MAC Address Override */
#define IF_EXT_IP_ADDR_PRE	14	/*To modify additional IP, it is the preivous */
dal_ret_t* dal_if_get_ip_settings(char *if_name,int iftype);

/* Change the IP settings
   ip_type: DHCP, STATIC, NO_IP 
   ip: available when type is STATIC, otherwise NULL
   netmask: same as above,
   mtu: <0 : ignore, 0: auto, >0 value
   external_ips, NULL: igmore*/
tsl_rv_t dal_if_set_ip_settings(char *if_name, dal_ret_t* param,int iftype);

/*************************************************************/

#define IF_DNS_TYPE 1		/* "Disabled", "DHCP", "STATIC" */
#define IF_DNS_PRIMARY 2	/* dns1 */
#define IF_DNS_SECONDARY 3	/* dns2 */
dal_ret_t* dal_if_get_dns_settings(char *if_name,int iftype);

tsl_rv_t  dal_if_set_dns_settings(char *if_name, dal_ret_t* param,int iftype);

/*************************************************************/

#define IF_DHCPD_TYPE 1 	/* "Disabled", "Enabled", "Rely" */
#define IF_DHCPD_RANGE 2	/* start_ip:end_ip:net_mask|start_ip:end_ip:net_mask */
#define IF_DHCPD_WINS_SERVER 3
#define IF_DHCPD_LEASE_TIME 4	/* lease time */
#define IF_DHCPD_PROVIDE_HOST_NAME 5 /* extend option */
#define IF_DHCPD_60_VCI	6	/* DHCP 60 option */
dal_ret_t* dal_if_get_dhcpserver_settings(char *if_name);

tsl_rv_t  dal_if_set_dhcpserver_settings(char *if_name, dal_ret_t* param);
tsl_rv_t  dal_if_restore_dhcpserver_settings(char *if_name);/*DHCPServerConfigurable*/

/* Get interface list which DHCP Server is running on */
dal_ret_t* dal_if_get_dhcpserver_list(void);

/*************************************************************/

#define IF_DHCPC_TYPE 1     /* "Disabled", "DHCP", "Static" */
#define IF_DHCPC_STATIC_IP 2
#define IF_DHCPC_STATIC_MASK 3
dal_ret_t* dal_if_get_dhcpclient_settings(char *if_name);

/*************************************************************/

#define IF_PPPOE_AUTH_USER 1
#define IF_PPPOE_AUTH_PASSWORD 2
#define IF_PPPOE_AUTH_PROTO 3 /* PAP,CHAP,MS-CHAP,MS-CHAP2*/
#define IF_PPPOE_CONN_TRIGGER 4 /* OnDemand/AlwaysOn */
#define IF_PPPOE_SERV_NAME 5 /* String */
#define IF_PPPOE_RECONN_TIME 6 /* String */
dal_ret_t* dal_if_get_ppp_settings(char *if_name);
tsl_rv_t  dal_if_set_PPP_settings(char *if_name, dal_ret_t* param);



/*************************************************************/

#define IF_STATUS_L2_LINK 1	/* Layer 2 link down/up */
#define IF_STATUS_L3_LINK 2     /* Layer 3 IP Connecting/Connected/Disconnected/...*/
#define IF_STATUS_TIME_SPAN 3	/* Uptime */
dal_ret_t* dal_if_get_status(char *if_name);

/*************************************************************/

#define IF_STATIS_PKTS_SENT 1
#define IF_STATIS_PKTS_RECV 2
#define IF_STATIS_BYTES_SENT 3
#define IF_STATIS_BYTES_RECV 4
#define IF_STATIS_PKTS_ERROR 5
#define IF_STATIS_BYTES_ERROR 6
#define IF_STATIS_PKTS_DROP 7
#define IF_STATIS_BYTES_DROP 8
dal_ret_t* dal_if_get_statistics(char *if_name);

/*************************************************************/

#define IF_ROUTE_MODE 1 	/* NULL, "NAT", "Route" */
#define IF_ROUTE_METRIC 2	/* Route Metric*/
#define IF_ROUTE_DEFAULT 3	/* If it is a default Route, "1", "0" */
dal_ret_t* dal_if_get_route_settings(char *if_name);
tsl_rv_t  dal_if_set_route_settings(char *if_name, dal_ret_t* param);

#define IF_ROUTE_TBL_OP 0   /*"add","remove"*/
#define IF_ROUTE_TBL_SUBNET 1
#define IF_ROUTE_TBL_SUBMASK 2
#define IF_ROUTE_GW 3
#define IF_ROUTE_TBL_METRIC 4
//#define IF_ROUTE_IF 5       /*only used in modify operation*/
#define IF_ROUTE_TBL_NAME 6
#define IF_ROUTE_TBL_STATUS 7

dal_ret_t** dal_if_get_route_tables(char *if_name, int* entries_num);

tsl_rv_t dal_if_op_route_tables(char *if_name,dal_ret_t* param);

/*************************************************************/

/* My Network, Get Host List, including IP/MASK,MAC,Hostname(DNS resolve),Connection Type(Eth/Wifi/Moca),Status(Active/Inactive/Disconnect),Remote Access enabled(DMZ/PortMapping),Lease Type(Dynamic/Static/Predifined), Lease Epire Time, Device Type(PC/STB/PRINTER/IP-Camera/NAS)*/
#define MN_HOST_HOSTNAME 	 1
#define MN_HOST_IP_ADDRESS 2
#define MN_HOST_MAC		 3
#define MN_HOST_CONN_TYPE 4 		/* "Ethernet", "Wireless", "MoCA" */
#define MN_HOST_STATUS 5	 	/* "Active", "Inactive" */
#define MN_HOST_REMOTE_ACCESS 6 	/* "Enabled", "Disabled" */
#define MN_HOST_LEASE_TYPE 7 		/* "DHCP", "STATIC", "RESERVED" */
#define MN_HOST_EXPIRE_TIME 8 
#define MN_HOST_DEV_TYPE 9		/* "PC"(default), "STB", "PRINTER", "IP-Camera", "NAS" */
#define MN_HOST_ADDR_SOURCE	10 /*DHCP,Static*/
#define MN_HOST_ICON			11
dal_ret_t** dal_mn_get_host_list(char *if_name, int* entries_num);

#define MN_HOST_ETH_NUM 1
#define MN_HOST_WIFI_NUM 2
#define MN_HOST_MOCA_NUM 3
dal_ret_t* dal_mn_get_type_counter(char *if_name);

#define MN_SET_HOST_MAC_ADDRESS 1
#define MN_SET_HOST_HOSTNAME 2
#define MN_SET_HOST_DEV_TYPE 3		/* "PC"(default), "STB", "PRINTER", "IP-Camera", "NAS" */
tsl_rv_t dal_mn_set_host_info(char *if_name, dal_ret_t* param);

/*************************************************************/

/* Firewall */
int getPID(char * fullname);
/*-------------------------------------------------*
 * the PID format in firewall :
 * the format of ->param[***_PID] is "[AIP];[0-9];[0-9];[0-9]"
 * please see the function tsl_bool_t valid_FW_MultiObj_PID(char * pid);
*-------------------------------------------------*/

#define FW_EABLE 1      /* true / false */
#define FW_LEVEL 2 /* ret->param[FW_LEVEL] = "Off"/"High"/"Medium"/"Low" */
#define FW_INVALUE 3
#define FW_OUTVALUE 4
dal_ret_t* dal_fw_get_level(void);
tsl_rv_t dal_fw_set_level(dal_ret_t* param);

#define FW_ACCESS_CTRL_ENABLED 1
#define FW_ACCESS_CTRL_LOCAL_HOSTNAME 2 /*Hostname or IP*/
#define FW_ACCESS_CTRL_PROTOCOL 3       /* TCP/UDP/BOTH */
#define FW_ACCESS_CTRL_LOCAL_PORT 4     /* Port, Port-Port, "Port-Port,Port-Port" */
#define FW_ACCESS_CTRL_REMOTE_PORT 5    /* same as above */
#define FW_ACCESS_CTRL_SCHD_REF 6       /* Schedule */
#define FW_ACCESS_CTRL_PID 7   /* persistent id, -1 means new rule */
#define FW_ACCESS_CTRL_NET_OBJ 8   /* point to a predefined network obj */
#define FW_ACCESS_CTRL_SCHD_REF_FLAG 9
#define FW_ACCESS_CTRL_SERVICENAME_FLAG 10
#define FW_ACCESS_CTRL_SERVICENAME 11
tsl_rv_t dal_fw_get_access_ctrl_list(tsl_bool_t allow, int *entries_num, dal_ret_t** outList);

tsl_rv_t dal_fw_addset_allow_access_ctrl(dal_ret_t* param);
/* based on FW_ACCESS_CTRL_PID to locate and delete the rule */
tsl_rv_t dal_fw_del_allow_access_ctrl(dal_ret_t* param);

tsl_rv_t dal_fw_addset_block_access_ctrl(dal_ret_t* param);
/* based on FW_ACCESS_CTRL_PID to locate and delete the rule */
tsl_rv_t dal_fw_del_block_access_ctrl(dal_ret_t* param);

#define FW_PMAP_ENABLED 1
#define FW_PMAP_LOCAL_HOSTNAME 2        /*Hostname or IP*/
#define FW_PMAP_REMOTE_HOSTNAME 3       /*Hostname or IP*/
#define FW_PMAP_PROTOCOL 4      /* TCP/UDP/BOTH */
#define FW_PMAP_REMOTE_SRC_PORT 5       /* Port, Port-Port, "Port-Port,Port-Port" */
#define FW_PMAP_REMOTE_DST_PORT 6       /* same as above */
#define FW_PMAP_LOCAL_PORT 7    /* same as above */
#define FW_PMAP_SCHD_REF 8      /* Schedule */
#define FW_PMAP_SCHD_REF_FLAG 9
#define FW_PMAP_PID 10   /* persistent id, -1 means new rule */
#define FW_PMAP_NET_OBJ 11   /* point to a predefined network obj */
#define FW_PMAP_SERVICENAME 12   /* point to a predefined pmap service */
#define FW_PMAP_SERVICENAME_FLAG 13
tsl_rv_t dal_fw_get_pmap_list(char *wan_conn, int *entries_num, dal_ret_t** outList);/* (entries_num != NULL) ppp0, eth10, wildcast ???*/

tsl_rv_t dal_fw_addset_pmap(char *conn_name, dal_ret_t* param);
/* based on FW_PMAP_PID to locate and delete the rule */
tsl_rv_t dal_fw_del_pmap(char *conn_name, dal_ret_t* param);

#define FW_DMZ_ENABLE 1 /* ret->param[FW_DMZ_ENABLE] = "true/false" */
#define FW_DMZ_IP 2 /* ret->param[FW_DMZ_IP] = "192.168.1.1" */
dal_ret_t* dal_fw_get_dmz(void);
tsl_rv_t dal_fw_set_dmz(dal_ret_t* param);

#define FW_NAT_ENABLE 1 /* ret->param[FW_NAT_ENABLE] = "true/false" */
#define FW_NAT_LOCALHOST_IP 2	/* 192.168.1.1 */
#define FW_NAT_PUBLIC_IP 3 /* 10.10.10.10 */
#define FW_NAT_WAN_INTERF_NAME 4    /* all_wan eth10 pppoe */
#define FW_NAT_ENABLE_PMAP 5	    /* true / false */
#define FW_NAT_PID 6
tsl_rv_t dal_fw_get_nat(int *entries_num, dal_ret_t** outList);	//(entries_num != NULL)
tsl_rv_t dal_fw_set_nat(dal_ret_t* param);  //explicit pid need
tsl_rv_t dal_fw_add_nat(dal_ret_t* param);
tsl_rv_t dal_fw_del_nat(dal_ret_t* param);  //explicit pid need

//local mangage
#define FW_LM_PRIMARY_TELNET 1   /* macro DAL_TRUE or DAL_FALSE */
#define FW_LM_SECOND_TELNET 2   /* macro DAL_TRUE or DAL_FALSE */
#define FW_LM_SECURE_TELNET 3   /* macro DAL_TRUE or DAL_FALSE */
tsl_rv_t dal_fw_get_localManage(dal_ret_t** dal_ret);
tsl_rv_t dal_fw_set_localManage(dal_ret_t* dal_ret);

//remote manage
#define FW_RM_PRIMARY_TELNET 1   /* macro DAL_TRUE or DAL_FALSE */
#define FW_RM_SECOND_TELNET 2   /* macro DAL_TRUE or DAL_FALSE */
#define FW_RM_SECURE_TELNET 3   /* macro DAL_TRUE or DAL_FALSE */
#define FW_RM_PRIMARY_HTTP 4   /* macro DAL_TRUE or DAL_FALSE */
#define FW_RM_SECOND_HTTP 5   /* macro DAL_TRUE or DAL_FALSE */
#define FW_RM_PRIMARY_HTTPS 6   /* macro DAL_TRUE or DAL_FALSE */
#define FW_RM_SECOND_HTTPS 7   /* macro DAL_TRUE or DAL_FALSE */
#define FW_RM_ICMP_ECHO_REQ 8   /* macro DAL_TRUE or DAL_FALSE */
#define FW_RM_UDP_TRACE_RQE 9   /* macro DAL_TRUE or DAL_FALSE */
tsl_rv_t dal_fw_get_remoteManage(dal_ret_t** dal_ret);
tsl_rv_t dal_fw_set_remoteManage(dal_ret_t* dal_ret);


#define FW_ADV_FILTER_ENABLE 1 /* ret->param[FW_ADV_FILTER_ENABLE] = "true|false" */
#define FW_ADV_FILTER_SRCIP 2
#define FW_ADV_FILTER_SRCIPMASK 3
#define FW_ADV_FILTER_DSTIP 4
#define FW_ADV_FILTER_DSTIPMASK 5
#define FW_ADV_FILTER_PROTOCOL 6
#define FW_ADV_FILTER_SRCPORT 7
#define FW_ADV_FILTER_DSTPORT 8
#define FW_ADV_FILTER_DSCPVALUE_CHECK 9
#define FW_ADV_FILTER_DSCPVALUE 10
//#define FW_ADV_FILTER_DSCPCLASS 10
#define FW_ADV_FILTER_PRIORITY_CHECK 11
#define FW_ADV_FILTER_PRIORITY 12
#define FW_ADV_FILTER_MATCHLENGTH_CHECK 13
#define FW_ADV_FILTER_MATCHLENGTH 14
#define FW_ADV_FILTER_MATCHLENGTH_MAX 15
#define FW_ADV_FILTER_ACTION	16  /* "Drop" | "Reject" | "AcceptConnect" | "AcceptPacket" */
#define FW_ADV_FILTER_LOGGING	17
#define FW_ADV_FILTER_SERVICENAME_FLAG 18
#define FW_ADV_FILTER_SERVICENAME 19
#define FW_ADV_FILTER_SCHEDULERNAME_FLAG 20
#define FW_ADV_FILTER_SCHEDULERNAME 21
#define FW_ADV_FILTER_DEVICENAME 22
#define FW_ADV_FILTER_PID   23

/* if you need output rule operation, please set input == false*/
tsl_rv_t dal_fw_get_adv_filter(int *entries_num, dal_ret_t** outList, char * interfName, tsl_bool_t input);
tsl_rv_t dal_fw_addset_adv_filter(dal_ret_t* dal_ret, char * interfName, tsl_bool_t input);
tsl_rv_t dal_fw_del_adv_filter(dal_ret_t* dal_ret, char * interfName, tsl_bool_t input);

#define FW_P_CTRL_ENABLE 1	    /* ret->param[FW_P_CTRL_ENABLE] = "true/false" */
#define FW_P_CTRL_PID 2		    /* persistent id, -1 means new rule */
#define FW_P_CTRL_IPADDRESS 3	    /* 192.168.1.2;192.168.1.3*/
#define FW_P_CTRL_URLADDRESS 4	    /* Keyword:sexy;Website:www.baidu.com*/
#define FW_P_CTRL_RULENAME 5
#define FW_P_CTRL_RULEDESCRIPTION 6
#define FW_P_CTRL_TIMESTART 7	    /* 01:00 */
#define FW_P_CTRL_TIMEEND 8	    /* 01:00 */
#define FW_P_CTRL_WEEKDAYS 9	    /* Mon,Tue,Wed,Thu,Fri,Sat,Sun */
#define FW_P_CTRL_MACADDRESS 10
#define FW_P_CTRL_FILTERTYPE 11	    /* "Exclude" "Accept" "block_all" */
//HOSTNAME
//INACTIVEFLAG
//INACTIVEWEEKDAYS

tsl_rv_t dal_fw_get_parent_ctrl(int *entries_num, dal_ret_t** outList);
tsl_rv_t dal_fw_addset_parent_ctrl(dal_ret_t* dal_ret);
tsl_rv_t dal_fw_del_parent_ctrl(dal_ret_t* dal_ret);

/*************************************************************/

/* Wireless */
void dal_wifi_settings_commit( void);
void dal_wifi_get_finish( void );
dal_ret_t* dal_wifi_get_ip_settings(char *if_name);

#define WIFI_CONNECTION_TYPE "Wireless %s Access Point"

#define WIFI_VAP_ENABLED 1
#define WIFI_VAP_SSID 2
#define WIFI_VAP_CHANNEL 3
#define WIFI_VAP_KEEP_CHANNEL 4
#define WIFI_VAP_SEC_MODE 5 /* OFF, WEP, WPA, WPA2, WPA+WPA2 */
#define WIFI_VAP_8021X 6 /* Enable/Disable */
#define WIFI_ADV_SSID_BCAST_X 7   //  Enabled/Disabled          - InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.SSIDAdvertisementEnabled 
#define WIFI_ADV_MAC_AUTH_X 8     // Enabled/Disabled           - InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.MACAddressControlEnabled 
#define WIFI_ADV_COMPAT_MODE_X 9   //   see above               - InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.Standard
#define IF_STATIS_PKTS_SENT_X 10   //                           - InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.TotalPacketsSent
#define IF_STATIS_PKTS_RECV_X 11   //                           - InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.TotalPacketsReceived

dal_ret_t* dal_vap_get_basic_status(char *vap_name);
tsl_rv_t dal_vap_set_basic_status(char *vap_name, dal_ret_t* param);


#define WIFI_ADV_RADIO_ENABLED 1/* Enabled/Disabled */
#define WIFI_ADV_SSID_BCAST 2	/* Enabled/Disabled */
#define WIFI_ADV_MAC_AUTH 3	/* Enabled/Disabled */
/*
		0: legacy 11b/g mixed 
		1: legacy 11B only 
		2: legacy 11A only
		3: legacy 11a/b/g mixed
		4: legacy 11G only
		5: 11ABGN mixed
		6: 11N only
		7: 11GN mixed
		8: 11AN mixed
		9: 11BGN mixed
*/
#define WIFI_ADV_COMPAT_MODE 4 	/* see above */
#define WIFI_ADV_TRANS_POWER 5	/* Percent */
#define WIFI_ADV_BEACON_INTERVAL 6	/* ms */
#define WIFI_ADV_DTIM_INTERVAL 7	/* ms */
#define WIFI_ADV_FRAG_THRESHOLD 8
#define WIFI_ADV_RTS_THRESHOLD 9
#define WIFI_ADV_WMM_ENABLED 10
#define WIFI_ADV_WMM_POWER_SAVE 11
#define WIFI_ADV_MAC_AUTH_MODE 12
#define WIFI_ADV_MAC_LIST 13

dal_ret_t* dal_wifi_get_adv_settings(char *wifi_name);
tsl_rv_t dal_wifi_set_wpa_settings(char *wifi_name, dal_ret_t* param);

#define WIFI_ADV_CTS_PROTECT_MODE 1
#define WIFI_ADV_CTS_PROTECT_TYPE 2
#define WIFI_ADV_FRAME_BUST_MAX 3
#define WIFI_ADV_FRAME_BUST_TIME 4
#define WIFI_ADV_MSDU_AGGREG 5
#define WIFI_ADV_MPDU_AGGREG 6
#define WIFI_ADV_80211N_GUARD 7
#define WIFI_ADV_TRANS_RATE 8
dal_ret_t* dal_wifi_get_adv_status(char *wifi_name);
tsl_rv_t dal_wifi_set_adv_status(char *wifi_name, dal_ret_t* param);
/*Get static information*/
dal_ret_t* dal_wifi_get_statistics(char *if_name);

/*TODO*/
tsl_rv_t dal_wifi_get_sta_list(char *wifi_name, int* entries_num, dal_ret_t** outList);

/*************************************************************/

/* Switch Port(LAN/WAN) */
#define SW_PORT_SPEED 1
#define SW_PORT_DUPLEX 2 /* Full Duplex/Half Duplex */
#define SW_PORT_STATUS 3 /*Connected/Disconnected/....*/
#define SW_PORT_CRCERROR 4 /* NUM */
#define SW_PORT_TYPE 5  /* LAN/WAN */
#define SW_PORT_ID 6  /* NUM */
tsl_rv_t dal_hw_switch_get_port_stat(int* entries_num, dal_ret_t** outList);

tsl_rv_t dal_hw_switch_set_port_config(dal_ret_t* param);

/*************************************************************/

#define DAL_MAIN_WAN_IF_NAME 1 /* Interface Name */
#define DAL_MAIN_WAN_IP_ADDRESS 2 /* IPv4 */
#define DAL_MAIN_WAN_NET_MASK 3 /* IPv4 netmask */
#define DAL_MAIN_WAN_MAC 4 /* IPv4 MAC Address */
#define DAL_MAIN_WAN_DEF_GW 5 /* IPv4 Default Gateway */
#define DAL_MAIN_WAN_DEF_DNS 6 /* IPv4 Default DNS */
#define DAL_MAIN_WAN_L3_TYPE 7 /* IP or PPP */
#define DAL_MAIN_WAN_CONN_TYPE 8 /* DHCP or Static */
#define DAL_MAIN_WAN_PHY_TYPE 9 /* Phsical WAN type */
#define DAL_MAIN_WAN_CONN_STATUS 10 /*connection status*/
dal_ret_t *dal_get_main_wan(void);
dal_ret_t *dal_get_main_wan_ext(void);

/*************************************************************/

#define DAL_DEVINFO_NAME 	1/*ModelName*/
#define DAL_DEVINFO_DES 	2/*description*/
#define DAL_DEVINFO_PC 		3 /*Product Class*/
#define DAL_DEVINFO_SN 		4 /*Serial Number*/
#define DAL_DEVINFO_HW 	5 /*Hardware Verison*/
#define DAL_DEVINFO_SW 	6 /*Software Verison*/
#define DAL_DEVINFO_UT 		7 /*Uptime*/
dal_ret_t *dal_get_device_info(void);

/*************************************************************/

#define DAL_IF_NAME 1		    		/* Description Name */
#define DAL_IF_CONN_STATUS 2		/* ConnectionStatus :"Unconfigured","Connecting","Connected","PendingDisconnect","Disconnecting",“Disconnected" */
dal_ret_t *dal_get_conn_summary(char *conn_name);

/*************************************************************/

#define DAL_MOCA_CHANNEL 1 /*Channel*/
#define DAL_MOCA_PRIVACY 2 /*Enable/Disable*/
#define DAL_MOCA_PASSWD 3 /* string */
#define DAL_MOCA_CM_RATIO 4 /* CM Ratio */
dal_ret_t *dal_get_moca_settings(char *if_name);
tsl_rv_t dal_set_moca_settings(dal_ret_t* param);

#define DAL_MOCA_LINK_STATUS 1/* Up/Down */
#define DAL_MOCA_TX_RATE 2/*Mbps*/
#define DAL_MOCA_RX_RATE 3/*Mbps*/
dal_ret_t *dal_get_moca_speed(char *if_name);

#define DAL_MOCA_ASSO_ID          1
#define DAL_MOCA_ASSO_MAC         2
#define DAL_MOCA_ASSO_TX_RATE     3
#define DAL_MOCA_ASSO_RX_RATE     4
#define DAL_MOCA_ASSO_PHY_TX_RATE 5
#define DAL_MOCA_ASSO_PHY_RX_RATE 6
dal_ret_t** dal_get_moca_associates(char *if_name);

/*************************************************************/

#define PING_DIAG_STATE            1  /* result of ping diagnostics or set to "Requested" to start the test */
#define PING_DIAG_DEST             2  /* destination ip address or hostname */
#define PING_DIAG_PACKET_NUM       3  /* number of pings */
#define PING_DIAG_PACKET_SIZE      4  /* ping packet size, "0" will use default packet size */
#define PING_DIAG_DSCP             5  /* DSCP */
#define PING_DIAG_SUCC_COUNT       6  /* number of success pings */
#define PING_DIAG_FAIL_COUNT       7  /* number of fail pings */
#define PING_DIAG_AVE_REP_TIME     8  /* average reply time */
#define PING_DIAG_MIN_REP_TIME     9  /* min reply time */
#define PING_DIAG_MAX_REP_TIME     10 /* max reply time */

/* get ping diagnostics result
 * key: InternetGatewayDevice.IPPingDiagnostics
 */
dal_ret_t* dal_diag_get_ping_result(void);

/* do ping diagnostics
 * key: InternetGatewayDevice.IPPingDiagnostics
 */
tsl_rv_t dal_diag_ping(dal_ret_t* param);

#define MAC_CLONE_INTERFACE     1 /* set mac of device */
#define MAC_CLONE_ENABLE        2 /* true means to use clone mac address / false means to restore factory mac address */
#define MAC_CLONE_ORIG_MAC      3 /* originally mac address */
#define MAC_CLONE_CLONE_MAC     4 /* clone mac address */
#define MAC_CLONE_PID           5 /* -1  a new ddns entry*/

/*
 * get mac cloning list
 * key: InternetGatewayDevice.DhcpMacClone
 */
dal_ret_t* dal_srv_get_mac_clone_list(int *num);

/*
 * set mac clone configuration
 * key: InternetGatewayDevice.DhcpMacClone
 */
tsl_rv_t dal_srv_set_mac_clone_cfg(dal_ret_t* param);

/*igmp info monitor*/
#define IGMP_GROUP_ADDR         1
#define IGMP_FILTER_MODE     2
#define IGMP_SOURCE_LIST       3
#define IGMP_LAST_RP_TM    4
#define IGMP_TOTAL_TM       5
#define IGMP_TOTAL_JOINS    6
#define IGMP_TOTAL_LEAVS          7

/**************************************************************************
 *  *      [FUNCTION NAME]:
 *   *              dal_get_igmp_monitor_info
 *    *
 *     *      [DESCRIPTION]:
 *      *              get igmp monitor obj info
 *       **************************************************************************/

dal_ret_t* dal_get_igmp_monitor_info(int *num);


/*igmp proxy*/
#define IGMP_PROXY_ENABLE         1
#define IGMP_PROXY_VERSION     2
#define IGMP_PROXY_FASTLEAVE       3
#define IGMP_PROXY_ROBUSTNESS    4
#define IGMP_PROXY_QI       5
#define IGMP_PROXY_QRI    6
#define IGMP_PROXY_LMQI          7
#define IGMP_PROXY_MAX_GROUPS    8
#define IGMP_PROXY_MAX_DATA_SOURCES    9
#define IGMP_PROXY_MAX_GROUP_MEMBERS 10
#define IGMP_PROXY_LAN2LAN_ENABLE 11


/**************************************************************************
 *  *      [FUNCTION NAME]:
 *   *              dal_get_igmp_proxy_info
 *    *
 *     *      [DESCRIPTION]:
 *      *              get igmp proxy obj info
 *       **************************************************************************/

dal_ret_t* dal_get_igmp_proxy_info(void);


/**************************************************************************
 *  *      [FUNCTION NAME]:
 *   *              dal_set_igmp_proxy_parm
 *    *
 *     *      [DESCRIPTION]:
 *      *              set igmp proxy obj parm value
 *       *************************************************************************/

tsl_rv_t dal_set_igmp_proxy_parm(dal_ret_t* parm);



#define DDNS_ENABLE             1
#define DDNS_HOST_NAME          2
#define DDNS_USER_NAME          3
#define DDNS_PASSWORD           4
#define DDNS_INTERFACE          5    
#define DDNS_PROVIDE_NAME       6   /* dynamic dns provider */
#define DDNS_WILDCARD           7   /* any URL is ok */
#define DDNS_MAIL_EXCHANGER     8   /* mail exchange server address, This will redirect all emails arriving at the DDNS address to the mail server */
#define DDNS_BACKUPMX           9   /* select this check box to designate the mail exchanger server to be a backup server */
#define DDNS_OFFLINE            10  /* Disable the DDNS feature by clicking this check box. This feature is availabe only to users who have purchased some                                       type of upgrade creadit from the DDNS provider. */
#define DDNS_SSL_MODE           11  /*  SSL mode include None , Chain,  Direct */
#define DDNS_PID                12  /* -1 means a new ddns entry */

/* get all ddns entries */
dal_ret_t* dal_ddns_get_list(int* num);
/* modify or add a ddns entry */
tsl_rv_t dal_ddns_set_entry(dal_ret_t* param);
/* delete a ddns entry */
tsl_rv_t dal_ddns_delete_entry(dal_ret_t* param);

#define SYS_CFG_HOST_NAME                  1 /* Wireless broadband router's hostname */
#define SYS_CFG_DOMAIN_NAME                2 /* Local domain */
#define SYS_CFG_REFRESH_MONITOR            3 /* AutorefreshSystemMonitoring */
#define SYS_CFG_PROMPT_LAN                 4 /* PromptPasswordOnLan */
#define SYS_CFG_WARN_CHANGED               5 /* WarnBeforeConfigurationChanged */
#define SYS_CFG_SESSION_LIFETIME           6 /* Session lifetime */
#define SYS_CFG_NUM_USERS                  7 /* Configure number of concurrent users that can be logged into the router */


/* get basic router settings */ 
dal_ret_t* dal_sys_get_basic_router_cfg(void);
/* set basic router settings */ 
tsl_rv_t dal_sys_set_basic_router_cfg(dal_ret_t* param);

#define SYS_CFG_LOG_ENABLE                 1 /* Enable/Disable logging */
#define SYS_CFG_LOG_LOW_NOTIFY_ENABLE      2 /* Enable/Disable system logging low capacity notification */
#define SYS_CFG_LOG_PERCENT_NOTIFY         3 /* System log allowed capacity before email notification */
#define SYS_CFG_LOG_BUF_SIZE               4 /* System log buffer size (kb) */
#define SYS_CFG_LOG_LEVEL                  5 /* Remote system notify level, "None", "Error", "Warning", "Information" */
#define SYS_CFG_LOG_IP_ADDR                6 /* System log remote system host IP address */
#define SYS_CFG_LOG_SEC_LOW_NOTIFY_ENABLE  7 /* Enable/Disable security logging low capacity notification */
#define SYS_CFG_LOG_SEC_PERCENT_NOTIFY     8 /* Security log allowed capacity before email notification */
#define SYS_CFG_LOG_SEC_BUF_SZIE           9 /* Security log buffer size (kb) */
#define SYS_CFG_LOG_SEC_LEVEL              10 /* Remote security notify level, "None", "Error", "Warning", "Information" */
#define SYS_CFG_LOG_SEC_IP_ADDR            11 /* Security log remote system host IP address */

/* get log settings of the router */ 
dal_ret_t* dal_sys_get_log_cfg(void);
/* set log settings of the router */ 
tsl_rv_t dal_sys_set_log_cfg(dal_ret_t* param);

#define SYS_CFG_MAIL_SERVER                1 /* Email server address */
#define SYS_CFG_MAIL_FROM_ADDR             2 /* From email address */
#define SYS_CFG_MAIL_PORT                  3 /* Port */
#define SYS_CFG_MAIL_AUTH_ENABLE           4 /* Server authentication required or not */
#define SYS_CFG_MAIL_AUTH_USERNAME         5 /* Username, if authentication is required */
#define SYS_CFG_MAIL_AUTH_PASSWORD         6 /* Password, if authentication is required */

/* get mail server settings */ 
dal_ret_t* dal_sys_get_mail_cfg(void);
/* set mail server settings */ 
tsl_rv_t dal_sys_set_mail_cfg(dal_ret_t* param);

#define SYS_CFG_AUTO_WAN_DETECT_ENABLE     1 /* Enable/Disable auto WAN detection */
#define SYS_CFG_AUTO_PPP_TIMEOUT           2 /* PPP timeout (seconds) */
#define SYS_CFG_AUTO_DHCP_TIMEOUT          3 /* DHCP timeout (seconds) */
#define SYS_CFG_AUTO_NUM_CYCLES            4 /* Number of cycles */
#define SYS_CFG_AUTO_CONTINUOUS_ENABLE     5 /* Enable/Disable auto detection continuous trying */

/* get auto wan detection configuration */ 
dal_ret_t* dal_sys_get_auto_wan_cfg(void);
/* set auto wan detection configuration */ 
tsl_rv_t dal_sys_set_auto_wan_cfg(dal_ret_t* param);

#define SYS_REMOTE_MAN_PRI_HTTP_PORT     1 /* Primary HTTP port */
#define SYS_REMOTE_MAN_SEC_HTTP_PORT     2 /* Secondary HTTP port */
#define SYS_REMOTE_MAN_PRI_HTTPS_PORT    3 /* Primary HTTPS port */
#define SYS_REMOTE_MAN_SEC_HTTPS_PORT    4 /* Secondary HTTPS port */ 
#define SYS_REMOTE_MAN_PRI_TEL_PORT      5 /* Primary Telnet port */
#define SYS_REMOTE_MAN_SEC_TEL_PORT      6 /* Secondary Telnet port */
#define SYS_REMOTE_MAN_TEL_SSL_PORT      7 /* Secure Telnet over SSL port */
#define SYS_REMOTE_MAN_PRI_HTTPS_AUTH    8 /* Primary HTTPS Management Client Authentication, "None","Optional", "Required" */
#define SYS_REMOTE_MAN_SEC_HTTPS_AUTH    9 /* Secondary HTTPS Management Client Authentication, "None","Optional", "Required" */
#define SYS_REMOTE_MAN_TEL_SSL_AUTH      10 /* Secure Telnet over SSL Client Authentication, "None","Optional", "Required" */

/* get remote management configuration */ 
dal_ret_t* dal_sys_get_remote_man_cfg(void);
/* set remote management configuration */ 
tsl_rv_t dal_sys_set_remote_man_cfg(dal_ret_t* param);

#define SYS_CFG_INTERCEPTION_HTTP        1 /* Intercept HTTP traffic When no Internet Connection is Available */
/* get interception http configuration */
dal_ret_t* dal_sys_get_intercept_http_cfg(void);
/* set interception http configuration */ 
tsl_rv_t dal_sys_set_intercept_http_cfg(dal_ret_t* param);


#define SIG_ALG_ENABLE 1   /* true / false */

/* get/set SIP ALG configuration
 * key : InternetGatewayDevice.X_ACTIONTEC_GUI_ADV_CFG.VoIPAlgs.SIPALG
 */
tsl_bool_t dal_fw_get_sip_alg(void);
tsl_rv_t dal_fw_set_sip_alg(tsl_bool_t param);

//Scheduler Rules:
#define SCHEDULER_RULE_NAME       1 /* Scheduler rule name */
#define SCHEDULER_RULE_STATUS     2 /* Scheduler rule status: active or inactive */
#define SCHEDULER_RULE_PID        3 /* -1 means a new scheduler rule */

/* get the schuduler rule list.
 * num: output value, return the number of the rules in the list;
 */
dal_ret_t* dal_ipt_get_sche_rule_list(tsl_int_t *num);
/* add or modify a new scheduler rule */
tsl_rv_t dal_ipt_set_sche_rule(dal_ret_t* param,tsl_char_t *pBuffer,tsl_int_t size);
/* delete a scheduler rule, explicit pid needed */
tsl_rv_t dal_ipt_del_sche_rule(dal_ret_t* param);

#define SCHEDULE_DAY           1 /* Days of week */
#define SCHEDULE_START_TIME    2 /* Hour range start time (format: ??) */
#define SCHEDULE_END_TIME      3 /* Hour range end time (format:??) */
#define SCHEDULE_PID           4 /* -1 means a new schedule */

/* get the schedule list by scheduler rule pid.
 * rule_pid: scheduler rule pid;
 * num: output value, return the number of the schedules in the list;
 */
dal_ret_t* dal_ipt_get_schedule_list(const tsl_char_t* rule_pid, tsl_int_t* num);
/* add a schedule into a specific scheduler rule */
tsl_rv_t dal_ipt_set_schedule(tsl_char_t* rule_pid,dal_ret_t* param);
/* delete a schedule of a specific scheduler rule, explicit pid needed */
tsl_rv_t dal_ipt_del_schedule(tsl_char_t* rule_pid, dal_ret_t* param);

#define MGCP_ALG_ENABLE 1   /* true / false */

/* get/set MGCP ALG configuration
 * key : InternetGatewayDevice.X_ACTIONTEC_GUI_ADV_CFG.VoIPAlgs.H323ALG
 */
tsl_rv_t dal_fw_get_mgcp_alg(void);
tsl_rv_t dal_fw_set_mgcp_alg(tsl_bool_t param);

//Routing:

#define ROUTE_DEST_IP              1  /* destination ip address */                   
#define ROUTE_DEST_NETMASK         2  /* destination netmask */                   
#define ROUTE_GATEWAY              3  /* gateway ip address */
#define ROUTE_INTERFACE            4  /* gateway interface name */
#define ROUTE_METRIC               5  /* route metric option */
#define ROUTE_STATUS               6  /* route rule status */
#define ROUTE_PID                  7  /* -1 means a new route rule */

/* get the route rule list.
 * num: output value, return the number of route rules in the list;
 */
dal_ret_t* dal_route_get_rule_list(tsl_int_t *num);
/* modify or add a route rule, explicit pid needed */
tsl_rv_t dal_route_set_rule(dal_ret_t* param);
/* delete a route rule, explicit pid needed */
tsl_rv_t dal_route_del_rule(dal_ret_t* param);

#define ROUTE_PROTOCOL_IGMP_ENABLE              1  /* IGMP */
#define ROUTE_PROTOCOL_DOMAIN_ROUTING_ENABLE    2  /* Domain Routing */

/* get route protocols configuration */
dal_ret_t* dal_route_get_proto_cfg(void);
/* modify route protocols configuration */
tsl_rv_t dal_route_set_proto_cfg(dal_ret_t* param);

/* DNS Host Mapping */
#define DNS_HOSTMAPPING_HOST_NAME       1  /* dns host name */                   
#define DNS_HOSTMAPPING_HOST_IP         2  /* dns host ip address */                   
#define DNS_HOSTMAPPING_HOST_SRC_TYPE   3  /* dns host type */
#define DNS_HOSTMAPPING_PID             4  /* -1 means a new dns host mapping */                       

/* get the dns host mapping list.
 * num: output value, return the number of the host mappings in the list;
 */
dal_ret_t* dal_dns_get_host_mapping_list(int *num);
/* modify or add a dns host mapping, explicit pid needed */
tsl_rv_t dal_dns_set_host_mapping(dal_ret_t* param);
/* delete a dns host mapping, explicit pid needed */
tsl_rv_t dal_dns_del_host_mapping(dal_ret_t* param);

/*User*/
#define USER_FULLNAME  1 /* full name */
#define USER_NAME      2 /* user name */
#define USER_PASSWORD  3 /* password */
#define USER_AUTH_LEVEL 4 /*0:None; 1:Administrator,2:Limited*/
#define USER_PID       5 /* -1 means a new user */


/*InternetGatewayDevice.X_ACTIONTEC_Multiple_Users*/

/* get a list of all users */
dal_ret_t* dal_sys_get_user_list(int* num);

/* get specific user info by pid */
dal_ret_t* dal_sys_get_user(tsl_32_t pid);

/* modify or add a user, explicit pid needed */
tsl_rv_t dal_sys_set_user(dal_ret_t* param);

/* delete a user, explicit pid needed */
tsl_rv_t dal_sys_del_user(dal_ret_t* param);

/****** QOS : Traffic Priority ******/

#define QOS_TP_ENABLE                  1  /* (bool) "true": active, "false": inactive traffic priority rule */
#define QOS_TP_DEVICE_NAME             2  /* (string) interface name of the device */
#define QOS_TP_TRAFFIC_DIRECTION       3  /* (bool) "true" means transmit, "false" means receive */
#define QOS_TP_MATCH_DEST_IP           4  /* (string) destination IP address, NULL means "ANY" */
#define QOS_TP_MATCH_DEST_IP_EXCLUDE   5  /* (bool) exclude destination IP address */
#define QOS_TP_MATCH_SRC_IP            6  /* (string) source IP adress, NULL means "ANY" */
#define QOS_TP_MATCH_SRC_IP_EXCLUDE    7  /* (bool) exclude source IP address */
#define QOS_TP_MATCH_DEST_MAC          8  /* (string) destination MAC address */
#define QOS_TP_MATCH_DEST_MAC_EXCLUDE  9  /* (bool) exclude destination MAC address */
#define QOS_TP_MATCH_SRC_MAC           10 /* (string) source MAC address */
#define QOS_TP_MATCH_SRC_MAC_EXCLUDE   11 /* (bool) exclude source MAC address */
#define QOS_TP_MATCH_PROTOCOL          12 /* (string) "TCP", "UDP", "TCP or UDP" */
#define QOS_TP_MATCH_PROTOCOL_EXCLUDE  13 /* (bool) exclude protocol */
#define QOS_TP_MATCH_START_DEST_PORT   14 /* (int) -1 means "ANY" */
#define QOS_TP_MATCH_END_DEST_PORT     15 /* (int) */
#define QOS_TP_MATCH_DEST_PORT_EXCLUDE 16 /* (bool) exclude destination port */
#define QOS_TP_MATCH_START_SRC_PORT    17 /* (int) -1 means "ANY" */
#define QOS_TP_MATCH_END_SRC_PORT      18 /* (int) */
#define QOS_TP_MATCH_SRC_PORT_EXCLUDE  19 /* (bool) exclude source port */
#define QOS_TP_MATCH_DSCP_VALUE        20 /* (int) */
#define QOS_TP_MATCH_PRIORITY_VALUE    21 /* (int) */
#define QOS_TP_SET_DSCP_VALUE          22 /* (int) -2 means "automatic", -1 means not used */
#define QOS_TP_SET_PRIORITY_VALUE      23 /* (int) */
#define QOS_TP_RULE_PID                24 /* (int) -1 means a new rule */

/* get the Traffic Priority rules list.
 * if_name: interface name of the device;
 * num: output value, return the number of the rules in the list;
 */
dal_ret_t* dal_qos_get_tp_rule_list(char *if_name, int* num);
/* modify or add a Traffic Priority rule, explicit pid needed */
tsl_rv_t dal_qos_set_tp_rule(dal_ret_t* param);
/* delete a Traffic Priority rule, explicit pid needed */
tsl_rv_t dal_qos_del_tp_rule(dal_ret_t* param);

/****** QOS : Traffic Shaping ******/

#define QOS_TS_ENABLE           1 /* (bool) "true": active, "false": inactive traffic shaping rule */
#define QOS_TS_DEVICE_NAME      2 /* (string) interface name of the device */
#define QOS_TS_TX_BANDWIDTH     3 /* (int) -1 means not configured, 0 means "unlimited" (kbps) */
#define QOS_TS_RX_BANDWIDTH     4 /* (int) -1 means not configured, 0 means "unlimited" (kbps) */
#define QOS_TS_RULE_PID         5 /* (int) -1 means a new rule */

/* get the Traffic Shaping rules list.
 * num: output value, return the number of the rules in the list;
 */
dal_ret_t* dal_qos_get_ts_rule_list(int* num);
/* modify or add a Traffic Shaping rule, explicit pid needed */
tsl_rv_t dal_qos_set_ts_rule(dal_ret_t* param);
/* delete a Traffic Shaping rule, explicit pid needed */
tsl_rv_t dal_qos_del_ts_rule(dal_ret_t* param);

/************* QoS : DSCP settings *************/
#define QOS_DSCP_MAP_VALUE      1 /* DSCP value in hex */
#define QOS_DSCP_MAP_LEVEL      2 /* "Low", "Medium", "High" */
#define QOS_DSCP_MAP_8021P      3 /* 802.1p value : int */
#define QOS_DSCP_MAP_QUEUE      4 /* queue number : int */
#define QOS_DSCP_MAP_PID        5 /* -1 means a new rule */

/* get the DSCP mapping list.
 * num: output value, return the number of the mapping rules in the list;
 */
dal_ret_t* dal_qos_get_dscp_map_list(int* num);
/* modify or add a DSCP mapping rule, explicit pid needed */
tsl_rv_t dal_qos_set_dscp_map_rule(dal_ret_t* param);
/* delete a DSCP mapping rule, explicit pid needed */
tsl_rv_t dal_qos_del_dscp_map_rule(dal_ret_t* param);

/************* QoS : 802.1p settings *************/
#define QOS_8021P_MAP_VALUE      1 /* 802.1p value */
#define QOS_8021P_MAP_LEVEL      2 /* "Low", "Medium", "High" */
#define QOS_8021P_MAP_QUEUE      3 /* queue number : int */
#define QOS_8021P_MAP_PID        4 /* -1 means a new rule */

/* get the 802.1p mapping list.
 * num: output value, return the number of the mapping rules in the list;
 */
dal_ret_t* dal_qos_get_8021p_map_list(int* num);
/* modify a 802.1p mapping rule, explicit pid needed */
tsl_rv_t dal_qos_set_8021p_map_rule(dal_ret_t* param);
/***************end of QoS**********************/


/****** UPDATE : Firmware Updating ******/

#define ROUTER_UPDATE_STATUS        		1 /* log Router Update Status */
#define ROUTER_UPDATE_FILENAME        		2 /* log Router Update Status */
#define ROUTER_UPDATE_SWVERSION        		3 /* log Router Update SWVERSION */
#define ROUTER_UPDATE_ACTION        		4 /* begin Router Update action */
#define ROUTER_UPDATE_IMAGE_CHECK      		5 /* image validation check */
/* get the router update status */
dal_ret_t* dal_sys_get_router_update_status(void);
/* set to do router firmware upgrade */
//tsl_rv_t dal_sys_do_router_update(dal_ret_t* param);

// if return value is TSL_RV_SUC, firmware can be updated by local
//tsl_rv_t dal_check_router_upgrade(void);


#define REMOTE_UPGRADE_CONFIG_TYPE        	1 /* Automatic check disabled, 
											   Check for new versions and upgrade,
                                             * Check for new versions and notify */
#define REMOTE_UPGRADE_CONFIG_INTERVAL    	2 /* Check URL INTERVAL */
#define REMOTE_UPGRADE_CONFIG_URL 			3 /* Check at URL */
#define REMOTE_UPGRADE_ACTION 				4 /* Check at URL */


#define REMOTE_UPGRADE_STATUS_NEXTCHECKTIME	1 /*  */
#define REMOTE_UPGRADE_STATUS_CHECKSTATUS   2 /* Remotely firmware check Version */
#define REMOTE_UPGRADE_STATUS_INTERNETVER   3 /* Remotely firmware check status */

/* get the remotely firmware upgrade status */
dal_ret_t* dal_sys_get_remote_upgrade_status(void);

/* get the remotely firmware upgrade config */
dal_ret_t* dal_sys_get_remote_upgrade_config(void);

/* set to do remotely firmware upgrade */
tsl_rv_t dal_sys_do_remote_upgrade(dal_ret_t* param);
//int http_upgrade_get(const char *url, char *buff, int nbytes, int *content_length); 


/*ARP Table*/
#define ARP_IP_ADDR        1 /* ip address */
#define ARP_MAC_ADDR       2 /* mac address */
#define ARP_DEVICE_NAME    3 /* device name */

/*proc/net/arp*/

/* get the arp table */
dal_ret_t* dal_srv_get_arp_table(int* num);
/*Port Configuration*/
#define PORT_CFG_INTERFACE         1 /* interface name of the device */
#define PORT_CFG_MANUAL_SPEED_DUPLEX              2 /* manual detect speed and deplex or not */
#define PORT_CFG_SPEED             3 /* speed: 10 / 100 / 1000 (M) */
#define PORT_CFG_DUPLEX            4 /* duplex: "half" or "full")*/
#define PORT_CFG_LINKSTATUS        5 /* true means connected, false means disconnected */

/* get port status by interface name */
dal_ret_t* dal_if_get_port_status(char *ifname);
/* set port configuration */
tsl_rv_t dal_if_set_port_cfg(dal_ret_t* param);

/*Configuration File*/
#define LOCAL_CONFIG_FILE			1
#define LOCAL_CONFIG_FILE_LENGTH	2
#define LOCAL_CONFIG_FILE_SRC		3
#define LOCAL_CONFIG_FILE_NAME      4


#define CONFIG_ENC_LENGTH  1024
#define CONFIG_FILE_MAX_LENGTH  1024*1024
/* get the router's current configuration*/
dal_ret_t* dal_sys_get_cfg_file();
/*load a previously saved configuration file*/
tsl_rv_t dal_sys_load_cfg_file(dal_ret_t* param);

/* restore all factory settings to default */
tsl_rv_t dal_sys_restore_def(void);

/*reboot the router */
tsl_rv_t dal_sys_reboot(void);

/*NTP Time Server */
#define SYS_DATETIME_TIME_SERVER           1 /* IP address or host name of the time server */
#define SYS_DATETIME_TIME_SERVER_PID       2 /* -1 means a new time server */

#define SYS_DATETIME_TIME_UPDATE_ENABLE    1 /* Enable or disable automatic time update */
#define SYS_DATETIME_TIME_UPDATE_PROTO     2 /* Currently will always be "NTP" */
#define SYS_DATETIME_TIME_UPDATE_INTERVAL  3 /* Automatic time update interval */
/* get the time server list.num: output value, return the number of time server in the list;*/
dal_ret_t* dal_sys_get_time_server_list(int *num);
/* modify or add a time server, explicit pid needed */
tsl_rv_t dal_sys_set_time_server(dal_ret_t* param);
/* delete a time server, explicit pid needed */
tsl_rv_t dal_sys_del_time_server(dal_ret_t* param);
/*get ntp client update config*/
dal_ret_t* dal_sys_get_ntpclient_cfg(void);
/*set ntp client update config*/
tsl_rv_t dal_sys_set_ntpclient(dal_ret_t* param);

#define SYS_DATETIME_TIME_SYNC_STATE       1 /* result of time sync or set to "Requested" to start the time sync */
/* get the time sync result */
dal_ret_t* dal_sys_get_time_sync_result(void);
/* set to "Requested" to do time sync */
tsl_rv_t dal_sys_do_time_sync(void);

/*Date and time*/
#define SYS_DATETIME_CURRENT_TIME    1 /* Local time */
/* get the current time of the router */
dal_ret_t* dal_sys_get_current_time(void);
#define SYS_DATETIME_TIME_ZONE            1 /* Time zone */
#define SYS_DATETIME_DAYLIGHT_ENABLE      2 /* Enable or disable daylight saving time */
#define SYS_DATETIME_DAYLIGHT_START_TIME  3 /* Daylight saving start time, format(??) */
#define SYS_DATETIME_DAYLIGHT_END_TIME    4 /* Daylight saving end time, format(??) */
#define SYS_DATETIME_DAYLIGHT_OFFSET    5 /* Daylight saving offset, format(??) */



/* get datetime configuration */
dal_ret_t* dal_sys_get_datetime_cfg(void);
/* set datetime configuration */
tsl_rv_t dal_sys_set_datatime_cfg(dal_ret_t* param);

#define SYS_CLOCK_CFG_YEAR      1 /* year */
#define SYS_CLOCK_CFG_MONTH     2 /* month */
#define SYS_CLOCK_CFG_DAY       3 /* day */
#define SYS_CLOCK_CFG_HOUR      4 /* hour */
#define SYS_CLOCK_CFG_MINUTE    5 /* minute */
#define SYS_CLOCK_CFG_SECOND    6 /* second */
/* get the clock setting of the router */
dal_ret_t* dal_sys_get_clock_cfg(void);
/* set the clock setting */
tsl_rv_t dal_sys_set_clock_cfg(dal_ret_t* param);


/* Port forward rules */
/*
 * InternetGatewayDevice.X_ACTIONTEC_Firewall.Services.{i}.
 * InternetGatewayDevice.X_ACTIONTEC_Firewall.Services.{i}.Enable
 * InternetGatewayDevice.X_ACTIONTEC_Firewall.Services.{i}.Name
 * InternetGatewayDevice.X_ACTIONTEC_Firewall.Services.{i}.Description
 * InternetGatewayDevice.X_ACTIONTEC_Firewall.Services.{i}.ReferenceCount
 * InternetGatewayDevice.X_ACTIONTEC_Firewall.Services.{i}.ServerPortsNumberOfEntries
 */
#define PORT_FWD_SERVICE_NAME         1  /* Service Name */
#define PORT_FWD_SERVICE_STS          2  /* Service status: Enable/Disable */
#define PORT_FWD_SERVICE_DESC         3  /* Service Description */
#define PORT_FWD_SERVICE_REF          4  /* Service ReferenceCount */
#define PORT_FWD_SERVICE_ADVANCED     5  /* if true will be recongnized as advanced service */
#define PORT_FWD_SERVICE_ENTRY        6  /* Service Entries */
#define PORT_FWD_SERVICE_PID          7  /* -1 means a new service */
dal_ret_t *dal_ipt_get_port_fwd_service_list(int *pnum);

/* add(or edit existed) a new port forwarding service entry */
int dal_ipt_set_port_fwd_service(dal_ret_t* pobj, char *pbuf, int buflen);

/* delete a port forwarding service, explicit pid needed */
int dal_ipt_del_port_fwd_service(char *pid);

#define PORT_FWD_STS                 1  /* 1->Enable, 0->Disable" */
#define PORT_FWD_PROTOCOL            2  /* "TCP", "UDP", "ICMP" or "Other" */
#define PORT_FWD_PRTCL_NUM           3
#define PORT_FWD_PRTCL_EXCLD         4  /* Exclude protocol */
#define PORT_FWD_STRT_SRC_PORT       5  /* 0 means "ANY" */
#define PORT_FWD_END_SRC_PORT        6
#define PORT_FWD_SRC_PORT_EXCLD      7  /* Exclude source port */
#define PORT_FWD_STRT_DST_PORT       8  /* 0 means "ANY" */
#define PORT_FWD_END_DST_PORT        9
#define PORT_FWD_DST_PORT_EXCLD      10 /* Exclude destination port */
#define PORT_FWD_ICMP_TYPE           11
#define PORT_FWD_ICMP_CODE           12
#define PORT_FWD_SRV_PORT_PID        13 /* -1 means a server port */
dal_ret_t *dal_ipt_get_server_port_list(char *svc_pid, int *pnum);

/* 
 * function name: dal_ipt_set_server_port
 * input:
 *       1. pid, service pid
 *       2. pobj->param[PORT_FWD_SRV_PORT_PID] = -1, then add a new instance
 * return value:
 *             TSL_RV_SUC-> success 
 *             TSL_RV_ERR-> failed
 */
int dal_ipt_set_server_port(const char* pid, dal_ret_t* pobj);
int dal_ipt_del_server_port(char* pid, dal_ret_t* pobj);
//modified by logan on 2012-06-15
tsl_rv_t dal_ipt_modify_server_port(const char* pid, dal_ret_t* pobj);

#if 0
#define PORT_FWD_STS                 1  /* 1->Enable, 0->Disable" */
#define PORT_FWD_PROTOCOL            2  /* "TCP", "UDP", "ICMP" or "Other" */
#define PORT_FWD_PRTCL_NUM           3
#define PORT_FWD_PRTCL_EXCLD         4  /* Exclude protocol */
#define PORT_FWD_STRT_SRC_PORT       5  /* 0 means "ANY" */
#define PORT_FWD_END_SRC_PORT        6
#define PORT_FWD_SRC_PORT_EXCLD      7  /* Exclude source port */
#define PORT_FWD_STRT_DST_PORT       8  /* 0 means "ANY" */
#define PORT_FWD_END_DST_PORT        9
#define PORT_FWD_DST_PORT_EXCLD      10 /* Exclude destination port */
#define PORT_FWD_ICMP_TYPE           11
#define PORT_FWD_ICMP_CODE           12
#define PORT_FWD_SRV_PORT_PID        13 /* -1 means a server port */
#endif
#define PORT_FWD_SVR_IDX             14 /* port service index, indicates which service name */
#define PORT_FWD_PORT_IDX            15 /* port rule index, indecates which port rule instance */
/*
 * Function name: dal_ipt_query_port_rule_list
 * 
 * Description: query all port rules list
 *
 * Input parameters:
 *              pnum -> how many port rule numbers was put
 * Return Value:
 *            output ->  port rule pointer
 */
dal_ret_t *dal_ipt_query_port_rule_list(int *pnum);

#define DHCP_ACL_FILTER_MODE     1  /* "Disable" , "Allow" or "Deny" */

/* get DHCP Access Control mac filtering mode */
dal_ret_t* dal_dhcp_get_acl_filter_mode(void);

/* set DHCP Access Control mac filtering mode */
tsl_rv_t dal_dhcp_set_acl_filter_mode(dal_ret_t* param);

#define DHCP_ACL_FILTER_ADDR     1  /* MAC address */
#define DHCP_ACL_FILTER_PID      2  /* -1 means a new ACL MAC filtering */

/* get the DHCP Access Control mac filtering address list
 * num: output value, return the number of the mac addresses in the list
 */
dal_ret_t* dal_dhcp_get_acl_filter_addr_list(int* num);

/* modify or add a mac filtering address, explicit pid needed */
tsl_rv_t dal_dhcp_set_acl_filter_addr(dal_ret_t* param);

/* delete a mac filtering address, explicit pid needed */
tsl_rv_t dal_dhcp_del_acl_filter_addr(dal_ret_t* param);


#ifdef SUPPORT_IPV6_MULTI_LAN
/**************************************************************************
 *	[FUNCTION NAME]:
 *	        dal_ipv6_bridge_add
 *
 *	[DESCRIPTION]:
 *	        add a bridge.
 *           while adding a bridge, please call this function to inform IPv6 module.
 *
 *	[PARAMETER]:
 *	        br_name[in] : bridge name
 *
 *	[RETURN]
 *              tsl_rv_suc 	:         SUCCESS
 *              other		:         FAIL
 *
 *	[History]
 *			@2011.9.9  uni.chen : create this interface
 **************************************************************************/
tsl_rv_t dal_ipv6_bridge_add(const char *br_name);

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        dal_ipv6_bridge_del
 *
 *	[DESCRIPTION]:
 *	        delete a bridge.
 *           while deleteing a bridge, please call this function to inform IPv6 module.
 *
 *	[PARAMETER]:
 *	        br_name[in] : bridge name
 *
 *	[RETURN]
 *              tsl_rv_suc 	:         SUCCESS
 *              other		:         FAIL
 *
 *	[History]
 *			@2011.9.9  uni.chen : create this interface
 **************************************************************************/
tsl_rv_t dal_ipv6_bridge_del(const char *br_name);

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        dal_ipv6_bridge_set_enable
 *
 *	[DESCRIPTION]:
 *	        enable/disable a bridge.
 *           while enable/disable a bridge, please call this function to inform IPv6 module.
 *
 *	[PARAMETER]:
 *	        br_name[in] 	: bridge name
 *	        enable[in] 	: 0,disable   ;  1,enable
 *
 *	[RETURN]
 *              tsl_rv_suc 	:         SUCCESS
 *              other		:         FAIL
 *
 *	[History]
 *			@2011.9.9  uni.chen : create this interface
 **************************************************************************/
tsl_rv_t dal_ipv6_bridge_set_enable(const char *br_name,const  tsl_bool_t enable);

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        dal_ipv6_bridge_add_port
 *
 *	[DESCRIPTION]:
 *	       add a port to a bridge.
 *           while adding a port to a bridge, please call this function to inform IPv6 module.
 *
 *	[PARAMETER]:
 *	        br_name[in] 	: bridge name
 *	        port_name[in] : port name
 *
 *	[RETURN]
 *              tsl_rv_suc 	:         SUCCESS
 *              other		:         FAIL
 *
 *	[History]
 *			@2011.9.9  uni.chen : create this interface
 **************************************************************************/
tsl_rv_t dal_ipv6_bridge_add_port(const char *br_name,const  char *port_name);

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        dal_ipv6_bridge_del_port
 *
 *	[DESCRIPTION]:
 *	       delete a port from a bridge.
 *           while deleting a port from a bridge, please call this function to inform IPv6 module.
 *
 *	[PARAMETER]:
 *	        br_name[in] 	: bridge name
 *	        port_name[in] : port name
 *
 *	[RETURN]
 *              tsl_rv_suc 	:         SUCCESS
 *              other		:         FAIL
 *
 *	[History]
 *			@2011.9.9  uni.chen : create this interface
 **************************************************************************/
tsl_rv_t dal_ipv6_bridge_del_port(const char*br_name,const  char *port_name);
#endif

//IP Address Distribution:

#define DHCP_DOMAIN_NAME           1    /* Domain name of DHCP Server */
#define DHCP_SERVICE_TYPE          2    /* "DHCP Server Or DHCP Relay Server" */
#define DHCP_START_IP_ADDR         3    /* DHCP Server start IP address */
#define DHCP_END_IP_ADDR           4    /* DHCP Server end IP address */
#define DHCP_SUBNET_MASK           5    /* Subnet mask */
#define DHCP_WINS_SERVER           6    /* WINS Server */
#define DHCP_LEASE_TIME            7    /* Lease time (minutes) */
#define DHCP_PROVIDE_HOSTNAME      8    /* Provide host name if not specified by client */
#define DHCP_ENABLE		   9    /* enable or disable dhcp server */
#define DHCP_RELAY_IP		   10    /* dhcp relay ip address */
#define DHCP_PID                   11   /* -1 means a new DHCP Server */

/* get dhcp server settings */
dal_ret_t *dal_dhcp_get_cfg(tsl_int_t * num);
/* modify dhcp server settings */
tsl_rv_t dal_dhcp_set_cfg(dal_ret_t * param);

#define DHCP_CONN_NAME             1    /* DHCP Connection name */
#define DHCP_CONN_HOST_NAME        2    /* DHCP Connnections host name */
#define DHCP_CONN_IP_ADDR          3    /* IP Address */
#define DHCP_CONN_PHY_ADDR         4    /* Physical Address */
#define DHCP_CONN_LEASE_TYPE       5    /* "Dynamic" or "Static" */
#define DHCP_CONN_STATUS           6    /* "Active" or "Inactive" */
#define DHCP_CONN_EXPIRE_TIME      7    /* Expires time (minute) */
#define DHCP_CONN_INTERFACE        8    /* bridge interface name */
#define DHCP_CONN_PID              9    /* -1 means a new DHCP connection */

/* get the dhcp connections list.
 * num: output value, return the number of dhcp connections in the list;
 */
dal_ret_t *dal_dhcp_get_conn_list(tsl_int_t * num);
/* modify or add a dhcp connection, explicit pid needed */
tsl_rv_t dal_dhcp_set_conn(dal_ret_t * param);
/* delete a dhcp connection, explicit pid needed */
tsl_rv_t dal_dhcp_del_conn(dal_ret_t * param);

#define DHCP_VCI                   1    /* Vendor Class ID */        
#define DHCP_Q0S                   2    /* Qos */
#define DHCP_IDDR                  3    /* ip address */
#define DHCP_PHY_ADDR              4    /* Physical Address */
#define DHCP_VCI_PID                   5    /* -1 means a new DHCP connection */

/* get dhcp server vci */
dal_ret_t *dal_dhcp_get_vci_info(tsl_int_t * num);

/**************************************************************************/
#define DSLITE_CONF_TYPE  		1		/* dslite config type : Auto / Manual */
#define DSLITE_AFTR_IPV6_ADDR  	2		/* AFTR IPv6 Address */
#define DSLITE_B4_IPV4_ADDR  	3		/* B4 IPv4 Address */
#define DSLITE_WAN_IPV6_ADDR  	4		/* B4 WAN IPv6 Address */
#define DSLITE_CONNECT_STATUS  	5		/* DSLite Connection Status */

dal_ret_t *dal_get_dslite_settings(char *if_name);
tsl_rv_t dal_set_dslite_settings(char *if_name, dal_ret_t* param);


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        dal_syslog_get_list
 *
 *	[DESCRIPTION]:
 *	        get the syslog content list .
 *          
 *
 *	[PARAMETER]:
 *	        type[in] 	        : the log type
 *	        entries_num[in/out] : specify the max size of list ,and return the list real size
 *
 *	[RETURN]
 *              facility 	:         log facility
 *              program		:         log identity
 *              level		:         log level
 *              timestamp	:         log timestamp
 *              content		:         log content
 *  NOTE : return section are all string.
 *
 *	[History]
 *			@2011.9.19  rayofox : create this interface
 **************************************************************************/
#define SYSLOG_ITEM_FACILITY 0 /*The syslog facility name*/
#define SYSLOG_ITEM_PROGRAM 1 /*The syslog program name*/
#define SYSLOG_ITEM_LEVEL 2 /*The syslog level*/
#define SYSLOG_ITEM_TIMESTAMP 3 /*The syslog timestamp*/
#define SYSLOG_ITEM_CONTENT 4 /*The syslog content*/

#define SYSLOG_TYPE_ALL 0
/*type for BHR2*/
#define SYSLOG_TYPE_VARLOG  1
#define SYSLOG_TYPE_FIREWALL 2
#define SYSLOG_TYPE_PERSISTENT 3
/*input max size through entries_num, 0 means no limited*/
dal_ret_t* dal_syslog_get_list(int type,int* entries_num);

//Universal Plug and Play:

#define UPNP_ENABLE                      1  /* Enable or Disable UPNP */
#define UPNP_ENABLE_AUTO_CLEANUP         2  /* Enable or Disable Automatic Cleanup of Old Unused UPnP Services */

/* get/set upnp configuration */
dal_ret_t* dal_upnp_get_cfg(void);
tsl_rv_t dal_upnp_set_cfg(dal_ret_t* param);     

// DNS Cache disable and enable:
#define DNS_DISABLE_CACHE	0

/* get/set DNS configuration */
tsl_rv_t dal_dns_set_cfg(dal_ret_t * param);
dal_ret_t *dal_dns_get_cfg(void);

/*
set/get wifi wmm rule
if_name: the name of wifi interface
e.g., ath0, ath1, ath2
sid: the value map to WIFI_WMM_ID
*/
#define WIFI_WMM_ID           1
#define WIFI_WMM_PRIORITY     2
#define WIFI_WMM_QUEUE        3
#define WIFI_WMM_DSCPValue    4
tsl_rv_t dal_wmm_add(char *if_name, char *sDSCPValue, char *squeue, char *spriority);
tsl_rv_t dal_wmm_delete(char *sid);
dal_ret_t* dal_wmm_get(char *if_name);
dal_ret_t *dal_wmm_getById(char *sid);

/*system setting about log*/
tsl_rv_t dal_syslog_enable_setting(char* swi);    /* the value of swi:e/d   e:enable,d:disable  */
tsl_rv_t dal_log_buf_size_setting(char* size,char* type);   /* size:100/100k/100M  type:var/fw */          
tsl_rv_t dal_write_log(char* nu,char* type);  /* for testing */
/**************************************************************************
 *	[FUNCTION NAME]:
 *	        dal_log_buf_get_list
 *
 *	[DESCRIPTION]:
 *	        get the syslog buf list used for GUI saving.
 *          
 *
 *	[PARAMETER]:
 *	        type[in] 	        : the log type(the same with dal_syslog_get_list)
 *	        log_format[in]      : specify the format of log saving
 *	[RETURN]
 *          length           	: length of return log buffer
 *          logfile     		: the address of log buffer
 **************************************************************************/
#define LOCAL_LOG_FILE              1
#define LOCAL_LOG_FILE_LENGTH       2
dal_ret_t* dal_log_buf_get_list(int type,char* log_format);
tsl_rv_t dal_clear_log(int type);

/* tr69 download */
#define TR69_DOWNLOAD_URL           1
#define TR69_DOWNLOAD_USERNAME      2
#define TR69_DOWNLOAD_PASSWORD      3
#define TR69_DOWNLOAD_FILETYPE      4
#define TR69_DOWNLOAD_DELAYSEC      5

#define TR69_DOWNLOAD_FILETYPE_FWIMAGE        1
#define TR69_DOWNLOAD_FILETYPE_CONFIG         2
#define TR69_DOWNLOAD_DELAYSEC_DEFAULT_VALUE  60

tsl_rv_t dal_tr69_download(dal_ret_t* download);
tsl_rv_t dal_tr69_download_ext(dal_ret_t* download, tsl_bool_t *b_restore_default);

/* tr69 upload */
#define TR69_UPLOAD_URL           1
#define TR69_UPLOAD_USERNAME      2
#define TR69_UPLOAD_PASSWORD      3
#define TR69_UPLOAD_DELAYSEC      4
tsl_rv_t dal_tr69_upload(dal_ret_t* upload);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_VAP_COUNT	4
enum{
	INDEX_WIFI_WPA_BEACON_TYPE,						//string value: one of {"WPA", "11i", "WPAand11i"}
		INDEX_WIFI_PRESHARED_KEY1,				//string value: 7<length<64
		INDEX_WIFI_PRESHARED_KEY2,				//string value: 7<length<64
		INDEX_WIFI_PRESHARED_KEY3,				//string value: 7<length<64
		INDEX_WIFI_PRESHARED_KEY4,				//string value: 7<length<64
		INDEX_WIFI_PRESHARED_KEY5,				//string value: 7<length<64
		INDEX_WIFI_PRESHARED_KEY6,				//string value: 7<length<64
		INDEX_WIFI_PRESHARED_KEY7,				//string value: 7<length<64
		INDEX_WIFI_PRESHARED_KEY8,				//string value: 7<length<64
		INDEX_WIFI_PRESHARED_KEY9,				//string value: 7<length<64
		INDEX_WIFI_PRESHARED_KEY10,				//string value: 7<length<64
		INDEX_WIFI_PRESHARED_KEY1_TYPE,			//Integer value: one of {0 - Hex, 1 - ASCII}
		INDEX_WIFI_PRESHARED_KEY2_TYPE,			//Integer value: one of {0 - Hex, 1 - ASCII}
		INDEX_WIFI_PRESHARED_KEY3_TYPE,			//Integer value: one of {0 - Hex, 1 - ASCII}
		INDEX_WIFI_PRESHARED_KEY4_TYPE,			//Integer value: one of {0 - Hex, 1 - ASCII}
		INDEX_WIFI_PRESHARED_KEY5_TYPE,			//Integer value: one of {0 - Hex, 1 - ASCII}
		INDEX_WIFI_PRESHARED_KEY6_TYPE,			//Integer value: one of {0 - Hex, 1 - ASCII}
		INDEX_WIFI_PRESHARED_KEY7_TYPE,			//Integer value: one of {0 - Hex, 1 - ASCII}
		INDEX_WIFI_PRESHARED_KEY8_TYPE,			//Integer value: one of {0 - Hex, 1 - ASCII}
		INDEX_WIFI_PRESHARED_KEY9_TYPE,			//Integer value: one of {0 - Hex, 1 - ASCII}
		INDEX_WIFI_PRESHARED_KEY10_TYPE,		//Integer value: one of {0 - Hex, 1 - ASCII}
		INDEX_WIFI_WPA_ENCRYPTION_MODE,			//string value: one of {"TKIPEncryption", "AESEncryption", "TKIPandAESEncryption"}
		INDEX_WIFI_WPA_AUTHENTICATION_MODE,		//string value: one of {"PSKAuthentication", "EAPAuthentication"}
		INDEX_WIFI_IEEE11I_ENCRYPTION_MODE,		//string value: one of {"TKIPEncryption", "AESEncryption", "TKIPandAESEncryption"}
		INDEX_WIFI_IEEE11I_AUTHENTICATION_MODE,	//string value: one of {"PSKAuthentication", "EAPAuthentication", "EAPandPSKAuthentication"}
		INDEX_WIFI_PRESHARED_KEY_INDEX,			//Integer value: [1, 10]
		INDEX_WIFI_WPA_8021X_SERVER_SECRET,			//string value: 
		INDEX_WIFI_WPA_8021X_SERVER_ADDRESS,		//string value: 
		INDEX_WIFI_WPA_8021X_SERVER_PORT,			//Integer value: 
		INDEX_WIFI_GROUP_KEY_UPDATE_INTERVAL,	//Integer value: [300, 1200]
	COUNT_PARAM_OF_WPA_SERIES
};

/*
Function:	dal_vap_set_wpa_series_data
Decriptions:	Set wpa/wpa2/mixed security data for special vap
Parameters:	
			index_vap: 	(IN) index of vap [0, 3]
			data:		(IN) security data
Return Value: 
			TSL_RV_SUC	- Success
			TSL_RV_FAIL	- Fail
*/
tsl_rv_t dal_vap_set_wpa_series_data(unsigned int index_vap, dal_ret_t* data);
/*
Function:	dal_vap_get_wpa_series_data
Decriptions:	Get wpa/wpa2/mixed security data for special vap
Parameters:	
			index_vap: 	(IN) index of vap [0, 3]
			
Return Value: 
			if success return the pointer of security data, else return NULL
*/
dal_ret_t* dal_vap_get_wpa_series_data(unsigned int index_vap);

enum{
	INDEX_WIFI_BASIC_BEACON_TYPE,						//string value: one of {"None", "Basic"}
		INDEX_WIFI_WEP_KEY1,				//string value: 1 byte<=length<=128 bits
		INDEX_WIFI_WEP_KEY2,				//string value: 1 byte<=length<=128 bits
		INDEX_WIFI_WEP_KEY3,				//string value: 1 byte<=length<=128 bits
		INDEX_WIFI_WEP_KEY4,				//string value: 1 byte<=length<=128 bits
		INDEX_WIFI_WEP_KEY1_TYPE,			//Integer value: one of {0 - Hex, 1 - ASCII}
		INDEX_WIFI_WEP_KEY2_TYPE,			//Integer value: one of {0 - Hex, 1 - ASCII}
		INDEX_WIFI_WEP_KEY3_TYPE,			//Integer value: one of {0 - Hex, 1 - ASCII}
		INDEX_WIFI_WEP_KEY4_TYPE,			//Integer value: one of {0 - Hex, 1 - ASCII}
		INDEX_WIFI_BASIC_ENCRYPTION_MODE,			//string value: one of {"None", "WEPEncryption"}
		INDEX_WIFI_BASIC_AUTHENTICATION_MODE,		//string value: one of {"None", "SharedAuthentication", "Auto", "EAPAuthentication"}
		INDEX_WIFI_WEP_KEY_INDEX,			//Integer value: [1, 4], "1"
		INDEX_WIFI_WEP_8021X_SERVER_SECRET,			//string value: 
		INDEX_WIFI_WEP_8021X_SERVER_ADDRESS,		//string value: 
		INDEX_WIFI_WEP_8021X_SERVER_PORT,			//Integer value: 
	COUNT_PARAM_OF_WEP_SERIES
};

/*
Function:	dal_vap_set_wep_series_data
Decriptions:	Set wep security data for special vap
Parameters:	
			index_vap: 	(IN) index of vap [0, 3]
			data:		(IN) security data
Return Value: 
			TSL_RV_SUC	- Success
			TSL_RV_FAIL	- Fail
*/
tsl_rv_t dal_vap_set_wep_series_data(unsigned int index_vap, dal_ret_t* data);
/*
Function:	dal_vap_get_wep_series_data
Decriptions:	Get wep security data for special vap
Parameters:	
			index_vap: 	(IN) index of vap [0, 3]
			
Return Value: 
			if success return the pointer of security data, else return NULL
*/
dal_ret_t* dal_vap_get_wep_series_data(unsigned int index_vap);

/*
Function:	dal_vap_commit_wlanconfiguration
Decriptions:	commit vap data after set
Parameters:	
			index_vap: 	(IN) index of vap [0, 3]
			
Return Value: 
			TSL_RV_SUC	- Success
			TSL_RV_FAIL	- Fail
*/
tsl_rv_t dal_vap_commit_wlanconfiguration(unsigned int index_vap);

enum{
	INDEX_WIFI_8021X_BEACON_TYPE,						//string value: one of {"None", "Basic", "WPA", "11i", "WPAand11i"}
		INDEX_WIFI_8021X_SERVER_SECRET,			//string value: 
		INDEX_WIFI_8021X_SERVER_ADDRESS,		//string value: 
		INDEX_WIFI_8021X_SERVER_PORT,			//Integer value: 
	COUNT_PARAM_OF_8021X_SERIES
};

/*
Function:	dal_vap_set_8021x_series_data
Decriptions:	Set wep security data for special vap
Parameters:	
			index_vap: 	(IN) index of vap [0, 3]
			data:		(IN) security data
Return Value: 
			TSL_RV_SUC	- Success
			TSL_RV_FAIL	- Fail
*/
tsl_rv_t dal_vap_set_8021x_series_data(unsigned int index_vap, dal_ret_t* data);
/*
Function:	dal_vap_get_8021x_series_data
Decriptions:	Get wep security data for special vap
Parameters:	
			index_vap: 	(IN) index of vap [0, 3]
			
Return Value: 
			if success return the pointer of security data, else return NULL
*/
dal_ret_t* dal_vap_get_8021x_series_data(unsigned int index_vap);

enum{
	INDEX_WIFI_WPS_ENABLE,				//Integer value: one of {0, non-zero}
		INDEX_WIFI_WPS_METHOD,			//string value:  one of {"PushButton", "Label", "Display", "Keypad"}
		INDEX_WIFI_WPS_PIN,				//string value: input for station pin, output for ap pin (8 digits)
	COUNT_PARAM_OF_WPS_SERIES
};
/*
PushButton	- PBC
Label		- use default pin
Display		- use random pin
Keypad		- set station pin
*/

/*
Function:	dal_vap_set_wps_series_data
Decriptions:	Set wps data for special vap
Parameters:	
			index_vap: 	(IN) index of vap [0, 3]
			data:		(IN) wps data
Return Value: 
			TSL_RV_SUC	- Success
			TSL_RV_FAIL	- Fail
*/
tsl_rv_t dal_vap_set_wps_series_data(unsigned int index_vap, dal_ret_t* data);

/*
Function:	dal_vap_get_wps_series_data
Decriptions:	Get wps data for special vap
Parameters:	
			index_vap: 	(IN) index of vap [0, 3]
			
Return Value: 
			if success return the pointer of wps data, else return NULL
*/
dal_ret_t* dal_vap_get_wps_series_data(unsigned int index_vap);

//add by logan on 2012-06-11
int dalCheckPmapDstPortExist(char *dst_port, char *server_name);
void dalFwGetPmapDstPortList(char *wan_conn_name, char *varValue);
#endif
