/**************************************************************************
 *	
 *	        tr098_wifi.c according to TR-98_Amendment_2.pdf
 *
 **************************************************************************/
#include <unistd.h> //for sleep
#include <time.h> //for time
#include <stdio.h> //for snprintf
#include <stdlib.h> //for malloc
#include <string.h>
#include <math.h>
#include "sha1.h"
#include "tr098_wifi.h"
#include "wifi_adapter_driver.h"


/**************************************************************************
 *	
 *	        constant definition
 *
 **************************************************************************/

#define STR002_STATUS_UP			"Up"
#define STR002_STATUS_ERROR			"Error"
#define STR002_STATUS_DISABLED		"Disabled"
#define STR002_STATUS_CONNECTING 	"Connecting"

#define STR009_BEACON_TYPE_NONE 	"None"
#define STR009_BEACON_TYPE_BASIC 	"Basic"
#define STR009_BEACON_TYPE_WPA 		"WPA"
#define STR009_BEACON_TYPE_11I 		"11i"
#define STR009_BEACON_TYPE_WPA11I 	"WPAand11i"

#define STR011_STANDARD_A		"a"	//a-only
#define STR011_STANDARD_B		"b"	//b-only
#define STR011_STANDARD_G		"g"	//bg
#define STR011_STANDARD_N		"n"	//bgn
#define STR011_STANDARD_N_ONLY	"n-only"	//n-only
#define STR011_STANDARD_G_ONLY	"g-only"	//g-only
#define STR011_STANDARD_AN		"an"
#define STR011_STANDARD_BN		"bn"
#define STR011_STANDARD_GN		"gn"

#define STR015_BASIC_ENCRYPTION_MODES_NONE	"None"
#define STR015_BASIC_ENCRYPTION_MODES_WEP	"WEPEncryption"

#define STR016_BASIC_AUTHENTICATION_MODE_OPEN	"None"	
#define STR016_BASIC_AUTHENTICATION_MODE_EAP	"EAPAuthentication"	
#define STR016_BASIC_AUTHENTICATION_MODE_SHARED	"SharedAuthentication"	
#define STR016_BASIC_AUTHENTICATION_MODE_AUTO	"Auto"	

#define STR017_WPA_ENCRYPTION_MODES_TKIP		"TKIPEncryption"
#define STR017_WPA_ENCRYPTION_MODES_AES			"AESEncryption"
#define STR017_WPA_ENCRYPTION_MODES_TKIP_AES	"TKIPandAESEncryption"

#define STR018_WPA_AUTHENTICATION_MODE_PSK	"PSKAuthentication"	
#define STR018_WPA_AUTHENTICATION_MODE_EAP	"EAPAuthentication"	

#define STR019_IEEE11I_ENCRYPTION_MODES_TKIP		"TKIPEncryption"
#define STR019_IEEE11I_ENCRYPTION_MODES_AES			"AESEncryption"
#define STR019_IEEE11I_ENCRYPTION_MODES_TKIP_AES	"TKIPandAESEncryption"

#define STR020_IEEE11I_AUTHENTICATION_MODE_PSK		"PSKAuthentication"	
#define STR020_IEEE11I_AUTHENTICATION_MODE_EAP		"EAPAuthentication"	
#define STR020_IEEE11I_AUTHENTICATION_MODE_EAP_PSK	"EAPandPSKAuthentication"	
	

#define STR037_DeviceOperationMode_InfrastructureAccessPoint	"InfrastructureAccessPoint"
#define STR037_DeviceOperationMode_WirelessBridge				"WirelessBridge"
#define STR037_DeviceOperationMode_WirelessRepeater				"WirelessRepeater"
#define STR037_DeviceOperationMode_WirelessStation				"WirelessStation"

#define STR066_CONFIG_METHODS_SUPPORTED_USBFLASHDRIVER 			"USBFlashDrive"
#define STR066_CONFIG_METHODS_SUPPORTED_ETHERNET 				"Ethernet"
#define STR066_CONFIG_METHODS_SUPPORTED_LABEL 					"Label"
#define STR066_CONFIG_METHODS_SUPPORTED_DISPLAY 				"Display"
#define STR066_CONFIG_METHODS_SUPPORTED_EXTERNEL_NFC_TOKEN 		"ExternalNFCToken"
#define STR066_CONFIG_METHODS_SUPPORTED_INTEGRATED_NFC_TOKEN 	"IntegratedNFCToken"
#define STR066_CONFIG_METHODS_SUPPORTED_NFC_INTERFACE 			"NFCInterface"
#define STR066_CONFIG_METHODS_SUPPORTED_PUSHBUTTON 				"PushButton"
#define STR066_CONFIG_METHODS_SUPPORTED_KEY_PAD 				"Keypad"

#define STR067_CONFIG_METHODS_ENABLED_PUSHBUTTON 	"PushButton"
#define STR067_CONFIG_METHODS_ENABLED_KEYPAD	 	"Keypad"
#define STR067_CONFIG_METHODS_ENABLED_DISPLAY	 	"Display"
#define STR067_CONFIG_METHODS_ENABLED_LABEL		 	"Label"

#define STR068_SETUP_LOCK_STATE_UNLOCKED					"Unlocked"
#define STR068_SETUP_LOCK_STATE_LOCKED_BY_LOCAL_MANAGEMENT	"LockedByLocalManagement"
#define STR068_SETUP_LOCK_STATE_LOCKED_BY_REMOTE_MANAGEMENT	"LockedByRemoteManagement"
#define STR068_SETUP_LOCK_STATE_PIN_RETRY_LIMIT_REACHED		"PINRetryLimitReached"
 
#define STR070_CONFIGURATION_STATE_NOT_CONFIGURED	"Not configured"
#define STR070_CONFIGURATION_STATE_CONFIGURED		"Configured"

#define STR071_LAST_CONFIGURATION_ERROR_NO_ERROR						"NoError"
#define STR071_LAST_CONFIGURATION_ERROR_DECRYPTION_CRC_FAILURE			"DecryptionCRCFailure"
#define STR071_LAST_CONFIGURATION_ERROR_SIGNAL_TOO_WEAK					"SignalTooWeak"
#define STR071_LAST_CONFIGURATION_ERROR_COULDNT_CONNECT_TO_REGISTRAR	"CouldntConnectToRegistrar"
#define STR071_LAST_CONFIGURATION_ERROR_ROGUE_ACTIVITY_SUSPECTED		"RogueActivitySuspected"
#define STR071_LAST_CONFIGURATION_ERROR_DEVICE_BUSY						"DeviceBusy"
#define STR071_LAST_CONFIGURATION_ERROR_SETUP_LOCKED					"SetupLocked"
#define STR071_LAST_CONFIGURATION_ERROR_MESSAGE_TIMEOUT					"MessageTimeout"
#define STR071_LAST_CONFIGURATION_ERROR_REGISTRATION_SESSION_TIMEOUT	"RegistrationSessionTimeout"
#define STR071_LAST_CONFIGURATION_ERROR_DEVICE_PASSWORD_AUTH_FAILURE	"DevicePasswordAuthFailure"

#define X10_CTS_PROTECT_MODE_NONE		"None"
#define X10_CTS_PROTECT_MODE_CTSONLY	"CTSONLY"
#define X10_CTS_PROTECT_MODE_RTSCTS		"RTSCTS"

#define X13_ACL_POLICY_ALLOW	0
#define X13_ACL_POLICY_DENY		1

#define DEFAULT_SALT_MAC	"00:11:22:33:44:55"

#define TIME_SEARCH_CHANNEL 3

/**************************************************************************
 *	
 *	        macro definition
 *
 **************************************************************************/
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
 *	        static function declaration
 *
 **************************************************************************/
static void StopWifiDaemon(char* devName);
static void StartWifiDaemon(char* devName);
static unsigned int GetDeviceOperationMode(char* DeviceOperationMode);
static unsigned int GetStandard(char* Standard);
static unsigned int GetCTSProtectMode(char* CTSProtectMode);
static int SetEncryption(PWLANConfiguration pNewData);
static int SetWPS(PWLANConfiguration pNewData, unsigned int first);
static int DoKeyPassphraseChanged(char* keyPassphrase, char* associatedDeviceMACAddress, unsigned char *key, unsigned int key_len);
static int pkcs5_pbkdf2(unsigned char *pass, unsigned int pass_len, unsigned char *salt, unsigned int salt_len, unsigned char *key, unsigned int key_len, unsigned int rounds);
static unsigned int Hex2String(unsigned char *hex, unsigned int hex_len, char *hexString);

/**************************************************************************
 *	
 *	        static function definition
 *
 **************************************************************************/
static void StopWifiDaemon(char* devName)
{
}
	
static void StartWifiDaemon(char* devName)
{
}
	
