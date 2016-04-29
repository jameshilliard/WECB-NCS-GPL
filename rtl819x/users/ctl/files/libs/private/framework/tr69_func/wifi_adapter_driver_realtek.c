/**************************************************************************
 *	
 *	        wifi_adapter_driver_madwifi.c
 *
 **************************************************************************/
#include <stdio.h> //for snprintf
#include <stdlib.h> //for malloc, system
#include <string.h> //for bcopy
#include <unistd.h> //for close
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <netinet/in.h>
#include <linux/wireless.h>
#include <signal.h>
#include <ctype.h>
#include <pthread.h>

#include <arpa/inet.h> //for inet_aton

#include "sha1.h"
#include "wifi_adapter_driver.h"
#include "libnetlink.h"
#include "ieee802_mib.h"
#include "OID.h"
#include "tr69_func_common.h"
#include "tsl_strconv.h"

#define SIZE_16		16
#define SIZE_20		20
#define SIZE_32		32
#define SIZE_64		64
#define SIZE_128	128
#define SIZE_256	256
#define MAX_STA_NUM			64	// max support sta number

#define SIOCMIBINIT		0x8B42
#define SIOCMIBSYNC		0x8B43

#define USE_REALTEK_SDK
#ifdef USE_REALTEK_SDK
#define WIFI_SIMPLE_CONFIG
#include "apmib.h"
extern int wlan_idx;	// interface index //0-wlan0 5G, 1-wlan1 2.4G
extern int vwlan_idx;	// 0-rootap, 1, vap0, 2-vap1, 3-vap2, 4-vap3
#define USE_REALTEK_MIB
#else
#define NUM_WLAN_INTERFACE 2
#define NUM_VWLAN 4

#define MAX_2G_CHANNEL_NUM_MIB		14
#define MAX_5G_CHANNEL_NUM_MIB		196

typedef struct wlan_rate{
unsigned int id;
unsigned char rate[SIZE_20];
}WLAN_RATE_T, *WLAN_RATE_Tp;
typedef enum { 
	MCS0=0x80, 
	MCS1=0x81, 
	MCS2=0x82,
	MCS3=0x83,
	MCS4=0x84,
	MCS5=0x85,
	MCS6=0x86,
	MCS7=0x87,
	MCS8=0x88,
	MCS9=0x89,
	MCS10=0x8a,
	MCS11=0x8b,
	MCS12=0x8c,
	MCS13=0x8d,
	MCS14=0x8e,
	MCS15=0x8f
	} RATE_11N_T;

/* WLAN sta info structure */
typedef struct wlan_sta_info {
	unsigned short	aid;
	unsigned char	addr[6];
	unsigned long	tx_packets;
	unsigned long	rx_packets;
	unsigned long	expired_time;	// 10 msec unit
	unsigned short	flag;
	unsigned char	txOperaRates;
	unsigned char	rssi;
	unsigned long	link_time;		// 1 sec unit
	unsigned long	tx_fail;
	unsigned long tx_bytes;
	unsigned long rx_bytes;
	unsigned char network;
	unsigned char ht_info;	// bit0: 0=20M mode, 1=40M mode; bit1: 0=longGI, 1=shortGI
	unsigned char	sq;
	unsigned char 	resv[5];
} WLAN_STA_INFO_T, *WLAN_STA_INFO_Tp;

#define STA_INFO_FLAG_ASOC          	0x04
#define SIOCGIWRTLSTAINFO   		0x8B30	// get station table information

typedef enum { 
	AP_MODE=0, 
	CLIENT_MODE=1, 
	WDS_MODE=2, 
	AP_WDS_MODE=3, 
	AP_MPP_MODE=4, 
	MPP_MODE=5, 
	MAP_MODE=6, 
	MP_MODE=7,
	P2P_SUPPORT_MODE=8
} WLAN_MODE_T;

#endif
/**************************************************************************
 *	
 *	        constant definition
 *
 **************************************************************************/

#define PREFIX_WLAN_DEVICE_NAME		"wlan"
#define PREFIX_VAP_DEVICE_NAME		"va"

#define PREFIX_HOSTAPD				"/var/run/hostapd-"
#define CONFIG_FILE_TYPE_HOSTAPD	"conf"
#define CONFIG_FILE_TYPE_ENCRYPTION	"enc"
#define CONFIG_FILE_TYPE_WPS		"wps"

#define SYMBOL_CURRENT_FREQ		"Current Frequency:"
#define SYMBOL_CHANNEL			"Channel"
#define SYMBOL_CURRENT_RATE		"Current Bit Rate:"
#define SYMBOL_HT_RATE          "HT Rates Supported (MCS codes):"
#define SYMBOL_EXTENDED_RATE    "Extended Rates Supported:"
#define SYMBOL_ADDR    			"ADDR"

#define STRING_DISABLED	"Disabled"
#define CHARACTER_COLON	':'
#define STRING_UNKNOWN	"unknown"

#define BAND_B	(1 << 0)
#define BAND_G	(1 << 1)
#define BAND_A	(1 << 2)
#define BAND_N	(1 << 3)

#define AP_CREATION_FLAG_VAP0	(1 << 0)
#define AP_CREATION_FLAG_VAP1	(1 << 1)
#define AP_CREATION_FLAG_VAP2	(1 << 2)
#define AP_CREATION_FLAG_VAP3	(1 << 3)

#define INVALID_HANDLE_VALUE	(-1)

#ifndef STRCMP
#define STRCMP(str1, str2) strcmp((NULL!=str1)?(str1):"", (NULL!=str2)?(str2):"")
#endif

#ifndef STRNCMP
#define STRNCMP(str1, str2, n) strncmp((NULL!=str1)?(str1):"", (NULL!=str2)?(str2):"", n) 
#endif

#ifndef STRNCPY
#define STRNCPY(str1, str2, n) strncpy((NULL!=str1)?(str1):"", (NULL!=str2)?(str2):"", n) 
#endif

/**************************************************************************
 *	
 *	        global variant definition
 *
 **************************************************************************/
#ifdef USE_REALTEK_SDK
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
static volatile unsigned int wlan_disabled = 0;
static volatile unsigned int wlan_disabled_mask = 0;
static int g_apmib_ready = 0;
#endif
static unsigned int wlan_op_mode[NUM_WLAN_INTERFACE] = {0};
static unsigned int ap_creation_flag[NUM_WLAN_INTERFACE] = {0};
static unsigned int radio_creation_flag = 0;
static struct wifi_mib mib_vap[NUM_WLAN_INTERFACE][NUM_VWLAN];
//changes in following table should be synced to MCS_DATA_RATEStr[] in 8190n_proc.c
WLAN_RATE_T rate_11n_table_20M_LONG[]={
	{MCS0, 	"6.5"},
	{MCS1, 	"13"},
	{MCS2, 	"19.5"},
	{MCS3, 	"26"},
	{MCS4, 	"39"},
	{MCS5, 	"52"},
	{MCS6, 	"58.5"},
	{MCS7, 	"65"},
	{MCS8, 	"13"},
	{MCS9, 	"26"},
	{MCS10, 	"39"},
	{MCS11, 	"52"},
	{MCS12, 	"78"},
	{MCS13, 	"104"},
	{MCS14, 	"117"},
	{MCS15, 	"130"},
	{0}
};
WLAN_RATE_T rate_11n_table_20M_SHORT[]={
	{MCS0, 	"7.2"},
	{MCS1, 	"14.4"},
	{MCS2, 	"21.7"},
	{MCS3, 	"28.9"},
	{MCS4, 	"43.3"},
	{MCS5, 	"57.8"},
	{MCS6, 	"65"},
	{MCS7, 	"72.2"},
	{MCS8, 	"14.4"},
	{MCS9, 	"28.9"},
	{MCS10, 	"43.3"},
	{MCS11, 	"57.8"},
	{MCS12, 	"86.7"},
	{MCS13, 	"115.6"},
	{MCS14, 	"130"},
	{MCS15, 	"144.5"},
	{0}
};
WLAN_RATE_T rate_11n_table_40M_LONG[]={
	{MCS0, 	"13.5"},
	{MCS1, 	"27"},
	{MCS2, 	"40.5"},
	{MCS3, 	"54"},
	{MCS4, 	"81"},
	{MCS5, 	"108"},
	{MCS6, 	"121.5"},
	{MCS7, 	"135"},
	{MCS8, 	"27"},
	{MCS9, 	"54"},
	{MCS10, 	"81"},
	{MCS11, 	"108"},
	{MCS12, 	"162"},
	{MCS13, 	"216"},
	{MCS14, 	"243"},
	{MCS15, 	"270"},
	{0}
};
WLAN_RATE_T rate_11n_table_40M_SHORT[]={
	{MCS0, 	"15"},
	{MCS1, 	"30"},
	{MCS2, 	"45"},
	{MCS3, 	"60"},
	{MCS4, 	"90"},
	{MCS5, 	"120"},
	{MCS6, 	"135"},
	{MCS7, 	"150"},
	{MCS8, 	"30"},
	{MCS9, 	"60"},
	{MCS10, 	"90"},
	{MCS11, 	"120"},
	{MCS12, 	"180"},
	{MCS13, 	"240"},
	{MCS14, 	"270"},
	{MCS15, 	"300"},
	{0}
};

/**************************************************************************
 *	
 *	        structure definition
 *
 **************************************************************************/

/**************************************************************************
 *	
 *	        static function declaration
 *
 **************************************************************************/
static int shell(const char *command);
static int wifi_get_statistics(char *devName, unsigned int *bytesRx, unsigned int *bytesTx,
		unsigned int *packetsRx, unsigned int *packetsTx, 
		unsigned int *errsRx, unsigned int *errsTx, 
		unsigned int *dropRx, unsigned int *dropTx, 
		unsigned int *fifoRx, unsigned int *fifoTx, 
		unsigned int *compressedRx, unsigned int *compressedTx, 
        unsigned int *frameRx, unsigned int *multicastRx, 
        unsigned int *collsTx, unsigned int *carrierTx);
static unsigned int AsciiCode2HexString(unsigned char *code, unsigned int code_len, char *hexString);
static unsigned int HexString2AsciiCode(char *hex, unsigned int hex_len, unsigned char *asciiCode);
static void sha1_vector(size_t num_elem, unsigned char *addr[], const size_t *len, unsigned char *mac);
static void generate_uuid_by_mac_addr(char *mac_addr, char *uuid);
static unsigned int wifi_get_associated_device_info(char* devName, WLAN_STA_INFO_Tp devInfo, unsigned int max_devices);
static int get_neigh(const struct sockaddr_nl *who, struct nlmsghdr *n, void *arg);
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
static int wifi_set_mib(char* devName, int id, void* value);
#else
static int wifi_set_parameter(char* devName, char* name, unsigned int value, char* str);
#endif
static int wifi_get_parameter(char* devName, char* name, char* value);
static void* routine_thread_message(void* param);
static int wifi_get_global_parameter(char* name, char* value);
static int wifi_set_global_parameter(char* name, unsigned int value, char* str);
#endif
static int getWlStaInfo( char *interface,  WLAN_STA_INFO_Tp pInfo );
static int iwpriv_set_mib(char* devName, char* name, unsigned int value, char* str);
static int iwpriv_get_mib(char* devName, char* name, char* value);
static int wifi_get_all_virtual_devices(char* wlanDevName, char* devNameList);
static int wifi_get_default_MACAddress(char *name, char *mac);


/**************************************************************************
 *	
 *	        static function definition
 *
 **************************************************************************/
static int shell(const char *command)
{
//	puts(command);

	return system(command);
}

static int wifi_get_statistics(char *devName, unsigned int *bytesRx, unsigned int *bytesTx,
		unsigned int *packetsRx, unsigned int *packetsTx, 
		unsigned int *errsRx, unsigned int *errsTx, 
		unsigned int *dropRx, unsigned int *dropTx, 
		unsigned int *fifoRx, unsigned int *fifoTx, 
		unsigned int *compressedRx, unsigned int *compressedTx, 
        unsigned int *frameRx, unsigned int *multicastRx, 
        unsigned int *collsTx, unsigned int *carrierTx)
{//done
	int rv = 0;
	FILE* fp = NULL;
	char line[SIZE_256] = {0};
	char* p = NULL;
	int data[SIZE_16] = {0};

	do{
		if(NULL==devName){
			break;
		}

		fp = fopen("/proc/net/dev", "r");
		if(NULL==fp){
			break;
		}

		while(NULL!=fgets(line, sizeof(line), fp)){
			p = strstr(line, devName);
			if(NULL==p){
				continue;
			}

			p += strlen(devName)+1;
			if(16!=sscanf(p, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
				data+0, data+1, data+2, data+3, data+4, data+5, data+6, data+7, 
				data+8, data+9, data+10, data+11, data+12, data+13, data+14, data+15)){
				break;
			}

			if(NULL!=bytesRx)
			{
				*bytesRx = data[0];
				rv++;
			}
			if(NULL!=packetsRx)
			{
				*packetsRx = data[1];
				rv++;
			}
			if(NULL!=errsRx)
			{
				*errsRx = data[2];
				rv++;
			}
			if(NULL!=dropRx)
			{
				*dropRx = data[3];
				rv++;
			}
			if(NULL!=fifoRx)
			{
				*fifoRx = data[4];
				rv++;
			}
			if(NULL!=frameRx)
			{
				*frameRx = data[5];
				rv++;
			}
			if(NULL!=compressedRx)
			{
				*compressedRx = data[6];
				rv++;
			}
			if(NULL!=multicastRx)
			{
				*multicastRx = data[7];
				rv++;
			}
			if(NULL!=bytesTx)
			{
				*bytesTx = data[8];
				rv++;
			}
			if(NULL!=packetsTx)
			{
				*packetsTx = data[9];
				rv++;
			}
			if(NULL!=errsTx)
			{
				*errsTx = data[10];
				rv++;
			}
			if(NULL!=dropTx)
			{
				*dropTx = data[11];
				rv++;
			}
			if(NULL!=fifoTx)
			{
				*fifoTx = data[12];
				rv++;
			}
			if(NULL!=collsTx)
			{
				*collsTx = data[13];
				rv++;
			}
			if(NULL!=carrierTx)
			{
				*carrierTx = data[14];
				rv++;
			}
			if(NULL!=compressedTx)
			{
				*compressedTx = data[15];
				rv++;
			}
		}
		
		fclose(fp);
	}while(0);

	return rv;
}

static unsigned int AsciiCode2HexString(unsigned char *code, unsigned int code_len, char *hexString)
{
	int i, j;
	unsigned char data[2];
	
	if(NULL==code||0==code_len||NULL==hexString){
		return 0;
	}

	for(i=code_len-1; i>=0; i--){
		data[1] = code[i]&0x0F;
		data[0] = code[i] >> 4;
		for(j=0; j<2; j++){
			if(data[j]<=9){
				data[j] += 0x30;
			}
			else if(data[j]>=0x0a&&data[j]<=0x0f){
				data[j] += 'a'-0x0a;
			}else{
				return 0;
			}
		}
		
		j = i<<1;
		hexString[j+1] = data[1];
		hexString[j+0] = data[0];
	}

	return (code_len<<1);
}

static unsigned int HexString2AsciiCode(char *hex, unsigned int hex_len, unsigned char *asciiCode)
{
 	int i, j;
 	unsigned char data[2];

 	if(NULL==hex||0==hex_len||hex_len%2!=0||NULL==asciiCode){
  		return 0;
 	}

 	for (i=0; i<hex_len; i+=2){
  		data[0] = hex[i];
  		data[1] = hex[i+1];

		for (j=0; j<2; j++){
			if (data[j]>='0'&&data[j]<='9'){
				data[j] -= 0x30;
			}
			else if (data[j]>='a'&&data[j]<='f'){
				data[j] -= 'a' - 0x0a;
			}else if (data[j]>='A'&&data[j]<='F'){
				data[j] -= 'A' - 0x0a;
			}else{
				return 0;
			}
		}
		
  		asciiCode[i>>1] = data[0] << 4 | data[1];
 	}

 	return (hex_len>>1);
}

static void sha1_vector(size_t num_elem, unsigned char *addr[], const size_t *len, unsigned char *mac)
{
	SHA1_CTX ctx;
	size_t i;

	SHA1Init(&ctx);
	for (i = 0; i < num_elem; i++)
		SHA1Update(&ctx, addr[i], len[i]);
	SHA1Final(mac, &ctx);
}

static void generate_uuid_by_mac_addr(char *mac_addr, char *uuid)
{
	unsigned char *addr[2];
	size_t len[2];
	unsigned char hash[SHA1_MAC_LEN] = {0};
	unsigned char nsid[SIZE_16] = {
		0x52, 0x64, 0x80, 0xf8,
		0xc9, 0x9b,
		0x4b, 0xe5,
		0xa6, 0x55,
		0x58, 0xed, 0x5f, 0x5d, 0x60, 0x84
	};

	addr[0] = nsid;
	len[0] = sizeof(nsid);
	addr[1] = (unsigned char *)mac_addr;
	len[1] = 6;
	sha1_vector(2, addr, len, hash);
	memcpy(uuid, hash, SIZE_16);

	/* Version: 5 = named-based version using SHA-1 */
	uuid[6] = (5 << 4) | (uuid[6] & 0x0f);

	/* Variant specified in RFC 4122 */
	uuid[8] = 0x80 | (uuid[8] & 0x3f);
}

static unsigned int wifi_get_associated_device_info(char* devName, WLAN_STA_INFO_Tp devInfo, unsigned int max_devices)
{//done
	int devices = 0;
	WLAN_STA_INFO_Tp pInfo;
	char *buff = NULL;
	
	do
	{
		if(NULL==devName||devName!=strstr(devName, PREFIX_WLAN_DEVICE_NAME)){
			printf("%s: Invalid device name %s\n", __func__, devName);
			break;
		}

		buff = calloc(1, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1));
		if ( buff == NULL ) {
			printf("Allocate buffer failed!\n");
			break;
		}
		
		if (getWlStaInfo(devName, (WLAN_STA_INFO_Tp)buff )<0){
			printf("Read wlan sta info failed!\n");
			break;
		}
		
		pInfo = (WLAN_STA_INFO_Tp)&buff[1*sizeof(WLAN_STA_INFO_T)];
		while(pInfo->aid && (pInfo->flag & STA_INFO_FLAG_ASOC)){
			if(NULL!=devInfo){
				memcpy(devInfo+devices, pInfo, sizeof(WLAN_STA_INFO_T));
			}
			devices++;
			if(devices>MAX_STA_NUM||devices>=max_devices){
				break;
			}
			pInfo++;
		}
	}while(0);

	if(NULL!=buff){
		free(buff);
		buff = NULL;
	}
	
	return devices;
}

