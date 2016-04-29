/**************************************************************************
 *	
 *	        tr181_wifi.c according to TR-181_Issue-2_Amendment-5.pdf
 *
 **************************************************************************/
#include <unistd.h> //for sleep
#include <time.h> //for time
#include <stdio.h> //for snprintf
#include <stdlib.h> //for malloc
#include <string.h>
#include <math.h>
#include "sha1.h"
#include "tr181_wifi.h"
#include "wifi_adapter_driver.h"


/**************************************************************************
 *	
 *	        constant definition
 *
 **************************************************************************/
#define PREFIX_PATH_WIFI 		"Device.WiFi"
#define PREFIX_PATH_RADIO 		"Device.WiFi.Radio"
#define PREFIX_PATH_SSID 		"Device.WiFi.SSID"
#define PREFIX_PATH_ACCESSPOINT "Device.WiFi.AccessPoint"
#define PREFIX_PATH_ENDPOINT 	"Device.WiFi.EndPoint"
#define TOKEN_PATH_STATS 		".Stats"
#define TOKEN_PATH_SECURITY		".Security"
#define TOKEN_PATH_ACCOUNTING	".Accounting"
#define TOKEN_PATH_WPS			".WPS"
#define TOKEN_PATH_ASSOCIATEDDEVICE 	".AssociatedDevice."

#define STR006_STATUS_Up				"Up"
#define STR006_STATUS_Down				"Down"
#define STR006_STATUS_Unknown			"Unknown"
#define STR006_STATUS_Dormant	 		"Dormant"
#define STR006_STATUS_NotPresent 		"NotPresent"
#define STR006_STATUS_LowerLayerDown	"LowerLayerDown"
#define STRING_STATUS_Error	 				"Error"
#define STR065_STATUS_Disabled				"Disabled"
#define STR065_STATUS_Enabled				"Enabled"
#define STR065_STATUS_Error_Misconfigured	"Error_Misconfigured"

#define STR013_BAND_2G 	"2.4GHz"
#define STR013_BAND_5G 	"5GHz"

#define STR023_BANDWIDTH_20MHz 	"20MHz"
#define STR023_BANDWIDTH_40MHz 	"40MHz"
#define STR023_BANDWIDTH_Auto 	"Auto"

#define STR024_CONTROL_CHANNEL_ABOVE 	"AboveControlChannel"
#define STR024_CONTROL_CHANNEL_BELOW 	"BelowControlChannel"
#define STR024_CONTROL_CHANNEL_AUTO 	"Auto"

#define STR025_GUARD_INTERVAL_400 	"400nsec"
#define STR025_GUARD_INTERVAL_800 	"800nsec"
#define STR025_GUARD_INTERVAL_AUTO 	"Auto"

#define STR078_SECURITY_MODE_None				"None"
#define STR078_SECURITY_MODE_WEP64				"WEP-64"
#define STR078_SECURITY_MODE_WEP128				"WEP-128"
#define STR078_SECURITY_MODE_WPA_Personal		"WPA-Personal"
#define STR078_SECURITY_MODE_WPA2_Personal		"WPA2-Personal"
#define STR078_SECURITY_MODE_MIXED_Personal		"WPA-WPA2-Personal"
#define STR078_SECURITY_MODE_WPA_Enterprise		"WPA-Enterprise"
#define STR078_SECURITY_MODE_WPA2_Enterprise	"WPA2-Enterprise"
#define STR078_SECURITY_MODE_MIXED_Enterprise	"WPA-WPA2-Enterprise"

#define STR099_CONFIG_METHODS_SUPPORTED_USBFLASHDRIVER 			"USBFlashDrive"
#define STR099_CONFIG_METHODS_SUPPORTED_ETHERNET 				"Ethernet"
#define STR099_CONFIG_METHODS_SUPPORTED_EXTERNEL_NFC_TOKEN 		"ExternalNFCToken"
#define STR099_CONFIG_METHODS_SUPPORTED_INTEGRATED_NFC_TOKEN 	"IntegratedNFCToken"
#define STR099_CONFIG_METHODS_SUPPORTED_NFC_INTERFACE 			"NFCInterface"
#define STR099_CONFIG_METHODS_SUPPORTED_PUSHBUTTON 				"PushButton"
#define STR099_CONFIG_METHODS_SUPPORTED_PIN 					"PIN"

#define X02_WEP_AUTHENTICATION_MODE_OPEN	"None"	
#define X02_WEP_AUTHENTICATION_MODE_SHARED	"SharedAuthentication"	
#define X02_WEP_AUTHENTICATION_MODE_AUTO	"Auto"	

#define X003_WPA_ENCRYPTION_MODES_TKIP		"TKIPEncryption"
#define X003_WPA_ENCRYPTION_MODES_AES		"AESEncryption"
#define X003_WPA_ENCRYPTION_MODES_TKIP_AES	"TKIPandAESEncryption"

#define LEN_MAC_ADDRESS			17
#define DEFAULT_SALT_MAC	"00:11:22:33:44:55"

#define MAX_RETRY_LIMIT		7

/**************************************************************************
 *	
 *	        macro definition
 *
 **************************************************************************/
#define CURRENT_TIME \
({ \
        struct timespec _t0; \
        clock_gettime(CLOCK_MONOTONIC, &_t0); \
        _t0.tv_sec; \
    })
#define DIFF_TIME(t) \
({ \
        struct timespec _t1; \
        clock_gettime(CLOCK_MONOTONIC, &_t1); \
        _t1.tv_sec-(t); \
    })

#ifndef STRCMP
#define STRCMP(str1, str2) strcmp((NULL!=str1)?(str1):"", (NULL!=str2)?(str2):"")
#endif
	
#ifndef STRNCMP
#define STRNCMP(str1, str2, n) strncmp((NULL!=str1)?(str1):"", (NULL!=str2)?(str2):"", n) 
#endif
	
#ifndef STRNCPY
#define STRNCPY(str1, str2, n) strncpy((NULL!=str1)?(str1):"", (NULL!=str2)?(str2):"", n) 
#endif
#ifdef _DEBUG
#define PRINT(format, arg...)	printf("%s():%d: -- "format, __func__, __LINE__, ##arg)
#else
#define PRINT(format, arg...)
#endif
/**************************************************************************
 *	
 *	        variable definition
 *
 **************************************************************************/
static time_t time_radio_changed[MAX_NUMBER_OF_ENTRIES_RADIO]={0};
static time_t time_ssid_changed[MAX_NUMBER_OF_ENTRIES_SSID]={0};


/**************************************************************************
 *	
 *	        static function declaration
 *
 **************************************************************************/
static unsigned int GetObjectFlag(char* objPathName, int* pIndex, int* pIndex2);
static unsigned int GetFrequencyBand(char* OperatingFrequencyBand);
static unsigned int GetStandard(unsigned int FrequencyBand, char* Standard);
static unsigned int GetChannelBandwidth(char* OperatingChannelBandwidth);
static unsigned int GetExtensionChannel(char* ExtensionChannel);
static unsigned int GetGuardInterval(char* GuardInterval);
static int SetSecurity(char* objPathName, PDEVICE_WIFI pNewData);
static int SetWPS(char* objPathName, PDEVICE_WIFI pNewData, unsigned int first);
static int DoKeyPassphraseChanged(char* keyPassphrase, char* associatedDeviceMACAddress, unsigned char *key, unsigned int key_len);
static int pkcs5_pbkdf2(unsigned char *pass, unsigned int pass_len, unsigned char *salt, unsigned int salt_len, unsigned char *key, unsigned int key_len, unsigned int rounds);
//static unsigned int Hex2String(unsigned char *hex, unsigned int hex_len, char *hexString);
static char* GetSSIDReferenceName(char *SSIDReference, PWIFI_SSID SSID, int* pIndex);
static unsigned int IsSSIDLowerLayersExist(char* SSIDLowerLayers, PWIFI_RADIO pRadio, int* pIndex);
static unsigned int IsValidHexadecimalString(char* key);

/**************************************************************************
 *	
 *	        static function definition
 *
 **************************************************************************/