static unsigned int GetDeviceOperationMode(char* DeviceOperationMode)
{
	unsigned int devOperationMode = 0;
	
	if(!STRCMP(DeviceOperationMode, STR037_DeviceOperationMode_InfrastructureAccessPoint)){
		devOperationMode = DEVICE_OPERATION_MODE_ACCESSPOINT;
	}else if(!STRCMP(DeviceOperationMode, STR037_DeviceOperationMode_WirelessStation)){
		devOperationMode = DEVICE_OPERATION_MODE_STATION;
	}else if(!STRCMP(DeviceOperationMode, STR037_DeviceOperationMode_WirelessBridge)){
		devOperationMode = DEVICE_OPERATION_MODE_BRIDGE;
	}else if(!STRCMP(DeviceOperationMode, STR037_DeviceOperationMode_WirelessRepeater)){
		devOperationMode = DEVICE_OPERATION_MODE_REPEATER;
	}

	return devOperationMode;
}

static unsigned int GetStandard(char* Standard)
{
	unsigned int standard = 0;
	
	if (!STRCMP(Standard, STR011_STANDARD_N_ONLY)){
		standard = MODE_80211_N;
	}else if(!STRCMP(Standard, STR011_STANDARD_G_ONLY)){
		standard = MODE_80211_G;
	}else if(!STRCMP(Standard, STR011_STANDARD_B)){
		standard = MODE_80211_B;				
	}else if(!STRCMP(Standard, STR011_STANDARD_A)){
		standard = MODE_80211_A;
	}else if(!STRCMP(Standard, STR011_STANDARD_G)){
		standard = MODE_80211_G|MODE_80211_B;
	}else if(!STRCMP(Standard, STR011_STANDARD_AN)){
		standard = MODE_80211_N|MODE_80211_A;
	}else if(!STRCMP(Standard, STR011_STANDARD_BN)){
		standard = MODE_80211_N|MODE_80211_B;
	}else if(!STRCMP(Standard, STR011_STANDARD_GN)){
		standard = MODE_80211_N|MODE_80211_G;
	}else{
		standard = MODE_80211_N|MODE_80211_G|MODE_80211_B;
	}

	return standard;
}

static unsigned int GetCTSProtectMode(char* CTSProtectMode)
{
	unsigned int protmode = 0;
	
	if(!STRCMP(CTSProtectMode, X10_CTS_PROTECT_MODE_RTSCTS)){
		protmode = CTS_PROTECT_MODE_RTSCTS;
	}else if(!STRCMP(CTSProtectMode, X10_CTS_PROTECT_MODE_CTSONLY)){
		protmode = CTS_PROTECT_MODE_CTSONLY;
	}else{
		protmode = CTS_PROTECT_MODE_NONE;
	}

	return protmode;
}

static int SetEncryption(PWLANConfiguration pNewData)
{
	int ret = 0;
	int i;
	unsigned int authenticationMode = 0;
	unsigned int encryptionModes = 0;
	unsigned int encryptionModes2 = 0;
	char* WEPKey[MAX_COUNT_WEPKEYS];
	unsigned int WEPKeyType[MAX_COUNT_WEPKEYS];
	char preSharedKey[SIZE_64+1] = {0};
	
	do {
		printf("Enter %s...\n", __func__);
		if (!STRCMP(pNewData->BeaconType, STR009_BEACON_TYPE_NONE)){//Off - No Encryption
			ret = wifi_set_security_off(pNewData->Name);
		}
		else if (!STRCMP(pNewData->BeaconType, STR009_BEACON_TYPE_BASIC)){//WEP
			if (!STRCMP(pNewData->BasicEncryptionModes, STR015_BASIC_ENCRYPTION_MODES_NONE)){//Open(Off) - No Encryption
				if (!STRCMP(pNewData->BasicAuthenticationMode, STR016_BASIC_AUTHENTICATION_MODE_EAP)){//WEP+8021X
					ret = wifi_set_security_8021x(pNewData->Name, pNewData->ext8021xServerAddress, pNewData->ext8021xServerPort, pNewData->ext8021xServerSecret);
					break;
				}

				ret = wifi_set_security_off(pNewData->Name);
			}
			else if (!STRCMP(pNewData->BasicEncryptionModes, STR015_BASIC_ENCRYPTION_MODES_WEP)){//WEP
				if (!STRCMP(pNewData->BasicAuthenticationMode, STR016_BASIC_AUTHENTICATION_MODE_EAP)){//WEP+8021X
					ret = wifi_set_security_wep_8021x(pNewData->Name, pNewData->ext8021xServerAddress, pNewData->ext8021xServerPort, pNewData->ext8021xServerSecret);
					break;
				}

				if (!STRCMP(pNewData->BasicAuthenticationMode, STR016_BASIC_AUTHENTICATION_MODE_OPEN)){//WEP+OPEN
					authenticationMode = AUTHENTICATION_MODE_OPEN;
				}
				else if (!STRCMP(pNewData->BasicAuthenticationMode, STR016_BASIC_AUTHENTICATION_MODE_SHARED)){//WEP+SHARE
					authenticationMode = AUTHENTICATION_MODE_SHARED;
				}
				else if (!STRCMP(pNewData->BasicAuthenticationMode, STR016_BASIC_AUTHENTICATION_MODE_AUTO)){//WEP+AUTO
					authenticationMode = AUTHENTICATION_MODE_AUTO;
				}else{
					ret = -1;
					break;
				}
				//084 - WEPKey
				for(i=0; i<MAX_COUNT_WEPKEYS; i++){
					WEPKey[i] = pNewData->WEPKey[i].WEPKey;
					WEPKeyType[i] = pNewData->WEPKey[i].WEPKeyType;
				}
				ret = wifi_set_security_wep_only(pNewData->Name, WEPKeyType, WEPKey, pNewData->WEPKeyIndex, pNewData->extCustomedWEPKey, authenticationMode);
			}else{
				ret = -2;
				break;
			}
		}
		else if (!STRCMP(pNewData->BeaconType, STR009_BEACON_TYPE_WPA)) {//WPA
			if (!STRCMP(pNewData->WPAEncryptionModes, STR017_WPA_ENCRYPTION_MODES_TKIP)){
				encryptionModes = WPA_ENCRYPTION_MODES_TKIP;
			}
			else if (!STRCMP(pNewData->WPAEncryptionModes, STR017_WPA_ENCRYPTION_MODES_AES)){
				encryptionModes = WPA_ENCRYPTION_MODES_AES;
			}
			else if (!STRCMP(pNewData->WPAEncryptionModes, STR017_WPA_ENCRYPTION_MODES_TKIP_AES)){
				encryptionModes = WPA_ENCRYPTION_MODES_TKIP_AES;
			}else{
				ret = -3;
				break;
			}
			
			//085 - PreSharedKey
			if (!STRCMP(pNewData->WPAAuthenticationMode, STR018_WPA_AUTHENTICATION_MODE_PSK)){//WPA+PSK
				if(pNewData->extPreSharedKeyIndex<1||pNewData->extPreSharedKeyIndex>MAX_COUNT_PRESHAREDKEYS){
					pNewData->extPreSharedKeyIndex = 1;
				}
				STRNCPY(preSharedKey,pNewData->PreSharedKey[pNewData->extPreSharedKeyIndex-1].PreSharedKey,SIZE_64);
				ret = wifi_set_security_wpa_psk(pNewData->Name, WPA_MODE_WPA, 
					encryptionModes, 0, preSharedKey, 
					pNewData->extGroupKeyUpdateInterval);
			}
			else if (!STRCMP(pNewData->WPAAuthenticationMode, STR018_WPA_AUTHENTICATION_MODE_EAP)){//WPA+8021x						
				wifi_set_security_wpa_eap(pNewData->Name, WPA_MODE_WPA, encryptionModes, 0, pNewData->ext8021xServerAddress, pNewData->ext8021xServerPort, pNewData->ext8021xServerSecret, pNewData->extGroupKeyUpdateInterval);
			}else{
				ret = -4;
				break;
			}
		}
		else if (!STRCMP(pNewData->BeaconType, STR009_BEACON_TYPE_11I)){//WPA2
			if (!STRCMP(pNewData->IEEE11iEncryptionModes, STR019_IEEE11I_ENCRYPTION_MODES_TKIP)){
				encryptionModes2 = WPA_ENCRYPTION_MODES_TKIP;
			}
			else if (!STRCMP(pNewData->IEEE11iEncryptionModes, STR019_IEEE11I_ENCRYPTION_MODES_AES)){
				encryptionModes2 = WPA_ENCRYPTION_MODES_AES;
			}
			else if (!STRCMP(pNewData->IEEE11iEncryptionModes, STR019_IEEE11I_ENCRYPTION_MODES_TKIP_AES)){
				encryptionModes2 = WPA_ENCRYPTION_MODES_TKIP_AES;
			}else{
				ret = -5;
				break;
			}
			if (!STRCMP(pNewData->IEEE11iAuthenticationMode, STR020_IEEE11I_AUTHENTICATION_MODE_PSK)){//WPA2+PSK
				if(pNewData->extPreSharedKeyIndex<1||pNewData->extPreSharedKeyIndex>MAX_COUNT_PRESHAREDKEYS){
					pNewData->extPreSharedKeyIndex = 1;
				}
				STRNCPY(preSharedKey,pNewData->PreSharedKey[pNewData->extPreSharedKeyIndex-1].PreSharedKey,SIZE_64);
				ret = wifi_set_security_wpa_psk(pNewData->Name, WPA_MODE_WPA2, 
					0, encryptionModes2, preSharedKey, 
					pNewData->extGroupKeyUpdateInterval);
			}
			else if (!STRCMP(pNewData->IEEE11iAuthenticationMode, STR020_IEEE11I_AUTHENTICATION_MODE_EAP)){//WPA2+8021x						
				wifi_set_security_wpa_eap(pNewData->Name, WPA_MODE_WPA2, 0, encryptionModes2, 
					pNewData->ext8021xServerAddress, 
					pNewData->ext8021xServerPort, 
					pNewData->ext8021xServerSecret, 
					pNewData->extGroupKeyUpdateInterval);
			}
			else if (!STRCMP(pNewData->IEEE11iAuthenticationMode, STR020_IEEE11I_AUTHENTICATION_MODE_EAP_PSK)){//WPA2+EAP+PSK
			}else{
				ret = -6;
				break;
			}
		}
		else if (!STRCMP(pNewData->BeaconType, STR009_BEACON_TYPE_WPA11I)){//WPA/WPA2
			if (!STRCMP(pNewData->WPAEncryptionModes, STR017_WPA_ENCRYPTION_MODES_TKIP)){
				encryptionModes = WPA_ENCRYPTION_MODES_TKIP;
			}
			else if (!STRCMP(pNewData->WPAEncryptionModes, STR017_WPA_ENCRYPTION_MODES_AES)){
				encryptionModes = WPA_ENCRYPTION_MODES_AES;
			}
			else if (!STRCMP(pNewData->WPAEncryptionModes, STR017_WPA_ENCRYPTION_MODES_TKIP_AES)){
				encryptionModes = WPA_ENCRYPTION_MODES_TKIP_AES;
			}else{
				ret = -3;
				break;
			}

			if (!STRCMP(pNewData->IEEE11iEncryptionModes, STR019_IEEE11I_ENCRYPTION_MODES_TKIP)){
				encryptionModes2 = WPA_ENCRYPTION_MODES_TKIP;
			}
			else if (!STRCMP(pNewData->IEEE11iEncryptionModes, STR019_IEEE11I_ENCRYPTION_MODES_AES)){
				encryptionModes2 = WPA_ENCRYPTION_MODES_AES;
			}
			else if (!STRCMP(pNewData->IEEE11iEncryptionModes, STR019_IEEE11I_ENCRYPTION_MODES_TKIP_AES)){
				encryptionModes2 = WPA_ENCRYPTION_MODES_TKIP_AES;
			}else{
				ret = -7;
				break;
			}
			if (!STRCMP(pNewData->IEEE11iAuthenticationMode, STR020_IEEE11I_AUTHENTICATION_MODE_PSK)){//mixed+PSK			
				if(pNewData->extPreSharedKeyIndex<1||pNewData->extPreSharedKeyIndex>MAX_COUNT_PRESHAREDKEYS){
					pNewData->extPreSharedKeyIndex = 1;
				}
				STRNCPY(preSharedKey,pNewData->PreSharedKey[pNewData->extPreSharedKeyIndex-1].PreSharedKey,SIZE_64);

				ret = wifi_set_security_wpa_psk(pNewData->Name, WPA_MODE_MIXED, 
					encryptionModes, encryptionModes2, preSharedKey, 
					pNewData->extGroupKeyUpdateInterval);
			}
			else if (!STRCMP(pNewData->IEEE11iAuthenticationMode, "EAPAuthentication")){//mixed+8021x						
				wifi_set_security_wpa_eap(pNewData->Name, WPA_MODE_MIXED, encryptionModes, encryptionModes2, pNewData->ext8021xServerAddress, pNewData->ext8021xServerPort, pNewData->ext8021xServerSecret, pNewData->extGroupKeyUpdateInterval);
			}else{
				ret = -8;
				break;
			}
		}else{
			ret = -9;
			break;
		}
	} while(0);
		
	return ret;
}

