/*
 *      Utiltiy function for setting dhcp
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "tr69_func_common.h"
#include "libtr69_func.h"
#include "ctl_validstrings.h"
#include "ctl_util.h"
#include "act_dhcpv4.h"
#include "tr69_cms_object.h"
#include "libtr69_func.h"
#include "tr69_func_common.h"
#include "tsl_strconv.h"

//#include "apmib.h"
//#include "sysconf.h"
//#include "sys_utility.h"
#define BR_IFACE_FILE "/var/system/br_iface"
#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
#define BR_IFACE_FILE2 "/var/system/br_iface2"
#endif
#define MESH_PATHSEL "/bin/pathsel"
#define BR_INIT_FILE "/tmp/bridge_init"
#define ETH_VLAN_SWITCH "/proc/disable_l2_table"
#define DHCPD_CONF_FILE "/var/udhcpd.conf"
#define DHCPD_LEASE_FILE "/var/lib/misc/udhcpd.leases"
#ifdef AEI_NCS_CFG_SYNC
#define DNSMASQ_HOSTS_CFG "/var/dnsmasq.hosts"
#define DNSMASQ_CFG "/var/dnsmasq.conf"
#define MY_EXTENDER "myextender"
#endif

#if 0
void set_lan_dhcpd(char *interface, int mode)
{
	char tmpBuff1[32]={0}, tmpBuff2[32]={0};
	int intValue=0, dns_mode=0;
	char line_buffer[100]={0};
	char tmp1[64]={0};
	char tmp2[64]={0};
	char *strtmp=NULL, *strtmp1=NULL;
	DHCPRSVDIP_T entry;
	int i, entry_Num=0;
#ifdef   HOME_GATEWAY
	char tmpBuff3[32]={0};
#endif
#if defined(CONFIG_RTL_ULINKER)
{//// must be gateway mode!
	int opmode;
	int auto_wan;
	apmib_get(MIB_OP_MODE,(void *)&opmode);
	if (opmode==GATEWAY_MODE) {
		apmib_get(MIB_ULINKER_AUTO,(void *)&auto_wan);
		system("brctl addif br0 usb0 > /dev/null 2>&1");
	#if 0
		if (auto_wan == 0)
			system("ifconfig usb0 0.0.0.0 > /dev/null 2>&1");
	#endif
	}
}
#endif
	sprintf(line_buffer,"interface %s\n",interface);
	write_line_to_file(DHCPD_CONF_FILE, 1, line_buffer);

	apmib_get(MIB_DHCP_CLIENT_START,  (void *)tmp1);
	strtmp= inet_ntoa(*((struct in_addr *)tmp1));
	sprintf(line_buffer,"start %s\n",strtmp);
	write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);

	apmib_get(MIB_DHCP_CLIENT_END,  (void *)tmp1);
	strtmp= inet_ntoa(*((struct in_addr *)tmp1));
	sprintf(line_buffer,"end %s\n",strtmp);
	write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);

	apmib_get(MIB_SUBNET_MASK,  (void *)tmp1);
	strtmp= inet_ntoa(*((struct in_addr *)tmp1));
	sprintf(line_buffer,"opt subnet %s\n",strtmp);
	write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);

	apmib_get(MIB_DHCP_LEASE_TIME, (void *)&intValue);
    if( (intValue==0) || (intValue<0) || (intValue>10080))
    {
		intValue = 480; //8 hours
		if(!apmib_set(MIB_DHCP_LEASE_TIME, (void *)&intValue))
		{
			printf("set MIB_DHCP_LEASE_TIME error\n");
		}

		apmib_update(CURRENT_SETTING);
    }
	intValue *= 60;
    sprintf(line_buffer,"opt lease %ld\n",intValue);
    write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);

	if(mode==1){//ap
		apmib_get( MIB_DEFAULT_GATEWAY,  (void *)tmp2);
		if (memcmp(tmp2, "\x0\x0\x0\x0", 4)){
			strtmp= inet_ntoa(*((struct in_addr *)tmp2));
			sprintf(line_buffer,"opt router %s\n",strtmp);
			write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);
		}


	}else{
		apmib_get(MIB_IP_ADDR,  (void *)tmp1);
		strtmp= inet_ntoa(*((struct in_addr *)tmp1));
		sprintf(line_buffer,"opt router %s\n",strtmp);
		write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);
#ifdef   HOME_GATEWAY
		apmib_get( MIB_DNS_MODE, (void *)&dns_mode);
		if(dns_mode==0){
			sprintf(line_buffer,"opt dns %s\n",strtmp); /*now strtmp is ip address value */
			write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);
		}