static unsigned int GetObjectFlag(char* objPathName, int* pIndex, int* pIndex2)
{
	unsigned int flag = 0;
	int index = 0;
	int index2 = 0;
	unsigned int NumberOfEntries = 0;

	do{
		if(NULL==objPathName){
			PRINT("objPathName can not be null\n");
			break;
		}
		if(objPathName!=strstr(objPathName, PREFIX_PATH_WIFI)){
			PRINT("invalid objPathName [%s], it should be begin with '%s'\n", objPathName, PREFIX_PATH_WIFI);
			break;
		}
		if(strlen(PREFIX_PATH_WIFI)==strlen(objPathName)){
			flag = OBJ_FLAG_WIFI;
			break;
		}
		
		if(objPathName==strstr(objPathName, PREFIX_PATH_RADIO)){
			NumberOfEntries = wifi_get_RadioNumberOfEntries();
			if(NumberOfEntries>MAX_NUMBER_OF_ENTRIES_RADIO){
				NumberOfEntries = MAX_NUMBER_OF_ENTRIES_RADIO;
			}
			index = atoi(objPathName+strlen(PREFIX_PATH_RADIO)+1);
			if(0==index||index>NumberOfEntries){
				PRINT("The instance number (%d) of objPathName [%s] is invalid, it should be between 1 and %d\n", 
					index, objPathName, NumberOfEntries);
				index = 0;
				break;
			}
			if(strlen(PREFIX_PATH_RADIO)+2+index/10==strlen(objPathName)){
				flag = OBJ_FLAG_WIFI_RADIO;
				break;
			}
			if(NULL!=strstr(objPathName, TOKEN_PATH_STATS)
				&&strlen(objPathName)==strlen(PREFIX_PATH_RADIO)+strlen(TOKEN_PATH_STATS)+2+index/10){
				flag = OBJ_FLAG_WIFI_RADIO_STATISTICS;
				break;
			}
			break;
		}
		
		if(objPathName==strstr(objPathName, PREFIX_PATH_SSID)){
			NumberOfEntries = wifi_get_SSIDNumberOfEntries();
			if(NumberOfEntries>MAX_NUMBER_OF_ENTRIES_SSID){
				NumberOfEntries = MAX_NUMBER_OF_ENTRIES_SSID;
			}
			index = atoi(objPathName+strlen(PREFIX_PATH_SSID)+1);
			if(0==index||index>NumberOfEntries){
				PRINT("The instance number (%d) of objPathName [%s] is invalid, it should be between 1 and %d\n", 
					index, objPathName, NumberOfEntries);
				index = 0;
				break;
			}
			if(strlen(PREFIX_PATH_SSID)+2+index/10==strlen(objPathName)){
				flag = OBJ_FLAG_WIFI_SSID;
				break;
			}
			if(NULL!=strstr(objPathName, TOKEN_PATH_STATS)
				&&strlen(objPathName)==strlen(PREFIX_PATH_SSID)+strlen(TOKEN_PATH_STATS)+2+index/10){
				flag = OBJ_FLAG_WIFI_SSID_STATISTICS;
				break;
			}
			break;
		}
		
		if(objPathName==strstr(objPathName, PREFIX_PATH_ACCESSPOINT)){
			NumberOfEntries = wifi_get_AccessPointNumberOfEntries();
			if(NumberOfEntries>MAX_NUMBER_OF_ENTRIES_ACCESSPOINT){
				NumberOfEntries = MAX_NUMBER_OF_ENTRIES_ACCESSPOINT;
			}
			index = atoi(objPathName+strlen(PREFIX_PATH_ACCESSPOINT)+1);
			if(0==index||index>NumberOfEntries){
				PRINT("The instance number (%d) of objPathName [%s] is invalid, it should be between 1 and %d\n", 
					index, objPathName, NumberOfEntries);
				index = 0;
				break;
			}
			if(strlen(PREFIX_PATH_ACCESSPOINT)+2+index/10==strlen(objPathName)){
				flag = OBJ_FLAG_WIFI_ACCESSPOINT;
				break;
			}
			if(NULL!=strstr(objPathName, TOKEN_PATH_SECURITY)
				&&strlen(objPathName)==strlen(PREFIX_PATH_ACCESSPOINT)+strlen(TOKEN_PATH_SECURITY)+2+index/10){
				flag = OBJ_FLAG_WIFI_ACCESSPOINT_SECURITY;
				break;
			}
			if(NULL!=strstr(objPathName, TOKEN_PATH_ACCOUNTING)
				&&strlen(objPathName)==strlen(PREFIX_PATH_ACCESSPOINT)+strlen(TOKEN_PATH_ACCOUNTING)+2+index/10){
				flag = OBJ_FLAG_WIFI_ACCESSPOINT_ACCOUNTING;
				break;
			}
			if(NULL!=strstr(objPathName, TOKEN_PATH_WPS)
				&&strlen(objPathName)==strlen(PREFIX_PATH_ACCESSPOINT)+strlen(TOKEN_PATH_WPS)+2+index/10){
				flag = OBJ_FLAG_WIFI_ACCESSPOINT_WPS;
				break;
			}
			if(NULL!=strstr(objPathName, TOKEN_PATH_ASSOCIATEDDEVICE)){
				index2 = atoi(objPathName+strlen(PREFIX_PATH_ACCESSPOINT)+2+index/10+strlen(TOKEN_PATH_ASSOCIATEDDEVICE));
				if(0==index2||index2>MAX_NUMBER_OF_ENTRIES_ASSOCIATEDDEVICES){
					PRINT("The instance number (%d) of objPathName [%s] is invalid, it should be between 1 and %d\n", 
						index2, objPathName, MAX_NUMBER_OF_ENTRIES_ASSOCIATEDDEVICES);
					index2 = 0;
					break;
				}
				flag = OBJ_FLAG_WIFI_ACCESSPOINT_ASSOCIATEDDEVICE;
				break;
			}
			break;
		}
		//other flags will be implemented in next version
		if(objPathName==strstr(objPathName, PREFIX_PATH_ENDPOINT)){
			NumberOfEntries = wifi_get_EndPointNumberOfEntries();
			if(NumberOfEntries>MAX_NUMBER_OF_ENTRIES_ENDPOINT){
				NumberOfEntries = MAX_NUMBER_OF_ENTRIES_ENDPOINT;
			}
			index = atoi(objPathName+strlen(PREFIX_PATH_ENDPOINT)+1);
			if(0==index||index>NumberOfEntries){
				PRINT("The instance number (%d) of objPathName [%s] is invalid, it should be between 1 and %d\n", 
					index, objPathName, NumberOfEntries);
				index = 0;
				break;
			}
			break;
		}
	}while(0);

	if(NULL!=pIndex){
		*pIndex = index-1;
	}

	if(NULL!=pIndex2){
		*pIndex2 = index2-1;
	}

	return flag;
}

static unsigned int GetFrequencyBand(char* OperatingFrequencyBand)
{
	unsigned int FrequencyBand = 0;

	do{
		if(!strcmp(STR013_BAND_2G, OperatingFrequencyBand)){
			FrequencyBand = BAND_2G;
		}else if(!strcmp(STR013_BAND_5G, OperatingFrequencyBand)){
			FrequencyBand = BAND_5G;
		}else{
			PRINT("Invalid OperatingFrequencyBand [%s]\n", OperatingFrequencyBand);
			break;
		}
	}while(0);

	return FrequencyBand;
}

static unsigned int GetStandard(unsigned int FrequencyBand, char* Standard)
{
	unsigned int standard = 0;
	
	if(NULL!=strstr(Standard, "n")){
		standard = MODE_80211_N;
	}
	if(BAND_2G==FrequencyBand){
		if(NULL!=strstr(Standard, "g")){
			standard |= MODE_80211_G;
		}
		if(NULL!=strstr(Standard, "b")){
			standard |= MODE_80211_B;
		}
	}else{
		if(NULL!=strstr(Standard, "a")){
			standard |= MODE_80211_A;
		}
	}

	return standard;
}

static unsigned int GetChannelBandwidth(char* OperatingChannelBandwidth)
{
	unsigned int channelBandwidth = 0;
	
	if(!strcmp(STR023_BANDWIDTH_20MHz, OperatingChannelBandwidth)){
		channelBandwidth = CHANNEL_WIDTH_20MHZ;
	}else if(!strcmp(STR023_BANDWIDTH_40MHz, OperatingChannelBandwidth)){
		channelBandwidth = CHANNEL_WIDTH_40MHZ;
	}else{
		channelBandwidth = CHANNEL_WIDTH_AUTO;
		sprintf(OperatingChannelBandwidth, "%s", STR023_BANDWIDTH_Auto);
	}

	return channelBandwidth;
}

static unsigned int GetExtensionChannel(char* ExtensionChannel)
{
	unsigned int extensionChannel = 0;
	
	if(!strcmp(STR024_CONTROL_CHANNEL_ABOVE, ExtensionChannel)){
		extensionChannel = CONTROL_CHANNEL_ABOVE;
	}else if(!strcmp(STR024_CONTROL_CHANNEL_BELOW, ExtensionChannel)){
		extensionChannel = CONTROL_CHANNEL_BELOW;
	}else{
		extensionChannel = CONTROL_CHANNEL_AUTO;
		sprintf(ExtensionChannel, "%s", STR024_CONTROL_CHANNEL_AUTO);
	}

	return extensionChannel;
}

static unsigned int GetGuardInterval(char* GuardInterval)
{
	unsigned int guardInterval = 0;
	
	if(!strcmp(STR025_GUARD_INTERVAL_400, GuardInterval)){
		guardInterval = GUARD_INTERVAL_400;
	}else if(!strcmp(STR025_GUARD_INTERVAL_800, GuardInterval)){
		guardInterval = GUARD_INTERVAL_800;
	}else{
		guardInterval = GUARD_INTERVAL_AUTO;
		sprintf(GuardInterval, "%s", STR025_GUARD_INTERVAL_AUTO);
	}

	return guardInterval;
}

static unsigned int GetWPAEncryptionMode(char* EncryptionMode)
{
	unsigned int encryptionMode = 0;
	
	if (!STRCMP(EncryptionMode, X003_WPA_ENCRYPTION_MODES_TKIP)){
		encryptionMode = WPA_ENCRYPTION_MODES_TKIP;
	}
	else if (!STRCMP(EncryptionMode, X003_WPA_ENCRYPTION_MODES_AES)){
		encryptionMode = WPA_ENCRYPTION_MODES_AES;
	}else{
		sprintf(EncryptionMode, "%s", X003_WPA_ENCRYPTION_MODES_TKIP_AES);
		encryptionMode = WPA_ENCRYPTION_MODES_TKIP_AES;
	}

	return encryptionMode;
}