static int get_neigh(const struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{//done
	int ret = 0;
	struct ndmsg *r = NLMSG_DATA(n);
	int len = n->nlmsg_len;
	struct rtattr * tb[NDA_MAX+1];
	char abuf[SIZE_256] = {0};
	unsigned char *p = NULL;
	
	do{
		if (n->nlmsg_type != RTM_NEWNEIGH && n->nlmsg_type != RTM_DELNEIGH) {
			break;//Not RTM_NEWNEIGH
		}
		len -= NLMSG_LENGTH(sizeof(*r));
		if (len < 0) {
			fprintf(stderr, "BUG: wrong nlmsg len %d\n", len);
			break;
		}
		parse_rtattr(tb, NDA_MAX, NDA_RTA(r), n->nlmsg_len - NLMSG_LENGTH(sizeof(*r)));
		p = (unsigned char*)RTA_DATA(tb[NDA_LLADDR]);
		if(NULL==p){
			break;
		}
		snprintf(abuf, sizeof(abuf), "%.02x:%.02x:%.02x:%.02x:%.02x:%.02x", p[0], p[1], p[2], p[3], p[4], p[5]);

		if(0!=strncasecmp((char*)arg, abuf, 17)){
			break;
		}
		
		p = (char*)RTA_DATA(tb[NDA_DST]);
		if(NULL==p){
			break;
		}
		snprintf((char*)arg, SIZE_16, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
		ret = -1;
	}while(0);

	return ret;
}

#ifdef USE_REALTEK_SDK
static int wifi_get_global_parameter(char* name, char* value)
{
	int ret = 0;
	char cmd[SIZE_256]={0};
	char buff[SIZE_128]={0};
	FILE *fp;
	char *p = NULL;

	do{
		if(NULL==value){
			printf("%s: The parameter value can not be null\n", __func__);
			ret = -1;
			break;
		}
		
		snprintf(cmd, sizeof(cmd), "flash get %s", name);
		snprintf(buff, sizeof(buff), "%s=", name);
		
		fp =  popen(cmd, "r");
		if(fp != NULL){
			while (NULL!=fgets(cmd, sizeof(cmd),fp)){
				strtok(cmd, "\n");
				p = strstr(cmd, buff);
				if (NULL!=p){
					strcpy(value, p+strlen(buff));
					//remove ["]
					if(value[0]=='\"'){
						memmove(value, value+1, strlen(value)-1);
					}
					strtok(value, "\"");
					ret = 0;
				}else{
					printf("%s: Fail to get [%s]\n", __func__, name);
					ret = -1;
				}
				break;
			}
			pclose(fp);
		}else{
			perror("popen");
			ret = -1;
		}
	}while(0);

	return ret;
}

static int wifi_set_global_parameter(char* name, unsigned int value, char* str)
{
	int ret = 0;
	char cmd[SIZE_128]={0};
	char old_value[SIZE_128]={0};

	do{
		if(wifi_get_global_parameter(name, old_value)<0){
			ret = -1;
			break;
		}
			
		if(str){
			if(str[0]==0){
				ret = -1;
				break;
			}
			if(0==strcmp(str, old_value)){
				break;
			}
			printf("%s: name=[%s], old=[%s], new=[%s]\n", __func__, name, old_value, str);
		}else{
			if(value==atoi(old_value)){
				break;
			}
			printf("%s: name=[%s], old=[%s], new=[%d]\n", __func__, name, old_value, value);
		}
		
		if(str){
			snprintf(cmd, sizeof(cmd), "flash set %s %s", name, str);
		}else{
			snprintf(cmd, sizeof(cmd), "flash set %s %d", name, value);
		}
		
		ret += shell(cmd);
		
	}while(0);

	return ret;
}

#ifdef USE_REALTEK_MIB
/*
static int wifi_get_mib(char* devName, int id, void* value)
{
		int ret = 0;
		unsigned int wlan_index = devName[4] - '0';
		unsigned int vap_index = devName[8] - '0';
	
		do{
			if(NULL==devName||devName!=strstr(devName, PREFIX_WLAN_DEVICE_NAME)){
				printf("%s: invalid device name [%s]\n", __func__, devName);
				ret = -1;
				break;
			}
			
			if(wlan_index>=NUM_WLAN_INTERFACE){
				printf("%s: The index of [%s] should be less than %d\n", __func__, devName, NUM_WLAN_INTERFACE);
				ret = -1;
				break;
			}
	
			if(NULL==value){
				printf("%s: The parameter value can not be null\n", __func__);
				ret = -1;
				break;
			}
			wlan_idx = wlan_index;
			
			if(5==strlen(devName)){
				vwlan_idx = 0;
			}else if(9==strlen(devName)){
				if(vap_index>=NUM_VWLAN){
					printf("%s: The index of [%s] should be less than %d\n", __func__, devName, NUM_VWLAN);
					ret = -1;
					break;
				}
				vwlan_idx = vap_index + 1;
			}else{
				printf("%s: invalid device name [%s]\n", __func__, devName);
				ret = -1;
				break;
			}

			ret = apmib_get(id, value);
		}while(0);
	
		return ret;
}
*/
static int wifi_set_mib(char* devName, int id, void* value)
{
	int ret = 0;
	unsigned int wlan_index = 0;
	unsigned int vap_index = 0;

	do{
		if(NULL==devName||devName!=strstr(devName, PREFIX_WLAN_DEVICE_NAME)){
			printf("%s: invalid device name [%s]\n", __func__, devName);
			ret = -1;
			break;
		}
		wlan_index = devName[4] - '0';
		vap_index = devName[8] - '0';
		if(wlan_index>=NUM_WLAN_INTERFACE){
			printf("%s: The index of [%s] should be less than %d\n", __func__, devName, NUM_WLAN_INTERFACE);
			ret = -1;
			break;
		}
		
		if(NULL==value){
			printf("%s: The parameter value can not be null\n", __func__);
			ret = -1;
			break;
		}
		wlan_idx = wlan_index;

		if(5==strlen(devName)){
			vwlan_idx = 0;
		}else if(9==strlen(devName)){
			if(vap_index>=NUM_VWLAN){
				printf("%s: The index of [%s] should be less than %d\n", __func__, devName, NUM_VWLAN);
				ret = -1;
				break;
			}
			
			vwlan_idx = vap_index + 1;
		}else{
			printf("%s: invalid device name [%s]\n", __func__, devName);
			ret = -1;
			break;
		}

//		printf("====>set [id=%d, value=%08x]\n", id, *(int*)value);		
		ret = !apmib_set(id, value);
		
	}while(0);

	return ret;
}
#else
static int wifi_set_parameter(char* devName, char* name, unsigned int value, char* str)
{
	int ret = 0;
	char cmd[SIZE_128] = {0};
	char old_value[SIZE_128] = {0};
	unsigned int wlan_index = 0;
	unsigned int vap_index = 0;

	do{
		if(wifi_get_parameter(devName, name, old_value)<0){
			ret = -1;
			break;
		}
			
		if(str){
			if(str[0]==0){
				ret = -1;
				break;
			}
			if(0==strcmp(str, old_value)){
				break;
			}
			printf("%s: name=[%s], old=[%s], new=[%s]\n", __func__, name, old_value, str);
		}else{
			if(value==atoi(old_value)){
				break;
			}
			printf("%s: name=[%s], old=[%s], new=[%d]\n", __func__, name, old_value, value);
		}
		
		if(NULL==devName||devName!=strstr(devName, PREFIX_WLAN_DEVICE_NAME)){
			printf("%s: invalid device name [%s]\n", __func__, devName);
			ret = -1;
			break;
		}
		wlan_index = devName[4] - '0';
		vap_index = devName[8] - '0';
		if(wlan_index>=NUM_WLAN_INTERFACE){
			printf("%s: The index of [%s] should be less than %d\n", __func__, devName, NUM_WLAN_INTERFACE);
			ret = -1;
			break;
		}
		
		if(5==strlen(devName)){
			if(str){
				snprintf(cmd, sizeof(cmd), "flash set WLAN%d_%s %s", wlan_index, name, str);
			}else{
				snprintf(cmd, sizeof(cmd), "flash set WLAN%d_%s %d", wlan_index, name, value);
			}
		}else if(9==strlen(devName)){
			if(vap_index>=NUM_VWLAN){
				printf("%s: The index of [%s] should be less than %d\n", __func__, devName, NUM_VWLAN);
				ret = -1;
				break;
			}
			
			if(str){
				snprintf(cmd, sizeof(cmd), "flash set WLAN%d_VAP%d_%s %s", wlan_index, vap_index, name, str);
			}else{
				snprintf(cmd, sizeof(cmd), "flash set WLAN%d_VAP%d_%s %d", wlan_index, vap_index, name, value);
			}
		}else{
			printf("%s: invalid device name [%s]\n", __func__, devName);
			ret = -1;
			break;
		}
		
		ret += shell(cmd);
		
	}while(0);

	return ret;
}
#endif
static int wifi_get_parameter(char* devName, char* name, char* value)
{
	int ret = 0;
	char cmd[SIZE_256] = {0};
	char buff[SIZE_128] = {0};
	unsigned int wlan_index = 0;
	unsigned int vap_index = 0;
	FILE *fp;
	char *p = NULL;

	do{
		if(NULL==devName||devName!=strstr(devName, PREFIX_WLAN_DEVICE_NAME)){
			printf("%s: invalid device name [%s]\n", __func__, devName);
			ret = -1;
			break;
		}
		wlan_index = devName[4] - '0';
		vap_index = devName[8] - '0';
		if(wlan_index>=NUM_WLAN_INTERFACE){
			printf("%s: The index of [%s] should be less than %d\n", __func__, devName, NUM_WLAN_INTERFACE);
			ret = -1;
			break;
		}

		if(NULL==value){
			printf("%s: The parameter value can not be null\n", __func__);
			ret = -1;
			break;
		}
		
		if(5==strlen(devName)){
			snprintf(cmd, sizeof(cmd), "flash get WLAN%d_%s", wlan_index, name);
			snprintf(buff, sizeof(buff), "WLAN%d_%s=", wlan_index, name);
		}else if(9==strlen(devName)){
			if(vap_index>=NUM_VWLAN){
				printf("%s: The index of [%s] should be less than %d\n", __func__, devName, NUM_VWLAN);
				ret = -1;
				break;
			}
			snprintf(cmd, sizeof(cmd), "flash get WLAN%d_VAP%d_%s", wlan_index, vap_index, name);
			snprintf(buff, sizeof(buff), "WLAN%d_VAP%d_%s=", wlan_index, vap_index, name);
		}else{
			printf("%s: invalid device name [%s]\n", __func__, devName);
			ret = -1;
			break;
		}
		
		fp =  popen(cmd, "r");
		if(fp != NULL){
			while (NULL!=fgets(cmd, sizeof(cmd),fp)){
				strtok(cmd, "\n");
				p = strstr(cmd, buff);
				if (NULL!=p){
					strcpy(value, p+strlen(buff));
					//remove ["]
					if(value[0]=='\"'){
						memmove(value, value+1, strlen(value)-1);
					}
					strtok(value, "\"");
					ret = 0;
				}else{
					if(strstr(name, "MACAC_ADDR")){
						value[0] = 0;
						break;
					}
					printf("%s: Fail to get [%s]\n", __func__, name);
					ret = -1;
				}
				break;
			}
			pclose(fp);
		}else{
			perror("popen");
			ret = -1;
		}
	}while(0);

	return ret;
}
#endif

static int getWlStaInfo(char* devName,  WLAN_STA_INFO_Tp pInfo )
{
	int ret = -1;
    int skfd=-1;
    struct iwreq wrq;

	do{
		skfd = socket(AF_INET, SOCK_DGRAM, 0);
		if(skfd<0){
			break;
		}
		/* Get wireless name */
		snprintf(wrq.ifr_name, sizeof(wrq.ifr_name), "%s", devName);
		if ( ioctl(skfd, SIOCGIWNAME, &wrq) < 0){
			/* If no wireless name : no wireless extensions */
			break;
		}
		
		wrq.u.data.pointer = (caddr_t)pInfo;
		wrq.u.data.length = sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1);
		memset(pInfo, 0, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1));
		
		snprintf(wrq.ifr_name, sizeof(wrq.ifr_name), "%s", devName);
		if (ioctl(skfd, SIOCGIWRTLSTAINFO, &wrq) < 0){
			break;
		}

		ret = 0;
	}while(0);

	if(-1!=skfd){
		close(skfd);
	}

	return ret;
}

static int iwpriv_set_mib(char* devName, char* name, unsigned int value, char* str)
{
	int ret = 0;
	char cmd[SIZE_128] = {0};
	unsigned int wlan_index = 0;
	unsigned int vap_index = 0;

	do{
		if(NULL==devName||devName!=strstr(devName, PREFIX_WLAN_DEVICE_NAME)){
			printf("%s: invalid device name [%s]\n", __func__, devName);
			ret = -1;
			break;
		}
		wlan_index = devName[4] - '0';
		vap_index = devName[8] - '0';
		if(wlan_index>=NUM_WLAN_INTERFACE){
			printf("%s: The index of [%s] should be less than %d\n", __func__, devName, NUM_WLAN_INTERFACE);
			ret = -1;
			break;
		}
		
		if(5==strlen(devName)){
		}else if(9==strlen(devName)){
			if(vap_index>=NUM_VWLAN){
				printf("%s: The index of [%s] should be less than %d\n", __func__, devName, NUM_VWLAN);
				ret = -1;
				break;
			}
		}else{
			printf("%s: invalid device name [%s]\n", __func__, devName);
			ret = -1;
			break;
		}
		
		if(str){
			snprintf(cmd, sizeof(cmd), "iwpriv %s set_mib %s=%s", devName, name, str);
		}else{
			snprintf(cmd, sizeof(cmd), "iwpriv %s set_mib %s=%d", devName, name, value);
		}

		ret += shell(cmd);
		
	}while(0);

	return ret;
}

static int iwpriv_get_mib(char* devName, char* name, char* value)
{
	int ret = 0;
	char cmd[SIZE_256] = {0};
	char buff[SIZE_128] = {0};
	unsigned int wlan_index = 0;
	unsigned int vap_index = 0;
	FILE *fp;
	char *p = NULL;

	do{
		if(NULL==devName||devName!=strstr(devName, PREFIX_WLAN_DEVICE_NAME)){
			printf("%s: invalid device name [%s]\n", __func__, devName);
			ret = -1;
			break;
		}
		wlan_index = devName[4] - '0';
		vap_index = devName[8] - '0';
		if(wlan_index>=NUM_WLAN_INTERFACE){
			printf("%s: The index of [%s] should be less than %d\n", __func__, devName, NUM_WLAN_INTERFACE);
			ret = -1;
			break;
		}

		if(NULL==value){
			printf("%s: The parameter value can not be null\n", __func__);
			ret = -1;
			break;
		}
		
		if(5==strlen(devName)){
		}else if(9==strlen(devName)){
			if(vap_index>=NUM_VWLAN){
				printf("%s: The index of [%s] should be less than %d\n", __func__, devName, NUM_VWLAN);
				ret = -1;
				break;
			}
		}else{
			printf("%s: invalid device name [%s]\n", __func__, devName);
			ret = -1;
			break;
		}

		snprintf(cmd, sizeof(cmd), "iwpriv %s get_mib %s", devName, name);
		snprintf(buff, sizeof(buff), "get_mib:");
		
		fp =  popen(cmd, "r");
		if(fp != NULL){
			while (NULL!=fgets(cmd, sizeof(cmd),fp)){
				strtok(cmd, "\n");
				p = strstr(cmd, buff);
				if (NULL!=p){
					strcpy(value, p+strlen(buff));
					ret = 0;
				}else{
					printf("%s: Fail to get [%s]\n", __func__, name);
					ret = -1;
				}
				break;
			}
			pclose(fp);
		}else{
			perror("popen");
			ret = -1;
		}
	}while(0);

	return ret;
}

static int wifi_get_all_virtual_devices(char* wlanDevName, char* devNameList)
{//only for tr-098
	int count = 0;
	char cmd[SIZE_256] = {0};
	FILE *fp;

	do {
		if (NULL==devNameList){
			break;
		}
		devNameList[0] = 0;
		sprintf(cmd, "ifconfig -a|grep %s|grep %s", PREFIX_WLAN_DEVICE_NAME, PREFIX_VAP_DEVICE_NAME);
		
		fp =  popen(cmd, "r");
		if(fp != NULL){
			while (NULL!=fgets(cmd, sizeof(cmd),fp)){
				if(0==STRNCMP(cmd, PREFIX_WLAN_DEVICE_NAME, strlen(PREFIX_WLAN_DEVICE_NAME))){
					strcat(devNameList, strtok(cmd, " \t"));
					strcat(devNameList, WLAN_DEVICE_NAME_DELIMITER);
					count++;
				}
			}
			pclose(fp);
		}		
	} while(0);

	printf("###############%s: devNameList=[%s]\n", __func__, devNameList);
	
	return count;
}


#ifdef USE_REALTEK_SDK
static void* routine_thread_message(void* param)
{
	fd_set		readfds; 
	struct timeval tv = {10, 0};	
	int status = -1;
	int i = 0;
	char cmd[SIZE_128] = {0};
	int fd = (int)param;
	static int aclset = 0;
	char name[SIZE_32] = {0};

	while(1)
	{
		FD_ZERO(&readfds);
		FD_SET(fd,&readfds);
		
		status = select(fd+1, &readfds, NULL, NULL, &tv);
		if (status < 0)
		{
			perror("select");
			break;
		}	
		
		if (status > 0)
		{
			tv.tv_sec = 10;
			if (FD_ISSET(fd, &readfds))
			{
//{fix coverity CID 29768: String not null terminated 				
				memset(cmd, 0, sizeof(cmd));
				read(fd, cmd, sizeof(cmd)-1);
//}fix coverity CID 29768: String not null terminated				
				i++;
				//printf("====> %s: (i=%d) cmd=[%s]\n", __func__, i, cmd);
				if(i==18){
					tv.tv_sec = 0;
				}
			}
			continue;
		}	

		tv.tv_sec = 5;

//{fix coverity CID 29799: Use of untrusted string value
		if(cmd!=strstr(cmd, "sysconf init")){
			continue;
		}

		strcpy(cmd, "sysconf init gw all");

        pthread_mutex_lock(&mutex);

		for(i=0; i<5; i++){
			if(i){
				sprintf(name, "wlan0-va%d", i-1);
			}else{
				sprintf(name, "%s", "wlan0");
			}
#ifdef USE_REALTEK_MIB
			if(wlan_disabled_mask&(1<<i)){
				int disabled = 0!=(wlan_disabled&(1<<i));
				wifi_set_mib(name, MIB_WLAN_WLAN_DISABLED, &disabled);
			}
#else
			wifi_set_parameter(name, "WLAN_DISABLED", 0!=(wlan_disabled&(1<<i)), NULL);
#endif
			if(i){
				sprintf(name, "wlan1-va%d", i-1);
			}else{
				sprintf(name, "%s", "wlan1");
			}
#ifdef USE_REALTEK_MIB
			if(wlan_disabled_mask&(1<<(i+8))){
				int disabled = 0!=(wlan_disabled&(1<<(i+8)));
				wifi_set_mib(name, MIB_WLAN_WLAN_DISABLED, &disabled);
			}
#else
			wifi_set_parameter(name, "WLAN_DISABLED", 0!=(wlan_disabled&(1<<(i+8))), NULL); 			
#endif
		}

		wlan_disabled_mask = 0;
        pthread_mutex_unlock(&mutex);
		if(!apmib_update(CURRENT_SETTING)){
			printf("fail to do apmib_update!\n");
			continue;
		}
		shell(cmd);

#ifdef AEI_WECB
        /* When wifi interface being ready,
         * trigger Higher Layer Object of Device.SSID.{i} once.
         * Now, Since WCB3000's GUI/CGI will not trigger data_center,
         * so moved trigger code into RTL SDK's sys_conf
         */
        //shell("cli -s Device.Bridging.Bridge.1.Port.3.X_ACTIONTEC_COM_Trigger int 1");
        //shell("cli -s Device.Bridging.Bridge.1.Port.4.X_ACTIONTEC_COM_Trigger int 1");
#endif
		if (!aclset) {
            // apmib becomes ready
            g_apmib_ready = 1;
            printf( "APMIB becomes ready\n" );
#ifdef AEI_ZERO_CONF
            // after APMIB ready, notice zeroconf
            snprintf( cmd, sizeof(cmd)-1,
                    "cli -s %s.X_ACTIONTEC_COM_Trigger int 1",
                     TR69_OID_ZERO_CONF_EXTENDER );
            shell( cmd );
#endif

#ifdef AEI_WIFI_2TX
            // Added RTL patch to enable 2TX function of 2.4G and 5G
            // 5G
            system( "ifconfig wlan0 down" );
            system( "iwpriv wlan0 set_mib tx2path=1" );
            system( "ifconfig wlan0 up" );
            // 2.4G
            system( "ifconfig wlan1 down" );
            system( "iwpriv wlan1 set_mib tx2path=1" );
            system( "ifconfig wlan1 up" );
#endif
			aclset = 1;
		}
		memset(cmd, 0, sizeof(cmd));
//}fix coverity CID 29799: Use of untrusted string value
	}   

	return NULL;
}
#endif