#endif
	}
	if((mode==1) 
#if 1
	||(mode==2 && dns_mode==1)
#endif
	){
#if defined(HOME_GATEWAY) && !defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)
		apmib_get( MIB_DNS1,  (void *)tmpBuff1);
		apmib_get( MIB_DNS2,  (void *)tmpBuff2);
		apmib_get( MIB_DNS3,  (void *)tmpBuff3);

		if (memcmp(tmpBuff1, "\x0\x0\x0\x0", 4)){
			strtmp= inet_ntoa(*((struct in_addr *)tmpBuff1));
			sprintf(line_buffer,"opt dns %s\n",strtmp);
			write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);
			intValue++;
		}
		if (memcmp(tmpBuff2, "\x0\x0\x0\x0", 4)){
			strtmp= inet_ntoa(*((struct in_addr *)tmpBuff2));
			sprintf(line_buffer,"opt dns %s\n",strtmp);
			write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);
			intValue++;
		}
		if (memcmp(tmpBuff3, "\x0\x0\x0\x0", 4)){
			strtmp= inet_ntoa(*((struct in_addr *)tmpBuff3));
			sprintf(line_buffer,"opt dns %s\n",strtmp);
			write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);
			intValue++;
		}
#endif

#ifdef CONFIG_DOMAIN_NAME_QUERY_SUPPORT
		apmib_get(MIB_IP_ADDR,  (void *)tmp1);
		strtmp= inet_ntoa(*((struct in_addr *)tmp1));
		sprintf(line_buffer,"opt dns %s\n",strtmp); /*now strtmp is ip address value */
		write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);
#endif //#ifdef CONFIG_DOMAIN_NAME_QUERY_SUPPORT

		if(intValue==0){ /*no dns option for dhcp server, use default gatewayfor dns opt*/

			if(mode==1){
				apmib_get( MIB_DEFAULT_GATEWAY,  (void *)tmp2);
				if (memcmp(tmp2, "\x0\x0\x0\x0", 4)){
					strtmp= inet_ntoa(*((struct in_addr *)tmp2));
					sprintf(line_buffer,"opt dns %s\n",strtmp);
					write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);
				}
			}else {
				apmib_get( MIB_IP_ADDR,  (void *)tmp2);
				if (memcmp(tmp2, "\x0\x0\x0\x0", 4)){
					strtmp= inet_ntoa(*((struct in_addr *)tmp2));
					sprintf(line_buffer,"opt dns %s\n",strtmp);
					write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);
				}
			}
		}
	}
	memset(tmp1, 0x00, 64);
	apmib_get( MIB_DOMAIN_NAME, (void *)&tmp1);
	if(tmp1[0]){
		sprintf(line_buffer,"opt domain %s\n",tmp1);
		write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);
	}