static int SetSecurity(char* objPathName, PDEVICE_WIFI pNewData)
{
	int ret = 0;
	unsigned int objFlag = 0;
	int index;
	char* devName = NULL;
	unsigned int authenticationMode = 0;
	unsigned int WEPKeyType[4] = {WEPKEY_TYPE_HEX, WEPKEY_TYPE_HEX, WEPKEY_TYPE_HEX, WEPKEY_TYPE_HEX};
	char* WEPKey[4] = {0};
	int wpa;
	unsigned int wpaEncryptionModes = 0;
	unsigned int wpa2EncryptionModes = 0;
	char key[SIZE_64+1] = {0};

	do {
		objFlag = GetObjectFlag(objPathName, &index, NULL);
		if(0==objFlag){
			PRINT("objPathName [%s] is invalid.\n", objPathName);
			ret = -1;
			break;
		}

		if(OBJ_FLAG_WIFI_ACCESSPOINT!=objFlag&&OBJ_FLAG_WIFI_ACCESSPOINT_SECURITY!=objFlag){
			PRINT("objPathName [%s] is invalid.\n", objPathName);
			ret = -1;
			break;
		}

		devName = GetSSIDReferenceName(pNewData->AccessPoint[index].SSIDReference,pNewData->SSID, NULL);
		if(NULL==devName){
			PRINT("Fail to GetSSIDReferenceName by [%s].\n", pNewData->AccessPoint[index].SSIDReference);
			ret = -1;
			break;
		}
		//077 - Reset
		if(pNewData->AccessPoint[index].Security.Reset){
			//ResetSecurity()
			pNewData->AccessPoint[index].Security.Reset = 0;
		}
		//078 - ModesSupported
		
		//079 - ModeEnabled
		if (!STRCMP(pNewData->AccessPoint[index].Security.ModeEnabled, STR078_SECURITY_MODE_None)){//Off - No Encryption
		//X01 - extRadiusEnabled
			if(pNewData->AccessPoint[index].Security.extRadiusEnabled){
				ret = wifi_set_security_8021x(devName, pNewData->AccessPoint[index].Security.RadiusServerIPAddr,
					pNewData->AccessPoint[index].Security.RadiusServerPort, pNewData->AccessPoint[index].Security.RadiusSecret);
			}else{
				ret = wifi_set_security_off(devName);
			}
			break;
		}

		if ((!STRCMP(pNewData->AccessPoint[index].Security.ModeEnabled, STR078_SECURITY_MODE_WEP64))
			||(!STRCMP(pNewData->AccessPoint[index].Security.ModeEnabled, STR078_SECURITY_MODE_WEP128))){//WEP
			if(pNewData->AccessPoint[index].Security.extRadiusEnabled){//WEP+8021X
				ret = wifi_set_security_wep_8021x(devName, pNewData->AccessPoint[index].Security.RadiusServerIPAddr, 
					pNewData->AccessPoint[index].Security.RadiusServerPort, pNewData->AccessPoint[index].Security.RadiusSecret);
			}else{//WEP only
				//080 - WEPKey
				if(((!STRCMP(pNewData->AccessPoint[index].Security.ModeEnabled, STR078_SECURITY_MODE_WEP64))&&(10!=strlen(pNewData->AccessPoint[index].Security.WEPKey)))
					||((!STRCMP(pNewData->AccessPoint[index].Security.ModeEnabled, STR078_SECURITY_MODE_WEP128))&&(26!=strlen(pNewData->AccessPoint[index].Security.WEPKey)))){
					PRINT("The length (%d) of WEPKey is invalid.\n", strlen(pNewData->AccessPoint[index].Security.WEPKey));
					ret = -1;
					break;
				}
				if(!IsValidHexadecimalString(pNewData->AccessPoint[index].Security.WEPKey)){
					PRINT("WEPKey [%s] is invalid hexadecimal string.\n", pNewData->AccessPoint[index].Security.WEPKey);
					ret = -1;
					break;
				}
				strcpy(key, pNewData->AccessPoint[index].Security.WEPKey);
				WEPKey[0] = WEPKey[1] = WEPKey[2] = WEPKey[3] = key;
				
				if (!STRCMP(pNewData->AccessPoint[index].Security.extWEPAuthenticationMode, X02_WEP_AUTHENTICATION_MODE_OPEN)){//WEP+OPEN
					authenticationMode = AUTHENTICATION_MODE_OPEN;
				}
				else if (!STRCMP(pNewData->AccessPoint[index].Security.extWEPAuthenticationMode, X02_WEP_AUTHENTICATION_MODE_SHARED)){//WEP+SHARE
					authenticationMode = AUTHENTICATION_MODE_SHARED;
				}else{
					authenticationMode = AUTHENTICATION_MODE_AUTO;
					snprintf(pNewData->AccessPoint[index].Security.extWEPAuthenticationMode, sizeof(pNewData->AccessPoint[index].Security.extWEPAuthenticationMode),
						"%s", X02_WEP_AUTHENTICATION_MODE_AUTO);
				}
				ret = wifi_set_security_wep_only(devName, WEPKeyType, WEPKey, 1, key, authenticationMode);
			}
			break;
		}

		if ((!STRCMP(pNewData->AccessPoint[index].Security.ModeEnabled, STR078_SECURITY_MODE_WPA_Personal))
			||(!STRCMP(pNewData->AccessPoint[index].Security.ModeEnabled, STR078_SECURITY_MODE_WPA2_Personal))
			||(!STRCMP(pNewData->AccessPoint[index].Security.ModeEnabled, STR078_SECURITY_MODE_MIXED_Personal))){//WPA+PSK
			//081 - PreSharedKey
			if(strlen(pNewData->AccessPoint[index].Security.PreSharedKey)<SIZE_8
				||strlen(pNewData->AccessPoint[index].Security.PreSharedKey)>SIZE_64){
				PRINT("The length (%d) of PreSharedKey is invalid.\n", strlen(pNewData->AccessPoint[index].Security.PreSharedKey));
				ret = -1;
				break;
			}
			if(SIZE_64==strlen(pNewData->AccessPoint[index].Security.PreSharedKey)&&!IsValidHexadecimalString(pNewData->AccessPoint[index].Security.PreSharedKey)){
				PRINT("PreSharedKey [%s] is invalid hexadecimal string.\n", pNewData->AccessPoint[index].Security.PreSharedKey);
				ret = -1;
				break;
			}
			strcpy(key, pNewData->AccessPoint[index].Security.PreSharedKey);

			if (!STRCMP(pNewData->AccessPoint[index].Security.ModeEnabled, STR078_SECURITY_MODE_WPA_Personal)){
				wpa = WPA_MODE_WPA;
			}else if(!STRCMP(pNewData->AccessPoint[index].Security.ModeEnabled, STR078_SECURITY_MODE_WPA2_Personal)){
				wpa = WPA_MODE_WPA2;
			}else{
				wpa = WPA_MODE_MIXED;
			}
			//X003 - extWPAEncryptionMode
			wpaEncryptionModes = GetWPAEncryptionMode(pNewData->AccessPoint[index].Security.extWPAEncryptionMode);
			//X004 - extWPA2EncryptionMode
			wpa2EncryptionModes = GetWPAEncryptionMode(pNewData->AccessPoint[index].Security.extWPA2EncryptionMode);
			//083 - RekeyingInterval
			ret = wifi_set_security_wpa_psk(devName, wpa, wpaEncryptionModes, wpa2EncryptionModes,
				key, pNewData->AccessPoint[index].Security.RekeyingInterval);
			break;
		}

		if ((!STRCMP(pNewData->AccessPoint[index].Security.ModeEnabled, STR078_SECURITY_MODE_WPA_Enterprise))
			||(!STRCMP(pNewData->AccessPoint[index].Security.ModeEnabled, STR078_SECURITY_MODE_WPA2_Enterprise))
			||(!STRCMP(pNewData->AccessPoint[index].Security.ModeEnabled, STR078_SECURITY_MODE_MIXED_Enterprise))){//WPA+Enterprise
			
			if (!STRCMP(pNewData->AccessPoint[index].Security.ModeEnabled, STR078_SECURITY_MODE_WPA_Enterprise)){
				wpa = WPA_MODE_WPA;
			}else if(!STRCMP(pNewData->AccessPoint[index].Security.ModeEnabled, STR078_SECURITY_MODE_WPA2_Enterprise)){
				wpa = WPA_MODE_WPA2;
			}else{
				wpa = WPA_MODE_MIXED;
			}
			//X003 - extWPAEncryptionMode
			wpaEncryptionModes = GetWPAEncryptionMode(pNewData->AccessPoint[index].Security.extWPAEncryptionMode);
			//X004 - extWPA2EncryptionMode
			wpa2EncryptionModes = GetWPAEncryptionMode(pNewData->AccessPoint[index].Security.extWPA2EncryptionMode);
			//083 - RekeyingInterval
			//084 - RadiusServerIPAddr
			//086 - RadiusServerPort
			//088 - RadiusSecret
			ret = wifi_set_security_wpa_eap(devName, wpa, wpaEncryptionModes, wpa2EncryptionModes,
				pNewData->AccessPoint[index].Security.RadiusServerIPAddr, pNewData->AccessPoint[index].Security.RadiusServerPort,
				pNewData->AccessPoint[index].Security.RadiusSecret, pNewData->AccessPoint[index].Security.RekeyingInterval);
			break;
		}
	} while(0);
		
	return ret;
}

static int SetWPS(char* objPathName, PDEVICE_WIFI pNewData, unsigned int first)
{
	int ret = 0;
//	unsigned int eap_server = 0;
//	unsigned int wps_state=WPS_STATE_DISABLED;
//	unsigned int wpa_psk = 0;
//	int i;
/*
	do{
		STRNCPY(pNewData->WPS.LastConfigurationError, 
			STR071_LAST_CONFIGURATION_ERROR_NO_ERROR, 
			sizeof(pNewData->WPS.LastConfigurationError)-1);
		//074 - Enable
		if(pNewData->WPS.Registrar[0].Enable){
			eap_server = 1;//Enable internal EAP server for EAP-WSC
		}else{
			eap_server = 0;
			STRNCPY(pNewData->WPS.LastConfigurationError, 
				STR071_LAST_CONFIGURATION_ERROR_COULDNT_CONNECT_TO_REGISTRAR, 
				sizeof(pNewData->WPS.LastConfigurationError)-1);
		}
		//072 - RegistrarNumberOfEntries
		pNewData->WPS.RegistrarNumberOfEntries = 0;
		for(i=0; i<MAX_COUNT_WPS_REGISTRAR; i++){
			if(pNewData->WPS.Registrar[i].Enable){
				pNewData->WPS.RegistrarNumberOfEntries++;
			}
		}

		//073 - RegistrarEstablished		
		pNewData->WPS.RegistrarEstablished = (pNewData->WPS.RegistrarNumberOfEntries>0);
		wpa_psk = (((!STRCMP(pNewData->WPAAuthenticationMode, STR018_WPA_AUTHENTICATION_MODE_PSK))
			&&(!STRCMP(pNewData->BeaconType, STR009_BEACON_TYPE_WPA)))
			||((!STRCMP(pNewData->IEEE11iAuthenticationMode, STR020_IEEE11I_AUTHENTICATION_MODE_PSK))
			&&((!STRCMP(pNewData->BeaconType, STR009_BEACON_TYPE_11I))
			||(!STRCMP(pNewData->BeaconType, STR009_BEACON_TYPE_WPA11I)))));
		//070 - ConfigurationState 
		if(wpa_psk&&pNewData->WPS.RegistrarEstablished){
			strcpy(pNewData->WPS.ConfigurationState, STR070_CONFIGURATION_STATE_CONFIGURED);
		}else{
			strcpy(pNewData->WPS.ConfigurationState, STR070_CONFIGURATION_STATE_NOT_CONFIGURED);
		}
		//061 - Enable
		if(pNewData->WPS.Enable){
			if(!STRCMP(pNewData->WPS.ConfigurationState, STR070_CONFIGURATION_STATE_CONFIGURED)){
				wps_state=WPS_STATE_CONFIGURED;
			}else{
				wps_state=WPS_STATE_NOT_CONFIGURED;
			}
		}else{
			wps_state=WPS_STATE_DISABLED;
		}
		//069 - SetupLock
		//SetupLockedState
		if(pNewData->WPS.SetupLock){
			STRNCPY(pNewData->WPS.SetupLockedState, 
				STR068_SETUP_LOCK_STATE_LOCKED_BY_REMOTE_MANAGEMENT, 
				sizeof(pNewData->WPS.SetupLockedState)-1);
			STRNCPY(pNewData->WPS.LastConfigurationError, 
				STR071_LAST_CONFIGURATION_ERROR_SETUP_LOCKED, 
				sizeof(pNewData->WPS.LastConfigurationError)-1);
		}
		else
		{
			STRNCPY(pNewData->WPS.SetupLockedState, 
				STR068_SETUP_LOCK_STATE_UNLOCKED, 
				sizeof(pNewData->WPS.SetupLockedState)-1);
		}
		//062 - DeviceName
		//063 - DevicePassword
		wifi_config_wps(pNewData->Name, eap_server, wps_state, pNewData->WPS.DeviceName, pNewData->WPS.SetupLock, pNewData->WPS.DevicePassword, pNewData->Enable);
		
		if(!pNewData->WPS.Enable){
			break;
		}
		
		if(first){//create
			STRNCPY(pNewData->WPS.ConfigMethodsEnabled , STR067_CONFIG_METHODS_ENABLED_LABEL, sizeof(pNewData->WPS.ConfigMethodsEnabled)-1);
			break;
		}

		//067 - ConfigMethodsEnabled
		if(!STRCMP(pNewData->WPS.ConfigMethodsEnabled , STR067_CONFIG_METHODS_ENABLED_PUSHBUTTON)){
			wifi_set_wps_push_button(pNewData->Name);
			break;
		}
		
		if(!STRCMP(pNewData->WPS.ConfigMethodsEnabled , STR067_CONFIG_METHODS_ENABLED_KEYPAD)){//DevicePassword
			wifi_set_wps_pin(pNewData->Name, pNewData->WPS.DevicePassword, pNewData->extWpsPinTimeout);
			break;
		}
		
		if(!STRCMP(pNewData->WPS.ConfigMethodsEnabled , STR067_CONFIG_METHODS_ENABLED_DISPLAY)){
			pNewData->WPS.DevicePassword = wifi_set_wps_ap_pin_random(pNewData->Name, pNewData->extWPSApPinTimeout);
			break;
		}
		
		if(!STRCMP(pNewData->WPS.ConfigMethodsEnabled , STR067_CONFIG_METHODS_ENABLED_LABEL)){//restore default
			pNewData->WPS.DevicePassword = pNewData->extWPSApPinDefault;
			wifi_set_wps_ap_pin_static(pNewData->Name, pNewData->WPS.DevicePassword, pNewData->extWPSApPinTimeout);
			break;
		}
	}while(0);
*/
	return ret;
}