static int wifi_get_default_MACAddress(char *name, char *mac)
{
        int ret = 0;
        unsigned int wlan_index = 0;
        unsigned int vap_index = 0;
        char cmd[SIZE_256] = {0};
        char buff[SIZE_128] = {0};
        FILE *fp;
        char *p = NULL;

        do{
                if(NULL==name||name!=strstr(name, PREFIX_WLAN_DEVICE_NAME)){
                        printf("%s: invalid device name [%s]\n", __func__, name);
                        ret = -1;
                        break;
                }
				wlan_index = name[4] - '0';
				vap_index = name[8] - '0';
                if(wlan_index>=NUM_WLAN_INTERFACE){
                        printf("%s: The index of [%s] should be less than %d\n", __func__, name, NUM_WLAN_INTERFACE);
                        ret = -1;
                        break;
                }

                if(NULL==mac){
                        printf("%s: The parameter mac can not be null\n", __func__);
                        ret = -1;
                        break;
                }

                if(5==strlen(name)){
                        snprintf(cmd, sizeof(cmd), "flash gethw HW_WLAN%d_WLAN_ADDR", wlan_index);
                        snprintf(buff, sizeof(buff), "HW_WLAN%d_WLAN_ADDR=", wlan_index);
                }else if(9==strlen(name)){
                        if(vap_index>=NUM_VWLAN){
                                printf("%s: The index of [%s] should be less than %d\n", __func__, name, NUM_VWLAN);
                                ret = -1;
                                break;
                        }
                        snprintf(cmd, sizeof(cmd), "flash gethw HW_WLAN%d_WLAN_ADDR%d", wlan_index, vap_index+1);
                        snprintf(buff, sizeof(buff), "HW_WLAN%d_WLAN_ADDR%d=", wlan_index, vap_index+1);
                }else{
                        printf("%s: invalid device name [%s]\n", __func__, name);
                        ret = -1;
                        break;
                }

                fp =  popen(cmd, "r");
                if(fp != NULL){
                        while (NULL!=fgets(cmd, sizeof(cmd),fp)){
                                strtok(cmd, "\n");
                                p = strstr(cmd, buff);
                                if (NULL!=p){
//{fix coverity CID 30815: Copy into fixed size buffer 
									snprintf(buff, sizeof(buff), "%s", p+strlen(buff));
//}fix coverity CID 30815: Copy into fixed size buffer 
                                    sprintf(mac, "%c%c:%c%c:%c%c:%c%c:%c%c:%c%c",
										buff[0], buff[1], buff[2], buff[3], buff[4], buff[5],
										buff[6], buff[7], buff[8], buff[9], buff[10], buff[11]);
									ret = 0;
                                }else{
                                        printf("%s: Fail to get [%s]\n", __func__, name);
                                        ret = -1;
                                }
                                break;
                        }
                        pclose(fp);
                }else{
                        perror("popen");
                        ret = -1;
                }
        }while(0);

        return ret;
}


/**************************************************************************
 *	
 *	        interface function definition
 *
 **************************************************************************/

int is_apmib_ready(void)
{
    return g_apmib_ready;
}

int wifi_get_MACAddress(char *name, char *mac)
{
	int ret = -1;
	int s = -1;
	struct ifreq ifr;

	do{
		s=socket(PF_INET, SOCK_DGRAM, 0);
		if(s<0){
			perror("socket");
			break;
		}
		memset(&ifr, 0x00, sizeof(ifr));
		snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", name);
		if (ioctl(s, SIOCGIFHWADDR, &ifr)<0){		
			perror("ioctl");
			break;
		}
		sprintf(mac,"%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
			(unsigned char)ifr.ifr_hwaddr.sa_data[0],
			(unsigned char)ifr.ifr_hwaddr.sa_data[1],
			(unsigned char)ifr.ifr_hwaddr.sa_data[2],
			(unsigned char)ifr.ifr_hwaddr.sa_data[3],
			(unsigned char)ifr.ifr_hwaddr.sa_data[4],
			(unsigned char)ifr.ifr_hwaddr.sa_data[5] );	
		ret = 0;
	}while(0);

	if(-1!=s){
		close(s);
	}
	
	return ret;
}

int wifi_destroy_ssid_device(char* devName)
{
	int ret = 0;
	unsigned int wlan_index = 0;//wlan0-va0
	unsigned int vap_index = 0;//wlan0-va0
	do{
		if((NULL==devName)||(devName!=strstr(devName, PREFIX_WLAN_DEVICE_NAME))){
			printf("%s: Invalid device name: %s\n", __func__, devName);
			ret = -1;
			break;
		}
		wlan_index = devName[4] - '0';//wlan0-va0
		vap_index = devName[8] - '0';//wlan0-va0
		if(wlan_index>=NUM_WLAN_INTERFACE){
			printf("%s: Invalid radio device index: %d\n", __func__, wlan_index);
			ret = -1;
			break;
		}
		
		if(strlen(devName)==5){//wlan0
			wlan_op_mode[wlan_index] = 0;
		}else if(strlen(devName)==9){
			if(vap_index>=NUM_VWLAN){
				printf("%s: Invalid VAP device index: %d\n", __func__, vap_index);
				ret = -1;
				break;
			}
			ap_creation_flag[wlan_index] &= ~(1 << (vap_index+1));
			if(AP_MODE+1==wlan_op_mode[wlan_index]&&0==ap_creation_flag[wlan_index]){
				wlan_op_mode[wlan_index] = 0;
			}
		}

		ret += wifi_close_device(devName);
		printf("[%s]: vap [%s] has been destroyed\n", __func__, devName);
	}while(0);

	return ret;
}

int wifi_create_ssid_device(char* radioName, DEVICE_OPERATION_MODE devOperationMode, char* devName)
{
	int ret = 0;
	unsigned int wlan_index = 0;//wlan0
	int i;
	char buff[SIZE_64] = {0};
	int skfd = -1;
	struct iwreq wrq;
	unsigned int phyBandSelect = 0;

	do {
		if((NULL==radioName)||(radioName!=strstr(radioName, PREFIX_WLAN_DEVICE_NAME))){
			printf("%s: Invalid radio device name: %s\n", __func__, radioName);
			ret = -1;
			break;
		}
		wlan_index = radioName[4] - '0';//wlan0
		if(devOperationMode==DEVICE_OPERATION_MODE_ACCESSPOINT){
			if(0==wlan_op_mode[wlan_index]){
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB

	int mode = AP_MODE;
				if(wifi_set_mib(radioName, MIB_WLAN_MODE, &mode)<0)
#else
				if(wifi_set_parameter(radioName, "MODE", AP_MODE, NULL)<0)
#endif
#else
				if(iwpriv_set_mib(radioName, "opmode", 16, NULL)<0)
#endif
				{
					ret = -1;
					break;
				}
				wlan_op_mode[wlan_index] = AP_MODE+1;
//				printf("%s: %s%d has been created\n", __func__, PREFIX_WLAN_DEVICE_NAME, wlan_index);
			}
			if(AP_MODE+1!=wlan_op_mode[wlan_index]){
				printf("%s: The op mode of radio device %s%d is not ap mode\n", __func__, PREFIX_WLAN_DEVICE_NAME, wlan_index);
				ret = -1;
				break;
			}

			ret = -1;
			for(i=0; i<NUM_VWLAN; i++){
				if(0==(ap_creation_flag[wlan_index]&(1<<(i+1)))){
					sprintf(devName, "%s-va%d", radioName, i);
					ap_creation_flag[wlan_index] |= (1<<(i+1));
					do{
						skfd = socket(AF_INET, SOCK_DGRAM, 0);
						if (skfd < 0) {
							perror("socket");
							break;
						}
						snprintf(wrq.ifr_name, sizeof(wrq.ifr_name), "%s", devName);
						if (ioctl(skfd, SIOCGIWNAME, &wrq) < 0) {
							perror("ioctl-SIOCGIWNAME");
							break;
						}
						sprintf(buff, "ifconfig %s down", devName);
						shell(buff);
						// get mib from driver
						wrq.u.data.pointer = (caddr_t)&mib_vap[wlan_index][i];
						wrq.u.data.length = sizeof(struct wifi_mib);
						if (ioctl(skfd, SIOCMIBINIT, &wrq) < 0) {
							printf("Get WLAN MIB failed!\n");
							break;
						}
					}while(0);
					
					if(-1!=skfd){
						close(skfd);
					}


					wifi_close_device(devName);

					if(0==wlan_index){//wlan0 is 5G interface
						phyBandSelect = 2; // 1-2G, 2-5G
					}else{
						phyBandSelect = 1; // 1-2G, 2-5G
					}
					
					ret = iwpriv_set_mib(devName, "phyBandSelect", phyBandSelect, NULL);
					
					printf("[%s]: vap [%s] has been created\n", __func__, devName);
					break;
				}
			}
			if(ret<0){
				printf("%s: Can not create vap, %d VAPs are already exist\n", __func__, NUM_VWLAN);
				break;
			}
			
		}else if(devOperationMode==DEVICE_OPERATION_MODE_STATION){
		//wlan0
			if(AP_MODE+1==wlan_op_mode[wlan_index]){
				for(i=0; i<NUM_VWLAN; i++){
					sprintf(buff, "%s-va%d", radioName, i);
					ret += wifi_destroy_ssid_device(buff);
				}
			}
			if(0!=wlan_op_mode[wlan_index]){
				ret = -1;
				printf("%s: invalid op mode %d\n", __func__, wlan_op_mode[wlan_index]-1);
				break;
			}
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
			int mode = CLIENT_MODE;
			if(wifi_set_mib(radioName, MIB_WLAN_MODE, &mode)<0)
#else
			if(wifi_set_parameter(radioName, "MODE", CLIENT_MODE, NULL)<0)
#endif
#else
			if(iwpriv_set_mib(radioName, "opmode", 8, NULL)<0)
#endif
			{
				ret = -1;
				break;
			}
			strcpy(devName, radioName);
			wlan_op_mode[wlan_index] = CLIENT_MODE+1;
			printf("[%s]: client [%s] has been created\n", __func__, devName);
		}else if(devOperationMode==DEVICE_OPERATION_MODE_BRIDGE){
			printf("%s: devOperationMode %d is not supported\n", __func__, devOperationMode);
			ret = -1;
		}else if(devOperationMode==DEVICE_OPERATION_MODE_REPEATER){
			printf("%s: devOperationMode %d is not supported\n", __func__, devOperationMode);
			ret = -1;
		}else{
			printf("%s: devOperationMode %d is not supported\n", __func__, devOperationMode);
			ret = -1;
		}
	} while(0);
	
	return ret;
}

int wifi_close_device(char* devName)
{
#if defined(USE_REALTEK_SDK)
	int index = wifi_get_device_index(devName);
	if(index<0){
		return -1;
	}else{
        pthread_mutex_lock(&mutex);
		if(5==strlen(devName)){
			wlan_disabled |= (1 << (index<<3));
			wlan_disabled_mask|= (1 << (index<<3));
		}else{
			wlan_disabled |= (1 << (((devName[4]-'0')<<3)+index+1));
			wlan_disabled_mask|= (1 << (((devName[4]-'0')<<3)+index+1));
		}
        pthread_mutex_unlock(&mutex);
		return 0;
	}
#else
	char cmd[SIZE_128] = {0};

	snprintf(cmd, sizeof(cmd), "ifconfig %s down", devName);

	return shell(cmd);
#endif
}

int wifi_open_device(char* devName)
{
#if defined(USE_REALTEK_SDK)
	int index = wifi_get_device_index(devName);
	if(index<0){
		return -1;
	}else{
        pthread_mutex_lock(&mutex);
		if(5==strlen(devName)){
			wlan_disabled &= ~(1 << (index<<3));
			wlan_disabled_mask|= (1 << (index<<3));
		}else{
			wlan_disabled &= ~(1 << (((devName[4]-'0')<<3)+index+1));
			wlan_disabled_mask|= (1 << (((devName[4]-'0')<<3)+index+1));
		}
        pthread_mutex_unlock(&mutex);
		return 0;
	}
#else
	char cmd[SIZE_128] = {0};
	
	snprintf(cmd, sizeof(cmd), "ifconfig %s up", devName);
	
	return shell(cmd);
#endif
}

int wifi_close_radio(char* radioName)
{
	return wifi_close_device(radioName);
}

int wifi_open_radio(char* radioName)
{
	return wifi_open_device(radioName);
}

int wifi_set_radio_BasicDataTransmitRates(char* devName, char* basicDataTransmitRates)
{//Only for tr-098
	//basic rate, bit0~bit11 for rate 1,2,5.5,11,6,9,12,18,24,36,48,54M
	char *p = NULL;
	char *delimiter = ",";
	unsigned int basic_rate = 0;
	int rate;

	p = strtok(basicDataTransmitRates, delimiter);
	while(p){
		rate = atoi(p);
		switch(rate){
			case 1:
				basic_rate |= (1<<0);
				break;
			case 2:
				basic_rate |= (1<<1);
				break;
			case 5:
				basic_rate |= (1<<2);
				break;
			case 11:
				basic_rate |= (1<<3);
				break;
			case 6:
				basic_rate |= (1<<4);
				break;
			case 9:
				basic_rate |= (1<<5);
				break;
			case 12:
				basic_rate |= (1<<6);
				break;
			case 18:
				basic_rate |= (1<<7);
				break;
			case 24:
				basic_rate |= (1<<8);
				break;
			case 36:
				basic_rate |= (1<<9);
				break;
			case 48:
				basic_rate |= (1<<10);
				break;
			case 54:
				basic_rate |= (1<<11);
				break;
			default:
				break;
		}
		p = strtok(NULL, delimiter);
	}
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
	return wifi_set_mib(devName, MIB_WLAN_BASIC_RATES, &basic_rate);
#else
	return wifi_set_parameter(devName, "BASIC_RATES", basic_rate, NULL);
#endif
#else
	return iwpriv_set_mib(devName, "basicrates", basic_rate, NULL);
#endif
}

int wifi_set_radio_OperationalDataTransmitRates(char* devName, char* operationalDataTransmitRates)
{//Only for tr-098
	int ret = 0;
	unsigned int rate_adaptive_enabled = 0;
	unsigned int fix_rate = 0;
	
	do{
		if(!STRCMP(operationalDataTransmitRates, "1M")){
			fix_rate = (1 << 0);
		}else if(!STRCMP(operationalDataTransmitRates, "2M")){
			fix_rate = (1 << 1);
		}else if(!STRCMP(operationalDataTransmitRates, "5.5M")){
			fix_rate = (1 << 2);
		}else if(!STRCMP(operationalDataTransmitRates, "11M")){
			fix_rate = (1 << 3);
		}else if(!STRCMP(operationalDataTransmitRates, "6M")){
			fix_rate = (1 << 4);
		}else if(!STRCMP(operationalDataTransmitRates, "9M")){
			fix_rate = (1 << 5);
		}else if(!STRCMP(operationalDataTransmitRates, "12M")){
			fix_rate = (1 << 6);
		}else if(!STRCMP(operationalDataTransmitRates, "18M")){
			fix_rate = (1 << 7);
		}else if(!STRCMP(operationalDataTransmitRates, "24M")){
			fix_rate = (1 << 8);
		}else if(!STRCMP(operationalDataTransmitRates, "36M")){
			fix_rate = (1 << 9);
		}else if(!STRCMP(operationalDataTransmitRates, "48M")){
			fix_rate = (1 << 10);
		}else if(!STRCMP(operationalDataTransmitRates, "54M")){
			fix_rate = (1 << 11);
		}else if(!STRNCMP(operationalDataTransmitRates, "MCS", 3)){
			fix_rate = (1 << (12+atoi(operationalDataTransmitRates+3)));
		}else{
			rate_adaptive_enabled = 1;
		}
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
		ret += wifi_set_mib(devName, MIB_WLAN_RATE_ADAPTIVE_ENABLED, &rate_adaptive_enabled);
		ret += wifi_set_mib(devName, MIB_WLAN_FIX_RATE, &fix_rate);
#else
		ret += wifi_set_parameter(devName, "RATE_ADAPTIVE_ENABLED", rate_adaptive_enabled, NULL);
		ret += wifi_set_parameter(devName, "FIX_RATE", fix_rate, NULL);
#endif
#else
		ret += iwpriv_set_mib(devName, "autorate", rate_adaptive_enabled, NULL);
		ret += iwpriv_set_mib(devName, "fixrate", fix_rate, NULL);
		ret += iwpriv_set_mib(devName, "oprates", fix_rate, NULL);
#endif
	}while(0);

	return ret;
}

int wifi_set_AutoRateFallBackEnabled(char* devName, unsigned int autoRateFallBackEnabled)
{//Only for tr-098
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
	int enabled = 0!=autoRateFallBackEnabled;
	return wifi_set_mib(devName, MIB_WLAN_RATE_ADAPTIVE_ENABLED, &enabled);
#else
	return wifi_set_parameter(devName, "RATE_ADAPTIVE_ENABLED", 0!=autoRateFallBackEnabled, NULL);
#endif
#else
	return iwpriv_set_mib(devName, "autorate", 0!=autoRateFallBackEnabled, NULL);
#endif
}

int wifi_set_SSIDAdvertisementEnabled(char* devName, unsigned int SSIDAdvertisementEnabled)
{
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
	int hidden = !SSIDAdvertisementEnabled;
	return wifi_set_mib(devName, MIB_WLAN_HIDDEN_SSID, &hidden);
#else
	return wifi_set_parameter(devName, "HIDDEN_SSID", !SSIDAdvertisementEnabled, NULL);
#endif
#else
	return iwpriv_set_mib(devName, "hiddenAP", !SSIDAdvertisementEnabled, NULL);
#endif
}