/*static dhcp entry static_lease 000102030405 192.168.1.199*/
	intValue=0;
	apmib_get(MIB_DHCPRSVDIP_ENABLED, (void *)&intValue);
	if(intValue==1){
		apmib_get(MIB_DHCPRSVDIP_TBL_NUM, (void *)&entry_Num);
		if(entry_Num>0){
			for (i=1; i<=entry_Num; i++) {
				*((char *)&entry) = (char)i;
				apmib_get(MIB_DHCPRSVDIP_TBL, (void *)&entry);
				sprintf(line_buffer, "static_lease %02x%02x%02x%02x%02x%02x %s\n", entry.macAddr[0], entry.macAddr[1], entry.macAddr[2],
				entry.macAddr[3], entry.macAddr[4], entry.macAddr[5], inet_ntoa(*((struct in_addr*)entry.ipAddr)));
				write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);
			}
		}
	}
	/* may not need to set ip again*/
	apmib_get(MIB_IP_ADDR,  (void *)tmp1);
	strtmp= inet_ntoa(*((struct in_addr *)tmp1));
	sprintf(tmpBuff1, "%s", strtmp);
	apmib_get(MIB_SUBNET_MASK,  (void *)tmp2);
	strtmp1= inet_ntoa(*((struct in_addr *)tmp2));
	sprintf(tmpBuff2, "%s", strtmp1);
	RunSystemCmd(NULL_FILE, "ifconfig", interface, tmpBuff1, "netmask", tmpBuff2,  NULL_STR);
	/*start dhcp server*/
	char tmpBuff4[100];
	sprintf(tmpBuff4,"udhcpd %s\n",DHCPD_CONF_FILE);
	system(tmpBuff4);
	//RunSystemCmd(stdout, "udhcpd", DHCPD_CONF_FILE, NULL_STR);


}
#endif
#ifdef AEI_NCS_CFG_SYNC
tsl_rv_t stop_dhcpd( )
{
    char cmdLine[128]={ 0 };
    snprintf(cmdLine, sizeof(cmdLine), "killall udhcpd >/dev/null 2>&1");
    ctldbg_module(CTL_MODULE_NCS_CFG_SYNC,"cmdLine=%s",cmdLine);
    system(cmdLine);
#ifdef AEI_FIREWALL
    snprintf(cmdLine, sizeof(cmdLine), "iptables -D INPUT -i br0 -d 255.255.255.255 -p udp "
        "--sport 68 --dport 67 -j ACCEPT  >/dev/null 2>&1");
    ctldbg_module(CTL_MODULE_NCS_CFG_SYNC,"cmdLine=%s",cmdLine);
    system(cmdLine);
#endif
    return TSL_RV_SUC;
}
tsl_rv_t start_dhcpd()
{
    tsl_char_t * pMinAddress = NULL;
    tsl_char_t * pMaxAddress = NULL;
    tsl_char_t * pSubnetMask = NULL;
    tsl_char_t * pDNSServers = NULL;
    tsl_int_t type;
    tsl_rv_t rv = TSL_RV_FAIL;
    FILE *fp=NULL;
    char cmdLine[128]={ 0 };

    const char *dhcpdCfg="\
start %s\n\
end %s\n\
interface br0\n\
option  subnet  %s\n\
opt     router  %s\n\
option  dns     %s\n\
option  lease   86400\n\
";
ctldbg_module(CTL_MODULE_NCS_CFG_SYNC,"begin");

    stop_dhcpd();
   
    if ((fp = fopen(DHCPD_CONF_FILE, "w")) == NULL)
    {
      /* error */
      ctllog_error( "failed to create %s\n", DHCPD_CONF_FILE);
      return TSL_RV_FAIL;
    }
    if( TSL_RV_SUC != (rv=ACCESS_LEAF( tr69_get_unfresh_leaf_data,
                    (void **) &pMinAddress, &type,
                    "Device.DHCPv4.Server.Pool.1.MinAddress"))) {
        ctllog_error( "Device.DHCPv4.Server.Pool.1.MinAddress" );
    }
ctldbg_module(CTL_MODULE_NCS_CFG_SYNC,"pMinAddress=%s",pMinAddress);

    if( TSL_RV_SUC != (rv=ACCESS_LEAF( tr69_get_unfresh_leaf_data,
                    (void **) &pMaxAddress, &type,
                    "Device.DHCPv4.Server.Pool.1.MaxAddress"))) {
        ctllog_error( "Device.DHCPv4.Server.Pool.1.MaxAddress" );
    }
ctldbg_module(CTL_MODULE_NCS_CFG_SYNC,"pMaxAddress=%s",pMaxAddress);

    if( TSL_RV_SUC != (rv=ACCESS_LEAF( tr69_get_unfresh_leaf_data,
                    (void **) &pSubnetMask, &type,
                    "Device.DHCPv4.Server.Pool.1.SubnetMask"))) {
        ctllog_error( "Device.DHCPv4.Server.Pool.1.SubnetMask" );
    }
ctldbg_module(CTL_MODULE_NCS_CFG_SYNC,"pSubnetMask=%s",pSubnetMask);

    if( TSL_RV_SUC != (rv=ACCESS_LEAF( tr69_get_unfresh_leaf_data,
                    (void **) &pDNSServers, &type,
                    "Device.DHCPv4.Server.Pool.1.DNSServers"))) {
        ctllog_error( "Device.DHCPv4.Server.Pool.1.DNSServers" );
    }
ctldbg_module(CTL_MODULE_NCS_CFG_SYNC,"pDNSServers=%s",pDNSServers);

    if(pMinAddress && pMaxAddress && pSubnetMask && pDNSServers)
        fprintf(fp, dhcpdCfg, pMinAddress, pMaxAddress, pSubnetMask ,pDNSServers,pDNSServers);
    
    fclose(fp);
    CTLMEM_FREE_BUF_AND_NULL_PTR( pMinAddress );
    CTLMEM_FREE_BUF_AND_NULL_PTR( pMaxAddress );
    CTLMEM_FREE_BUF_AND_NULL_PTR( pSubnetMask );
    CTLMEM_FREE_BUF_AND_NULL_PTR( pDNSServers );
    
#ifdef AEI_FIREWALL
    snprintf(cmdLine, sizeof(cmdLine), "iptables -A INPUT -i br0 -d 255.255.255.255 -p udp "
        "--sport 68 --dport 67 -j ACCEPT  >/dev/null 2>&1");
    ctldbg_module(CTL_MODULE_NCS_CFG_SYNC,"cmdLine=%s",cmdLine);
    system(cmdLine);
#endif

    snprintf(cmdLine, sizeof(cmdLine), "/bin/udhcpd %s &", DHCPD_CONF_FILE);
    ctldbg_module(CTL_MODULE_NCS_CFG_SYNC,"cmdLine=%s",cmdLine);
    system(cmdLine);
ctldbg_module(CTL_MODULE_NCS_CFG_SYNC,"end");

    return rv;
}