static int DoKeyPassphraseChanged(char* keyPassphrase, char* associatedDeviceMACAddress, unsigned char *key, unsigned int key_len)
{
	char salt[LEN_MAC_ADDRESS+1]={0};

	if(LEN_MAC_ADDRESS==strlen(associatedDeviceMACAddress)){
		strcpy(salt, associatedDeviceMACAddress);
	}else{
		strcpy(salt, DEFAULT_SALT_MAC);
	}

	return pkcs5_pbkdf2(keyPassphrase, strlen(keyPassphrase), salt, LEN_MAC_ADDRESS, key, key_len, 4096);
}

/*
 * Password-Based Key Derivation Function 2 (PKCS #5 v2.0).
 * Code based on IEEE Std 802.11-2007, Annex H.4.2.
 */
static int pkcs5_pbkdf2(unsigned char *pass, unsigned int pass_len, 
	unsigned char *salt, unsigned int salt_len, unsigned char *key, 
	unsigned int key_len, unsigned int rounds)
{
#define SIZE_MAX 20
#define MIN(a, b) ((a)<(b))?(a):(b)
	unsigned char *asalt, obuf[SHA1_DIGEST_LENGTH];
	unsigned char d1[SHA1_DIGEST_LENGTH], d2[SHA1_DIGEST_LENGTH];
	unsigned int i, j;
	unsigned int count;
	unsigned int r;

	if (rounds < 1 || key_len == 0)
		return -1;
	if (salt_len == 0 || salt_len > SIZE_MAX - 1)
		return -1;
	if ((asalt = malloc(salt_len + 4)) == NULL)
		return -1;

	memcpy(asalt, salt, salt_len);

	for (count = 1; key_len > 0; count++) {
		asalt[salt_len + 0] = (count >> 24) & 0xff;
		asalt[salt_len + 1] = (count >> 16) & 0xff;
		asalt[salt_len + 2] = (count >> 8) & 0xff;
		asalt[salt_len + 3] = count & 0xff;
		hmac_sha1(asalt, salt_len + 4, pass, pass_len, d1);
		memcpy(obuf, d1, sizeof(obuf));

		for (i = 1; i < rounds; i++) {
			hmac_sha1(d1, sizeof(d1), pass, pass_len, d2);
			memcpy(d1, d2, sizeof(d1));
			for (j = 0; j < sizeof(obuf); j++)
				obuf[j] ^= d1[j];
		}

		r = MIN(key_len, SHA1_DIGEST_LENGTH);
		memcpy(key, obuf, r);
		key += r;
		key_len -= r;
	};
	bzero(asalt, salt_len + 4);
	free(asalt);
	bzero(d1, sizeof(d1));
	bzero(d2, sizeof(d2));
	bzero(obuf, sizeof(obuf));

	return 0;
}
/*
static unsigned int Hex2String(unsigned char *hex, unsigned int hex_len, char *hexString)
{//0x01 0xFF 0x02 ==> "01FF02"
	int i, j;
	unsigned char data[2];
	
	if(NULL==hex||0==hex_len||NULL==hexString){
		return 0;
	}

	for(i=hex_len-1; i>=0; i--){
		data[1] = hex[i]&0x0F;
		data[0] = hex[i] >> 4;
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

	return (hex_len<<1);
}
*/
static char* GetSSIDReferenceName(char *SSIDReference, PWIFI_SSID SSID, int* pIndex)
{
	char* p = NULL;
	unsigned int index = 0;

	do{
		if(NULL==SSIDReference||NULL==SSID){
			PRINT("SSIDReference or SSID can not be null\n");
			break;
		}
		if(SSIDReference!=strstr(SSIDReference, PREFIX_PATH_SSID)){
			PRINT("SSIDReference [%s] must begin with %s\n", SSIDReference, PREFIX_PATH_SSID);
			break;
		}
		index = atoi(SSIDReference+strlen(PREFIX_PATH_SSID)+1);
		if(0==index||index>MAX_NUMBER_OF_ENTRIES_SSID){
			PRINT("The index (%d) of SSIDReference [%s] is invalid\n", index, SSIDReference);
			break;
		}
		if(0==SSID[index-1].Name[0]){
			PRINT("The name of SSIDReference [%s] is invalid\n", SSIDReference);
			break;
		}
		p = SSID[index-1].Name;
		if(pIndex){
			*pIndex = index -1;
		}
	}while(0);

	return p;
}

static unsigned int IsSSIDLowerLayersExist(char* SSIDLowerLayers, PWIFI_RADIO pRadio, int* pIndex)
{
	unsigned int exist = 0;

	do{
		if(OBJ_FLAG_WIFI_RADIO!=GetObjectFlag(SSIDLowerLayers, pIndex, NULL)){
			PRINT("LowerLayers [%s] is invalid.\n", SSIDLowerLayers);
			break;
		}

		exist = wifi_device_is_exist(pRadio[*pIndex].Name);
	}while(0);

	return exist;
}

static unsigned int IsValidHexadecimalString(char* key)
{
	unsigned int valid = 1;
	int i;

	do{
		if(NULL==key||0==strlen(key)){
			PRINT("The key [%s] is invalid hexadecimal string.\n", key);
			valid = 0;
			break;
		}
		
		if(0!=(strlen(key)&0x01)){
			PRINT("The key [%s] is invalid hexadecimal string.\n", key);
			valid = 0;
			break;
		}

		for(i=0; i<strlen(key); i++){
			if((key[i]<'0'||key[i]>'9')
				&&(key[i]<'a'||key[i]>'f')
				&&(key[i]<'A'||key[i]>'F')){
				PRINT("The key [%s] is invalid hexadecimal string.\n", key);
				valid = 0;
				break;
			}
		}
	}while(0);

	return valid;
}

/**************************************************************************
 *	
 *	        interface function definition
 *
 **************************************************************************/

int tr181WiFiObjectDestroy(char* objPathName, PDEVICE_WIFI pCurData)
{
	int ret = 0;
	int index, i;
	unsigned int objFlag = 0;

	do{
		objFlag = GetObjectFlag(objPathName, &index, NULL);
		if(0==objFlag){
			ret = -1;
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ACCESSPOINT){
			objFlag = GetObjectFlag(pCurData->AccessPoint[index].SSIDReference, &i, NULL);
			if(OBJ_FLAG_WIFI_SSID==objFlag){
				ret = wifi_destroy_ssid_device(pCurData->SSID[i].Name);
			}
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_RADIO){
			ret += wifi_destroy_radio_device(pCurData->Radio[index].Name);
		}
	}while(0);	

	return ret;
}