int wifi_set_WMMEnable(char* devName, unsigned int WMMEnable)
{
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
	int enabled = 0!=WMMEnable;
	return wifi_set_mib(devName, MIB_WLAN_WMM_ENABLED, &enabled);
#else
	return wifi_set_parameter(devName, "WMM_ENABLED", 0!=WMMEnable, NULL);
#endif
#else
	return iwpriv_set_mib(devName, "qos_enable", 0!=WMMEnable, NULL);
#endif
}

int wifi_set_UAPSDEnable(char* devName, unsigned int UAPSDEnable)
{
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
    int enabled = 0!=UAPSDEnable;

    return wifi_set_mib(devName, MIB_WLAN_UAPSD_ENABLED, &enabled);
#else
    return wifi_set_parameter(devName, "UAPSD_ENABLED", 0!=UAPSDEnable, NULL);
#endif
#else
    return iwpriv_set_mib(devName, "apsd_enable", 0!=UAPSDEnable, NULL);
#endif
}

int wifi_get_wmm_parameters(char* devName, WMM_CLASS_NUMBER classNumber, unsigned int* aifs, unsigned int* cwmin, unsigned int* cwmax, unsigned int* txoplimit, unsigned int* noackpolicy, unsigned int isAP)
{//Only for tr-098
	int ret = 0;
	char name[SIZE_128] = {0};
	char* item[WMM_CLASS_NUMBER_COUNT] = {"be", "bk", "vi", "vo"};
	char value[SIZE_128] = {0};
	char *p = NULL;

	do{
		if(!aifs||!cwmin||!cwmax||!txoplimit||!noackpolicy){
			ret = -1;
			break;
		}
		
		if(classNumber>=WMM_CLASS_NUMBER_COUNT){
			ret = -1;
			break;
		}
		
		snprintf(name, sizeof(name), "%s_%sq_%s", isAP?"ap":"sta", item[classNumber], "aifs");
		if(iwpriv_get_mib(devName, name, value)){
			ret = -1;
			break;
		}
		p = strrchr(value, ' ');
		if(NULL==p){
			ret = -1;
			break;
		}
		*aifs = atoi(p+1);
		
		snprintf(name, sizeof(name), "%s_%sq_%s", isAP?"ap":"sta", item[classNumber], "cwmin");
		if(iwpriv_get_mib(devName, name, value)){
			ret = -1;
			break;
		}
		p = strrchr(value, ' ');
		if(NULL==p){
			ret = -1;
			break;
		}
		*cwmin = atoi(p+1);
		
		snprintf(name, sizeof(name), "%s_%sq_%s", isAP?"ap":"sta", item[classNumber], "cwmax");
		if(iwpriv_get_mib(devName, name, value)){
			ret = -1;
			break;
		}
		p = strrchr(value, ' ');
		if(NULL==p){
			ret = -1;
			break;
		}
		*cwmax = atoi(p+1);
		
		snprintf(name, sizeof(name), "%s_%sq_%s", isAP?"ap":"sta", item[classNumber], "txoplimit");
		if(iwpriv_get_mib(devName, name, value)){
			ret = -1;
			break;
		}
		p = strrchr(value, ' ');
		if(NULL==p){
			ret = -1;
			break;
		}
		*txoplimit = atoi(p+1);
		
		if(iwpriv_get_mib(devName, "txnoack", value)){
			ret = -1;
			break;
		}
		p = strrchr(value, ' ');
		if(NULL==p){
			ret = -1;
			break;
		}
		*noackpolicy = atoi(p+1);
	}while(0);
	
	return ret;
}

int wifi_set_wmm_parameters(char* devName, WMM_CLASS_NUMBER classNumber, 
	unsigned int aifs, unsigned int cwmin, unsigned int cwmax, 
	unsigned int txoplimit, unsigned int noackpolicy, unsigned int isAP)
{//Only for tr-098
	int ret = 0;
	char name[SIZE_128] = {0};
	char* item[WMM_CLASS_NUMBER_COUNT] = {"be", "bk", "vi", "vo"};

	do{
		if(classNumber>=WMM_CLASS_NUMBER_COUNT){
			ret = -1;
			break;
		}
		
		snprintf(name, sizeof(name), "%s_%sq_%s", isAP?"ap":"sta", item[classNumber], "aifs");
		ret += iwpriv_set_mib(devName, name, aifs, NULL);
		snprintf(name, sizeof(name), "%s_%sq_%s", isAP?"ap":"sta", item[classNumber], "cwmin");
		ret += iwpriv_set_mib(devName, name, cwmin, NULL);
		snprintf(name, sizeof(name), "%s_%sq_%s", isAP?"ap":"sta", item[classNumber], "cwmax");
		ret += iwpriv_set_mib(devName, name, cwmax, NULL);
		snprintf(name, sizeof(name), "%s_%sq_%s", isAP?"ap":"sta", item[classNumber], "txoplimit");
		ret += iwpriv_set_mib(devName, name, txoplimit, NULL);
		
		ret += iwpriv_set_mib(devName, "txnoack", noackpolicy, NULL);
	}while(0);
	
	return ret;
}

int wifi_set_BeaconInterval(char* devName, unsigned int beaconInterval)
{
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
	return wifi_set_mib(devName, MIB_WLAN_BEACON_INTERVAL, &beaconInterval);
#else
	return wifi_set_parameter(devName, "BEACON_INTERVAL", beaconInterval, NULL);
#endif
#else
	return iwpriv_set_mib(devName, "bcnint", beaconInterval, NULL);
#endif
}

int wifi_set_DTIMInterval(char* devName, unsigned int DTIMInterval)
{
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
	return wifi_set_mib(devName, MIB_WLAN_DTIM_PERIOD, &DTIMInterval);
#else
	return wifi_set_parameter(devName, "DTIM_PERIOD", DTIMInterval, NULL);
#endif
#else
	return iwpriv_set_mib(devName, "dtimperiod", DTIMInterval, NULL);
#endif
}

int wifi_set_CTSProtectMode(char* devName, unsigned int CTSProtectMode)
{
	char wlanName[SIZE_32] = {0};
	strncpy(wlanName, devName, 5);
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
	int disabled = !CTSProtectMode;
	return wifi_set_mib(wlanName, MIB_WLAN_PROTECTION_DISABLED, &disabled);
#else
	return wifi_set_parameter(wlanName, "PROTECTION_DISABLED", !CTSProtectMode, NULL);
#endif
#else
	return iwpriv_set_mib(wlanName, "disable_protection", !CTSProtectMode, NULL);
#endif
}

int wifi_set_FragmentationThreshold(char* devName, unsigned int fragmentationThreshold)
{
	char wlanName[SIZE_32] = {0};
	strncpy(wlanName, devName, 5);
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
	return wifi_set_mib(wlanName, MIB_WLAN_FRAG_THRESHOLD, &fragmentationThreshold);
#else
	return wifi_set_parameter(wlanName, "FRAG_THRESHOLD", fragmentationThreshold, NULL);
#endif
#else
	return iwpriv_set_mib(wlanName, "fragthres", fragmentationThreshold, NULL);
#endif
}

int wifi_set_RTSThreshold(char* devName, unsigned int RTSThreshold)
{
	char wlanName[SIZE_32] = {0};
	strncpy(wlanName, devName, 5);
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
	return wifi_set_mib(wlanName, MIB_WLAN_RTS_THRESHOLD, &RTSThreshold);
#else
	return wifi_set_parameter(wlanName, "RTS_THRESHOLD", RTSThreshold, NULL);
#endif
#else
	return iwpriv_set_mib(wlanName, "rtsthres", RTSThreshold, NULL);
#endif
}

int wifi_set_MSDULimit(char* devName, unsigned int MSDULimit)
{
	return iwpriv_set_mib(devName, "amsdu", 0!=MSDULimit, NULL);
}

int wifi_set_MPDULimit(char* devName, unsigned int MPDULimit)
{
	char wlanName[SIZE_32] = {0};
	strncpy(wlanName, devName, 5);
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
	int ampdu = 0!=MPDULimit;
	return wifi_set_mib(wlanName, MIB_WLAN_AGGREGATION, &ampdu);
#else
	return wifi_set_parameter(wlanName, "AGGREGATION", 0!=MPDULimit, NULL);
#endif
#else
	return iwpriv_set_mib(wlanName, "ampdu", 0!=MPDULimit, NULL);
#endif
}

int wifi_set_wds(char* devName, unsigned int wds)
{
	return iwpriv_set_mib(devName, "wds_enable", wds>0, NULL);
}

int wifi_set_ssid(char* devName, char* ssid)
{
	int ret = 0;
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
	ret += wifi_set_mib(devName, MIB_WLAN_WSC_SSID, ssid);
	ret += wifi_set_mib(devName, MIB_WLAN_SSID, ssid);
#else
	ret = wifi_set_parameter(devName, "SSID", 0, ssid);
#endif
#else
	ret = iwpriv_set_mib(devName, "ssid", 0, ssid);
#endif
	return ret;
}

int wifi_set_bssid(char* devName, char* bssid)
{//for client mode
	char cmd[SIZE_128] = {0};

	snprintf(cmd, sizeof(cmd), "iwconfig %s ap %s", devName, bssid);
	
	return shell(cmd);
}

int wifi_config_wps(char* devName, unsigned int eap_server, unsigned int wps_state, 
	char* device_name, unsigned int ap_setup_locked, unsigned int ap_pin, unsigned int enabled)
{
	int ret = 0;

#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
	int disable = 0==wps_state||0==eap_server;
	int cfg = 2==wps_state;
	ret += wifi_set_mib(devName, MIB_WLAN_WSC_DISABLE, &disable);
	ret += wifi_set_mib(devName, MIB_WLAN_WSC_CONFIGURED, &cfg);
#else
	//WSC_DISABLE
	ret += wifi_set_parameter(devName, "WSC_DISABLE", 0==wps_state||0==eap_server, NULL);
	//WSC_CONFIGURED
	ret += wifi_set_parameter(devName, "WSC_CONFIGURED", 2==wps_state, NULL);
#endif
	//DEVICE_NAME
	ret += wifi_set_global_parameter("DEVICE_NAME", 0, device_name);
	//4. ap_setup_locked=0 not supported in realtek
	//	fprintf(fp, "ap_setup_locked=%d\n", ap_setup_locked);
	//HW_WSC_PIN
	ret += wifi_set_global_parameter("HW_WSC_PIN", ap_pin, NULL);
#else
	ret = iwpriv_set_mib(devName, "wsc_enable", wps_state&&eap_server, NULL);
	//DEVICE_NAME
	//4. ap_setup_locked=0 not supported in realtek
	//	fprintf(fp, "ap_setup_locked=%d\n", ap_setup_locked);
	//HW_WSC_PIN
#endif

	if(enabled){
	}
	
	return ret;
}

int wifi_set_wps_ap_pin_static(char* devName, unsigned int devicePassword, unsigned int timeout)
{//timeout = 0 : forever
	char cmd[SIZE_128] = {0};

	snprintf(cmd, sizeof(cmd), "hostapd_cli -p %s%s  wps_ap_pin set %.08d %d", PREFIX_HOSTAPD, devName, devicePassword, timeout);
	
	return shell(cmd);
}

unsigned int wifi_set_wps_ap_pin_random(char* devName, unsigned int timeout)
{//timeout = 0 : forever
	const unsigned int min_password = 10000000;
	unsigned int device_password = 0;
	char cmd[SIZE_128] = {0};
	FILE *fp;
	char wps_random_pin[SIZE_16] = {0};

	do{
		snprintf(cmd, sizeof(cmd), "hostapd_cli -p %s%s  -i %s wps_ap_pin random %d", PREFIX_HOSTAPD, devName, devName, timeout);
		
		do{
			fp =  popen(cmd, "r");
			if(NULL==fp){
				device_password = 0;
				break;
			}

			if (NULL!=fgets(wps_random_pin, sizeof(wps_random_pin),fp)){
				device_password = atoi(wps_random_pin);
			}

			pclose(fp);
		}while(device_password<min_password);
	}while(0);
	
	return device_password;
}

int wifi_set_wps_push_button(char* devName)
{
	char cmd[SIZE_128] = {0};

	snprintf(cmd, sizeof(cmd), "hostapd_cli -p %s%s  wps_pbc", PREFIX_HOSTAPD, devName);
	
	return shell(cmd);
}

int wifi_set_wps_pin(char* devName, unsigned int devicePassword, unsigned int timeout)
{//timeout = 0 : forever
	char cmd[SIZE_128] = {0};

	snprintf(cmd, sizeof(cmd), "hostapd_cli -p %s%s  wps_pin any %.08d %d", PREFIX_HOSTAPD, devName, devicePassword, timeout);
	
	return shell(cmd);
}

int wifi_set_security_off(char* devName)
{
	int ret = 0;
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
	int value = 0;
	ret += wifi_set_mib(devName, MIB_WLAN_ENABLE_1X, &value);
	ret += wifi_set_mib(devName, MIB_WLAN_ENCRYPT, &value); // 0 - NONE
	value = 1;
    ret += wifi_set_mib(devName, MIB_WLAN_WSC_DISABLE, &value);   //0x00 - enable, 0x01 - disable
#else
	ret += wifi_set_parameter(devName, "ENABLE_1X", 0, NULL);
	ret += wifi_set_parameter(devName, "ENCRYPT", 0, NULL); // 0 - NONE
#endif
#else
	ret += iwpriv_set_mib(devName, "encmode", 0, NULL);
	ret += iwpriv_set_mib(devName, "802_1x", 0, NULL);
	ret += iwpriv_set_mib(devName, "psk_enable", 0, NULL);
    ret += iwpriv_set_mib(devName, "wsc_enable", 0, NULL);
#endif
	
	return ret;
}

int wifi_set_security_8021x(char* devName, char* server, unsigned int port, char* secret)
{
	int ret = 0;

#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
	int value = 1;
    ret += wifi_set_mib(devName, MIB_WLAN_WSC_DISABLE, &value);   //0x00 - enable, 0x01 - disable
	ret += wifi_set_mib(devName, MIB_WLAN_RS_IP, server);
	ret += wifi_set_mib(devName, MIB_WLAN_RS_PORT, &port);
	ret += wifi_set_mib(devName, MIB_WLAN_RS_PASSWORD, secret);
	ret += wifi_set_mib(devName, MIB_WLAN_ENABLE_1X, &value);
	value = 0;
	ret += wifi_set_mib(devName, MIB_WLAN_ENCRYPT, &value);
#else
	ret += wifi_set_parameter(devName, "RS_IP", 0, server);
	ret += wifi_set_parameter(devName, "RS_PORT", port, NULL);
	ret += wifi_set_parameter(devName, "RS_PASSWORD", 0, secret);
	ret += wifi_set_parameter(devName, "ENABLE_1X", 1, NULL);
	ret += wifi_set_parameter(devName, "ENCRYPT", 0, NULL);
#endif
#else
	//auth wlan1-va0 br0 auth /var/wpa-wlan1-va0.conf
	//encryption = 0
	ret += iwpriv_set_mib(devName, "encmode", 0, NULL);
	ret += iwpriv_set_mib(devName, "802_1x", 1, NULL);
	ret += iwpriv_set_mib(devName, "psk_enable", 0, NULL);
    ret += iwpriv_set_mib(devName, "wsc_enable", 0, NULL);
#endif
	
	return ret;
}

int wifi_set_security_wep_only(char* devName, unsigned int WEPKeyType[4], char* WEPKey[4], unsigned int WEPKeyIndex, char* customedWEPKey, AUTHENTICATION_MODE authenticationMode)
{
	int ret = 0;
#ifndef USE_REALTEK_MIB
	char buff[SIZE_128] = {0};
#endif
	char key[SIZE_128] = {0};
	int i;
#ifdef USE_REALTEK_SDK
    int wps_disabled = 1;
#endif
	unsigned int wep_key_type = 0;
	unsigned int wep = 0;

	do {
        /* wifi certified: in wep mode, wps should be disable; and fixed QA-Bug#30318*/
#ifdef USE_REALTEK_SDK
        ret += wifi_set_mib(devName, MIB_WLAN_WSC_DISABLE, &wps_disabled);   //0x00 - enable, 0x01 - disable
#else
		ret += iwpriv_set_mib(devName, "wsc_enable", 0, NULL);
#endif

		//WEP_DEFAULT_KEY
		if (WEPKeyIndex<1||WEPKeyIndex>4){
			WEPKeyIndex = 1;
		}
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
		int value = WEPKeyIndex-1;
		ret += wifi_set_mib(devName, MIB_WLAN_WEP_DEFAULT_KEY, &value);
#else
		ret += wifi_set_parameter(devName, "WEP_DEFAULT_KEY", WEPKeyIndex-1, NULL);
#endif
#else
		ret += iwpriv_set_mib(devName, "wepdkeyid", WEPKeyIndex-1, NULL);
#endif
		//WEP_KEY_TYPE
		if(WEPKeyType[WEPKeyIndex-1]){
			wep_key_type = WEPKEY_TYPE_HEX;
		}else{
			wep_key_type = WEPKEY_TYPE_ASCII;
		}
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
	ret += wifi_set_mib(devName, MIB_WLAN_WEP_KEY_TYPE, &wep_key_type);
#else
	ret += wifi_set_parameter(devName, "WEP_KEY_TYPE", wep_key_type, NULL);
#endif
#else
#endif
		//WEP64_KEY1 / WEP128_KEY4
		if(customedWEPKey){
			STRNCPY(key, customedWEPKey, 26);
		}else{
			STRNCPY(key, WEPKey[WEPKeyIndex-1], 26);
		}
		if(WEPKEY_TYPE_ASCII==wep_key_type){
			AsciiCode2HexString(key, strlen(key), key);
		}else{
		}
		//WEP
		wep = ((strlen(key)<<2)>SIZE_64)?2:1;
		printf("wep key is [%s], wep=%d\n", key, wep);
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
		ret += wifi_set_mib(devName, MIB_WLAN_WEP, &wep);
#else
		ret += wifi_set_parameter(devName, "WEP", wep, NULL);
#endif
#else
#endif
		//KEY
		for(i=0; i<4; i++){
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
			HexString2AsciiCode(key, strlen(key), key);
			ret += wifi_set_mib(devName, (wep==1)?(MIB_WLAN_WEP64_KEY1+i):(MIB_WLAN_WEP128_KEY1+i), key);
#else
			sprintf(buff, "WEP%d_KEY%d", (wep==1)?SIZE_64:SIZE_128, i+1);
			ret += wifi_set_parameter(devName, buff, 0, key);
#endif
#else
			sprintf(buff, "wepkey%d", i+1);
			ret += iwpriv_set_mib(devName, buff, 0, key);
#endif
		}
		//AUTH_TYPE
		if(AUTHENTICATION_MODE_OPEN==authenticationMode){
			authenticationMode = 0;
		}else if(AUTHENTICATION_MODE_SHARED==authenticationMode){
			authenticationMode = 1;
		}else{//AUTHENTICATION_MODE_AUTO
			authenticationMode = 2;
		}
		
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
		ret += wifi_set_mib(devName, MIB_WLAN_AUTH_TYPE, &authenticationMode);
		value = 0;
		ret += wifi_set_mib(devName, MIB_WLAN_ENABLE_1X, &value);
		value = 1;
		ret += wifi_set_mib(devName, MIB_WLAN_ENCRYPT, &value); // 1 - WEP
#else
		ret += wifi_set_parameter(devName, "AUTH_TYPE", authenticationMode, NULL);
		//WEPONLY
		ret += wifi_set_parameter(devName, "ENABLE_1X", 0, NULL);
		ret += wifi_set_parameter(devName, "ENCRYPT", 1, NULL); // 1 - WEP
#endif
#else
		ret += iwpriv_set_mib(devName, "authtype", authenticationMode, NULL);
		ret += iwpriv_set_mib(devName, "802_1x", 0, NULL);
		ret += iwpriv_set_mib(devName, "encmode", (wep==1)?1:5, NULL);
		ret += iwpriv_set_mib(devName, "psk_enable", 0, NULL);
#endif
	} while(0);
	
	return ret;
}