tsl_rv_t stop_dnsmasq( )
{
    char cmdLine[128]={ 0 };
#ifdef AEI_FIREWALL
    snprintf(cmdLine, sizeof(cmdLine), "iptables -D INPUT -i br0  -p udp "
        "--dport 53 -j ACCEPT  >/dev/null 2>&1");
    ctldbg_module(CTL_MODULE_NCS_CFG_SYNC,"cmdLine=%s",cmdLine);
    system(cmdLine);
#endif
    snprintf(cmdLine, sizeof(cmdLine), "killall dnsmasq >/dev/null 2>&1");
    ctldbg_module(CTL_MODULE_NCS_CFG_SYNC,"cmdLine=%s",cmdLine);
    system(cmdLine);
    return TSL_RV_SUC;
}

tsl_rv_t start_dnsmasq()
{
    const char *dnsmasqHostsCfg="192.168.99.254 %s";

    const char *dnsmasqCfg="\
no-resolv\n\
no-poll\n\
no-hosts\n\
addn-hosts=%s\n\
interface=br0\n\
no-dhcp-interface=br0\n\
";

    FILE *fpHosts=NULL;
    FILE *fp=NULL;
    char cmdLine[128]={ 0 };
    ctldbg_module(CTL_MODULE_NCS_CFG_SYNC,"begin");

    stop_dnsmasq();

    if ((fpHosts= fopen(DNSMASQ_HOSTS_CFG, "w")) == NULL)
    {
      /* error */
      ctllog_error( "failed to create %s\n", DHCPD_CONF_FILE);
      return TSL_RV_FAIL;
    }
    fprintf(fpHosts, dnsmasqHostsCfg, MY_EXTENDER);
    fclose(fpHosts);

    if ((fp = fopen(DNSMASQ_CFG, "w")) == NULL)
    {
      /* error */
      ctllog_error( "failed to create %s\n", DHCPD_CONF_FILE);
      return TSL_RV_FAIL;
    }

    fprintf(fp, dnsmasqCfg, DNSMASQ_HOSTS_CFG);
    fclose(fp);
#ifdef AEI_FIREWALL
    snprintf(cmdLine, sizeof(cmdLine), "iptables -A INPUT -i br0  -p udp "
        "--dport 53 -j ACCEPT  >/dev/null 2>&1");
    ctldbg_module(CTL_MODULE_NCS_CFG_SYNC,"cmdLine=%s",cmdLine);
    system(cmdLine);
#endif

    snprintf(cmdLine, sizeof(cmdLine), "/bin/dnsmasq -C %s -d &", DNSMASQ_CFG);
    ctldbg_module(CTL_MODULE_NCS_CFG_SYNC,"cmdLine=%s",cmdLine);
    system(cmdLine);

    ctldbg_module(CTL_MODULE_NCS_CFG_SYNC,"end");

    return TSL_RV_SUC;
}