static int SetWPS(PWLANConfiguration pNewData, unsigned int first)
{
	int ret = 0;
	unsigned int eap_server = 0;
	unsigned int wps_state=WPS_STATE_DISABLED;
	unsigned int wpa_psk = 0;
	int i;

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

/**************************************************************************
 *	
 *	        interface function definition
 *
 **************************************************************************/

int tr098WiFiObjectDestroy(unsigned int objFlag, PWLANConfiguration pCurData)
{
	if(objFlag&OBJ_FLAG_WLAN){
		StopWifiDaemon(pCurData->Name);
		return wifi_destroy_ssid_device(pCurData->Name);
	}

	return 0;
}

int tr098WiFiObjectCreate(unsigned int objFlag, PWLANConfiguration pNewData)
{
	int ret = 0;
	char name[SIZE_32];
	unsigned int device_up_flag = 0;
	unsigned int devOperationMode = 0;
	int index = -1;
	int i;

	do{
		if(objFlag&OBJ_FLAG_WLAN){
			//037 - DeviceOperationMode
			devOperationMode = GetDeviceOperationMode(pNewData->DeviceOperationMode);
			if(0==devOperationMode){//invalid DeviceOperationMode
				ret = -1;
				break;
			}

			if(pNewData->Name[0]){
				wifi_destroy_ssid_device(pNewData->Name);
			}

			//x15
			ret = wifi_create_ssid_device(pNewData->extRadioDevName, devOperationMode, name);
			if(ret<0){
				break;
			}
			//003 - Name
			STRNCPY(pNewData->Name, name, sizeof(pNewData->Name)-1);
			StartWifiDaemon(pNewData->Name);
			
			device_up_flag = wifi_close_all_virtual_devices(pNewData->extRadioDevName);
			
			//028 - RadioEnabled
			if(!pNewData->RadioEnabled){
				pNewData->Enable = 0;
			}
			//001 - Enable
			if(pNewData->Enable){
				index = wifi_get_device_index(name);
				device_up_flag |= (0x01<<index);
			}
			//x15
			wifi_close_radio(pNewData->extRadioDevName);
			
			///////////////////////////////////////////////////////////////////////
			// Set Radio Parameters
			///////////////////////////////////////////////////////////////////////
			//030 - TransmitPower	
			if (pNewData->TransmitPower>100){
				pNewData->TransmitPower = 100;
			}
			wifi_set_radio_TransmitPower(name,pNewData->TransmitPower);
			//011 - Standard	
			wifi_set_radio_standards(name, GetStandard(pNewData->Standard));	
			//006 - Channel
			//007 - AutoChannelEnable
			//x22 - extChannelWidth	
			//x23 - extControlSideBand	
			wifi_set_radio_channel_info(name,pNewData->extChannelWidth, pNewData->extControlSideBand,pNewData->AutoChannelEnable,0,pNewData->Channel);
			//002 - Status (ReadOnly)
			//005 - MaxBitRate
			wifi_set_radio_MaxBitRate(name, pNewData->MaxBitRate);
			//008 - SSID
			if (0==pNewData->SSID[0]){
				if(wifi_get_default_ssid(pNewData->extRadioDevName, name, pNewData->SSID)<0){
					sprintf(pNewData->SSID, "%.08X", (unsigned int)time(0));
				}
			}	
			wifi_set_ssid(name, pNewData->SSID);
			//010 - MACAddressControlEnabled
			//x13 - extMACPolicy
			//x14 - extMACList
			//x30 - extACLComment
			wifi_set_acl(name, pNewData->MACAddressControlEnabled, pNewData->extMACList, pNewData->extACLComment, X13_ACL_POLICY_DENY==pNewData->extMACPolicy);
			//014 - WEPEncryptionLevel (ReadOnly)
			//021 - PossibleChannels (ReadOnly)
			//022 - BasicDataTransmitRates
			wifi_set_radio_BasicDataTransmitRates(name, pNewData->BasicDataTransmitRates);
			//023 - OperationalDataTransmitRates
			wifi_set_radio_OperationalDataTransmitRates(name, pNewData->OperationalDataTransmitRates);
			//024 - PossibleDataTransmitRates (ReadOnly)   
			//025 - InsecureOOBAccessEnabled	   
			wifi_set_InsecureOOBAccessEnabled(name, pNewData->InsecureOOBAccessEnabled);
			//026 - BeaconAdvertisementEnabled
			wifi_set_BeaconAdvertisementEnabled(name, pNewData->BeaconAdvertisementEnabled);
			if (pNewData->BeaconAdvertisementEnabled){
				//027 - SSIDAdvertisementEnabled
				wifi_set_SSIDAdvertisementEnabled(name, pNewData->SSIDAdvertisementEnabled);
			}
			//029 - TransmitPowerSupported (ReadOnly)	   
			//031 - AutoRateFallBackEnabled
			wifi_set_AutoRateFallBackEnabled(name, pNewData->AutoRateFallBackEnabled);
			//032 - LocationDescription
			wifi_set_LocationDescription(name, pNewData->LocationDescription);
			//033 - RegulatoryDomain
			wifi_set_radio_RegulatoryDomain(name, pNewData->RegulatoryDomain);
			//034 - TotalPSKFailures (ReadOnly)
			//035 - TotalIntegrityFailures (ReadOnly)
			//036 - ChannelsInUse (ReadOnly)
			//041 - WMMSupported (ReadOnly)
			//042 - UAPSDSupported (ReadOnly)
			//043 - WMMEnable
			wifi_set_WMMEnable(name, pNewData->WMMEnable);
			//044 - UAPSDEnable
			if(!pNewData->WMMEnable){
				pNewData->UAPSDEnable = 0;
			}
			wifi_set_UAPSDEnable(name, pNewData->UAPSDEnable);
			//045 - TotalBytesSent (ReadOnly)
			//046 - TotalBytesReceived (ReadOnly)
			//047 - TotalPacketsSent (ReadOnly)
			//048 - TotalPacketsReceived (ReadOnly)
			//049 - TotalAssociations (ReadOnly)
			
			//x06 - extBeaconInterval
			wifi_set_BeaconInterval(name, pNewData->extBeaconInterval);
			//x07 - extDTIMInterval
			wifi_set_DTIMInterval(name, pNewData->extDTIMInterval);
			//x08 - extFragmentationThreshold
			wifi_set_FragmentationThreshold(name, pNewData->extFragmentationThreshold);
			//x09 - extRTSThreshold
			wifi_set_RTSThreshold(name, pNewData->extRTSThreshold);
			//x10 - extCTSProtectMode
			wifi_set_CTSProtectMode(name, GetCTSProtectMode(pNewData->extCTSProtectMode));
			//x11 - extMSDULimit
			wifi_set_MSDULimit(name, pNewData->extMSDULimit);
			//x12 - extMPDULimit
			wifi_set_MPDULimit(name, pNewData->extMPDULimit);
			//x16 - extWDS
			wifi_set_wds(name, pNewData->extWDS);
			//x24 - extPreambleType
			wifi_set_PreambleType(name, pNewData->extPreambleType);
			//x25 - extIAPPEnabled
			wifi_set_IAPPEnable(name, pNewData->extIAPPEnable);
			//x26 - extShortGuardInterval
			wifi_set_radio_GuardInterval(name, pNewData->extShortGuardInterval);
			//x27 - extSpaceTimeBlockCoding
			wifi_set_SpaceTimeBlockCoding(name, pNewData->extSpaceTimeBlockCoding);
		}
		switch(devOperationMode){
			case DEVICE_OPERATION_MODE_ACCESSPOINT:
				if(objFlag&OBJ_FLAG_WLAN){
					//004 - BSSID = '00:03:7F:BE:F1:13' (ReadOnly)
					//040 - AuthenticationServiceMode
					wifi_set_AuthenticationServiceMode(name, pNewData->AuthenticationServiceMode);
					//x17 - extAPBridge
					wifi_set_IsolationEnable(name, !pNewData->extAPBridge);
				}
				if(objFlag&OBJ_FLAG_AP_WMM_PARAMETER){
					//088 - AIFSN
					//089 - ECWMin
					//090 - ECWMax
					//091 - TXOP
					//092 - AckPolicy
					for(i=0; i<MAX_COUNT_WMM_ENTRIES; i++){
						wifi_set_wmm_parameters(pNewData->Name, i, pNewData->APWMMParameter[i].AIFSN,
							pow(2, pNewData->APWMMParameter[i].ECWMin)-1, pow(2, pNewData->APWMMParameter[i].ECWMax)-1,
							pNewData->APWMMParameter[i].TXOP*32, !pNewData->APWMMParameter[i].AckPolicy, 1);
					}
				}
				break;
			case DEVICE_OPERATION_MODE_STATION:
				if(objFlag&OBJ_FLAG_WLAN){
					//004 - BSSID = '00:03:7F:BE:F1:13' 
					wifi_set_bssid(name, pNewData->BSSID);
				}
				if(objFlag&OBJ_FLAG_STA_WMM_PARAMETER){
					//093 - AIFSN
					//094 - ECWMin
					//095 - ECWMax
					//096 - TXOP
					//097 - AckPolicy
					for(i=0; i<MAX_COUNT_WMM_ENTRIES; i++){
						wifi_set_wmm_parameters(pNewData->Name, i, pNewData->STAWMMParameter[i].AIFSN,
							pow(2, pNewData->STAWMMParameter[i].ECWMin)-1, pow(2, pNewData->STAWMMParameter[i].ECWMax)-1,
							pNewData->STAWMMParameter[i].TXOP*32, !pNewData->STAWMMParameter[i].AckPolicy, 0);
					}
				}
				break;
				case DEVICE_OPERATION_MODE_BRIDGE:
				case DEVICE_OPERATION_MODE_REPEATER:
				//038 - DistanceFromRoot
				wifi_set_DistanceFromRoot(name, pNewData->DistanceFromRoot);
				//039 - PeerBSSID
				wifi_set_PeerBSSID(name, pNewData->PeerBSSID);
				break;
		}
		if(objFlag&(OBJ_FLAG_WLAN|OBJ_FLAG_WEP_KEY|OBJ_FLAG_PRE_SHARED_KEY)){
			///////////////////////////////////////////////////////////////////////
			// Set Encryption Parameters
			///////////////////////////////////////////////////////////////////////
			//013 - KeyPassphrase	(default)	
			//009 - BeaconType
			//012 - WEPKeyIndex
			//015 - BasicEncryptionModes
			//016 - BasicAuthenticationMode
			//017 - WPAEncryptionModes
			//018 - WPAAuthenticationMode
			//019 - IEEE11iEncryptionModes
			//020 - IEEE11iAuthenticationMode
			//084 - WEPKey
			//085 - PreSharedKey
			//x01 - extPreSharedKeyIndex
			//x02 - extGroupKeyUpdateInterval
			//x03 - ext8021xServerAddress
			//x04 - ext8021xServerPort
			//x05 - ext8021xServerSecret
			SetEncryption(pNewData);
		}
		if(objFlag&(OBJ_FLAG_WLAN|OBJ_FLAG_WPS|OBJ_FLAG_WPS_REGISTRAR)){
			//WPS
			//061 - Enable
			//062 - DeviceName
			//063 - DevicePassword
			//067 - ConfigMethodsEnabled
			//069 - SetupLock
			//070 - ConfigurationState
			//074 - Enable
			SetWPS(pNewData, 1);
		}
		///////////////////////////////////////////////////////////////////////
		
		if(objFlag&OBJ_FLAG_WLAN){
			if(pNewData->RadioEnabled){
				wifi_open_radio(pNewData->extRadioDevName);
				wifi_open_all_virtual_devices(pNewData->extRadioDevName, device_up_flag);
				if(pNewData->Enable){
					if(0==pNewData->Channel){
						sleep(TIME_SEARCH_CHANNEL);
					}
				}
			}
		}

		wifi_commit(pNewData->Name);
#ifdef WIFI_DRIVER_REALTEK	
		//044 - UAPSDEnable
		if(!pNewData->WMMEnable){
			pNewData->UAPSDEnable = 0;
		}
		wifi_set_UAPSDEnable(pNewData->Name, pNewData->UAPSDEnable);
#endif		
	}while(0);

	return ret;
}

int tr098WiFiObjectModify(unsigned int objFlag, PWLANConfiguration pCurData, PWLANConfiguration pNewData)
{
	int ret = 0;
	char name[SIZE_32];
	unsigned int device_up_flag = 0;
	unsigned int devOperationMode = 0;
	int i;

	do{
		printf("Enter %s...\n", __func__);
		if(objFlag){
			//037 - DeviceOperationMode
			devOperationMode = GetDeviceOperationMode(pNewData->DeviceOperationMode);
			if(0==devOperationMode){//invalid DeviceOperationMode
				ret = -1;
				break;
			}
			if(STRCMP(pNewData->DeviceOperationMode,pCurData->DeviceOperationMode)){
				StopWifiDaemon(pCurData->Name);
				wifi_destroy_ssid_device(pCurData->Name);
				//x15
				printf("name=[%s]\n\n", pNewData->Name);
				ret = wifi_create_ssid_device(pNewData->extRadioDevName, devOperationMode, name);
				printf("ret=%d, extRadioDevName=%s, name=[%s], devOperationMode=%d\n\n", ret, pNewData->extRadioDevName, name, devOperationMode);
				if(ret<0){
					break;
				}
				//003 - Name
				STRNCPY(pNewData->Name, name, sizeof(pNewData->Name)-1);
				StartWifiDaemon(pNewData->Name);
			}
			
			STRNCPY(name, pNewData->Name, sizeof(name)-1);
		}
		
		if(objFlag&OBJ_FLAG_WLAN){
			wifi_close_device(name);
			
			//028 - RadioEnabled
			if(!pNewData->RadioEnabled){
				pNewData->Enable = 0;
				wifi_close_radio(pNewData->extRadioDevName);
			}
			
			///////////////////////////////////////////////////////////////////////
			// Set Radio Parameters
			///////////////////////////////////////////////////////////////////////
			if(pNewData->AutoChannelEnable!=pCurData->AutoChannelEnable
				|| pNewData->Channel!=pCurData->Channel
				|| pNewData->TransmitPower!=pCurData->TransmitPower
				|| STRCMP(pNewData->Standard, pCurData->Standard)
                || (pNewData->X_ACTIONTEC_COM_AutoChannelRefresh && pNewData->AutoChannelEnable)
                ){
				device_up_flag = wifi_close_all_virtual_devices(pNewData->extRadioDevName);
				wifi_close_radio(pNewData->extRadioDevName);
				
				//030 - TransmitPower	
				if (pNewData->TransmitPower>100){
					pNewData->TransmitPower = 100;
				}
				wifi_set_radio_TransmitPower(name,pNewData->TransmitPower);
				//011 - Standard	
				wifi_set_radio_standards(name, GetStandard(pNewData->Standard));	
				//006 - Channel
				//007 - AutoChannelEnable
				//x22 - extChannelWidth 
				//x23 - extControlSideBand	
				wifi_set_radio_channel_info(name,pNewData->extChannelWidth, pNewData->extControlSideBand,pNewData->AutoChannelEnable,0,pNewData->Channel);
			}
			///////////////////////////////////////////////////////////////////////
			//002 - Status (ReadOnly)
			//005 - MaxBitRate
			if(STRCMP(pNewData->MaxBitRate, pCurData->MaxBitRate)){
				wifi_set_radio_MaxBitRate(name, pNewData->MaxBitRate);
			}
			//008 - SSID
			if (STRCMP(pNewData->SSID, pCurData->SSID)
				&&0!=pNewData->SSID[0]){
				wifi_set_ssid(name, pNewData->SSID);
			}	
			
			//010 - MACAddressControlEnabled
			//x13 - extMACPolicy
			//x14 - extMACList
			//x30 - extACLComment
			if(pNewData->MACAddressControlEnabled!=pCurData->MACAddressControlEnabled
				|| pNewData->extMACPolicy!=pCurData->extMACPolicy
				|| strcmp(pNewData->extMACList, pCurData->extMACList)){
				wifi_set_acl(name, pNewData->MACAddressControlEnabled, pNewData->extMACList, pNewData->extACLComment, X13_ACL_POLICY_DENY==pNewData->extMACPolicy);
			}
			//014 - WEPEncryptionLevel (ReadOnly)
			//021 - PossibleChannels (ReadOnly)
			//022 - BasicDataTransmitRates
			if(STRCMP(pNewData->BasicDataTransmitRates, pCurData->BasicDataTransmitRates)){
				wifi_set_radio_BasicDataTransmitRates(name, pNewData->BasicDataTransmitRates);
			}
			//023 - OperationalDataTransmitRates
			if(STRCMP(pNewData->OperationalDataTransmitRates, pCurData->OperationalDataTransmitRates)){
				wifi_set_radio_OperationalDataTransmitRates(name, pNewData->OperationalDataTransmitRates);
			}
			//024 - PossibleDataTransmitRates (ReadOnly)   
			//025 - InsecureOOBAccessEnabled
			if(pNewData->InsecureOOBAccessEnabled!=pCurData->InsecureOOBAccessEnabled){
				wifi_set_InsecureOOBAccessEnabled(name, pNewData->InsecureOOBAccessEnabled);
			}
			//026 - BeaconAdvertisementEnabled
			//027 - SSIDAdvertisementEnabled
			if(pNewData->BeaconAdvertisementEnabled!=pCurData->BeaconAdvertisementEnabled
				|| pNewData->SSIDAdvertisementEnabled!=pCurData->SSIDAdvertisementEnabled){
				wifi_set_BeaconAdvertisementEnabled(name, pNewData->BeaconAdvertisementEnabled);
				if (pNewData->BeaconAdvertisementEnabled){
					wifi_set_SSIDAdvertisementEnabled(name, pNewData->SSIDAdvertisementEnabled);
				}
			}
			//029 - TransmitPowerSupported (ReadOnly)	   
			//031 - AutoRateFallBackEnabled
			if(pNewData->AutoRateFallBackEnabled!=pCurData->AutoRateFallBackEnabled){
				wifi_set_AutoRateFallBackEnabled(name, pNewData->AutoRateFallBackEnabled);
			}
			//032 - LocationDescription
			if(STRCMP(pNewData->LocationDescription, pCurData->LocationDescription)){
				wifi_set_LocationDescription(name, pNewData->LocationDescription);
			}
			//033 - RegulatoryDomain
			if(STRCMP(pNewData->RegulatoryDomain, pCurData->RegulatoryDomain)){
				wifi_set_radio_RegulatoryDomain(name, pNewData->RegulatoryDomain);
			}
			//034 - TotalPSKFailures (ReadOnly)
			//035 - TotalIntegrityFailures (ReadOnly)
			//036 - ChannelsInUse (ReadOnly)
			//041 - WMMSupported (ReadOnly)
			//043 - WMMEnable
			if(pNewData->WMMEnable!=pCurData->WMMEnable){
				wifi_set_WMMEnable(name, pNewData->WMMEnable);
			}
			//042 - UAPSDSupported (ReadOnly)
			//044 - UAPSDEnable
			if(pNewData->UAPSDEnable!=pCurData->UAPSDEnable){
				if(!pNewData->WMMEnable){
					pNewData->UAPSDEnable = 0;
				}
				wifi_set_UAPSDEnable(name, pNewData->UAPSDEnable);
			}
			//045 - TotalBytesSent (ReadOnly)
			//046 - TotalBytesReceived (ReadOnly)
			//047 - TotalPacketsSent (ReadOnly)
			//048 - TotalPacketsReceived (ReadOnly)
			//049 - TotalAssociations (ReadOnly)
			
			//x06 - extBeaconInterval
			if(pNewData->extBeaconInterval!=pCurData->extBeaconInterval){
				wifi_set_BeaconInterval(name, pNewData->extBeaconInterval);
			}
			//x07 - extDTIMInterval
			if(pNewData->extDTIMInterval!=pCurData->extDTIMInterval){
				wifi_set_DTIMInterval(name, pNewData->extDTIMInterval);
			}
			//x08 - extFragmentationThreshold
			if(pNewData->extFragmentationThreshold!=pCurData->extFragmentationThreshold){
				wifi_set_FragmentationThreshold(name, pNewData->extFragmentationThreshold);
			}
			//x09 - extRTSThreshold
			if(pNewData->extRTSThreshold!=pCurData->extRTSThreshold){
				wifi_set_RTSThreshold(name, pNewData->extRTSThreshold);
			}
			//x10 - extCTSProtectMode
			if(STRCMP(pNewData->extCTSProtectMode, pCurData->extCTSProtectMode)){
				wifi_set_CTSProtectMode(name, GetCTSProtectMode(pNewData->extCTSProtectMode));
			}
			//x11 - extMSDULimit
			if(pNewData->extMSDULimit!=pCurData->extMSDULimit){
				wifi_set_MSDULimit(name, pNewData->extMSDULimit);
			}
			//x12 - extMPDULimit
			if(pNewData->extMPDULimit!=pCurData->extMPDULimit){
				wifi_set_MPDULimit(name, pNewData->extMPDULimit);
			}
			//x16 - extWDS
			if(pNewData->extWDS!=pCurData->extWDS){
				wifi_set_wds(name, pNewData->extWDS);
			}
			//x24 - extPreambleType
			if(pNewData->extPreambleType!=pCurData->extPreambleType){
				wifi_set_PreambleType(name, pNewData->extPreambleType);
			}
			//x25 - extIAPPEnable
			if(pNewData->extIAPPEnable!=pCurData->extIAPPEnable){
				wifi_set_IAPPEnable(name, pNewData->extIAPPEnable);
			}
			//x26 - extShortGuardInterval
			if(pNewData->extShortGuardInterval!=pCurData->extShortGuardInterval){
				wifi_set_radio_GuardInterval(name, pNewData->extShortGuardInterval);
			}
			//x27 - extSpaceTimeBlockCoding
			if(pNewData->extSpaceTimeBlockCoding!=pCurData->extSpaceTimeBlockCoding){
				wifi_set_SpaceTimeBlockCoding(name, pNewData->extSpaceTimeBlockCoding);
			}
		}
		switch(devOperationMode){
			case DEVICE_OPERATION_MODE_ACCESSPOINT:
				if(objFlag&OBJ_FLAG_WLAN){
					//004 - BSSID = '00:03:7F:BE:F1:13' (ReadOnly)
					//040 - AuthenticationServiceMode
					if(STRCMP(pNewData->AuthenticationServiceMode, pCurData->AuthenticationServiceMode)){
						wifi_set_AuthenticationServiceMode(name, pNewData->AuthenticationServiceMode);
					}
					//x17 - extAPBridge
					if(pNewData->extAPBridge!=pCurData->extAPBridge){
						wifi_set_IsolationEnable(name, !pNewData->extAPBridge);
					}
				}
				if(objFlag&OBJ_FLAG_AP_WMM_PARAMETER){
					//088 - AIFSN
					//089 - ECWMin
					//090 - ECWMax
					//091 - TXOP
					//092 - AckPolicy
					for(i=0; i<MAX_COUNT_WMM_ENTRIES; i++){
						if(pNewData->APWMMParameter[i].AIFSN!=pCurData->APWMMParameter[i].AIFSN
							||pNewData->APWMMParameter[i].ECWMin!=pCurData->APWMMParameter[i].ECWMin
							||pNewData->APWMMParameter[i].ECWMax!=pCurData->APWMMParameter[i].ECWMax
							||pNewData->APWMMParameter[i].TXOP!=pCurData->APWMMParameter[i].TXOP
							||pNewData->APWMMParameter[i].AckPolicy!=pCurData->APWMMParameter[i].AckPolicy
							){
							wifi_set_wmm_parameters(name, i, pNewData->APWMMParameter[i].AIFSN,
								pow(2, pNewData->APWMMParameter[i].ECWMin)-1, pow(2, pNewData->APWMMParameter[i].ECWMax)-1,
								pNewData->APWMMParameter[i].TXOP*32, !pNewData->APWMMParameter[i].AckPolicy, 1);
						}
					}
				}
				break;
			case DEVICE_OPERATION_MODE_STATION:
				if(objFlag&OBJ_FLAG_WLAN){
					//004 - BSSID = '00:03:7F:BE:F1:13'
					if(STRCMP(pNewData->BSSID, pCurData->BSSID)){
						wifi_set_bssid(name, pNewData->BSSID);
					}
				}
				if(objFlag&OBJ_FLAG_STA_WMM_PARAMETER){
					//093 - AIFSN
					//094 - ECWMin
					//095 - ECWMax
					//096 - TXOP
					//097 - AckPolicy
					for(i=0; i<MAX_COUNT_WMM_ENTRIES; i++){
						if(pNewData->STAWMMParameter[i].AIFSN!=pCurData->STAWMMParameter[i].AIFSN
							|| pNewData->STAWMMParameter[i].ECWMin!=pCurData->STAWMMParameter[i].ECWMin
							|| pNewData->STAWMMParameter[i].ECWMax!=pCurData->STAWMMParameter[i].ECWMax
							|| pNewData->STAWMMParameter[i].TXOP!=pCurData->STAWMMParameter[i].TXOP
							|| pNewData->STAWMMParameter[i].AckPolicy!=pCurData->STAWMMParameter[i].AckPolicy
							){
							wifi_set_wmm_parameters(name, i, pNewData->STAWMMParameter[i].AIFSN,
								pow(2, pNewData->STAWMMParameter[i].ECWMin)-1, pow(2, pNewData->STAWMMParameter[i].ECWMax)-1,
								pNewData->STAWMMParameter[i].TXOP*32, !pNewData->STAWMMParameter[i].AckPolicy, 0);
						}
					}
				}
				break;
			case DEVICE_OPERATION_MODE_BRIDGE:
			case DEVICE_OPERATION_MODE_REPEATER:
				//038 - DistanceFromRoot
				if(pNewData->DistanceFromRoot!=pCurData->DistanceFromRoot){
					wifi_set_DistanceFromRoot(name, pNewData->DistanceFromRoot);
				}
				//039 - PeerBSSID
				if(STRCMP(pNewData->PeerBSSID, pCurData->PeerBSSID)){
					wifi_set_PeerBSSID(name, pNewData->PeerBSSID);
				}
				break;
		}
		if(objFlag&(OBJ_FLAG_WLAN|OBJ_FLAG_WEP_KEY|OBJ_FLAG_PRE_SHARED_KEY)){
			///////////////////////////////////////////////////////////////////////
			// Set Encryption Parameters
			///////////////////////////////////////////////////////////////////////
			//013 - KeyPassphrase
			if(STRCMP(pNewData->KeyPassphrase, pCurData->KeyPassphrase)
				&&strlen(pNewData->KeyPassphrase)>=SIZE_8
				&&strlen(pNewData->KeyPassphrase)<SIZE_64){
				unsigned char buff[SIZE_26+1] = {0};
				ret = DoKeyPassphraseChanged(pNewData->KeyPassphrase, pNewData->BSSID, buff, SIZE_26>>1);
				if(0==ret){
					if(SIZE_26==Hex2String(buff, SIZE_26>>1, (char*)buff)){
						for(i=0; i<MAX_COUNT_WEPKEYS; i++){
							STRNCPY(pNewData->WEPKey[i].WEPKey, (char*)buff, sizeof(pNewData->WEPKey[i].WEPKey)-1);
							printf("[%s:%d] wepkey%d=%s\n", __func__, __LINE__, i, pNewData->WEPKey[i].WEPKey);
						}
					}
					STRNCPY(pNewData->PreSharedKey[0].KeyPassphrase,pNewData->KeyPassphrase,SIZE_64);
				}
			}
			//009 - BeaconType
			//012 - WEPKeyIndex
			//015 - BasicEncryptionModes
			//016 - BasicAuthenticationMode
			//017 - WPAEncryptionModes
			//018 - WPAAuthenticationMode
			//019 - IEEE11iEncryptionModes
			//020 - IEEE11iAuthenticationMode
			//084 - WEPKey
			//085 - PreSharedKey
			//086 - KeyPassphrase
			//087 - AssociatedDeviceMACAddress
			//x01 - extPreSharedKeyIndex
			for(i=0; i<MAX_COUNT_PRESHAREDKEYS; i++){
				if(STRCMP(pNewData->PreSharedKey[i].KeyPassphrase, pCurData->PreSharedKey[i].KeyPassphrase)
					&&strlen(pNewData->PreSharedKey[i].KeyPassphrase)>=SIZE_8
					&&strlen(pNewData->PreSharedKey[i].KeyPassphrase)<SIZE_64){
					unsigned char buff[SIZE_64+1] = {0};
					ret = DoKeyPassphraseChanged(
						pNewData->PreSharedKey[i].KeyPassphrase, 
						pNewData->PreSharedKey[i].AssociatedDeviceMACAddress, 
						buff, SIZE_32);
					int j;
					for(j=0; j<32; j++){
						printf("%.02x ", buff[j]);
					}
					if(0==ret){
						if(SIZE_64==Hex2String(buff, SIZE_32, (char*)buff)){
							STRNCPY(pNewData->PreSharedKey[i].PreSharedKey,(char*)buff,SIZE_64);
							printf("i=%d, KeyPassphrase=%s, PreSharedKey=%s\n", i,
								pNewData->PreSharedKey[i].KeyPassphrase,
								pNewData->PreSharedKey[i].PreSharedKey);
						}
						if(0==i){
							STRNCPY(pNewData->KeyPassphrase,pNewData->PreSharedKey[i].KeyPassphrase,SIZE_64);
						}
					}
				}
			}
			//x02 - extGroupKeyUpdateInterval
			//x03 - ext8021xServerAddress
			//x04 - ext8021xServerPort
			//x05 - ext8021xServerSecret
			SetEncryption(pNewData);
		}
		if(objFlag&(OBJ_FLAG_WLAN|OBJ_FLAG_WPS|OBJ_FLAG_WPS_REGISTRAR)){
			//WPS
			//061 - Enable
			//062 - DeviceName
			//063 - DevicePassword
			//067 - ConfigMethodsEnabled
			//069 - SetupLock
			//070 - ConfigurationState
			//074 - Enable
			SetWPS(pNewData, 0);
		}
		///////////////////////////////////////////////////////////////////////
		
		if(objFlag&OBJ_FLAG_WLAN){
			if(pNewData->RadioEnabled){
				wifi_open_radio(pNewData->extRadioDevName);
				wifi_open_all_virtual_devices(pNewData->extRadioDevName, device_up_flag);
				if(pNewData->Enable){
					wifi_open_device(name);
					if(0==pNewData->Channel){
						sleep(TIME_SEARCH_CHANNEL);
					}
				}else{
					wifi_close_device(name);
				}
			}
		}
		
		wifi_commit(pNewData->Name);
#ifdef WIFI_DRIVER_REALTEK	
		//044 - UAPSDEnable
		if(pNewData->UAPSDEnable!=pCurData->UAPSDEnable){
			if(!pNewData->WMMEnable){
				pNewData->UAPSDEnable = 0;
			}
			wifi_set_UAPSDEnable(pNewData->Name, pNewData->UAPSDEnable);
		}
#endif		
	}while(0);

	return ret;
}
	
int tr098WiFiObjectGet(unsigned int objFlag, PWLANConfiguration pCurData)
{
	int ret = 0;
	int  status = 0;
	char bssid[SIZE_20] = {0};
	char buff[SIZE_64];
	char uuid[SIZE_40];
	unsigned int i;
	unsigned int aifs, cwmin, cwmax, txoplimit, noackpolicy;

	do{
		if(objFlag&OBJ_FLAG_WLAN){
			//02 - Status 
			status = wifi_get_device_status(pCurData->Name);
			switch(status){
				case DEVICE_STATUS_Up:
					strcpy(pCurData->Status, STR002_STATUS_UP);
					break;
				case DEVICE_STATUS_Down:
					strcpy(pCurData->Status, STR002_STATUS_DISABLED);
					break;
				case DEVICE_STATUS_Dormant:
					strcpy(pCurData->Status, STR002_STATUS_CONNECTING);
					break;
				default:
					strcpy(pCurData->Status, STR002_STATUS_ERROR);
					break;
			}
			//04 - BSSID
			if(0==wifi_get_BSSID(pCurData->Name, bssid)){
				STRNCPY(pCurData->BSSID, bssid, sizeof(pCurData->BSSID)-1);
			}
			//06 - Channel
			pCurData->Channel = wifi_get_radio_channel(pCurData->Name);
						
			//14 - WEPEncryptionLevel 
			if(0==wifi_get_WEPEncryptionLevel(pCurData->Name, buff)){
				STRNCPY(pCurData->WEPEncryptionLevel, buff, sizeof(pCurData->WEPEncryptionLevel)-1);
			}
			//21 - PossibleChannels 
			if(0==wifi_get_radio_PossibleChannels(pCurData->Name, buff, sizeof(buff))){
				STRNCPY(pCurData->PossibleChannels, buff, sizeof(pCurData->PossibleChannels)-1);
			}
			//24 - PossibleDataTransmitRates
			if(0==wifi_get_radio_PossibleDataTransmitRates(pCurData->Name, buff)){
				STRNCPY(pCurData->PossibleDataTransmitRates, buff, sizeof(pCurData->PossibleDataTransmitRates)-1);
			}
			//29 - TransmitPowerSupported 
			if(0==wifi_get_radio_TransmitPowerSupported(pCurData->Name, buff, sizeof(buff))){
				STRNCPY(pCurData->TransmitPowerSupported, buff, sizeof(pCurData->TransmitPowerSupported)-1);
			}
			//34 - TotalPSKFailures -relevant only to WPA and WPA2
			pCurData->TotalPSKFailures = wifi_get_TotalPSKFailures(pCurData->Name);
			//35 - TotalIntegrityFailures -relevant only to WPA and WPA2
			pCurData->TotalIntegrityFailures = wifi_get_TotalIntegrityFailures(pCurData->Name);
			//36 - ChannelsInUse 
			if(0==wifi_get_radio_ChannelsInUse(pCurData->Name, buff)){
				STRNCPY(pCurData->ChannelsInUse, buff, sizeof(pCurData->ChannelsInUse)-1);
			}
			//41 - WMMSupported	
			pCurData->WMMSupported = wifi_get_WMMSupported(pCurData->Name);
			//42 - UAPSDSupported
			pCurData->UAPSDSupported = wifi_get_UAPSDSupported(pCurData->Name);
			//45 - TotalBytesSent 
			pCurData->TotalBytesSent = wifi_get_BytesSent(pCurData->Name);
			//46 - TotalBytesReceived 
			pCurData->TotalBytesReceived = wifi_get_BytesReceived(pCurData->Name);
			//47 - TotalPacketsSent 
			pCurData->TotalPacketsSent = wifi_get_PacketsSent(pCurData->Name);
			//48 - TotalPacketsReceived 
			pCurData->TotalPacketsReceived = wifi_get_PacketsReceived(pCurData->Name);
			//49 - TotalAssociations 	
			pCurData->TotalAssociations = wifi_get_associated_device_count(pCurData->Name);
			
		}
		
		if(objFlag&OBJ_FLAG_STATISTICS){
			//050 - ErrorsSent
			pCurData->Stats.ErrorsSent = wifi_get_ErrorsSent(pCurData->Name);
			//051 - ErrorsReceived
			pCurData->Stats.ErrorsReceived = wifi_get_ErrorsReceived(pCurData->Name);
			//052 - UnicastPacketsSent
			pCurData->Stats.UnicastPacketsSent = wifi_get_UnicastPacketsSent(pCurData->Name);
			//053 - UnicastPacketsReceived
			pCurData->Stats.UnicastPacketsReceived = wifi_get_UnicastPacketsReceived(pCurData->Name);
			//054 - DiscardPacketsSent
			pCurData->Stats.DiscardPacketsSent = wifi_get_DiscardPacketsSent(pCurData->Name);
			//055 - DiscardPacketsReceived
			pCurData->Stats.DiscardPacketsReceived = wifi_get_DiscardPacketsReceived(pCurData->Name);
			//056 - MulticastPacketsSent
			pCurData->Stats.MulticastPacketsSent = wifi_get_MulticastPacketsSent(pCurData->Name);
			//057 - MulticastPacketsReceived
			pCurData->Stats.MulticastPacketsReceived = wifi_get_MulticastPacketsReceived(pCurData->Name);
			//058 - BroadcastPacketsSent
			pCurData->Stats.BroadcastPacketsSent = wifi_get_BroadcastPacketsSent(pCurData->Name);
			//059 - BroadcastPacketsReceived
			pCurData->Stats.BroadcastPacketsReceived = wifi_get_BroadcastPacketsReceived(pCurData->Name);
			//060 - UnknownProtoPacketsReceived
			pCurData->Stats.UnknownProtoPacketsReceived = wifi_get_UnknownProtoPacketsReceived(pCurData->Name);
		}
		
		if(objFlag&OBJ_FLAG_WPS){
			//063 - DevicePassword
			pCurData->WPS.DevicePassword = wifi_get_wps_ap_pin(pCurData->Name);
			//064 - UUID
			if(0==wifi_get_wps_uuid(pCurData->Name, uuid, sizeof(uuid))){
				STRNCPY(pCurData->WPS.UUID, uuid, sizeof(pCurData->WPS.UUID)-1);
			}
			//065 - Version
			pCurData->WPS.Version = wifi_get_wps_version(pCurData->Name);
			//066 - ConfigMethodsSupported
			unsigned int flags = wifi_get_wps_ConfigMethodsSupported(pCurData->Name);
			memset(pCurData->WPS.ConfigMethodsSupported, 0, sizeof(pCurData->WPS.ConfigMethodsSupported));
			if(flags&WPS_CONFIG_METHODS_FLAG_USBFLASHDRIVER){
				strcat(pCurData->WPS.ConfigMethodsSupported, pCurData->WPS.ConfigMethodsSupported[0]?",":"");
				strcat(pCurData->WPS.ConfigMethodsSupported, STR066_CONFIG_METHODS_SUPPORTED_USBFLASHDRIVER);
			}
			if(flags&WPS_CONFIG_METHODS_FLAG_ETHERNET){
				strcat(pCurData->WPS.ConfigMethodsSupported, pCurData->WPS.ConfigMethodsSupported[0]?",":"");
				strcat(pCurData->WPS.ConfigMethodsSupported, STR066_CONFIG_METHODS_SUPPORTED_ETHERNET);
			}
			if(flags&WPS_CONFIG_METHODS_FLAG_LABEL){
				strcat(pCurData->WPS.ConfigMethodsSupported, pCurData->WPS.ConfigMethodsSupported[0]?",":"");
				strcat(pCurData->WPS.ConfigMethodsSupported, STR066_CONFIG_METHODS_SUPPORTED_LABEL);
			}
			if(flags&WPS_CONFIG_METHODS_FLAG_DISPLAY){
				strcat(pCurData->WPS.ConfigMethodsSupported, pCurData->WPS.ConfigMethodsSupported[0]?",":"");
				strcat(pCurData->WPS.ConfigMethodsSupported, STR066_CONFIG_METHODS_SUPPORTED_DISPLAY);
			}
			if(flags&WPS_CONFIG_METHODS_FLAG_EXTERNEL_NFC_TOKEN){
				strcat(pCurData->WPS.ConfigMethodsSupported, pCurData->WPS.ConfigMethodsSupported[0]?",":"");
				strcat(pCurData->WPS.ConfigMethodsSupported, STR066_CONFIG_METHODS_SUPPORTED_EXTERNEL_NFC_TOKEN);
			}
			if(flags&WPS_CONFIG_METHODS_FLAG_INTEGRATED_NFC_TOKEN){
				strcat(pCurData->WPS.ConfigMethodsSupported, pCurData->WPS.ConfigMethodsSupported[0]?",":"");
				strcat(pCurData->WPS.ConfigMethodsSupported, STR066_CONFIG_METHODS_SUPPORTED_INTEGRATED_NFC_TOKEN);
			}
			if(flags&WPS_CONFIG_METHODS_FLAG_NFC_INTERFACE){
				strcat(pCurData->WPS.ConfigMethodsSupported, pCurData->WPS.ConfigMethodsSupported[0]?",":"");
				strcat(pCurData->WPS.ConfigMethodsSupported, STR066_CONFIG_METHODS_SUPPORTED_NFC_INTERFACE);
			}
			if(flags&WPS_CONFIG_METHODS_FLAG_PUSHBUTTON){
				strcat(pCurData->WPS.ConfigMethodsSupported, pCurData->WPS.ConfigMethodsSupported[0]?",":"");
				strcat(pCurData->WPS.ConfigMethodsSupported, STR066_CONFIG_METHODS_SUPPORTED_PUSHBUTTON);
			}
			if(flags&WPS_CONFIG_METHODS_FLAG_PIN){
				strcat(pCurData->WPS.ConfigMethodsSupported, pCurData->WPS.ConfigMethodsSupported[0]?",":"");
				strcat(pCurData->WPS.ConfigMethodsSupported, STR066_CONFIG_METHODS_SUPPORTED_KEY_PAD);
			}
			//068 - SetupLockedState (get value in set func)
			//070 - ConfigurationState (get value in set func)
			//071 - LastConfigurationError (get value in set func)
			//072 - RegistrarNumberOfEntries (get value in set func)
			//073 - RegistrarEstablished (get value in set func)
		}
		
		if(objFlag&OBJ_FLAG_ASSOCIATED_DEVICE){
			pCurData->TotalAssociations = wifi_get_associated_device_count(pCurData->Name);
			for(i=0; i<pCurData->TotalAssociations; i++){
				//077 - AssociatedDeviceMACAddress
				if(wifi_get_associated_device_mac_address(pCurData->Name, i, buff)<0){
					continue;
				}
				STRNCPY(pCurData->AssociatedDevice[i].AssociatedDeviceMACAddress, buff,
					sizeof(pCurData->AssociatedDevice[i].AssociatedDeviceMACAddress)-1);
				//078 - AssociatedDeviceIPAddress
				if(0==wifi_get_associated_device_ip_address(pCurData->Name, pCurData->AssociatedDevice[i].AssociatedDeviceMACAddress, buff)){
					STRNCPY(pCurData->AssociatedDevice[i].AssociatedDeviceIPAddress, buff,
						sizeof(pCurData->AssociatedDevice[i].AssociatedDeviceIPAddress)-1);
				}
				//079 - AssociatedDeviceAuthenticationState
				pCurData->AssociatedDevice[i].AssociatedDeviceAuthenticationState = 
					wifi_get_associated_device_authentication_state(pCurData->Name, pCurData->AssociatedDevice[i].AssociatedDeviceMACAddress);
				//080 - LastRequestedUnicastCipher
				if(0==wifi_get_associated_device_last_requested_unicast_cipher(pCurData->Name, 
					pCurData->AssociatedDevice[i].AssociatedDeviceMACAddress, buff)){
					STRNCPY(pCurData->AssociatedDevice[i].LastRequestedUnicastCipher, buff,
						sizeof(pCurData->AssociatedDevice[i].LastRequestedUnicastCipher)-1);
				}
				//081 - LastRequestedMulticastCipher
				if(0==wifi_get_associated_device_last_requested_multicast_cipher(pCurData->Name, 
					pCurData->AssociatedDevice[i].AssociatedDeviceMACAddress, buff)){
					STRNCPY(pCurData->AssociatedDevice[i].LastRequestedMulticastCipher, buff,
						sizeof(pCurData->AssociatedDevice[i].LastRequestedMulticastCipher)-1);
				}
				//082 - LastPMKId
				if(0==wifi_get_associated_device_last_pairwise_master_key_id(pCurData->Name, 
					pCurData->AssociatedDevice[i].AssociatedDeviceMACAddress, buff)){
					STRNCPY(pCurData->AssociatedDevice[i].LastPMKId, buff,
						sizeof(pCurData->AssociatedDevice[i].LastPMKId)-1);
				}
				//083 - LastDataTransmitRate
				if(0==wifi_get_associated_device_last_data_transmit_rate(pCurData->Name, 
					pCurData->AssociatedDevice[i].AssociatedDeviceMACAddress, buff)){
					STRNCPY(pCurData->AssociatedDevice[i].LastDataTransmitRate, buff,
						sizeof(pCurData->AssociatedDevice[i].LastDataTransmitRate)-1);
				}
			}
		}
		
		if(objFlag&OBJ_FLAG_WEP_KEY){
		}
		
		if(objFlag&OBJ_FLAG_PRE_SHARED_KEY){
		}
		
		if(objFlag&OBJ_FLAG_AP_WMM_PARAMETER){
			for(i=0; i<WMM_CLASS_NUMBER_COUNT; i++){
				if(0==wifi_get_wmm_parameters(pCurData->Name, i, &aifs, &cwmin, &cwmax, &txoplimit, &noackpolicy, 1)){
					//088 - AIFSN
					pCurData->APWMMParameter[i].AIFSN = aifs;
					//089 - ECWMin
					pCurData->APWMMParameter[i].ECWMin = log(cwmin+1) / log(2);
					//090 - ECWMax
					pCurData->APWMMParameter[i].ECWMax = log(cwmax+1) / log(2);
					//091 -TXOP 
					pCurData->APWMMParameter[i].TXOP = txoplimit / 32;
					//092 - AckPolicy
					pCurData->APWMMParameter[i].AckPolicy = !noackpolicy;
				}
			}
		}
		
		if(objFlag&OBJ_FLAG_STA_WMM_PARAMETER){
			for(i=0; i<WMM_CLASS_NUMBER_COUNT; i++){
				if(0==wifi_get_wmm_parameters(pCurData->Name, i, &aifs, &cwmin, &cwmax, &txoplimit, &noackpolicy, 0)){
					//088 - AIFSN
					pCurData->STAWMMParameter[i].AIFSN = aifs;
					//089 - ECWMin
					pCurData->STAWMMParameter[i].ECWMin = log(cwmin+1) / log(2);
					//090 - ECWMax
					pCurData->STAWMMParameter[i].ECWMax = log(cwmax+1) / log(2);
					//091 -TXOP 
					pCurData->STAWMMParameter[i].TXOP = txoplimit / 32;
					//092 - AckPolicy
					pCurData->STAWMMParameter[i].AckPolicy = !noackpolicy;
				}
			}
		}
		
		if(objFlag&OBJ_FLAG_WPS_REGISTRAR){
			for(i=0; i<MAX_COUNT_WPS_REGISTRAR; i++){
				//075 - UUID
				if(0==wifi_get_wps_registrar_uuid(pCurData->Name, i, buff)){
					STRNCPY(pCurData->WPS.Registrar[i].UUID, buff, sizeof(pCurData->WPS.Registrar[i].UUID)-1);
				}
				//076 - DeviceName
				if(0==wifi_get_wps_registrar_device_name(pCurData->Name, i, buff)){
					STRNCPY(pCurData->WPS.Registrar[i].DeviceName, buff, sizeof(pCurData->WPS.Registrar[i].DeviceName)-1);
				}
			}
		}
		
	}while(0);

	return ret;
}