int wifi_set_security_wep_8021x(char* devName, char* server, unsigned int port, char* secret)
{
	int ret = 0;

#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
	struct in_addr ip;
	int value = 1;

	if (!inet_aton(server, &ip)){
		perror("inet_aton");
		ret += -1;
	}else{
		ret += wifi_set_mib(devName, MIB_WLAN_RS_IP, &ip);
	}
	ret += wifi_set_mib(devName, MIB_WLAN_RS_PORT, &port);
	ret += wifi_set_mib(devName, MIB_WLAN_RS_PASSWORD, secret);
	ret += wifi_set_mib(devName, MIB_WLAN_ENABLE_1X, &value);
	ret += wifi_set_mib(devName, MIB_WLAN_ENCRYPT, &value);
    ret += wifi_set_mib(devName, MIB_WLAN_WSC_DISABLE, &value);   //0x00 - enable, 0x01 - disable
#else
	ret += wifi_set_parameter(devName, "RS_IP", 0, server);
	ret += wifi_set_parameter(devName, "RS_PORT", port, NULL);
	ret += wifi_set_parameter(devName, "RS_PASSWORD", 0, secret);
	ret += wifi_set_parameter(devName, "ENABLE_1X", 1, NULL);
	ret += wifi_set_parameter(devName, "ENCRYPT", 1, NULL);
#endif
#else
	//auth wlan1-va0 br0 auth /var/wpa-wlan1-va0.conf
	//encryption = 1
	//wepKey = 1 for WEP64, 2 for WEP128
	//wepGroupKey = WEPKey
	ret += iwpriv_set_mib(devName, "802_1x", 1, NULL);
	ret += iwpriv_set_mib(devName, "encmode", 1, NULL);
	ret += iwpriv_set_mib(devName, "psk_enable", 0, NULL);
    ret += iwpriv_set_mib(devName, "wsc_enable", 0, NULL);
#endif
	
	return ret;
}

int wifi_set_security_wpa_psk(char* devName, WPA_MODE wpa, WPA_ENCRYPTION_MODES wpaEncryptionModes, WPA_ENCRYPTION_MODES wpa2EncryptionModes, char* preSharedKey, unsigned int RekeyingInterval)
{
	int ret = 0;

#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
	int value = 2;
	ret += wifi_set_mib(devName, MIB_WLAN_WPA_AUTH, &value);	// 2 - psk
	value = 0;
	ret += wifi_set_mib(devName, MIB_WLAN_ENABLE_1X, &value);	// 0 - psk
	value = (wpa<<1);
	ret += wifi_set_mib(devName, MIB_WLAN_ENCRYPT, &value);// 2 - WPA; 4 - WPA2; 6 - Mixed
	value = ((wpa&0x1)<<1) | ((wpa&0x2) <<4);
	ret += wifi_set_mib(devName, MIB_WLAN_WSC_AUTH, &value);//0x01 - Open, 0x02 - WPA; 0x20 - WPA2; 0x22 - Mixed
	ret += wifi_set_mib(devName, MIB_WLAN_WPA_CIPHER_SUITE, &wpaEncryptionModes);// 1 - TKIP; 2 - AES; 3 - Mixed
	ret += wifi_set_mib(devName, MIB_WLAN_WPA2_CIPHER_SUITE, &wpa2EncryptionModes);// 1 - TKIP; 2 - AES; 3 - Mixed
	value = (wpaEncryptionModes | wpa2EncryptionModes) << 2;
	ret += wifi_set_mib(devName, MIB_WLAN_WSC_ENC, &value);// 1 - Open, 2 - WEP; 4 - TKIP; 8 - AES
	ret += wifi_set_mib(devName, MIB_WLAN_WSC_PSK, preSharedKey);
	ret += wifi_set_mib(devName, MIB_WLAN_WPA_PSK, preSharedKey);
	ret += wifi_set_mib(devName, MIB_WLAN_WPA_GROUP_REKEY_TIME, &RekeyingInterval);
	value = (SIZE_64==strlen(preSharedKey));
	ret += wifi_set_mib(devName, MIB_WLAN_PSK_FORMAT, &value);// 0 - ASCII; 1 - HEX
#else
	ret += wifi_set_parameter(devName, "WPA_AUTH", 2, NULL);	// 2 - psk
	ret += wifi_set_parameter(devName, "ENABLE_1X", 0, NULL);	// 0 - psk
	ret += wifi_set_parameter(devName, "ENCRYPT", (wpa<<1), NULL);// 2 - WPA; 4 - WPA2; 6 - Mixed
	ret += wifi_set_parameter(devName, "WPA_CIPHER_SUITE", wpaEncryptionModes, NULL);// 1 - TKIP; 2 - AES; 3 - Mixed
	ret += wifi_set_parameter(devName, "WPA2_CIPHER_SUITE", wpa2EncryptionModes, NULL);// 1 - TKIP; 2 - AES; 3 - Mixed
	ret += wifi_set_parameter(devName, "WPA_PSK", 0, preSharedKey);
	ret += wifi_set_parameter(devName, "WPA_GROUP_REKEY_TIME", RekeyingInterval, NULL);
	ret += wifi_set_parameter(devName, "PSK_FORMAT", (SIZE_64==strlen(preSharedKey)), NULL);// 0 - ASCII; 1 - HEX
#endif
#else
	ret += iwpriv_set_mib(devName, "gk_rekey", RekeyingInterval, NULL);
	ret += iwpriv_set_mib(devName, "passphrase", 0, preSharedKey);
	ret += iwpriv_set_mib(devName, "wpa2_cipher", (((wpa2EncryptionModes&0x2)<<2)|((wpa2EncryptionModes&0x1)<<1)), NULL);
	ret += iwpriv_set_mib(devName, "wpa_cipher", (((wpaEncryptionModes&0x2)<<2)|((wpaEncryptionModes&0x1)<<1)), NULL);
	ret += iwpriv_set_mib(devName, "802_1x", 0, NULL);
	ret += iwpriv_set_mib(devName, "psk_enable", wpa, NULL);
	ret += iwpriv_set_mib(devName, "encmode", 2, NULL);
#endif
	
	return ret;
}

int wifi_set_security_wpa_eap(char* devName, WPA_MODE wpa, WPA_ENCRYPTION_MODES wpaEncryptionModes, WPA_ENCRYPTION_MODES wpa2EncryptionModes, char* server, unsigned int port, char* secret, unsigned int RekeyingInterval)
{
	int ret = 0;

#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
	struct in_addr ip;

	int value = 1;
    ret += wifi_set_mib(devName, MIB_WLAN_WSC_DISABLE, &value);   //0x00 - enable, 0x01 - disable
	ret += wifi_set_mib(devName, MIB_WLAN_WPA_AUTH, &value);	// 1 - eap
	ret += wifi_set_mib(devName, MIB_WLAN_ENABLE_1X, &value);	// 0 - 802.1x
	value = (wpa<<1);
	ret += wifi_set_mib(devName, MIB_WLAN_ENCRYPT, &value);// 2 - WPA; 4 - WPA2; 6 - Mixed
	ret += wifi_set_mib(devName, MIB_WLAN_WPA_CIPHER_SUITE, &wpaEncryptionModes);// 1 - TKIP; 2 - AES; 3 - Mixed
	ret += wifi_set_mib(devName, MIB_WLAN_WPA2_CIPHER_SUITE, &wpa2EncryptionModes);// 1 - TKIP; 2 - AES; 3 - Mixed
	ret += wifi_set_mib(devName, MIB_WLAN_WPA_GROUP_REKEY_TIME, &RekeyingInterval);
	if (!inet_aton(server, &ip)){
		perror("inet_aton");
		ret += -1;
	}else{
		ret += wifi_set_mib(devName, MIB_WLAN_RS_IP, &ip);
	}
	ret += wifi_set_mib(devName, MIB_WLAN_RS_PORT, &port);
	ret += wifi_set_mib(devName, MIB_WLAN_RS_PASSWORD, secret);
#else
	ret += wifi_set_parameter(devName, "WPA_AUTH", 1, NULL);	// 1 - eap
	ret += wifi_set_parameter(devName, "ENABLE_1X", 1, NULL);	// 0 - 802.1x
	ret += wifi_set_parameter(devName, "ENCRYPT", (wpa<<1), NULL);// 2 - WPA; 4 - WPA2; 6 - Mixed
	ret += wifi_set_parameter(devName, "WPA_CIPHER_SUITE", wpaEncryptionModes, NULL);// 1 - TKIP; 2 - AES; 3 - Mixed
	ret += wifi_set_parameter(devName, "WPA2_CIPHER_SUITE", wpa2EncryptionModes, NULL);// 1 - TKIP; 2 - AES; 3 - Mixed
	ret += wifi_set_parameter(devName, "WPA_GROUP_REKEY_TIME", RekeyingInterval, NULL);
	ret += wifi_set_parameter(devName, "RS_IP", 0, server);
	ret += wifi_set_parameter(devName, "RS_PORT", port, NULL);
	ret += wifi_set_parameter(devName, "RS_PASSWORD", 0, secret);
#endif
#else
	//auth wlan1-va0 br0 auth /var/wpa-wlan1-va0.conf
	//encryption = 6// 2 - WPA; 4 - WPA2; 6 - Mixed
	//enable1x = 1
	//authentication = 1
	//unicastCipher = 2
	//wpa2UnicastCipher = 2
	//psk = "1234567890123456789012345678901234567890123456789012345678901234"
	//groupRekeyTime = 0
	//rsPort = 1812
	//rsIP = 192.168.1.100
	//rsPassword = "12345678"
	ret += iwpriv_set_mib(devName, "gk_rekey", RekeyingInterval, NULL);
	ret += iwpriv_set_mib(devName, "wpa2_cipher", (((wpa2EncryptionModes&0x2)<<2)|((wpa2EncryptionModes&0x1)<<1)), NULL);
	ret += iwpriv_set_mib(devName, "wpa_cipher", (((wpaEncryptionModes&0x2)<<2)|((wpaEncryptionModes&0x1)<<1)), NULL);
	ret += iwpriv_set_mib(devName, "802_1x", 1, NULL);
	ret += iwpriv_set_mib(devName, "psk_enable", 0, NULL);
	ret += iwpriv_set_mib(devName, "encmode", 2, NULL);
    ret += iwpriv_set_mib(devName, "wsc_enable", 0, NULL);
#endif
	
	return ret;
}

int wifi_set_acl(char* devName, unsigned int enabled, char* macList, char* comment, unsigned int policyDeny)
{
	int ret = 0;
	unsigned int count = 0;
	char* p = NULL;
	char* q = NULL;
	char buff[SIZE_256] = {0};
	char *saveptr1, *saveptr2;
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
	int value;
	MACFILTER_T macEntry;
#else
#endif
#endif

	if (enabled){
		//Flush the current ACL list
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
		ret += wifi_set_mib(devName, MIB_WLAN_AC_ADDR_DELALL, &macEntry);
#else
		ret += wifi_set_parameter(devName, "MACAC_ADDR", 0, "delall");
#endif
#else
		ret += iwpriv_set_mib(devName, "aclnum", 0, NULL);
#endif
		//add specific MAC addresses to the Access Control List
		p = strtok_r(macList, WLAN_ACL_MAC_LIST_DELIMITER, &saveptr1);
		if(p){
			if(comment){
				q = strtok_r(comment, WLAN_ACL_MAC_LIST_COMMENT_DELIMITER, &saveptr2);
			}
			
			do{
				if(17==strlen(p)){
					memmove(p+2, p+3, strlen(p+3)+1);
					memmove(p+4, p+5, strlen(p+5)+1);
					memmove(p+6, p+7, strlen(p+7)+1);
					memmove(p+8, p+9, strlen(p+9)+1);
					memmove(p+10, p+11, strlen(p+11)+1);
				}
				if(12==strlen(p)){
					snprintf(buff, sizeof(buff), "%s %s", p, q?:"");
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
					HexString2AsciiCode(p, 12, macEntry.macAddr);
					snprintf(macEntry.comment, sizeof(macEntry.comment), "%s", q?:"");
					ret += wifi_set_mib(devName, MIB_WLAN_AC_ADDR_ADD, &macEntry);
#else
					ret += wifi_set_parameter(devName, "MACAC_ADDR add", 0, buff);
#endif
#else
					ret += iwpriv_set_mib(devName, "acladdr", 0, buff);
#endif
					count++;
				}
				if(q){
					q=strtok_r(NULL, WLAN_ACL_MAC_LIST_COMMENT_DELIMITER, &saveptr2);
				}
			}while(NULL!=(p=strtok_r(NULL, WLAN_ACL_MAC_LIST_DELIMITER, &saveptr1)));
		}
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
		ret += wifi_set_mib(devName, MIB_WLAN_MACAC_NUM, &count);
#else
		ret += wifi_set_parameter(devName, "MACAC_NUM", count, NULL);
#endif
#else
//		ret += iwpriv_set_mib(devName, "aclnum", count, NULL);
//When acl is added, the aclnum will be increased automatically.
#endif
		
		// set acl policy
		// 1 - Only ALLOW association with MAC addresses on the list
		// 2 - DENY association with any MAC address on the list
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
		value = policyDeny?2:1;
		ret += wifi_set_mib(devName, MIB_WLAN_MACAC_ENABLED, &value);
#else
		ret += wifi_set_parameter(devName, "MACAC_ENABLED", policyDeny?2:1, NULL);
#endif
#else
		ret += iwpriv_set_mib(devName, "aclmode", policyDeny?2:1, NULL);
#endif
	}
	else
	{//Disable ACL checking 
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
		value = 0;
		ret += wifi_set_mib(devName, MIB_WLAN_MACAC_ENABLED, &value);
#else
		ret += wifi_set_parameter(devName, "MACAC_ENABLED", 0, NULL);
#endif
#else
		ret += iwpriv_set_mib(devName, "aclmode", 0, NULL);
#endif
	}
	
	return ret;
}

DEVICE_STATUS wifi_get_device_status(char *devName)
{
	DEVICE_STATUS  status = DEVICE_STATUS_Error;
	int sock;
	struct ifreq ifr;

	do{
		if (NULL==devName){
			printf("%s: device name can not be null\n", __func__);
			break;
		}
		sock = socket(AF_INET, SOCK_DGRAM, 0);
		if(sock<0){
			perror("socket");
			break;
		}
//{fix coverity CID 29788: Copy into fixed size buffer 		
		snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", devName);
//}fix coverity CID 29788: Copy into fixed size buffer		
		if (ioctl(sock, SIOCGIFFLAGS, &ifr) != -1){
			if (( ifr.ifr_flags & IFF_UP) == IFF_UP ) {
				status = DEVICE_STATUS_Up;
			}
			else if ((ifr.ifr_flags & IFF_RUNNING) == IFF_RUNNING){
				status = DEVICE_STATUS_Dormant;
			}else  {
				status = DEVICE_STATUS_Down;
			}
		}else{
			status = DEVICE_STATUS_NotPresent;
		}
		close(sock);
	}while(0);

	return status;
}

int wifi_get_device_index(char* devName)
{
	int index = -1;

	do {
		if (NULL==devName||devName!=strstr(devName, PREFIX_WLAN_DEVICE_NAME)){
			printf("%s: Invalid device name %s\n", __func__, devName);
			break;
		}

		if(5==strlen(devName)){
			index = devName[4] - '0';//wlan0
			if(index<0||index>=NUM_WLAN_INTERFACE){
				printf("%s: Invalid device name %s\n", __func__, devName);
				index = -1;
			}
			break;
		}

		if(9==strlen(devName)){
			index = devName[8] - '0';//wlan0-va3
			if(index<0||index>=NUM_VWLAN){
				printf("%s: Invalid device name %s\n", __func__, devName);
				index = -1;
			}
			break;
		}

		printf("%s: Invalid device name %s\n", __func__, devName);
	} while(0);
	
	return index;
}

unsigned int wifi_close_all_virtual_devices(char* wlanDevName)
{//Only for tr-098
	unsigned int flag = 0;
	char cmd[SIZE_32] = {0};
	char devNameList[SIZE_256] = {0};
	char *p = NULL;
	int index = -1;
	int  status = DEVICE_STATUS_Error;

	do {
		if(0==wifi_get_all_virtual_devices(wlanDevName, devNameList)){
			break;
		}
		
		p = strtok(devNameList, WLAN_DEVICE_NAME_DELIMITER);
		
		do{			
			index = wifi_get_device_index(p);
			status = wifi_get_device_status(p);
			if(index>=0&&DEVICE_STATUS_Up==status){
				flag |= (0x01<<index);
			}
			snprintf(cmd, sizeof(cmd), "ifconfig %s down", p );
			if(0!=shell(cmd)){
				break;
			}
		}while(NULL!=(p=strtok(NULL, WLAN_DEVICE_NAME_DELIMITER)));
		
	} while(0);
	
	return flag;
}

void wifi_open_all_virtual_devices(char* wlanDevName, unsigned int flag)
{//Only for tr-098
	int index = 0;
	char cmd[SIZE_32] = {0};

	printf("###############%s: flag=0x%.08X\n", __func__, flag);
	while(flag){
		if(flag&IFF_UP){
			snprintf(cmd, sizeof(cmd), "ifconfig %s-%s%d up", wlanDevName, PREFIX_VAP_DEVICE_NAME, index );
			if(0!=shell(cmd)){
				break;
			}
		}
		flag >>= 1;
		index++;
	}
}

int wifi_get_BSSID(char* devName, char* bssid)
{
	return wifi_get_MACAddress(devName, bssid);
}

unsigned int wifi_get_radio_channel(char* devName)
{
	unsigned int channel = 0;
	char value[SIZE_128] = {0};

	do{
		if(iwpriv_get_mib(devName, "channel", value)){
			break;
		}
		channel = atoi(value);
	}while(0);

	return channel;
}

int wifi_get_WEPEncryptionLevel(char* devName, char* supportedKeySizes)
{//Only for tr-098
	int ret = 0;
	char value[SIZE_128]={0};

	do
	{
		if(NULL==supportedKeySizes){
			ret = -1;
			break;
		}
#ifdef USE_REALTEK_SDK
		if(wifi_get_parameter(devName, "WEP", value)<0){
			ret = -1;
			break;
		}
#else
#endif
		switch(atoi(value)){
			case 2:
				strcpy(supportedKeySizes, "104-bit");
				break;
			case 1:
				strcpy(supportedKeySizes, "40-bit");
				break;
			default:
				strcpy(supportedKeySizes, "Disabled");
				break;
		}
	}while(0);

	return ret;
}