#endif
/*create deconfig script for dhcp client*/
void Create_script(char *script_path, char *iface, int network, char *ipaddr, char *mask, char *gateway)
{       
    unsigned char tmpbuf[100];
    int fh;

    fh = open(script_path, O_RDWR|O_CREAT|O_TRUNC, S_IRWXO|S_IRWXG);
    if (fh < 0) { 
        fprintf(stderr, "Create %s file error!\n", script_path);
        return;
    }
    //if(network==LAN_NETWORK){
    if(network==0){
        sprintf((char *)tmpbuf, "%s", "#!/bin/sh\n");
        write(fh, tmpbuf, strlen((char *)tmpbuf));
        sprintf((char *)tmpbuf, "ifconfig %s %s netmask %s\n", iface, ipaddr, mask);
        write(fh, tmpbuf, strlen((char *)tmpbuf));
#ifdef AEI_WECB
#ifdef AEI_TR181
        sprintf((char *)tmpbuf, "cli -s Device.IP.Interface.1.IPv4Address.1.IPAddress string %s\n", ipaddr );
        write(fh, tmpbuf, strlen((char *)tmpbuf));
        sprintf((char *)tmpbuf, "cli -s Device.IP.Interface.1.IPv4Address.1.SubnetMask string %s\n", mask );
        write(fh, tmpbuf, strlen((char *)tmpbuf));
#else
        sprintf((char *)tmpbuf, "cli -s Device.LAN.IPAddress string %s\n", ipaddr );
        write(fh, tmpbuf, strlen((char *)tmpbuf));
        sprintf((char *)tmpbuf, "cli -s Device.LAN.SubnetMask string %s\n", mask );
        write(fh, tmpbuf, strlen((char *)tmpbuf));
        sprintf((char *)tmpbuf, "cli -s Device.LAN.DefaultGateway string %s\n", gateway );
        write(fh, tmpbuf, strlen((char *)tmpbuf));
#endif
#endif
        sprintf((char *)tmpbuf, "while route del default dev %s\n", iface);
        write(fh, tmpbuf, strlen((char *)tmpbuf));
        sprintf((char *)tmpbuf, "%s\n", "do :");
        write(fh, tmpbuf, strlen((char *)tmpbuf));
        sprintf((char *)tmpbuf, "%s\n", "done");
        write(fh, tmpbuf, strlen((char *)tmpbuf));
        sprintf((char *)tmpbuf, "route add -net default gw %s dev %s\n", gateway, iface);
        write(fh, tmpbuf, strlen((char *)tmpbuf));
    }
#if 0
    if(network==WAN_NETWORK){
        sprintf((char *)tmpbuf, "%s", "#!/bin/sh\n");
        write(fh, tmpbuf, strlen((char *)tmpbuf));

#if 0 //def CONFIG_POCKET_ROUTER_SUPPORT //it needn't do this
        sprintf((char *)tmpbuf, "sysconf disc dhcpc\n");
        write(fh, tmpbuf, strlen((char *)tmpbuf));          
#endif

        sprintf((char *)tmpbuf, "ifconfig %s 0.0.0.0\n", iface);
        write(fh, tmpbuf, strlen((char *)tmpbuf));

    }
#endif
    close(fh);
}