int tr181WiFiObjectCreate(char* objPathName, PDEVICE_WIFI pNewData)
{
	int ret = 0;
	unsigned int objFlag = 0;
	int i, j;
	char name[SIZE_32];
	unsigned int FrequencyBand = 0;
	unsigned int standard = 0;
	unsigned int channelBandwidth = 0;
	unsigned int extensionChannel = 0;
	unsigned int guardInterval = 0;
	int index_radio = -1;
	int index_ssid = -1;

	do{
		objFlag = GetObjectFlag(objPathName, &i, &j);
		if(0==objFlag){
			ret = -1;
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI){
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_RADIO_STATISTICS){
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_SSID){
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_SSID_STATISTICS){
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ACCESSPOINT_SECURITY){
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ACCESSPOINT_ACCOUNTING){
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ACCESSPOINT_WPS){
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ACCESSPOINT_ASSOCIATEDDEVICE){
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ENDPOINT_STATISTICS){
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ENDPOINT_SECURITY){
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ENDPOINT_PROFILE){
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ENDPOINT_PROFILE_SECURITY){
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ENDPOINT_WPS){
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_RADIO){
			if(pNewData->Radio[i].Name[0]){
				wifi_destroy_radio_device(pNewData->Radio[i].Name);
			}
			//014 - OperatingFrequencyBand
			FrequencyBand = GetFrequencyBand(pNewData->Radio[i].OperatingFrequencyBand);
			if(0==FrequencyBand){
				ret = -1;
				break;
			}

			ret = wifi_create_radio_device(FrequencyBand, name);
			if(ret<0){
				break;
			}
			//008 - Name
			snprintf(pNewData->Radio[i].Name, sizeof(pNewData->Radio[i].Name), "%s", name);
			wifi_close_radio(name);
			//009 - LastChange
			time_radio_changed[i] = CURRENT_TIME;
			//010 - LowerLayers :Since Radio is a layer 1 interface, it is expected that LowerLayers will not be used
			memset(pNewData->Radio[i].LowerLayers, 0, sizeof(pNewData->Radio[i].LowerLayers));
			//016 - OperatingStandards
			standard = GetStandard(FrequencyBand, pNewData->Radio[i].OperatingStandards);
			if(0==standard){
				ret = -1;
				break;
			}
			wifi_set_radio_standards(name, standard);
			//023 - OperatingChannelBandwidth
			channelBandwidth = GetChannelBandwidth(pNewData->Radio[i].OperatingChannelBandwidth);
			//024 - ExtensionChannel
			extensionChannel = GetExtensionChannel(pNewData->Radio[i].ExtensionChannel);
			//019 - Channel
			//021 - AutoChannelEnable
			//022 - AutoChannelRefreshPeriod
			ret = wifi_set_radio_channel_info(name, channelBandwidth, extensionChannel,
				pNewData->Radio[i].AutoChannelEnable,
				pNewData->Radio[i].AutoChannelRefreshPeriod,
				pNewData->Radio[i].Channel);
			if(ret<0){
				break;
			}
			//025 - GuardInterval
			guardInterval = GetGuardInterval(pNewData->Radio[i].GuardInterval);
			wifi_set_radio_GuardInterval(name, guardInterval);
			//026 - MCS
			wifi_set_radio_MCS(name, pNewData->Radio[i].MCS);
			//028 - TransmitPower
			wifi_set_radio_TransmitPower(name, pNewData->Radio[i].TransmitPower);
			//030 - IEEE80211hEnabled
			wifi_set_radio_IEEE80211hEnabled(name, pNewData->Radio[i].IEEE80211hEnabled);			
			//031 - RegulatoryDomain
			wifi_set_radio_RegulatoryDomain(name, pNewData->Radio[i].RegulatoryDomain);
			//005 - Enable
			if(pNewData->Radio[i].Enable){
				wifi_open_radio(name);
			}
			wifi_commit(name);
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ACCESSPOINT){
			//067 - SSIDReference
			if(OBJ_FLAG_WIFI_SSID!=GetObjectFlag(pNewData->AccessPoint[i].SSIDReference, &index_ssid, NULL)){
				PRINT("SSIDReference [%s] is invalid.\n", pNewData->AccessPoint[i].SSIDReference);
				ret = -1;
				break;
			}
			if(0!=pNewData->SSID[index_ssid].Name[0]){
				wifi_destroy_ssid_device(pNewData->SSID[index_ssid].Name);
			}
			//045 - LowerLayers
			if(!IsSSIDLowerLayersExist(pNewData->SSID[index_ssid].LowerLayers, pNewData->Radio, &index_radio)){
				PRINT("LowerLayers [%s] is invalid.\n", pNewData->SSID[index_ssid].LowerLayers);
				ret = -1;
				break;
			}
			ret = wifi_create_ssid_device(pNewData->Radio[index_radio].Name, DEVICE_OPERATION_MODE_ACCESSPOINT, name);
			if(ret<0){
				break;
			}
			//043 - Name
			STRNCPY(pNewData->SSID[index_ssid].Name, name, sizeof(pNewData->SSID[index_ssid].Name)-1);
			//040 - Enable
			pNewData->SSID[index_ssid].Enable = pNewData->AccessPoint[i].Enable;
			wifi_close_device(pNewData->SSID[index_ssid].Name);
			//041 - Status
			//042 - Alias
			snprintf(pNewData->SSID[index_ssid].Alias, sizeof(pNewData->SSID[index_ssid].Alias),
				"%s", pNewData->AccessPoint[i].Alias);
			//044 - LastChange
			time_ssid_changed[index_ssid] = CURRENT_TIME;
			//046 - BSSID
			//047 - MACAddress
			//048 - SSID
			if (0==pNewData->SSID[index_ssid].SSID[0]){
				if(wifi_get_default_ssid(pNewData->Radio[index_radio].Name, name, pNewData->SSID[index_ssid].SSID)<0){
					sprintf(pNewData->SSID[index_ssid].SSID, "%.08X", (unsigned int)time(0));
				}
			}	
			wifi_set_ssid(pNewData->SSID[index_ssid].Name, pNewData->SSID[index_ssid].SSID);

			//065 - Status
			//066 - Alias
			//068 - SSIDAdvertisementEnabled
			wifi_set_SSIDAdvertisementEnabled(pNewData->SSID[index_ssid].Name, pNewData->AccessPoint[i].SSIDAdvertisementEnabled);
			//069 - RetryLimit
			if(pNewData->AccessPoint[i].RetryLimit>MAX_RETRY_LIMIT){
				PRINT("Invalid RetryLimit value!\n");
				ret = -1;
				break;
			}
			wifi_set_ShortRetryLimit(pNewData->SSID[index_ssid].Name, pNewData->AccessPoint[i].RetryLimit);
			//070 - WMMCapability
			//071 - UAPSDCapability
			//072 - WMMEnable
			wifi_set_WMMEnable(pNewData->SSID[index_ssid].Name, pNewData->AccessPoint[i].WMMEnable);
			//073 - UAPSDEnable
			wifi_set_UAPSDEnable(pNewData->SSID[index_ssid].Name, pNewData->AccessPoint[i].UAPSDEnable);
			//074 - AssociatedDeviceNumberOfEntries
			//075 - MaxAssociatedDevices
			wifi_set_MaxAssociatedDevices(pNewData->SSID[index_ssid].Name, pNewData->AccessPoint[i].MaxAssociatedDevices);
			//076 - IsolationEnable
			wifi_set_IsolationEnable(pNewData->SSID[index_ssid].Name, pNewData->AccessPoint[i].IsolationEnable);

			if(0==pNewData->AccessPoint[i].Security.PreSharedKey[0]
				||0==pNewData->AccessPoint[i].Security.WEPKey[0]){
				char defaultWEPKey[SIZE_26+1]={0};
				char defaultPreSharedKey[SIZE_64+1]={0};
#if defined( AEI_WECB_CUSTOMER_NCS)
                if(wifi_get_default_key(WLAN1_VA0_IF_NAME, 0, SIZE_8, defaultWEPKey, defaultPreSharedKey))
#elif defined(AEI_WECB_CUSTOMER_TELUS)
                // SW-Task #104001 
                if(wifi_get_default_key(WLAN1_VA0_IF_NAME, 0, SIZE_10, defaultWEPKey, defaultPreSharedKey))
#else
                if(wifi_get_default_key(pNewData->SSID[index_ssid].Name, 0, SIZE_16, defaultWEPKey, defaultPreSharedKey))
#endif
                {
					PRINT("failed in wifi_get_default_key()\n");
					ret = -1;
					break;
				}
				if(0==pNewData->AccessPoint[i].Security.PreSharedKey[0]){
					STRNCPY(pNewData->AccessPoint[i].Security.PreSharedKey,defaultPreSharedKey,
						sizeof(pNewData->AccessPoint[i].Security.PreSharedKey)-1);
					PRINT("use default PreSharedKey [%s]\n", pNewData->AccessPoint[i].Security.PreSharedKey);
				}
				if(0==pNewData->AccessPoint[i].Security.WEPKey[0]){
					STRNCPY(pNewData->AccessPoint[i].Security.WEPKey,defaultWEPKey,
						sizeof(pNewData->AccessPoint[i].Security.WEPKey)-1);
					PRINT("use default WEPKey [%s]\n", pNewData->AccessPoint[i].Security.WEPKey);
				}
			}
			
			SetSecurity(objPathName, pNewData);

			SetWPS(objPathName, pNewData, 1);
			//064 - Enable
			if(pNewData->AccessPoint[i].Enable){
				wifi_open_device(pNewData->SSID[index_ssid].Name);
			}
			wifi_commit(pNewData->SSID[index_ssid].Name);
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ENDPOINT){
			break;
		}
		
	}while(0);

	return ret;
}