int wifi_get_radio_PossibleChannels(char* devName, char* availableChannelList, unsigned int bufLen)
{
/*
For example, for 802.11b and North America, would be "1-11".
*/
	if(NULL==devName||NULL==availableChannelList){
		return -1;
	}
	
	int index = devName[4] - '0';

	if(0==index){//wlan0 is 5G interface
		snprintf(availableChannelList, bufLen, "%s", "36,40,44,48,149,153,157,161,165");
	}else{//think that other is 2G interface
		snprintf(availableChannelList, bufLen, "%s", "1-11");
	}

	return 0;
}

int wifi_get_radio_PossibleDataTransmitRates(char* devName, char* bitrateList)
{//Only for tr-098
	int ret = -1;
	int supported_rates = 0;
	int bit = 0;
	
	do {
		if(NULL==bitrateList){
			printf("%s: parameter bitrateList can not be null\n", __func__);
			break;
		}
#ifdef USE_REALTEK_SDK
		ret = wifi_get_parameter(devName, "SUPPORTED_RATES", bitrateList);
		if(ret<0){
			break;
		}

		supported_rates = atoi(bitrateList);
#else
		ret = iwpriv_get_mib(devName, "oprates", bitrateList);
		if(ret<0){
			break;
		}
		{//0  0  15  240
			char *p = NULL;
			char *delimiter = " ";
			supported_rates = 0;
			p = strtok(bitrateList, delimiter);
			while(p){
				supported_rates <<= 8;
				supported_rates += atoi(p);
				p = strtok(NULL, delimiter);
			}
		}
#endif
		bitrateList[0] = 0;
		while(supported_rates){
			if(0x1&supported_rates){
				if(bitrateList[0]){
					strcat(bitrateList, ",");
				}
				switch(bit){
					case 0:
						strcat(bitrateList, "1");
						break;
					case 1:
						strcat(bitrateList, "2");
						break;
					case 2:
						strcat(bitrateList, "5.5");
						break;
					case 3:
						strcat(bitrateList, "11");
						break;
					case 4:
						strcat(bitrateList, "6");
						break;
					case 5:
						strcat(bitrateList, "9");
						break;
					case 6:
						strcat(bitrateList, "12");
						break;
					case 7:
						strcat(bitrateList, "18");
						break;
					case 8:
						strcat(bitrateList, "24");
						break;
					case 9:
						strcat(bitrateList, "36");
						break;
					case 10:
						strcat(bitrateList, "48");
						break;
					case 11:
						strcat(bitrateList, "54");
						break;
					default:
						break;
				}
			}
			supported_rates >>= 1;
			bit++;
		}
		
	} while(0);
	
	return ret;
}

int wifi_get_radio_TransmitPowerSupported(char* devName, char* transmitPowerSupported, unsigned int bufLen)
{
	if(transmitPowerSupported){
		snprintf(transmitPowerSupported, bufLen, "%s", "15,35,50,70,100");
		return 0;
	}

	return -1;
}

int wifi_get_radio_ChannelsInUse(char* devName, char* channelsInUse)
{
	char wlanName[SIZE_32] = {0};
	strncpy(wlanName, devName, 5);
#ifdef USE_REALTEK_SDK
	return wifi_get_parameter(wlanName, "CHANNEL", channelsInUse);
#else
	return iwpriv_get_mib(wlanName, "channel", channelsInUse);
#endif
}

unsigned int wifi_get_WMMSupported(char* devName)
{
	return 1;
}

unsigned int wifi_get_UAPSDSupported(char* devName)
{
	return 1;
}