void set_lan_dhcpc(char *iface, _Dhcpv4ClientObject *pDhcpc)
{
	char script_file[100], deconfig_script[100], pid_file[100];
	//char *strtmp=NULL;
	char Ip[32] = {0};
    char Mask[32] = {0};
    char Gateway[32] = {0};
	char cmdBuff[200];
#ifdef  HOME_GATEWAY
	int intValue=0;
#endif
    tsl_char_t strHostname[BUFLEN_32] = {0};

    tsl_rv_t rv = TSL_RV_FAIL;
    tsl_char_t * pValue = NULL;
    tsl_int_t type;
    tsl_int_t snLen = 0;

	sprintf(script_file, "/usr/share/udhcpc/%s.sh", iface); /*script path*/
	sprintf(deconfig_script, "/usr/share/udhcpc/%s.deconfig", iface);/*deconfig script path*/
	sprintf(pid_file, "/etc/udhcpc/udhcpc-%s.pid", iface); /*pid path*/
#if 0
	apmib_get( MIB_IP_ADDR,  (void *)tmp);
	strtmp= inet_ntoa(*((struct in_addr *)tmp));
	sprintf(Ip, "%s",strtmp);

	apmib_get( MIB_SUBNET_MASK,  (void *)tmp);
	strtmp= inet_ntoa(*((struct in_addr *)tmp));
	sprintf(Mask, "%s",strtmp);

	apmib_get( MIB_DEFAULT_GATEWAY,  (void *)tmp);
	strtmp= inet_ntoa(*((struct in_addr *)tmp));
	sprintf(Gateway, "%s",strtmp);
#endif
    if( pDhcpc ) {
        snprintf(Ip, sizeof(Ip), "%s", pDhcpc->X_ACTIONTEC_COM_DefaultIPAddress);
        snprintf(Mask, sizeof(Mask), "%s", pDhcpc->X_ACTIONTEC_COM_DefaultSubnetMask);
        snprintf(Gateway, sizeof(Gateway),"%s", pDhcpc->X_ACTIONTEC_COM_DefaultIPRouters);
    } else {
        snprintf(Ip, sizeof(Ip), "%s", "0.0.0.0");
        snprintf(Mask, sizeof(Mask),"%s", "0.0.0.0");
        snprintf(Gateway, sizeof(Gateway), "%s", "0.0.0.0");
    }
	Create_script(deconfig_script, iface, 0 /*LAN_NETWORK*/, Ip, Mask, Gateway);

    /* Generate Hostname */
    /* per Verizon requirement,
            Hostname in BHR GUI and TR-69 MUST be "WECB-xx:xx",
            where xx:xx is the last 4 digits of the WECB's SN */
    /* per RFC 1035 - Domain names - implementation and specification
            2.3.1. Preferred name syntax
            The labels must follow the rules for ARPANET host names.  They must
            start with a letter, end with a letter or digit, and have as interior
            characters only letters, digits, and hyphen.  There are also some
            restrictions on the length.  Labels must be 63 characters or less.
       So using "WECB-xxxx" here */
    if( TSL_RV_SUC != (rv=ACCESS_LEAF( tr69_get_unfresh_leaf_data,
                    (void **) &pValue, &type,
                    "%s.SerialNumber", TR69_OID_DEVICE_INFO ))) {
        ctllog_warn( "Fail to get serial number" );
    }
    snLen = tsl_strlen( pValue );
    if( snLen >= 4 ) {
        snprintf( strHostname, sizeof(strHostname), "WECB-%c%c%c%c",
                    pValue[snLen-4], pValue[snLen-3],
                    pValue[snLen-2], pValue[snLen-1] );
    } else {
        snprintf( strHostname, sizeof(strHostname), "WECB" );
    }
    CTLMEM_FREE_BUF_AND_NULL_PTR( pValue );

	//RunSystemCmd(NULL_FILE, "udhcpc", "-i", iface, "-p", pid_file, "-s", script_file,  "-n", "-x", NULL_STR);
	//sprintf(cmdBuff, "udhcpc -i %s -p %s -s %s -n &", iface, pid_file, script_file);
	sprintf(cmdBuff, "udhcpc -i %s -p %s -s %s -h %s &",
                    iface, pid_file, script_file, strHostname );
	system(cmdBuff);
}

tsl_rv_t stop_dhcpc( _Dhcpv4ClientObject *p_new_data )
{
      tsl_char_t strPara[BUFLEN_256] = {0};
    /* todo: search pid and then kill */
    system("killall udhcpc >/dev/null 2>&1");
    system("rm -f /etc/udhcpc/udhcpc-*.pid >/dev/null 2>&1");
    if( p_new_data ) {
        CTLMEM_REPLACE_STRING( p_new_data->DHCPStatus, CTLVS_INIT );

        CTLMEM_REPLACE_STRING( p_new_data->IPAddress, "" );
        CTLMEM_REPLACE_STRING( p_new_data->subnetMask, "" );
        CTLMEM_REPLACE_STRING( p_new_data->IPRouters, "" );
        CTLMEM_REPLACE_STRING( p_new_data->DNSServers, "" );
        snprintf(strPara, sizeof(strPara), "%s.X_ACTIONTEC_COM_InternetStatus", p_new_data->interface );
        tr69_set_unfresh_leaf_data( strPara, (void *)CTLVS_DOWN,TR69_NODE_LEAF_TYPE_STRING);
    }
    return TSL_RV_SUC;
}

tsl_void_t triggerDHCPv4Cient( tsl_void_t )
{
    tsl_bool b_trigger = TSL_B_TRUE;
    tr69_set_leaf_data( "Device.DHCPv4.Client.1.X_ACTIONTEC_COM_Trigger",
            (void *)&b_trigger, TR69_NODE_LEAF_TYPE_UINT );
}