int tr181WiFiObjectModify(char* objPathName, PDEVICE_WIFI pCurData, PDEVICE_WIFI pNewData)
{
	int ret = 0;
	unsigned int objFlag = 0;
	int i, j, k;
	unsigned int FrequencyBand = 0;
	unsigned int standard = 0;
	unsigned int channelBandwidth = 0;
	unsigned int extensionChannel = 0;
	unsigned int guardInterval = 0;
	int index_radio = -1;
	int index_ssid = -1;

	do{
		objFlag = GetObjectFlag(objPathName, &i, &j);
		if(0==objFlag){
			ret = -1;
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI){//ReadOnly
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_RADIO_STATISTICS){//ReadOnly
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_SSID_STATISTICS){//ReadOnly
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ACCESSPOINT_ACCOUNTING){//Not supported now
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ACCESSPOINT_ASSOCIATEDDEVICE){//ReadOnly
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ENDPOINT){//Not supported now
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ENDPOINT_STATISTICS){//ReadOnly
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ENDPOINT_SECURITY){//ReadOnly
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ENDPOINT_PROFILE){//Not supported now
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ENDPOINT_PROFILE_SECURITY){//Not supported now
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ENDPOINT_WPS){//Not supported now
			break;
		}

		if(objFlag&OBJ_FLAG_WIFI_RADIO){
			//014 - OperatingFrequencyBand
			if(STRCMP(pNewData->Radio[i].OperatingFrequencyBand,pCurData->Radio[i].OperatingFrequencyBand)){
				FrequencyBand = GetFrequencyBand(pNewData->Radio[i].OperatingFrequencyBand);
				if(0==FrequencyBand){
					ret = -1;
					break;
				}
				wifi_destroy_radio_device(pCurData->Radio[i].Name);
				//008 - Name
				ret = wifi_create_radio_device(FrequencyBand, pNewData->Radio[i].Name);
				if(ret<0){
					break;
				}
			}

			wifi_close_radio(pNewData->Radio[i].Name);
			//009 - LastChange
			time_radio_changed[i] = CURRENT_TIME;
			//010 - LowerLayers :Since Radio is a layer 1 interface, it is expected that LowerLayers will not be used
			memset(pNewData->Radio[i].LowerLayers, 0, sizeof(pNewData->Radio[i].LowerLayers));
			//016 - OperatingStandards
			if(STRCMP(pNewData->Radio[i].OperatingStandards,pCurData->Radio[i].OperatingStandards)){
				FrequencyBand = GetFrequencyBand(pNewData->Radio[i].OperatingFrequencyBand);
				standard = GetStandard(FrequencyBand, pNewData->Radio[i].OperatingStandards);
				if(0==standard){
					ret = -1;
					break;
				}
				wifi_set_radio_standards(pNewData->Radio[i].Name, standard);
			}
			if(STRCMP(pNewData->Radio[i].OperatingChannelBandwidth,pCurData->Radio[i].OperatingChannelBandwidth)
				||STRCMP(pNewData->Radio[i].ExtensionChannel,pCurData->Radio[i].ExtensionChannel)
				||(pNewData->Radio[i].AutoChannelEnable!=pCurData->Radio[i].AutoChannelEnable)
				||(pNewData->Radio[i].AutoChannelRefreshPeriod!=pCurData->Radio[i].AutoChannelRefreshPeriod)
				||(pNewData->Radio[i].Channel!=pCurData->Radio[i].Channel)
                ||(pNewData->Radio[i].X_ACTIONTEC_COM_AutoChannelRefresh && 
                    pNewData->Radio[i].AutoChannelEnable)
				){
				//023 - OperatingChannelBandwidth
				channelBandwidth = GetChannelBandwidth(pNewData->Radio[i].OperatingChannelBandwidth);
				//024 - ExtensionChannel
				extensionChannel = GetExtensionChannel(pNewData->Radio[i].ExtensionChannel);
				//019 - Channel
				//021 - AutoChannelEnable
				//022 - AutoChannelRefreshPeriod
				ret = wifi_set_radio_channel_info(pNewData->Radio[i].Name, channelBandwidth, extensionChannel,
					pNewData->Radio[i].AutoChannelEnable,
					pNewData->Radio[i].AutoChannelRefreshPeriod,
					pNewData->Radio[i].Channel);
				if(ret<0){
					break;
				}
			}
			//025 - GuardInterval
			if(STRCMP(pNewData->Radio[i].GuardInterval,pCurData->Radio[i].GuardInterval)){
				guardInterval = GetGuardInterval(pNewData->Radio[i].GuardInterval);
				wifi_set_radio_GuardInterval(pNewData->Radio[i].Name, guardInterval);
			}
			//026 - MCS
			if(pNewData->Radio[i].MCS!=pCurData->Radio[i].MCS){
				wifi_set_radio_MCS(pNewData->Radio[i].Name, pNewData->Radio[i].MCS);
			}
			//028 - TransmitPower
			if(pNewData->Radio[i].TransmitPower!=pCurData->Radio[i].TransmitPower){
				wifi_set_radio_TransmitPower(pNewData->Radio[i].Name, pNewData->Radio[i].TransmitPower);
			}
			//030 - IEEE80211hEnabled
			if(pNewData->Radio[i].IEEE80211hEnabled!=pCurData->Radio[i].IEEE80211hEnabled){
				wifi_set_radio_IEEE80211hEnabled(pNewData->Radio[i].Name, pNewData->Radio[i].IEEE80211hEnabled);			
			}
			//031 - RegulatoryDomain
			if(STRCMP(pNewData->Radio[i].RegulatoryDomain,pCurData->Radio[i].RegulatoryDomain)){
				wifi_set_radio_RegulatoryDomain(pNewData->Radio[i].Name, pNewData->Radio[i].RegulatoryDomain);
			}
			//005 - Enable
			if(pNewData->Radio[i].Enable){
				wifi_open_radio(pNewData->Radio[i].Name);
			}
			wifi_commit(pNewData->Radio[i].Name);
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_SSID){
			if(0==pNewData->SSID[i].Name[0]){
				PRINT("SSID name changed from %s to NULL\n", pCurData->SSID[i].Name);
				break;
			}
			//045 - LowerLayers
			if(!IsSSIDLowerLayersExist(pNewData->SSID[i].LowerLayers, pNewData->Radio, &index_radio)){
				PRINT("LowerLayers [%s] is invalid.\n", pNewData->SSID[i].LowerLayers);
				ret = -1;
				break;
			}
			if(STRCMP(pNewData->SSID[i].LowerLayers,pCurData->SSID[i].LowerLayers)){
				for(k=0; k<MAX_NUMBER_OF_ENTRIES_ACCESSPOINT; k++){
					if(!STRCMP(pNewData->AccessPoint[k].SSIDReference, objPathName)){
						wifi_destroy_ssid_device(pNewData->SSID[i].Name);
						ret = wifi_create_ssid_device(pNewData->Radio[index_radio].Name, DEVICE_OPERATION_MODE_ACCESSPOINT, pNewData->SSID[i].Name);
						break;
					}
					
				}
				if(ret<0){
					break;
				}
				for(k=0; k<MAX_NUMBER_OF_ENTRIES_ENDPOINT; k++){
					if(!STRCMP(pNewData->EndPoint[k].SSIDReference, objPathName)){
						wifi_destroy_ssid_device(pNewData->SSID[i].Name);
						ret = wifi_create_ssid_device(pNewData->Radio[index_radio].Name, DEVICE_OPERATION_MODE_STATION, pNewData->SSID[i].Name);
						break;
					}				
				}
				if(ret<0){
					break;
				}
			}
			wifi_close_device(pNewData->SSID[i].Name);
			for(k=0; k<MAX_NUMBER_OF_ENTRIES_ACCESSPOINT; k++){
				if(!STRCMP(objPathName,pNewData->AccessPoint[k].SSIDReference)){
					pNewData->AccessPoint[k].Enable = pNewData->SSID[i].Enable;
				}
			}
			//044 - LastChange
			time_ssid_changed[i] = CURRENT_TIME;
			//048 - SSID
			if (STRCMP(pNewData->SSID[i].SSID, pCurData->SSID[i].SSID)
				&&0!=pNewData->SSID[i].SSID[0]){
				wifi_set_ssid(pNewData->SSID[i].Name, pNewData->SSID[i].SSID);
			}	
			if(pNewData->SSID[i].Enable){
				wifi_open_device(pNewData->SSID[i].Name);
			}
			wifi_commit(pNewData->SSID[i].Name);
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ACCESSPOINT){
			//067 - SSIDReference
			char* pName = GetSSIDReferenceName(pNewData->AccessPoint[i].SSIDReference,pNewData->SSID, &index_ssid);
			if(NULL==pName){
				PRINT("SSIDReference [%s] is invalid.\n", pNewData->AccessPoint[i].SSIDReference);
				ret = -1;
				break;
			}
			//045 - LowerLayers
			if(!IsSSIDLowerLayersExist(pNewData->SSID[index_ssid].LowerLayers, pNewData->Radio, &index_radio)){
				PRINT("LowerLayers [%s] is invalid.\n", pNewData->SSID[index_ssid].LowerLayers);
				ret = -1;
				break;
			}
			if(STRCMP(pNewData->AccessPoint[i].SSIDReference,pCurData->AccessPoint[i].SSIDReference)){
				//043 - Name
				wifi_destroy_ssid_device(pName);
				ret = wifi_create_ssid_device(pNewData->Radio[index_radio].Name, DEVICE_OPERATION_MODE_ACCESSPOINT, pName);
				if(ret<0){
					break;
				}
				char* pName2 = GetSSIDReferenceName(pCurData->AccessPoint[i].SSIDReference,pNewData->SSID, NULL);
				if(NULL!=pName2&&0!=pName2[0]){
					wifi_destroy_ssid_device(pName2);
					memset(pName2, 0, SIZE_64);
				}
			}
			//040 - Enable
			pNewData->SSID[index_ssid].Enable = pNewData->AccessPoint[i].Enable;
			wifi_close_device(pName);
			//042 - Alias
			snprintf(pNewData->SSID[index_ssid].Alias, sizeof(pNewData->SSID[index_ssid].Alias),
				"%s", pNewData->AccessPoint[i].Alias);

			//065 - Status
			//066 - Alias
			//068 - SSIDAdvertisementEnabled
			if(pNewData->AccessPoint[i].SSIDAdvertisementEnabled!=pCurData->AccessPoint[i].SSIDAdvertisementEnabled){
				wifi_set_SSIDAdvertisementEnabled(pName, pNewData->AccessPoint[i].SSIDAdvertisementEnabled);
			}
			//069 - RetryLimit
			if(pNewData->AccessPoint[i].RetryLimit!=pCurData->AccessPoint[i].RetryLimit){
				if(pNewData->AccessPoint[i].RetryLimit>MAX_RETRY_LIMIT){
					PRINT("Invalid RetryLimit value!\n");
					ret = -1;
					break;
				}
				wifi_set_ShortRetryLimit(pName, pNewData->AccessPoint[i].RetryLimit);
			}
			//070 - WMMCapability
			//071 - UAPSDCapability
			//072 - WMMEnable
			if(pNewData->AccessPoint[i].WMMEnable!=pCurData->AccessPoint[i].WMMEnable){
				wifi_set_WMMEnable(pName, pNewData->AccessPoint[i].WMMEnable);
			}
			//073 - UAPSDEnable
			if(pNewData->AccessPoint[i].UAPSDEnable!=pCurData->AccessPoint[i].UAPSDEnable){
				wifi_set_UAPSDEnable(pName, pNewData->AccessPoint[i].UAPSDEnable);
			}
			//074 - AssociatedDeviceNumberOfEntries
			//075 - MaxAssociatedDevices
			if(pNewData->AccessPoint[i].MaxAssociatedDevices!=pCurData->AccessPoint[i].MaxAssociatedDevices){
				wifi_set_MaxAssociatedDevices(pName, pNewData->AccessPoint[i].MaxAssociatedDevices);
			}
			//076 - IsolationEnable
			if(pNewData->AccessPoint[i].IsolationEnable!=pCurData->AccessPoint[i].IsolationEnable){
				wifi_set_IsolationEnable(pName, pNewData->AccessPoint[i].IsolationEnable);
			}
			//064 - Enable
			if(pNewData->AccessPoint[i].Enable){
				wifi_open_device(pName);
			}
			wifi_commit(pName);
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ACCESSPOINT_SECURITY){
			char* pName = GetSSIDReferenceName(pNewData->AccessPoint[i].SSIDReference,pNewData->SSID, NULL);
			if(NULL==pName){
				PRINT("SSIDReference [%s] is invalid.\n", pNewData->AccessPoint[i].SSIDReference);
				ret = -1;
				break;
			}
			if(STRCMP(pNewData->AccessPoint[i].Security.KeyPassphrase, pCurData->AccessPoint[i].Security.KeyPassphrase)
				&&strlen(pNewData->AccessPoint[i].Security.KeyPassphrase)>=SIZE_8
				&&strlen(pNewData->AccessPoint[i].Security.KeyPassphrase)<SIZE_64){
				DoKeyPassphraseChanged(
					pNewData->AccessPoint[i].Security.KeyPassphrase, 
					DEFAULT_SALT_MAC, 
					pNewData->AccessPoint[i].Security.PreSharedKey, SIZE_32);
			}
			wifi_close_device(pName);
			ret = SetSecurity(objPathName, pNewData);
			//064 - Enable
			if(pNewData->AccessPoint[i].Enable){
				wifi_open_device(pName);
			}
			if(0==ret){
				wifi_commit(pName);
			}
			break;
		}

		if(objFlag&OBJ_FLAG_WIFI_ACCESSPOINT_WPS){
			char* pName = GetSSIDReferenceName(pNewData->AccessPoint[i].SSIDReference,pNewData->SSID, NULL);
			if(NULL==pName){
				PRINT("SSIDReference [%s] is invalid.\n", pNewData->AccessPoint[i].SSIDReference);
				ret = -1;
				break;
			}
			wifi_close_device(pName);
			ret = SetWPS(objPathName, pNewData, 0);
			//064 - Enable
			if(pNewData->AccessPoint[i].Enable){
				wifi_open_device(pName);
			}
			if(0==ret){
				wifi_commit(pName);
			}
			break;
		}
	}while(0);

	return ret;
}
	