unsigned int wifi_get_BytesSent(char* devName)
{
	unsigned int data = 0;
	
	wifi_get_statistics(devName, NULL, &data, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		
	return data;
}

unsigned int wifi_get_BytesReceived(char* devName)
{
	unsigned int data = 0;
	
	wifi_get_statistics(devName, &data, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		
	return data;
}

unsigned int wifi_get_PacketsSent(char* devName)
{
	unsigned int data = 0;
	
	wifi_get_statistics(devName, NULL, NULL, NULL, &data, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		
	return data;
}

unsigned int wifi_get_PacketsReceived(char* devName)
{
	unsigned int data = 0;
	
	wifi_get_statistics(devName, NULL, NULL, &data, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		
	return data;
}

unsigned int wifi_get_ErrorsSent(char* devName)
{
	unsigned int data = 0;
	
	wifi_get_statistics(devName, NULL, NULL, NULL, NULL, NULL, &data, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		
	return data;
}

unsigned int wifi_get_ErrorsReceived(char* devName)
{
	unsigned int data = 0;
	
	wifi_get_statistics(devName, NULL, NULL, NULL, NULL, &data, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		
	return data;
}

unsigned int wifi_get_UnicastPacketsSent(char* devName)
{
	unsigned int data = 0;
	
	wifi_get_statistics(devName, NULL, NULL, NULL, &data, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		
	return data;
}

unsigned int wifi_get_UnicastPacketsReceived(char* devName)
{
	unsigned int data = 0;
	
	wifi_get_statistics(devName, NULL, NULL, &data, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		
	return data;
}

unsigned int wifi_get_DiscardPacketsSent(char* devName)
{
	unsigned int data = 0;
	
	wifi_get_statistics(devName, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &data,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		
	return data;
}

unsigned int wifi_get_DiscardPacketsReceived(char* devName)
{
	unsigned int data = 0;
	
	wifi_get_statistics(devName, NULL, NULL, NULL, NULL, NULL, NULL, &data, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		
	return data;
}

unsigned int wifi_get_MulticastPacketsSent(char* devName)
{
	unsigned int data = 0;
	
	wifi_get_statistics(devName, NULL, NULL, NULL, &data, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		
	return data;
}

unsigned int wifi_get_MulticastPacketsReceived(char* devName)
{
	unsigned int data = 0;
	
	wifi_get_statistics(devName, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, &data, NULL, NULL);
		
	return data;
}

unsigned int wifi_get_BroadcastPacketsSent(char* devName)
{
	unsigned int data = 0;
	
	wifi_get_statistics(devName, NULL, NULL, NULL, &data, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		
	return data;
}

unsigned int wifi_get_BroadcastPacketsReceived(char* devName)
{
	unsigned int data = 0;
	
	wifi_get_statistics(devName, NULL, NULL, &data, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		
	return data;
}

unsigned int wifi_get_UnknownProtoPacketsReceived(char* devName)
{
	unsigned int data = 0;
	
	wifi_get_statistics(devName, NULL, NULL, NULL, NULL, NULL, NULL, &data, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		
	return data;
}

unsigned int wifi_get_wps_ConfigMethodsSupported(char* devName)
{
	return (WPS_CONFIG_METHODS_FLAG_LABEL|WPS_CONFIG_METHODS_FLAG_DISPLAY
		|WPS_CONFIG_METHODS_FLAG_PUSHBUTTON|WPS_CONFIG_METHODS_FLAG_PIN);
}

unsigned int wifi_get_wps_version(char* devName)
{
	return (0x20);
}


int wifi_get_wps_uuid(char* devName, char* uuid, unsigned int bufLen)
{
	int ret = 0;
	char MAC[SIZE_20] = {0};
	char buff[SIZE_64] = {0};
	int i;

	do{
		if(NULL==devName||NULL==uuid||0==bufLen){
			ret = -1;
			break;
		}
		
		if(wifi_get_MACAddress(devName, MAC)<0){
			ret = -1;
			break;
		}
		
		for(i=0; i<6; i++)	HexString2AsciiCode(MAC+i*3, 2, (unsigned char*)MAC+i);
		generate_uuid_by_mac_addr(MAC, buff);
		AsciiCode2HexString((unsigned char*)buff, SIZE_16, buff);
		buff[36] = 0;
		memmove(buff+24, buff+20, 12);
		buff[23] = '-';
		memmove(buff+19, buff+16, 4);
		buff[18] = '-';
		memmove(buff+14, buff+12, 4);
		buff[13] = '-';
		memmove(buff+9, buff+8, 4);
		buff[8] = '-';
//{fix coverity CID 29758: Dereference before null check		
		snprintf(uuid, bufLen, "%s", buff);
//}fix coverity CID 29758: Dereference before null check		
	}while(0);

	return ret;
}

unsigned int wifi_get_wps_ap_pin(char* devName)
{
	unsigned int password = 0;
	char cmd[SIZE_256] = {0};
	char wps_ap_pin[SIZE_16] = {0};
	FILE *fp;
	
	do
	{
		memset(wps_ap_pin, 0, sizeof(wps_ap_pin));
		sprintf(cmd, "hostapd_cli -p %s%s -i %s wps_ap_pin get", PREFIX_HOSTAPD, devName, devName);

		fp =  popen(cmd, "r");
		if(NULL==fp){
			break;
		}

		if (NULL!=fgets(wps_ap_pin, sizeof(wps_ap_pin),fp)){
			password = atoi(wps_ap_pin);
		}

		pclose(fp);
	
	}while(0);
	
	return password;
}

unsigned int wifi_get_associated_device_authentication_state(char* devName, char* macAddr)
{
	int i;
	char mac[SIZE_20] = {0};
	WLAN_STA_INFO_T devInfo[MAX_COUNT_ASSOCIATED_DEVICES];
	unsigned int count = wifi_get_associated_device_info(devName, devInfo, MAX_COUNT_ASSOCIATED_DEVICES);
	
	for(i=0; i<count; i++){
		sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", 
			devInfo[i].addr[0],devInfo[i].addr[1],
			devInfo[i].addr[2],devInfo[i].addr[3],
			devInfo[i].addr[4],devInfo[i].addr[5]);
		if(!STRNCMP(mac,macAddr,strlen(mac))){
			//fprintf(stderr, "devInfo[%d].flag=%d\n", i, devInfo[i].flag);
			return !!(devInfo[i].flag&0x04);// if (flags & 0x04) then authenticated is true. Otherwise is false
		}
	}
	
	return 0;
}

int wifi_get_associated_device_last_data_transmit_rate(char* devName, char* macAddr, char* lastDataTransmitRate)
{//Only for tr-098
	int ret = -1;
	
	unsigned int rate = wifi_get_associated_device_LastDataUplinkRate(devName, macAddr);

	do{
		if(0==rate){
			break;
		}
		if(rate%1000){
			sprintf(lastDataTransmitRate, "%d%s", rate/1000, ".5"); 
		}else{
			sprintf(lastDataTransmitRate, "%d", rate/1000); 
		}
		ret = 0;
	}while(0);

	return ret;
}

int wifi_get_associated_device_ip_address(char* devName, char* macAddr, char* ipAddr)
{//Only for tr-098
	int ret = -1;
//{fix coverity CID 29577: Improper use of negative value	
	struct rtnl_handle rth;
	char buff[SIZE_20] = {0};
	
	do{
		ipAddr[0] = 0;
		
		if (rtnl_open(&rth, 0) < 0){
			perror("open netlink" );
			break;
		}
		if (rtnl_wilddump_request(&rth, AF_INET, RTM_GETNEIGH) < 0){
			perror("rtnl_wilddump_request");
			break;
		}

		STRNCPY(buff, macAddr, sizeof(buff)-1);
		rtnl_dump_filter(&rth, get_neigh, buff, NULL, NULL);
		if(strlen(buff)<7||strlen(buff)>15){
			printf("%s: %s is invalid ip address\n", __func__, buff);
			break;
		}

		STRNCPY(ipAddr, buff, sizeof(buff)-1);

		ret = 0;

	}while(0);
	
	if(rth.fd>=0) {
		rtnl_close(&rth);
	}
//}fix coverity CID 29577: Improper use of negative value	
	return ret;
}

unsigned int wifi_get_associated_device_count(char* devName)
{
	return wifi_get_associated_device_info(devName, NULL, MAX_COUNT_ASSOCIATED_DEVICES);
}

int wifi_get_associated_device_mac_address(char* devName, unsigned int index, char* macAddr)
{
	WLAN_STA_INFO_T devInfo[MAX_COUNT_ASSOCIATED_DEVICES];
	unsigned int count = wifi_get_associated_device_info(devName, devInfo, MAX_COUNT_ASSOCIATED_DEVICES);

	macAddr[0] = 0;
	if(index<count){
		sprintf(macAddr, "%02x:%02x:%02x:%02x:%02x:%02x", 
			devInfo[index].addr[0],devInfo[index].addr[1],
			devInfo[index].addr[2],devInfo[index].addr[3],
			devInfo[index].addr[4],devInfo[index].addr[5]);
		return 0;
	}
	
	return -1;
}

int wifi_commit(char* devName)
{
	int ret = 0;
#ifdef USE_REALTEK_SDK
	char cmd[SIZE_128] = {0};
	unsigned int wlan_index = 0;
	unsigned int vap_index = 0;
	static pthread_t handle_thread = 0;
	static int pipefd[2] = {-1, -1};

	do{
		if(NULL==devName||devName!=strstr(devName, PREFIX_WLAN_DEVICE_NAME)){
			printf("%s: invalid device name [%s]\n", __func__, devName);
			ret = -1;
			break;
		}
		wlan_index = devName[4] - '0';
		vap_index = devName[8] - '0';
		if(wlan_index>=NUM_WLAN_INTERFACE){
			printf("%s: The index of [%s] should be less than %d\n", __func__, devName, NUM_WLAN_INTERFACE);
			ret = -1;
			break;
		}
		
		if(5==strlen(devName)){
		}else if(9==strlen(devName)){
			if(vap_index>=NUM_VWLAN){
				printf("%s: The index of [%s] should be less than %d\n", __func__, devName, NUM_VWLAN);
				ret = -1;
				break;
			}
		}else{
			printf("%s: invalid device name [%s]\n", __func__, devName);
			ret = -1;
			break;
		}
		
		if(0==handle_thread){
			if(pipe(pipefd)==-1)
			{
				perror("pipe");
				ret = -1;
				break;
			}
			if(pthread_create(&handle_thread, NULL, routine_thread_message,(void*)pipefd[0])<0)
			{
				close(pipefd[0]); 
				close(pipefd[1]); 
				ret = -1;
				break;
			}
								
			//printf("%s: handle_thread=%d, pipefd[0]=%d\n", __func__, handle_thread, pipefd[0]);
		}
		
		snprintf(cmd, sizeof(cmd), "%s", "sysconf init gw all");

		write(pipefd[1], cmd, strlen(cmd));
	}while(0);
#endif

	return ret;
}

int wifi_set_PreambleType(char* devName, unsigned int preambleType)
{//preambleType:	0 - Long; 1 - Short
	char wlanName[SIZE_32] = {0};
	strncpy(wlanName, devName, 5);
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
	return wifi_set_mib(wlanName, MIB_WLAN_PREAMBLE_TYPE, &preambleType);
#else
	return wifi_set_parameter(wlanName, "PREAMBLE_TYPE", preambleType, NULL);
#endif
#else
	return iwpriv_set_mib(wlanName, "preamble", preambleType, NULL);
#endif
}

int wifi_set_IAPPEnable(char* devName, unsigned int enable)
{
	char wlanName[SIZE_32] = {0};
	strncpy(wlanName, devName, 5);
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
	int disable = !enable;
	return wifi_set_mib(wlanName, MIB_WLAN_IAPP_DISABLED, &disable);
#else
	return wifi_set_parameter(wlanName, "IAPP_DISABLED", !enable, NULL);
#endif
#else
	return iwpriv_set_mib(wlanName, "iapp_enable", enable, NULL);
#endif
}

int wifi_set_SpaceTimeBlockCoding(char* devName, unsigned int STBC)
{
	char wlanName[SIZE_32] = {0};
	strncpy(wlanName, devName, 5);
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
	int value = 0!=STBC;
	return wifi_set_mib(wlanName, MIB_WLAN_STBC_ENABLED, &value);
#else
	return wifi_set_parameter(wlanName, "STBC_ENABLED", 0!=STBC, NULL);
#endif
#else
	return iwpriv_set_mib(wlanName, "stbc", 0!=STBC, NULL);
#endif
}

unsigned int wifi_get_RadioNumberOfEntries(void)
{
	return 2;
}

unsigned int wifi_get_SSIDNumberOfEntries(void)
{
	return 8;
}

unsigned int wifi_get_AccessPointNumberOfEntries(void)
{
	return wifi_get_SSIDNumberOfEntries();
}

unsigned int wifi_get_EndPointNumberOfEntries(void)
{
	return 0;
}

int wifi_get_radio_MaxBitRate(char *devName)
{//Not Implemented
	return 0;
}

unsigned int wifi_get_radio_SupportedFrequencyBands(char *devName)
{
	int band = 0;
	char cmd[SIZE_128]={0};
	unsigned int wlan_index = 0;

	do{
		if(NULL==devName||devName!=strstr(devName, PREFIX_WLAN_DEVICE_NAME)){
			printf("%s: invalid device name [%s]\n", __func__, devName);
			break;
		}
		wlan_index = devName[4] - '0';
		if(wlan_index>=NUM_WLAN_INTERFACE){
			printf("%s: The index of [%s] should be less than %d\n", __func__, devName, NUM_WLAN_INTERFACE);
			break;
		}
		sprintf(cmd, "ifconfig %s >/dev/null 2>/dev/null", devName);
		if(0!=shell(cmd)){
			break;
		}
		switch(wlan_index){
			case 0:
				band = BAND_2G;
				break;
			case 1:
				band = BAND_2G|BAND_5G;
				break;
			default:
				break;
		}
		
	}while(0);

	return band;
}

unsigned int wifi_get_radio_SupportedStandards(char *devName)
{
	int standard = 0;
	char cmd[SIZE_128]={0};
	unsigned int wlan_index = 0;

	do{
		if(NULL==devName||devName!=strstr(devName, PREFIX_WLAN_DEVICE_NAME)){
			printf("%s: invalid device name [%s]\n", __func__, devName);
			break;
		}
		wlan_index = devName[4] - '0';
		if(wlan_index>=NUM_WLAN_INTERFACE){
			printf("%s: The index of [%s] should be less than %d\n", __func__, devName, NUM_WLAN_INTERFACE);
			break;
		}
		sprintf(cmd, "ifconfig %s >/dev/null 2>/dev/null", devName);
		if(0!=shell(cmd)){
			break;
		}
		switch(wlan_index){
			case 0:
				standard = MODE_80211_B|MODE_80211_G|MODE_80211_N;
				break;
			case 1:
				standard = MODE_80211_A|MODE_80211_N;
				break;
			default:
				break;
		}
		
	}while(0);

	return standard;
}
	
unsigned int wifi_get_radio_AutoChannelSupported(char* devName)
{
	return 1;
}
	
unsigned int wifi_get_radio_IEEE80211hSupported(char* devName)
{//对802.11a的补充，确保其符合关于5GHz无线局域网的欧洲标准。
	int ieee80211h = 0;
	char cmd[SIZE_128]={0};
	unsigned int wlan_index = 0;

	do{
		if(NULL==devName||devName!=strstr(devName, PREFIX_WLAN_DEVICE_NAME)){
			printf("%s: invalid device name [%s]\n", __func__, devName);
			break;
		}
		wlan_index = devName[4] - '0';
		if(wlan_index>=NUM_WLAN_INTERFACE){
			printf("%s: The index of [%s] should be less than %d\n", __func__, devName, NUM_WLAN_INTERFACE);
			break;
		}
		sprintf(cmd, "ifconfig %s >/dev/null 2>/dev/null", devName);
		if(0!=shell(cmd)){
			break;
		}
		switch(wlan_index){
			case 0://5G
				ieee80211h = 1;
				break;
			case 1://2.4G
			default:
				break;
		}
		
	}while(0);

	return ieee80211h;
}

unsigned int wifi_get_SecurityModesSupported(char* devName)
{
	unsigned int modes = 0;

	modes = SECURITY_MODE_NONE|SECURITY_MODE_WEP64|SECURITY_MODE_WEP128
		|SECURITY_MODE_PERSONAL_WPA|SECURITY_MODE_PERSONAL_WPA2|SECURITY_MODE_PERSONAL_MIXED
		|SECURITY_MODE_ENTERPRISE_WPA|SECURITY_MODE_ENTERPRISE_WPA2|SECURITY_MODE_ENTERPRISE_MIXED;

	return modes;
}

unsigned int wifi_get_associated_device_LastDataDownlinkRate(char* devName, char* macAddr)
{//Not Implemented
	return 0;
}

unsigned int wifi_get_associated_device_LastDataUplinkRate(char* devName, char* macAddr)
{
	unsigned int rate = 0;
	unsigned int i;
	WLAN_STA_INFO_T devInfo[MAX_COUNT_ASSOCIATED_DEVICES];
	unsigned char	addr[6]={0};
	unsigned int count = 0;
	int rateid=0;

	do{
		count = wifi_get_associated_device_info(devName, devInfo, MAX_COUNT_ASSOCIATED_DEVICES);
		if(0==count){
			printf("%s: no associated device found\n", __func__);
			break;
		}
		if(NULL==macAddr||17!=strlen(macAddr)){
			printf("%s: Invalid mac address %s\n", __func__, macAddr);
		}
		for(i=0; i<6; i++){
			if(0==HexString2AsciiCode(macAddr+i*3, 2, addr+i)){
				break;
			}
		}
		
		if(i<6){
			printf("%s: Invalid mac address %s\n", __func__, macAddr);
			break;
		}
		for(i=0; i<count; i++){
			if(!memcmp(devInfo[i].addr, addr, 6)){
				if((devInfo[i].txOperaRates & 0x80) != 0x80){	
					if(devInfo[i].txOperaRates%2){
						rate = (devInfo[i].txOperaRates+1)*500;
					}else{
						rate = (devInfo[i].txOperaRates)*500;
					}
				}else{
					if((devInfo[i].ht_info & 0x1)==0){ //20M
						if((devInfo[i].ht_info & 0x2)==0){//long
							for(rateid=0; rateid<16;rateid++){
								if(rate_11n_table_20M_LONG[rateid].id == devInfo[i].txOperaRates){
									rate = (unsigned int)(rate_11n_table_20M_LONG[rateid].rate)*1000;
									break;
								}
							}
						}else if((devInfo[i].ht_info & 0x2)==0x2){//short
							for(rateid=0; rateid<16;rateid++){
								if(rate_11n_table_20M_SHORT[rateid].id == devInfo[i].txOperaRates){
									rate = (unsigned int)(rate_11n_table_20M_SHORT[rateid].rate)*1000;
									break;
								}
							}
						}
					}else if((devInfo[i].ht_info & 0x1)==0x1){//40M
						if((devInfo[i].ht_info & 0x2)==0){//long
							
							for(rateid=0; rateid<16;rateid++){
								if(rate_11n_table_40M_LONG[rateid].id == devInfo[i].txOperaRates){
									rate = (unsigned int)(rate_11n_table_40M_LONG[rateid].rate)*1000;
									break;
								}
							}
						}else if((devInfo[i].ht_info & 0x2)==0x2){//short
							for(rateid=0; rateid<16;rateid++){
								if(rate_11n_table_40M_SHORT[rateid].id == devInfo[i].txOperaRates){
									rate = (unsigned int)(rate_11n_table_40M_SHORT[rateid].rate)*1000;
									break;
								}
							}
						}
					}
				}	
				break;
			}
		}
	}while(0);
	
	return rate;
}

int wifi_get_associated_device_SignalStrength(char* devName, char* macAddr)
{
		int i;
		char mac[SIZE_20]={0};
		WLAN_STA_INFO_T devInfo[MAX_COUNT_ASSOCIATED_DEVICES];
		unsigned int count = wifi_get_associated_device_info(devName, devInfo, MAX_COUNT_ASSOCIATED_DEVICES);
	
		for(i=0; i<count; i++){
			sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", 
				devInfo[i].addr[0],devInfo[i].addr[1],
				devInfo[i].addr[2],devInfo[i].addr[3],
				devInfo[i].addr[4],devInfo[i].addr[5]);
			if(!STRNCMP(mac,macAddr,strlen(mac))){
				//fprintf(stderr, "devInfo[%d].rssi=%d\n", i, devInfo[i].rssi);
				return (devInfo[i].rssi-100);//dBm
			}
		}
		
		return 0;
}

unsigned int wifi_get_associated_device_Retransmissions(char* devName, char* macAddr)
{//Not Implemented
	return 0;
}

unsigned int wifi_get_associated_device_Active(char* devName, char* macAddr)
{
	int i;
	char mac[SIZE_20]={0};
	WLAN_STA_INFO_T devInfo[MAX_COUNT_ASSOCIATED_DEVICES];
	unsigned int count = wifi_get_associated_device_info(devName, devInfo, MAX_COUNT_ASSOCIATED_DEVICES);

	for(i=0; i<count; i++){
		sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", 
			devInfo[i].addr[0],devInfo[i].addr[1],
			devInfo[i].addr[2],devInfo[i].addr[3],
			devInfo[i].addr[4],devInfo[i].addr[5]);
		if(!STRNCMP(mac,macAddr,strlen(mac))){
			//fprintf(stderr, "devInfo[%d].expired_time=%ld\n", i, devInfo[i].expired_time);
			// if (expired_time == 30000) then station is active.
			//if (expired_time < 30000) then station is idle for ((30000 - expired_time) / 100) seconds
			if(devInfo[i].expired_time<30000){//expired_time - [0, 30000] unit : 0.01s
				return 0;
			}else{
				return 1;
			}
		}
	}
	
	return 0;
}

unsigned int wifi_get_associated_device_LinkQuality(char* devName, char* macAddr)
{
	int i;
	char mac[SIZE_20]={0};
	WLAN_STA_INFO_T devInfo[MAX_COUNT_ASSOCIATED_DEVICES];
	unsigned int count = wifi_get_associated_device_info(devName, devInfo, MAX_COUNT_ASSOCIATED_DEVICES);

	for(i=0; i<count; i++){
		sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", 
			devInfo[i].addr[0],devInfo[i].addr[1],
			devInfo[i].addr[2],devInfo[i].addr[3],
			devInfo[i].addr[4],devInfo[i].addr[5]);
		if(!STRNCMP(mac,macAddr,strlen(mac))){
			//fprintf(stderr, "devInfo[%d].sq=%d\n", i, devInfo[i].sq);
			return devInfo[i].sq;
		}
	}
	
	return 0;
}

int wifi_destroy_radio_device(char* devName)
{
	int ret = 0;
	char cmd[SIZE_256]={0};
	unsigned int wlan_index = 0;//wlan0

	do{
		if((NULL==devName)||(devName!=strstr(devName, PREFIX_WLAN_DEVICE_NAME))){
			printf("%s: Invalid device name: %s\n", __func__, devName);
			ret = -1;
			break;
		}
		wlan_index = devName[4] - '0';//wlan0
		if(wlan_index>=NUM_WLAN_INTERFACE){
			printf("%s: Invalid radio device index: %d\n", __func__, wlan_index);
			ret = -1;
			break;
		}
		radio_creation_flag &= ~(1 << wlan_index);
		snprintf(cmd, sizeof(cmd), "ifconfig %s down", devName );
		ret += shell(cmd);
	}while(0);

	return ret;
}

int wifi_create_radio_device(unsigned int FrequencyBand, char* devName)
{
	int ret = -1;
	char cmd[SIZE_128]={0};
	int index = 0;

	do{
#ifdef USE_REALTEK_SDK
		if (!apmib_init()) {
			printf("Initialize AP MIB failed!\n");
			ret = -1;
			break;
		}
#endif
		if(NULL==devName){
			printf("%s():%d: devName can not be null\n", __func__, __LINE__);
			break;
		}
		if(BAND_2G!=FrequencyBand&&BAND_5G!=FrequencyBand){
			printf("%s():%d: invalid frequency band [%u].\n", __func__, __LINE__, FrequencyBand);
			break;
		}

		index = (BAND_2G==FrequencyBand);
		
		if(0==(radio_creation_flag&(1<<index))){
			snprintf(cmd, sizeof(cmd), "ifconfig %s%d >/dev/null 2>/dev/null", PREFIX_WLAN_DEVICE_NAME, index);
			if(shell(cmd)!=0){
				printf("[%s]: fail to create radio device\n", __func__);
				break;
			}
			sprintf(devName, "%s%d", PREFIX_WLAN_DEVICE_NAME, index);
			radio_creation_flag |= (1<<index);
			ret = 0;
			printf("[%s]: radio device [%s] has been created\n", __func__, devName);
			break;
		}
	}while(0);
	
	return ret;
}

int wifi_set_radio_standards(char* devName, unsigned int standard)
{
	int ret = 0;
	unsigned int band = 0;
	char name[SIZE_32]={0};
	int i;
	
	//WLAN0_BAND=11	bgn
	if(MODE_80211_N==standard){
		band=BAND_N;//11n
	}else if(MODE_80211_G==standard){
		band=BAND_G;//11g
	}else if(MODE_80211_B==standard){
		band=BAND_B;//11b
	}else if(MODE_80211_A==standard){
		band=BAND_A;//11a
	}else if(MODE_80211_N&standard){
		band=BAND_N;
		if(MODE_80211_A&standard){
			band |= BAND_A;//11an
		}else{
			if(MODE_80211_B&standard){
				band |= BAND_B;//11bn
			}
			if(MODE_80211_G&standard){
				band |= BAND_G;//11gn
			}
			//11bgn
		}
	}else if(MODE_80211_G&standard){
		band=BAND_B|BAND_G;//11bg
	}else{//auto
		band=0;
	}
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
	ret = wifi_set_mib(devName, MIB_WLAN_BAND, &band); 
	if(0==ret){
		for(i=0; i<4; i++){
			snprintf(name, sizeof(name), "wlan%d-va%d", devName[4] - '0', i);
			ret += wifi_set_mib(name, MIB_WLAN_BAND, &band); 
		}
	}
#else
	ret = wifi_set_parameter(devName, "BAND", band, NULL); 
#endif
#else
	ret = iwpriv_set_mib(devName, "band", band, NULL);
	if(0==ret){
		for(i=0; i<4; i++){
			snprintf(name, sizeof(name), "wlan%d-va%d", devName[4] - '0', i);
			ret += iwpriv_set_mib(name, "band", band, NULL);
		}
	}
#endif
	return ret;
}

int wifi_set_radio_channel_info(char* devName, CHANNEL_WIDTH channelBandwidth, EXTENSION_CHANNEL extensionChannel,
	unsigned int autoChannelEnable, unsigned int autoChannelRefreshPeriod, unsigned int channel)
{
	int ret = 0;
	unsigned int bandWidth = 0;
	unsigned int ctrlSideBand = 2;
	unsigned int coexist = 0;

	switch(channelBandwidth){
		case CHANNEL_WIDTH_20MHZ:
			bandWidth = 0;
			coexist = 0;
			break;
		case CHANNEL_WIDTH_40MHZ:
			bandWidth = 1;
			coexist = 0;
			break;
		case CHANNEL_WIDTH_AUTO:
		default:// Auto
			bandWidth = 1;
			coexist = 1;
			break;
	}

	if(autoChannelEnable){
		channel = 0;
	};

/*
Issue:	2. How to set "Auto" for side band parameter?
Answer:	==> In our wlan driver, you must set correct side band parameter. But most customer will fix the sideband parameters on their webpages and don't need customer to select.
*/
	if(1==bandWidth){
		switch(extensionChannel){
			case CONTROL_CHANNEL_ABOVE:
				switch(channel){
					case 0:
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
					case 6:
					case 7:
					case 8:
					case 9:
					case 36:
					case 44:
					case 149:
					case 157:
					case 165:
						ctrlSideBand = 1;//0-Lower, 1-Upper
						break;
					default:
						printf("Primary Channel %d with AboveControlChannel is not avaliable.\n", channel);
						return -1;
				}
				break;
			case CONTROL_CHANNEL_BELOW:
				switch(channel){
					case 0:
					case 5:
					case 6:
					case 7:
					case 8:
					case 9:
					case 10:
					case 11:
					case 40:
					case 48:
					case 153:
					case 161:
						ctrlSideBand = 0;//0-Lower, 1-Upper
						break;
					default:
						printf("Primary Channel %d with BelowControlChannel is not avaliable.\n", channel);
						return -1;
				}
				break;
			case CONTROL_CHANNEL_AUTO:
			default:
				switch(channel){
					case 1:
					case 2:
					case 3:
					case 4:
					case 36:
					case 44:
					case 149:
					case 157:
					case 165:
						ctrlSideBand = 1;//0-Lower, 1-Upper
						break;
					case 10:
					case 11:
					case 40:
					case 48:
					case 153:
					case 161:
						ctrlSideBand = 0;//0-Lower, 1-Upper
						break;
					default:
						ctrlSideBand = 2;
						break;
				}
				break;
		}
	}

/*
Issue:	3. Does the Realtek SDK support TR-181 parameter "AutoChannelRefreshPeriod" or not?
	AutoChannelRefreshPeriod - The time period in seconds between two consecutive automatic channel selections. A value of 0 means that the automatic channel selection is done only at boot time.
Answer:	==> Currently SDK only support auto channel selection at boot up or system re-init.
*/
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
	//WLAN0_CHANNEL_BONDING=1//0-20MHz, 1-40MHz(Auto)
	ret += wifi_set_mib(devName, MIB_WLAN_CHANNEL_BONDING, &bandWidth);
	int value = 0!=coexist;
	ret += wifi_set_mib(devName, MIB_WLAN_COEXIST_ENABLED, &value);//(Auto)
	//WLAN0_CONTROL_SIDEBAND=0//0-Lower, 1-Upper
	if(ctrlSideBand<2){
		ret += wifi_set_mib(devName, MIB_WLAN_CONTROL_SIDEBAND, &ctrlSideBand);
	}
	ret += wifi_set_mib(devName, MIB_WLAN_CHANNEL, &channel);
#else
	//WLAN0_CHANNEL_BONDING=1//0-20MHz, 1-40MHz(Auto)
	ret += wifi_set_parameter(devName, "CHANNEL_BONDING", bandWidth, NULL);
	ret += wifi_set_parameter(devName, "COEXIST_ENABLED", 0!=coexist, NULL);//(Auto)
	//WLAN0_CONTROL_SIDEBAND=0//0-Lower, 1-Upper
	if(ctrlSideBand<2){
		ret += wifi_set_parameter(devName, "CONTROL_SIDEBAND", ctrlSideBand, NULL);
	}
	ret += wifi_set_parameter(devName, "CHANNEL", channel, NULL);
#endif
#else
	ret += iwpriv_set_mib(devName, "use40M", bandWidth, NULL);
	ret += iwpriv_set_mib(devName, "coexist", 0!=coexist, NULL);
	//1-Below, 2-Above
	if(ctrlSideBand<2){
		ret += iwpriv_set_mib(devName, "2ndchoffset", ctrlSideBand+1, NULL);
	}
	ret += iwpriv_set_mib(devName, "channel", channel, NULL);
#endif
	if(1==bandWidth){
		ret += iwpriv_set_mib(devName, "Non40MSTADeny", !coexist, NULL);
	}
	
	return ret;
}

int wifi_set_radio_TransmitPower(char* devName, int transmitPower)
{
	unsigned int rf_power_scale = 0;
	unsigned int pwr_offset = 0;
	
	//WLAN0_RFPOWER_SCALE=0	100
	//WLAN0_RFPOWER_SCALE=1	70
	//WLAN0_RFPOWER_SCALE=2	50
	//WLAN0_RFPOWER_SCALE=3	35
	//WLAN0_RFPOWER_SCALE=4	15
	if(transmitPower>70){
		rf_power_scale = 0;
		pwr_offset = 0;
	}else if(transmitPower>50){
		rf_power_scale = 1;
		pwr_offset = 3;
	}else if(transmitPower>35){
		rf_power_scale = 2;
		pwr_offset = 6;
	}else if(transmitPower>15){
		rf_power_scale = 3;
		pwr_offset = 9;
	}else{
		rf_power_scale = 4;
		pwr_offset = 17;
	}

#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
	return wifi_set_mib(devName, MIB_WLAN_RFPOWER_SCALE, &rf_power_scale);
#else
	return wifi_set_parameter(devName, "RFPOWER_SCALE", rf_power_scale, NULL);
#endif
#else
	int ret = 0;
	struct wifi_mib mib_data;
	int skfd = -1;
	struct iwreq wrq;
	int i = 0;

	do{
		if(!pwr_offset){
			break;
		}
		
		skfd = socket(AF_INET, SOCK_DGRAM, 0);
		if (skfd < 0) {
			perror("socket");
			ret = -1;
			break;
		}
		snprintf(wrq.ifr_name, sizeof(wrq.ifr_name), "%s", devName);
		if (ioctl(skfd, SIOCGIWNAME, &wrq) < 0) {
			perror("ioctl-SIOCGIWNAME");
			ret = -1;
			break;
		}
		// get mib from driver
		wrq.u.data.pointer = (caddr_t)&mib_data;
		wrq.u.data.length = sizeof(struct wifi_mib);
		if (ioctl(skfd, SIOCMIBINIT, &wrq) < 0) {
			printf("Get WLAN MIB failed!\n");
			ret = -1;
			break;
		}
		for (i=0; i<MAX_2G_CHANNEL_NUM_MIB; i++) {
			if(mib_data.dot11RFEntry.pwrlevelCCK_A[i] != 0){ 
				if ((mib_data.dot11RFEntry.pwrlevelCCK_A[i] - pwr_offset) >= 1)
					mib_data.dot11RFEntry.pwrlevelCCK_A[i] -= pwr_offset;
				else
					mib_data.dot11RFEntry.pwrlevelCCK_A[i] = 1;
			}
			if(mib_data.dot11RFEntry.pwrlevelCCK_B[i] != 0){ 
				if ((mib_data.dot11RFEntry.pwrlevelCCK_B[i] - pwr_offset) >= 1)
					mib_data.dot11RFEntry.pwrlevelCCK_B[i] -= pwr_offset;
				else
					mib_data.dot11RFEntry.pwrlevelCCK_B[i] = 1;
			}
			if(mib_data.dot11RFEntry.pwrlevelHT40_1S_A[i] != 0){ 
				if ((mib_data.dot11RFEntry.pwrlevelHT40_1S_A[i] - pwr_offset) >= 1)
					mib_data.dot11RFEntry.pwrlevelHT40_1S_A[i] -= pwr_offset;
				else
					mib_data.dot11RFEntry.pwrlevelHT40_1S_A[i] = 1;
			}
			if(mib_data.dot11RFEntry.pwrlevelHT40_1S_B[i] != 0){ 
				if ((mib_data.dot11RFEntry.pwrlevelHT40_1S_B[i] - pwr_offset) >= 1)
					mib_data.dot11RFEntry.pwrlevelHT40_1S_B[i] -= pwr_offset;
				else
					mib_data.dot11RFEntry.pwrlevelHT40_1S_B[i] = 1;
			}
		}	
		
		for (i=0; i<MAX_5G_CHANNEL_NUM_MIB; i++) {
			if(mib_data.dot11RFEntry.pwrlevel5GHT40_1S_A[i] != 0){ 
				if ((mib_data.dot11RFEntry.pwrlevel5GHT40_1S_A[i] - pwr_offset) >= 1)
					mib_data.dot11RFEntry.pwrlevel5GHT40_1S_A[i] -= pwr_offset;
				else
					mib_data.dot11RFEntry.pwrlevel5GHT40_1S_A[i] = 1;					
			}
			if(mib_data.dot11RFEntry.pwrlevel5GHT40_1S_B[i] != 0){ 
				if ((mib_data.dot11RFEntry.pwrlevel5GHT40_1S_B[i] - pwr_offset) >= 1)
					mib_data.dot11RFEntry.pwrlevel5GHT40_1S_B[i] -= pwr_offset;
				else
					mib_data.dot11RFEntry.pwrlevel5GHT40_1S_B[i] = 1;
			}
		}
		if (ioctl(skfd, SIOCMIBSYNC, &wrq) < 0) {
			printf("Set WLAN MIB failed!\n");
			ret = -1;
			break;
		}
	}while(0);

	if(-1!=skfd){
		close(skfd);
	}

	return ret;
#endif
}

int wifi_set_radio_GuardInterval(char* devName, GUARD_INTERVAL guardInterval)
{//SHORT_GI : 0-disable, 1-enable
/*
Issue: 5. How to set "Auto" for guard interval parameter?
Answer:     ==> If you enable SGI in wlan driver, our behavior is like Auto you needed. If STA support SGI, AP will send w/ SGI. If STA don't support SGI, AT will send pkts w/ LGI.

*/
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
	int value = GUARD_INTERVAL_800!=guardInterval;
	return wifi_set_mib(devName, MIB_WLAN_SHORT_GI, &value);
#else
	return wifi_set_parameter(devName, "SHORT_GI", GUARD_INTERVAL_800!=guardInterval, NULL);
#endif
#else
	int ret = 0;
	char value[SIZE_32]={0};

	do{
		if(iwpriv_get_mib(devName, "coexist", value)<0){
			ret = -1;
			break;
		}

		if(atoi(value)){
			ret += iwpriv_set_mib(devName, "shortGI20M", GUARD_INTERVAL_800!=guardInterval, NULL);
			ret += iwpriv_set_mib(devName, "shortGI40M", GUARD_INTERVAL_800!=guardInterval, NULL);
		}else{
			if(iwpriv_get_mib(devName, "use40M", value)<0){
				ret = -1;
				break;
			}
			if(atoi(value)){
				ret = iwpriv_set_mib(devName, "shortGI40M", GUARD_INTERVAL_800!=guardInterval, NULL);
			}else{
				ret = iwpriv_set_mib(devName, "shortGI20M", GUARD_INTERVAL_800!=guardInterval, NULL);
			}
		}
		
	}while(0);

	return ret;
#endif
}

int wifi_set_radio_MCS(char* devName, int MCS)
{
	char operationalDataTransmitRates[SIZE_32]={0};
/*
Issue:	4. How to set TR-181 parameter "MCS" ? does your SDK support it as below?
	MCS - The Modulation Coding Scheme index (applicable to 802.11n specifications only). Values from 0 to 15 MUST be supported ([802.11n-2009]). A value of -1 indicates automatic selection of the MCS index.
Answer:	==> Our SDK could fix AP TX rate as MCS0~MCS15. Do you need such function ?
*/
	if(NULL==devName||MCS<-1||MCS>15){
		return -1;
	}
	if(-1==MCS){
		MCS = 15;
	}
	sprintf(operationalDataTransmitRates, "MCS%d", MCS);
	return wifi_set_radio_OperationalDataTransmitRates(devName, operationalDataTransmitRates);
}

int wifi_set_radio_IEEE80211hEnabled(char* devName, unsigned int IEEE80211hEnabled)
{//Not Implemented
	return 0;
}

int wifi_set_radio_RegulatoryDomain(char* devName, char* RegulatoryDomain)
{
	enum WIFI_REG_DOMAIN {
		DOMAIN_FCC		= 1,
		DOMAIN_IC		= 2,
		DOMAIN_ETSI 	= 3,
		DOMAIN_SPAIN	= 4,
		DOMAIN_FRANCE	= 5,
		DOMAIN_MKK		= 6,
		DOMAIN_ISRAEL	= 7,
		DOMAIN_MKK1 	= 8,
		DOMAIN_MKK2 	= 9,
		DOMAIN_MKK3 	= 10,
		DOMAIN_NCC		= 11,
		DOMAIN_RUSSIAN	= 12,
		DOMAIN_CN		= 13,
		DOMAIN_MAX
	};
	int ret = 0;
	int regdomain = 0;
	
	do{
		if(!wifi_device_is_exist(devName)){
			ret = -1;
			break;
		}
		if(NULL==RegulatoryDomain||3!=strlen(RegulatoryDomain)){
			ret = -1;
			break;
		}
		if(' '==RegulatoryDomain[2]){//all environments
		}else if('O'==RegulatoryDomain[2]){//outside 
		}else if('I'==RegulatoryDomain[2]){//inside
		}else{
			ret = -1;
			break;
		}
		RegulatoryDomain[2] = 0;
		if(!strcmp(RegulatoryDomain, "CN")){
			regdomain = DOMAIN_CN;
		}else if(!strcmp(RegulatoryDomain, "RU")){
			regdomain = DOMAIN_RUSSIAN;
		}else if(!strcmp(RegulatoryDomain, "IL")){
			regdomain = DOMAIN_ISRAEL;
		}else if(!strcmp(RegulatoryDomain, "FR")){
			regdomain = DOMAIN_FRANCE;
		}else if(!strcmp(RegulatoryDomain, "ES")){
			regdomain = DOMAIN_SPAIN;
		}else{
			regdomain = DOMAIN_FCC;
		}

		ret = iwpriv_set_mib(devName, "regdomain", regdomain, NULL);
	}while(0);
	
	return ret;
}

int wifi_set_ShortRetryLimit(char* devName, unsigned int RetryLimit)
{
	if(NULL==devName||devName!=strstr(devName, PREFIX_WLAN_DEVICE_NAME)){
		return -1;
	}
	
	return iwpriv_set_mib(devName, "shortretry", RetryLimit, NULL);
}

int wifi_set_MaxAssociatedDevices(char* devName, unsigned int MaxAssociatedDevices)
{
	if(MaxAssociatedDevices>MAX_STA_NUM){
		MaxAssociatedDevices = MAX_STA_NUM;
	}

	return iwpriv_set_mib(devName, "stanum", MaxAssociatedDevices, NULL);
}

int wifi_set_IsolationEnable(char* devName, unsigned int IsolationEnable)
{
#ifdef USE_REALTEK_SDK
#ifdef USE_REALTEK_MIB
	return wifi_set_mib(devName, MIB_WLAN_BLOCK_RELAY, &IsolationEnable);
#else
	return wifi_set_parameter(devName, "BLOCK_RELAY", IsolationEnable, NULL);
#endif
#else
	return iwpriv_set_mib(devName, "block_relay", IsolationEnable, NULL);
#endif
}

unsigned int wifi_device_is_exist(char* devName)
{
	char cmd[SIZE_128]={0};

	if(NULL==devName||0==strlen(devName)){
		return 0;
	}
	
	snprintf(cmd, sizeof(cmd), "ifconfig %s >/dev/null 2>/dev/null", devName);

	return (0==shell(cmd));
}

int wifi_get_default_ssid(char* radioDevName, char *name, char *ssid)
{
	int ret = 0;
#if defined(AEI_WECB_CUSTOMER_NCS)
    int wlan_index = -1;
    int vap_index = -1;
    int snLen = 0;
    char sn[DEVICE_SERIAL_NUMBER_MAX_LENGTH] = {0};
    char sn_suffix[8] = {0};
    char vap_suffix[8] = {0};

    do{
        wlan_index = wifi_get_device_index(radioDevName);
        vap_index = wifi_get_device_index(name);
        if(wlan_index < 0 || vap_index < 0){
            ret = -1;
            break;
        }

        switch(vap_index)
        {
            case 0:
                memset(vap_suffix, 0, sizeof(vap_suffix));
                break;
            case 1:
            case 2:
            case 3:
            default:
                sprintf(vap_suffix, "-%d", vap_index + 1);
        }


        if(tr69_common_func_get_serial_number(sn, sizeof(sn)))
        {
            snLen = tsl_strlen(sn);
            if(snLen >= 4)
            {
                sprintf(sn_suffix, "%c%c%c%c",
                        sn[snLen-4], sn[snLen-3], sn[snLen-2], sn[snLen-1]);
            }
            else
            {
                sprintf(sn_suffix, "%s", sn);
            }
        }

        sprintf(ssid, "Actiontec-%s-%s%s", sn_suffix, wlan_index?"2.4G":"5G", vap_suffix);
    }while(0);
#elif defined(AEI_WECB_CUSTOMER_TELUS)
    int wlan_index = -1;
    int vap_index = -1;
    int snLen = 0;
    char sn[DEVICE_SERIAL_NUMBER_MAX_LENGTH] = {0};
    char sn_suffix[8] = {0};
    char vap_suffix[8] = {0};

    do{
        wlan_index = wifi_get_device_index(radioDevName);
        vap_index = wifi_get_device_index(name);
        if(wlan_index < 0 || vap_index < 0){
            ret = -1;
            break;
        }

        switch(vap_index)
        {
            case 0:
                memset(vap_suffix, 0, sizeof(vap_suffix));
                break;
            case 1:
            case 2:
            case 3:
            default:
                sprintf(vap_suffix, "-%d", vap_index + 1);
        }

        if(tr69_common_func_get_serial_number(sn, sizeof(sn)))
        {
            snLen = tsl_strlen(sn);
            if(snLen >= 4)
            {
                sprintf(sn_suffix, "%c%c%c%c", 
                        sn[snLen-4], sn[snLen-3], sn[snLen-2], sn[snLen-1]);
            }
            else
            {
                sprintf(sn_suffix, "%s", sn);
            }
        }
        sprintf(ssid, "TELUS%s-%s%s", sn_suffix, wlan_index?"2.4G":"5G", vap_suffix);

    }while(0);
#else
	char buff[SIZE_20]={0};
	int index = -1;

	do{
		index = wifi_get_device_index(radioDevName);
		if(index<0){
			ret = -1;
			break;
		}
		memset(buff, 0, sizeof(buff));
		ret = wifi_get_default_MACAddress(name, buff);
		if(ret<0){
			break;
		}
		sprintf(ssid, "HOME-%c%c%c%c-%s", toupper(buff[12]), toupper(buff[13]), 
			toupper(buff[15]), toupper(buff[16]), index?"2.4G":"5G");
	}while(0);
#endif

	return ret;
}

void mac_decrease(char *buff)
{
	int i = 0;
	char str[8];
	long x = 0;

	for(i=5; i>=0; i--){
		x = i<<1;
		snprintf(str, sizeof(str), "%c%c", buff[x], buff[x+1]);
		x = strtol(str, NULL, 16);
		x--;
		snprintf(str, sizeof(str), "%.02x", (unsigned char)x);
		memcpy(buff+(i<<1), str, 2);
		if(x>=0){
			break;
		}
	}
}


int wifi_get_default_key(char *name, unsigned int isWEPKey128, unsigned int sizePreSharedKey, char *defaultWEPKey, char *defaultPreSharedKey)
{
	int ret = 0;
	char default_key[SIZE_64+1]={0};
    char *p = NULL;
	int i = 0;
	unsigned long seed1, seed2;
    char buff[SIZE_64]={0};
	unsigned long temp;
	unsigned int a, b;

#ifdef AEI_WECB_CUSTOMER_NCS
    #define DEFAULT_KEY_FORMAT  "%02x%02x%02x%02x"
#else
    #define DEFAULT_KEY_FORMAT  "%02X%02X%02X%02X"
#endif

	do{
		memset(buff, 0, sizeof(buff));
		ret = wifi_get_default_MACAddress(name, buff);
		if(ret<0){
			break;
		}
		//printf("MAC=[%s]\n", buff);//17bytes
		p = buff;
		i = 0;
		while(*p){
			if((*p>='0'&&*p<='9')
				||(*p>='a'&&*p<='f')){
				buff[i++] = *p;
			}else if(*p>='A'&&*p<='F'){
				buff[i++] = *p - 'A' + 'a';
			}
			p++;
		}
		buff[i] = 0;
		//printf("MAC2=[%s]\n", buff);//12bytes
		mac_decrease(buff);
		if(1!=sscanf (buff, "%x", &a)){//last 8 characters
			ret = -1;
			break;
		}
		seed1 = a;
		//printf("seed1=[0x%x]\n", seed1);//last 8 characters
		buff[8] = 0;
		if(1!=sscanf (buff, "%x", &b)){//first 8 characters
			ret = -1;
			break;
		}
		seed2 = b;
		//printf("seed2=[0x%x]\n", seed2);//first 8 characters
		
        temp = seed2;
		temp >>= 0;
        temp = ~temp;
        temp += seed1;
        sprintf(default_key, DEFAULT_KEY_FORMAT, (unsigned char)(temp>>24)&0xff, (unsigned char)(temp>>16), (unsigned char)(temp>>8), (unsigned char)temp);

		temp = seed2;
		temp >>= 1;
        temp = ~temp;
        temp += seed1;
        sprintf(buff, DEFAULT_KEY_FORMAT, (unsigned char)(temp>>24)&0xff, (unsigned char)(temp>>16), (unsigned char)(temp>>8), (unsigned char)temp);
        strcat(default_key, buff);

		temp = seed2;
		temp >>= 2;
        temp = ~temp;
        temp += seed1;
        sprintf(buff, DEFAULT_KEY_FORMAT, (unsigned char)(temp>>24)&0xff, (unsigned char)(temp>>16), (unsigned char)(temp>>8), (unsigned char)temp);
        strcat(default_key, buff);

		temp = seed2;
		temp >>= 3;
        temp = ~temp;
        temp += seed1;
        sprintf(buff, DEFAULT_KEY_FORMAT, (unsigned char)(temp>>24)&0xff, (unsigned char)(temp>>16), (unsigned char)(temp>>8), (unsigned char)temp);
        strcat(default_key, buff);

		temp = seed2;
		temp >>= 4;
        temp = ~temp;
        temp += seed1;
        sprintf(buff, DEFAULT_KEY_FORMAT, (unsigned char)(temp>>24)&0xff, (unsigned char)(temp>>16), (unsigned char)(temp>>8), (unsigned char)temp);
        strcat(default_key, buff);

		temp = seed2;
		temp >>= 5;
        temp = ~temp;
        temp += seed1;
        sprintf(buff, DEFAULT_KEY_FORMAT, (unsigned char)(temp>>24)&0xff, (unsigned char)(temp>>16), (unsigned char)(temp>>8), (unsigned char)temp);
        strcat(default_key, buff);

		temp = seed2;
		temp >>= 6;
        temp = ~temp;
        temp += seed1;
        sprintf(buff, DEFAULT_KEY_FORMAT, (unsigned char)(temp>>24)&0xff, (unsigned char)(temp>>16), (unsigned char)(temp>>8), (unsigned char)temp);
        strcat(default_key, buff);

		temp = seed2;
		temp >>= 7;
        temp = ~temp;
        temp += seed1;
        sprintf(buff, DEFAULT_KEY_FORMAT, (unsigned char)(temp>>24)&0xff, (unsigned char)(temp>>16), (unsigned char)(temp>>8), (unsigned char)temp);
        strcat(default_key, buff);

		//printf("default_key=[%s]\n", default_key);
		ret = -1;

		if(NULL!=defaultWEPKey){
			if(isWEPKey128){
				snprintf(defaultWEPKey, 26+1, "%s", default_key);
			}else{
				snprintf(defaultWEPKey, 10+1, "%s", default_key);
			}

			ret = 0;
		}
		if(defaultPreSharedKey&&sizePreSharedKey>=8){
			if(sizePreSharedKey<SIZE_64){
				snprintf(defaultPreSharedKey, sizePreSharedKey+1, "%s", default_key);
			}else{
				snprintf(defaultPreSharedKey, SIZE_64+1, "%s", default_key);
			}

			ret = 0;
		}
	}while(0);

	return ret;
}

///////////////////////////////////////////////////////////////////////////////
int wifi_set_radio_MaxBitRate(char* devName, char* maxBitRate)
{//Only for tr-098
	return -1;
}

int wifi_set_LocationDescription(char* devName, char* locationDescription)
{//Only for tr-098
	return -1;
}

int wifi_set_PeerBSSID(char* devName, char* peerBSSID)
{//Only for tr-098
	return -1;
}

int wifi_set_AuthenticationServiceMode(char* devName, char* authenticationServiceMode)
{//Only for tr-098
	return -1;
}

int wifi_set_InsecureOOBAccessEnabled(char* devName, unsigned int insecureOOBAccessEnabled)
{//Only for tr-098
	return -1;
}

int wifi_set_DistanceFromRoot(char* devName, unsigned int distanceFromRoot)
{//Only for tr-098
	return -1;
}

int wifi_set_BeaconAdvertisementEnabled(char* devName, unsigned int beaconAdvertisementEnabled)
{//Only for tr-098
	return -1;
}

unsigned int wifi_get_TotalPSKFailures(char* devName)
{//Only for tr-098
	return -1;
}

unsigned int wifi_get_TotalIntegrityFailures(char* devName)
{//Only for tr-098
	return -1;
}

int wifi_get_associated_device_last_requested_unicast_cipher(char* devName, char* macAddr, char* lastRequestedUnicastCipher)
{//Only for tr-098
	return -1;
}

int wifi_get_associated_device_last_requested_multicast_cipher(char* devName, char* macAddr, char* lastRequestedMulticastCipher)
{//Only for tr-098
	return -1;
}

int wifi_get_associated_device_last_pairwise_master_key_id(char* devName, char* macAddr, char* lastPMKId)
{//Only for tr-098
	return -1;
}

int wifi_get_wps_registrar_uuid(char* devName, unsigned int index, char* uuid)
{//Only for tr-098
	return -1;
}

int wifi_get_wps_registrar_device_name(char* devName, unsigned int index, char* registrarDeviceName)
{//Only for tr-098
	return -1;
}


///////////////////////////////////////////////////////////////////////////////