int tr181WiFiObjectGet(char* objPathName, PDEVICE_WIFI pCurData)
{
	int ret = 0;
	int i, j;
	char *p = NULL;
	unsigned int value;
	char buff[SIZE_128];
	unsigned int objFlag = 0;
//	PRINT("%s\n", objPathName);

	do{
		if(NULL==pCurData){
			PRINT("pCurData can not be null\n");
			ret = -1;
			break;
		}

		objFlag = GetObjectFlag(objPathName, &i, &j);
		if(0==objFlag){
			ret = -1;
			break;
		}
		
		if(objFlag&OBJ_FLAG_WIFI){
			//001 - RadioNumberOfEntries
			pCurData->RadioNumberOfEntries = wifi_get_RadioNumberOfEntries();
			//002 - SSIDNumberOfEntries
			pCurData->SSIDNumberOfEntries = wifi_get_SSIDNumberOfEntries();
			//003 - AccessPointNumberOfEntries
			pCurData->AccessPointNumberOfEntries = wifi_get_AccessPointNumberOfEntries();
			//004 - EndPointNumberOfEntries
			pCurData->EndPointNumberOfEntries = wifi_get_EndPointNumberOfEntries();
		}
		
		if(objFlag&OBJ_FLAG_WIFI_RADIO){
			//005 - Enable
			//006 - Status
			switch(wifi_get_device_status(pCurData->Radio[i].Name)){
				case DEVICE_STATUS_Error:
					p = STRING_STATUS_Error;
					break;
				case DEVICE_STATUS_Up:
					p = STR006_STATUS_Up;
					break;
				case DEVICE_STATUS_Down:
					p = STR006_STATUS_Down;
					break;
				case DEVICE_STATUS_Dormant:
					p = STR006_STATUS_Dormant;
					break;
				case DEVICE_STATUS_NotPresent:
					p = STR006_STATUS_NotPresent;
					break;
				case DEVICE_STATUS_LowerLayerDown:
					p = STR006_STATUS_LowerLayerDown;
					break;
				case DEVICE_STATUS_Unknown:
				default:
					p = STR006_STATUS_Unknown;
					break;
			}
			
			snprintf(pCurData->Radio[i].Status, sizeof(pCurData->Radio[i].Status), "%s", p);
			if(STR006_STATUS_NotPresent==p){
				break;
			}
			//007 - Alias
			//008 - Name
			//009 - LastChange
			pCurData->Radio[i].LastChange = DIFF_TIME(time_radio_changed[i]);
			//010 - LowerLayers :Since Radio is a layer 1 interface, it is expected that LowerLayers will not be used
			memset(pCurData->Radio[i].LowerLayers, 0, sizeof(pCurData->Radio[i].LowerLayers));
			//011 - Upstream
			pCurData->Radio[i].Upstream = 0;
			//012 - MaxBitRate
			pCurData->Radio[i].MaxBitRate = wifi_get_radio_MaxBitRate(pCurData->Radio[i].Name);
			//013 - SupportedFrequencyBands
			switch(wifi_get_radio_SupportedFrequencyBands(pCurData->Radio[i].Name)){
				case BAND_2G:
					snprintf(pCurData->Radio[i].SupportedFrequencyBands, 
						sizeof(pCurData->Radio[i].SupportedFrequencyBands), "%s", STR013_BAND_2G);
					break;
				case BAND_5G:
					snprintf(pCurData->Radio[i].SupportedFrequencyBands, 
						sizeof(pCurData->Radio[i].SupportedFrequencyBands), "%s", STR013_BAND_5G);
					break;
				case (BAND_2G|BAND_5G):
					snprintf(pCurData->Radio[i].SupportedFrequencyBands, 
						sizeof(pCurData->Radio[i].SupportedFrequencyBands), "%s,%s", STR013_BAND_2G, STR013_BAND_5G);
					break;
				default:
					memset(pCurData->Radio[i].SupportedFrequencyBands, 0, sizeof(pCurData->Radio[i].SupportedFrequencyBands));
					break;
			}
			//014 - OperatingFrequencyBand
			//015 - SupportedStandards
			memset(pCurData->Radio[i].SupportedStandards, 0, sizeof(pCurData->Radio[i].SupportedStandards));
			value = wifi_get_radio_SupportedStandards(pCurData->Radio[i].Name);
			if(value&MODE_80211_A){
				strcat(pCurData->Radio[i].SupportedStandards, "a");
			}				
			if(value&MODE_80211_B){
				strcat(pCurData->Radio[i].SupportedStandards, "b");
			}
			if(value&MODE_80211_G){
				strcat(pCurData->Radio[i].SupportedStandards, pCurData->Radio[i].SupportedStandards[0]?",g":"g");
			}
			if(value&MODE_80211_N){
				strcat(pCurData->Radio[i].SupportedStandards, pCurData->Radio[i].SupportedStandards[0]?",n":"n");
			}
			//016 - OperatingStandards
			//017 - PossibleChannels
			wifi_get_radio_PossibleChannels(pCurData->Radio[i].Name, pCurData->Radio[i].PossibleChannels, sizeof(pCurData->Radio[i].PossibleChannels));
			//018 - ChannelsInUse
			wifi_get_radio_ChannelsInUse(pCurData->Radio[i].Name, pCurData->Radio[i].ChannelsInUse);
			//019 - Channel
			pCurData->Radio[i].Channel = wifi_get_radio_channel(pCurData->Radio[i].Name);
			//020 - AutoChannelSupported
			pCurData->Radio[i].AutoChannelSupported = wifi_get_radio_AutoChannelSupported(pCurData->Radio[i].Name);
			//021 - AutoChannelEnable
			//022 - AutoChannelRefreshPeriod
			//023 - OperatingChannelBandwidth
			//024 - ExtensionChannel
			//025 - GuardInterval
			//026 - MCS
			//027 - TransmitPowerSupported
			wifi_get_radio_TransmitPowerSupported(pCurData->Radio[i].Name, pCurData->Radio[i].TransmitPowerSupported, sizeof(pCurData->Radio[i].TransmitPowerSupported));
			//028 - TransmitPower
			//029 - IEEE80211hSupported
			pCurData->Radio[i].IEEE80211hSupported = wifi_get_radio_IEEE80211hSupported(pCurData->Radio[i].Name);
			//030 - IEEE80211hEnabled
			//031 - RegulatoryDomain
		}
		
		if(objFlag&OBJ_FLAG_WIFI_RADIO_STATISTICS){
			if(!wifi_device_is_exist(pCurData->Radio[i].Name)){
				ret = -1;
				break;
			}
			//032 - BytesSent
			pCurData->Radio[i].Stats.BytesSent = wifi_get_BytesSent(pCurData->Radio[i].Name);
			//033 - BytesReceived
			pCurData->Radio[i].Stats.BytesReceived = wifi_get_BytesReceived(pCurData->Radio[i].Name);
			//034 - PacketsSent
			pCurData->Radio[i].Stats.PacketsSent = wifi_get_PacketsSent(pCurData->Radio[i].Name);
			//035 - PacketsReceived
			pCurData->Radio[i].Stats.PacketsReceived = wifi_get_PacketsReceived(pCurData->Radio[i].Name);
			//036 - ErrorsSent
			pCurData->Radio[i].Stats.ErrorsSent = wifi_get_ErrorsSent(pCurData->Radio[i].Name);
			//037 - ErrorsReceived
			pCurData->Radio[i].Stats.ErrorsReceived = wifi_get_ErrorsReceived(pCurData->Radio[i].Name);
			//038 - DiscardPacketsSent
			pCurData->Radio[i].Stats.DiscardPacketsSent = wifi_get_DiscardPacketsSent(pCurData->Radio[i].Name);
			//039 - DiscardPacketsReceived
			pCurData->Radio[i].Stats.DiscardPacketsReceived = wifi_get_DiscardPacketsReceived(pCurData->Radio[i].Name);
		}
		
		if(objFlag&OBJ_FLAG_WIFI_SSID){
			//040 - Enable
			//041 - Status	
			switch(wifi_get_device_status(pCurData->SSID[i].Name)){
				case DEVICE_STATUS_Error:
					p = STRING_STATUS_Error;
					break;
				case DEVICE_STATUS_Up:
					p = STR006_STATUS_Up;
					break;
				case DEVICE_STATUS_Down:
					p = STR006_STATUS_Down;
					break;
				case DEVICE_STATUS_Dormant:
					p = STR006_STATUS_Dormant;
					break;
				case DEVICE_STATUS_NotPresent:
					p = STR006_STATUS_NotPresent;
					break;
				case DEVICE_STATUS_LowerLayerDown:
					p = STR006_STATUS_LowerLayerDown;
					break;
				case DEVICE_STATUS_Unknown:
				default:
					p = STR006_STATUS_Unknown;
					break;
			}
			
			snprintf(pCurData->SSID[i].Status, sizeof(pCurData->SSID[i].Status), "%s", p);
			if(STR006_STATUS_NotPresent==p){
				break;
			}
			//042 - Alias 
			//043 - Name
			//044 - LastChange
			pCurData->SSID[i].LastChange = DIFF_TIME(time_ssid_changed[i]);
			//045 - LowerLayers
			//046 - BSSID
			wifi_get_BSSID(pCurData->SSID[i].Name, pCurData->SSID[i].BSSID);
			//047 - MACAddress
			wifi_get_MACAddress(pCurData->SSID[i].Name, pCurData->SSID[i].MACAddress);
			//048 - SSID
		}
		
		if(objFlag&OBJ_FLAG_WIFI_SSID_STATISTICS){
			if(!wifi_device_is_exist(pCurData->SSID[i].Name)){
				ret = -1;
				break;
			}
			//049 - BytesSent
			pCurData->SSID[i].Stats.BytesSent = wifi_get_BytesSent(pCurData->SSID[i].Name);
			//050 - BytesReceived
			pCurData->SSID[i].Stats.BytesReceived = wifi_get_BytesReceived(pCurData->SSID[i].Name);
			//051 - PacketsSent
			pCurData->SSID[i].Stats.PacketsSent = wifi_get_PacketsSent(pCurData->SSID[i].Name);
			//052 - PacketsReceived
			pCurData->SSID[i].Stats.PacketsReceived = wifi_get_PacketsReceived(pCurData->SSID[i].Name);
			//053 - ErrorsSent
			pCurData->SSID[i].Stats.ErrorsSent = wifi_get_ErrorsSent(pCurData->SSID[i].Name);
			//054 - ErrorsReceived
			pCurData->SSID[i].Stats.ErrorsReceived = wifi_get_ErrorsReceived(pCurData->SSID[i].Name);
			//055 - UnicastPacketsSent
			pCurData->SSID[i].Stats.UnicastPacketsSent = wifi_get_UnicastPacketsSent(pCurData->SSID[i].Name);
			//056 - UnicastPacketsReceived
			//057 - DiscardPacketsSent
			pCurData->SSID[i].Stats.DiscardPacketsSent = wifi_get_DiscardPacketsSent(pCurData->SSID[i].Name);
			//058 - DiscardPacketsReceived
			pCurData->SSID[i].Stats.DiscardPacketsReceived = wifi_get_DiscardPacketsReceived(pCurData->SSID[i].Name);
			//059 - MulticastPacketsSent
			pCurData->SSID[i].Stats.MulticastPacketsSent = wifi_get_MulticastPacketsSent(pCurData->SSID[i].Name);
			//060 - MulticastPacketsReceived
			pCurData->SSID[i].Stats.MulticastPacketsReceived = wifi_get_MulticastPacketsReceived(pCurData->SSID[i].Name);
			//061 - BroadcastPacketsSent
			pCurData->SSID[i].Stats.BroadcastPacketsSent = wifi_get_BroadcastPacketsSent(pCurData->SSID[i].Name);
			//062 - BroadcastPacketsReceived
			pCurData->SSID[i].Stats.BroadcastPacketsReceived = wifi_get_BroadcastPacketsReceived(pCurData->SSID[i].Name);
			//063 - UnknownProtoPacketsReceived
			pCurData->SSID[i].Stats.UnknownProtoPacketsReceived = wifi_get_UnknownProtoPacketsReceived(pCurData->SSID[i].Name);
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ACCESSPOINT){
			char* pName = GetSSIDReferenceName(pCurData->AccessPoint[i].SSIDReference,pCurData->SSID, NULL);
			if(NULL==pName){
				ret = -1;
				break;
			}
			if(!wifi_device_is_exist(pName)){
				ret = -1;
				break;
			}
			//064 - Enable
			//065 - Status
			if(pCurData->AccessPoint[i].Enable){
				if(OBJ_FLAG_WIFI_SSID!=GetObjectFlag(pCurData->AccessPoint[i].SSIDReference, NULL, NULL)){
					snprintf(pCurData->AccessPoint[i].Status, sizeof(pCurData->AccessPoint[i].Status),
						"%s", STR065_STATUS_Error_Misconfigured);
					pCurData->AccessPoint[i].Enable = 0;
				}else{
					snprintf(pCurData->AccessPoint[i].Status, sizeof(pCurData->AccessPoint[i].Status),
						"%s", STR065_STATUS_Enabled);
				}
			}else{
				snprintf(pCurData->AccessPoint[i].Status, sizeof(pCurData->AccessPoint[i].Status),
					"%s", STR065_STATUS_Disabled);
			}
			//066 - Alias
			//067 - SSIDReference
			//068 - SSIDAdvertisementEnabled
			//069 - RetryLimit
			//070 - WMMCapability
			pCurData->AccessPoint[i].WMMCapability = wifi_get_WMMSupported(pName);
			//071 - UAPSDCapability
			pCurData->AccessPoint[i].UAPSDCapability = wifi_get_UAPSDSupported(pName);
			//072 - WMMEnable
			//073 - UAPSDEnable
			//074 - AssociatedDeviceNumberOfEntries
			pCurData->AccessPoint[i].AssociatedDeviceNumberOfEntries = wifi_get_associated_device_count(pName);
			//075 - MaxAssociatedDevices
			//076 - IsolationEnable
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ACCESSPOINT_SECURITY){
			char* pName = GetSSIDReferenceName(pCurData->AccessPoint[i].SSIDReference,pCurData->SSID, NULL);
			if(NULL==pName){
				ret = -1;
				break;
			}
			if(!wifi_device_is_exist(pName)){
				ret = -1;
				break;
			}
			//077 - Reset
			pCurData->AccessPoint[i].Security.Reset = 0;
			//078 - ModesSupported
			memset(pCurData->AccessPoint[i].Security.ModesSupported, 0,
				sizeof(pCurData->AccessPoint[i].Security.ModesSupported));
			value = wifi_get_SecurityModesSupported(pName);
			if(value&SECURITY_MODE_NONE){
				strcat(pCurData->AccessPoint[i].Security.ModesSupported, STR078_SECURITY_MODE_None);
			}
			if(value&SECURITY_MODE_WEP64){
				if(pCurData->AccessPoint[i].Security.ModesSupported[0]) strcat(pCurData->AccessPoint[i].Security.ModesSupported, ",");
				strcat(pCurData->AccessPoint[i].Security.ModesSupported, STR078_SECURITY_MODE_WEP64);
			}
			if(value&SECURITY_MODE_WEP128){
				if(pCurData->AccessPoint[i].Security.ModesSupported[0]) strcat(pCurData->AccessPoint[i].Security.ModesSupported, ",");
				strcat(pCurData->AccessPoint[i].Security.ModesSupported, STR078_SECURITY_MODE_WEP128);
			}
			if(value&SECURITY_MODE_PERSONAL_WPA){
				if(pCurData->AccessPoint[i].Security.ModesSupported[0]) strcat(pCurData->AccessPoint[i].Security.ModesSupported, ",");
				strcat(pCurData->AccessPoint[i].Security.ModesSupported, STR078_SECURITY_MODE_WPA_Personal);
			}
			if(value&SECURITY_MODE_PERSONAL_WPA2){
				if(pCurData->AccessPoint[i].Security.ModesSupported[0]) strcat(pCurData->AccessPoint[i].Security.ModesSupported, ",");
				strcat(pCurData->AccessPoint[i].Security.ModesSupported, STR078_SECURITY_MODE_WPA2_Personal);
			}
			if(value&SECURITY_MODE_PERSONAL_MIXED){
				if(pCurData->AccessPoint[i].Security.ModesSupported[0]) strcat(pCurData->AccessPoint[i].Security.ModesSupported, ",");
				strcat(pCurData->AccessPoint[i].Security.ModesSupported, STR078_SECURITY_MODE_MIXED_Personal);
			}
			if(value&SECURITY_MODE_ENTERPRISE_WPA){
				if(pCurData->AccessPoint[i].Security.ModesSupported[0]) strcat(pCurData->AccessPoint[i].Security.ModesSupported, ",");
				strcat(pCurData->AccessPoint[i].Security.ModesSupported, STR078_SECURITY_MODE_WPA_Enterprise);
			}
			if(value&SECURITY_MODE_ENTERPRISE_WPA2){
				if(pCurData->AccessPoint[i].Security.ModesSupported[0]) strcat(pCurData->AccessPoint[i].Security.ModesSupported, ",");
				strcat(pCurData->AccessPoint[i].Security.ModesSupported, STR078_SECURITY_MODE_WPA2_Enterprise);
			}
			if(value&SECURITY_MODE_ENTERPRISE_MIXED){
				if(pCurData->AccessPoint[i].Security.ModesSupported[0]) strcat(pCurData->AccessPoint[i].Security.ModesSupported, ",");
				strcat(pCurData->AccessPoint[i].Security.ModesSupported, STR078_SECURITY_MODE_MIXED_Enterprise);
			}
			//079 - ModeEnabled
			//080 - WEPKey
			//081 - PreSharedKey
			//082 - KeyPassphrase
			//083 - RekeyingInterval
			//084 - RadiusServerIPAddr
			//085 - SecondaryRadiusServerIPAddr
			//086 - RadiusServerPort
			//087 - SecondaryRadiusServerPort
			//088 - RadiusSecret
			//089 - SecondaryRadiusSecret
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ACCESSPOINT_ACCOUNTING){
			//090 - Enable
			//091 - ServerIPAddr
			//092 - SecondaryServerIPAddr
			//093 - ServerPort
			//094 - SecondaryServerPort
			//095 - Secret
			//096 - SecondarySecret
			//097 - InterimInterval
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ACCESSPOINT_WPS){
			char* pName = GetSSIDReferenceName(pCurData->AccessPoint[i].SSIDReference,pCurData->SSID, NULL);
			if(NULL==pName){
				ret = -1;
				break;
			}
			if(!wifi_device_is_exist(pName)){
				ret = -1;
				break;
			}
			//098 - Enable
			//099 - ConfigMethodsSupported
			value = wifi_get_wps_ConfigMethodsSupported(pName);
			memset(pCurData->AccessPoint[i].WPS.ConfigMethodsSupported, 0, 
				sizeof(pCurData->AccessPoint[i].WPS.ConfigMethodsSupported));
			if(value&WPS_CONFIG_METHODS_FLAG_USBFLASHDRIVER){
				strcat(pCurData->AccessPoint[i].WPS.ConfigMethodsSupported, STR099_CONFIG_METHODS_SUPPORTED_USBFLASHDRIVER);
			}
			if(value&WPS_CONFIG_METHODS_FLAG_ETHERNET){
				if(pCurData->AccessPoint[i].WPS.ConfigMethodsSupported[0]) strcat(pCurData->AccessPoint[i].WPS.ConfigMethodsSupported, ",");
				strcat(pCurData->AccessPoint[i].WPS.ConfigMethodsSupported, STR099_CONFIG_METHODS_SUPPORTED_ETHERNET);
			}
			if(value&WPS_CONFIG_METHODS_FLAG_EXTERNEL_NFC_TOKEN){
				if(pCurData->AccessPoint[i].WPS.ConfigMethodsSupported[0]) strcat(pCurData->AccessPoint[i].WPS.ConfigMethodsSupported, ",");
				strcat(pCurData->AccessPoint[i].WPS.ConfigMethodsSupported, STR099_CONFIG_METHODS_SUPPORTED_EXTERNEL_NFC_TOKEN);
			}
			if(value&WPS_CONFIG_METHODS_FLAG_INTEGRATED_NFC_TOKEN){
				if(pCurData->AccessPoint[i].WPS.ConfigMethodsSupported[0]) strcat(pCurData->AccessPoint[i].WPS.ConfigMethodsSupported, ",");
				strcat(pCurData->AccessPoint[i].WPS.ConfigMethodsSupported, STR099_CONFIG_METHODS_SUPPORTED_INTEGRATED_NFC_TOKEN);
			}
			if(value&WPS_CONFIG_METHODS_FLAG_NFC_INTERFACE){
				if(pCurData->AccessPoint[i].WPS.ConfigMethodsSupported[0]) strcat(pCurData->AccessPoint[i].WPS.ConfigMethodsSupported, ",");
				strcat(pCurData->AccessPoint[i].WPS.ConfigMethodsSupported, STR099_CONFIG_METHODS_SUPPORTED_NFC_INTERFACE);
			}
			if(value&WPS_CONFIG_METHODS_FLAG_PUSHBUTTON){
				if(pCurData->AccessPoint[i].WPS.ConfigMethodsSupported[0]) strcat(pCurData->AccessPoint[i].WPS.ConfigMethodsSupported, ",");
				strcat(pCurData->AccessPoint[i].WPS.ConfigMethodsSupported, STR099_CONFIG_METHODS_SUPPORTED_PUSHBUTTON);
			}
			if(value&WPS_CONFIG_METHODS_FLAG_PIN){
				if(pCurData->AccessPoint[i].WPS.ConfigMethodsSupported[0]) strcat(pCurData->AccessPoint[i].WPS.ConfigMethodsSupported, ",");
				strcat(pCurData->AccessPoint[i].WPS.ConfigMethodsSupported, STR099_CONFIG_METHODS_SUPPORTED_PIN);
			}
			//100 - ConfigMethodsEnabled
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ACCESSPOINT_ASSOCIATEDDEVICE){
			char* pName = GetSSIDReferenceName(pCurData->AccessPoint[i].SSIDReference,pCurData->SSID, NULL);
			if(NULL==pName){
				ret = -1;
				break;
			}
			if(!wifi_device_is_exist(pName)){
				ret = -1;
				break;
			}
			p = strstr(objPathName, TOKEN_PATH_ASSOCIATEDDEVICE);
			if(NULL==p){
				PRINT("invalid objPathName [%s] for objFlag [%d], it should be contain '%s'\n", objPathName, objFlag, TOKEN_PATH_ASSOCIATEDDEVICE);
				ret = -1;
				break;
			}
			pCurData->AccessPoint[i].AssociatedDeviceNumberOfEntries = wifi_get_associated_device_count(pName);
			if(pCurData->AccessPoint[i].AssociatedDeviceNumberOfEntries>MAX_NUMBER_OF_ENTRIES_ASSOCIATEDDEVICES){
				pCurData->AccessPoint[i].AssociatedDeviceNumberOfEntries = MAX_NUMBER_OF_ENTRIES_ASSOCIATEDDEVICES;
			}

			//101 - MACAddress
			if(wifi_get_associated_device_mac_address(pName, j, buff)<0){
				break;
			}
			snprintf(pCurData->AccessPoint[i].AssociatedDevice[j].MACAddress,
				sizeof(pCurData->AccessPoint[i].AssociatedDevice[j].MACAddress), "%s", buff);
			//102 - AuthenticationState
			pCurData->AccessPoint[i].AssociatedDevice[j].AuthenticationState = 
				wifi_get_associated_device_authentication_state(pName,
				pCurData->AccessPoint[i].AssociatedDevice[j].MACAddress);
			//103 - LastDataDownlinkRate
			pCurData->AccessPoint[i].AssociatedDevice[j].LastDataDownlinkRate = 
				wifi_get_associated_device_LastDataDownlinkRate(pName,
				pCurData->AccessPoint[i].AssociatedDevice[j].MACAddress);
			//104 - LastDataUplinkRate
			pCurData->AccessPoint[i].AssociatedDevice[j].LastDataUplinkRate = 
				wifi_get_associated_device_LastDataUplinkRate(pName,
				pCurData->AccessPoint[i].AssociatedDevice[j].MACAddress);
			//105 - SignalStrength
			pCurData->AccessPoint[i].AssociatedDevice[j].SignalStrength = 
				wifi_get_associated_device_SignalStrength(pName,
				pCurData->AccessPoint[i].AssociatedDevice[j].MACAddress);
			//106 - Retransmissions
			pCurData->AccessPoint[i].AssociatedDevice[j].Retransmissions = 
				wifi_get_associated_device_Retransmissions(pName,
				pCurData->AccessPoint[i].AssociatedDevice[j].MACAddress);
			//107 - Active
			pCurData->AccessPoint[i].AssociatedDevice[j].Active = 
				wifi_get_associated_device_Active(pName,
				pCurData->AccessPoint[i].AssociatedDevice[j].MACAddress);
			//x05 - extLinkQuality
			pCurData->AccessPoint[i].AssociatedDevice[j].extLinkQuality = 
				wifi_get_associated_device_LinkQuality(pName,
				pCurData->AccessPoint[i].AssociatedDevice[j].MACAddress);
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ENDPOINT){
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ENDPOINT_STATISTICS){
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ENDPOINT_SECURITY){
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ENDPOINT_PROFILE){
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ENDPOINT_PROFILE_SECURITY){
		}
		
		if(objFlag&OBJ_FLAG_WIFI_ENDPOINT_WPS){
		}
		
	}while(0);

	return ret;
}

