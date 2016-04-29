/**************************************************************************
 *	
 *	        wifi_data_openwrt.h
 *
 **************************************************************************/
#ifndef WIFI_DATA_OPENWRT_H
#define WIFI_DATA_OPENWRT_H
#include <time.h>
#include "ctl_mem.h"
#include "tr69_func.h"
#include "tr69_cms_object.h"

#define SUPPORTED_TR181

#ifdef SUPPORTED_TR098
#include "tr098_wifi.h"
#endif

#ifdef SUPPORTED_TR181
#include "tr181_wifi.h"
#endif

#define WIFI_DRIVER_REALTEK

#define MIN_OBJ_GET_INTERVAL	5	//seconds

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

#ifdef SUPPORTED_TR098
static inline int tr098_wifi_data_load(PWLANConfiguration pWiFiData, 
	LanWlanObject *pWLANConfiguration, 
	LanWlanStatsObject* pStats, 
	LanWlanWPSObject* pWPS, 
	LanWlanWPSRegistrarObject* pRegistrar[MAX_COUNT_WPS_REGISTRAR], 
	LanWlanAssociatedDeviceEntryObject* pAssociatedDevice[MAX_COUNT_ASSOCIATED_DEVICES], 
	LanWlanWepKeyObject* pWEPKey[MAX_COUNT_WEPKEYS], 
	LanWlanPreSharedKeyObject* pPreSharedKey[MAX_COUNT_PRESHAREDKEYS], 
	APWMMParameterObject* pAPWMMParameter[MAX_COUNT_WMM_ENTRIES])
{
	int ret = 0;
	char *p = NULL;
	int i;
	
	do {
		if(NULL==pWiFiData){
			ret = -1;
			break;
		}
		
		memset(pWiFiData, 0, sizeof(WLANConfiguration));
		//x15
#ifdef WIFI_DRIVER_MADWIFI
		strcpy(pWiFiData->extRadioDevName, "wifi0");
#elif defined WIFI_DRIVER_REALTEK
		strcpy(pWiFiData->extRadioDevName, "wlan0");
#else
#error "extRadioDevName is not defined!"
#endif
		
		if(pWLANConfiguration){
			//001
			pWiFiData->Enable = pWLANConfiguration->enable;
			//002
			STRNCPY(pWiFiData->Status, pWLANConfiguration->status, sizeof(pWiFiData->Status)-1);
			//003
			STRNCPY(pWiFiData->Name, pWLANConfiguration->name, sizeof(pWiFiData->Name)-1);
			//004 
			STRNCPY(pWiFiData->BSSID, pWLANConfiguration->BSSID, sizeof(pWiFiData->BSSID)-1);
			//005
			STRNCPY(pWiFiData->MaxBitRate, pWLANConfiguration->maxBitRate, sizeof(pWiFiData->MaxBitRate)-1);
			//006
			pWiFiData->Channel = pWLANConfiguration->channel;		
			//007
			pWiFiData->AutoChannelEnable = pWLANConfiguration->autoChannelEnable;
            pWiFiData->X_ACTIONTEC_COM_AutoChannelRefresh = pWLANConfiguration->X_ACTIONTEC_COM_AutoChannelRefresh;
			//008
			STRNCPY(pWiFiData->SSID, pWLANConfiguration->SSID, sizeof(pWiFiData->SSID)-1);
			//009
			STRNCPY(pWiFiData->BeaconType, pWLANConfiguration->beaconType, sizeof(pWiFiData->BeaconType)-1);
			//010
			pWiFiData->MACAddressControlEnabled = pWLANConfiguration->MACAddressControlEnabled;
			//011
			STRNCPY(pWiFiData->Standard, pWLANConfiguration->standard, sizeof(pWiFiData->Standard)-1);
			//012
			pWiFiData->WEPKeyIndex = pWLANConfiguration->WEPKeyIndex;
			//013
			STRNCPY(pWiFiData->KeyPassphrase, pWLANConfiguration->keyPassphrase, sizeof(pWiFiData->KeyPassphrase)-1);
			//014
			STRNCPY(pWiFiData->WEPEncryptionLevel, pWLANConfiguration->WEPEncryptionLevel, sizeof(pWiFiData->WEPEncryptionLevel)-1);
			//015
			STRNCPY(pWiFiData->BasicEncryptionModes, pWLANConfiguration->basicEncryptionModes, sizeof(pWiFiData->BasicEncryptionModes)-1);
			//016
			STRNCPY(pWiFiData->BasicAuthenticationMode, pWLANConfiguration->basicAuthenticationMode, sizeof(pWiFiData->BasicAuthenticationMode)-1);
			//017
			STRNCPY(pWiFiData->WPAEncryptionModes, pWLANConfiguration->WPAEncryptionModes, sizeof(pWiFiData->WPAEncryptionModes)-1);
			//018
			STRNCPY(pWiFiData->WPAAuthenticationMode, pWLANConfiguration->WPAAuthenticationMode, sizeof(pWiFiData->WPAAuthenticationMode)-1);
			//019
			STRNCPY(pWiFiData->IEEE11iEncryptionModes, pWLANConfiguration->IEEE11iEncryptionModes, sizeof(pWiFiData->IEEE11iEncryptionModes)-1);
			//020
			STRNCPY(pWiFiData->IEEE11iAuthenticationMode, pWLANConfiguration->IEEE11iAuthenticationMode, sizeof(pWiFiData->IEEE11iAuthenticationMode)-1);
			//021
			STRNCPY(pWiFiData->PossibleChannels, pWLANConfiguration->possibleChannels, sizeof(pWiFiData->PossibleChannels)-1);
			//022
			STRNCPY(pWiFiData->BasicDataTransmitRates, pWLANConfiguration->basicDataTransmitRates, sizeof(pWiFiData->BasicDataTransmitRates)-1);
			//023
			STRNCPY(pWiFiData->OperationalDataTransmitRates, pWLANConfiguration->operationalDataTransmitRates, sizeof(pWiFiData->OperationalDataTransmitRates)-1);
			//024
			STRNCPY(pWiFiData->PossibleDataTransmitRates, pWLANConfiguration->possibleDataTransmitRates, sizeof(pWiFiData->PossibleDataTransmitRates)-1);
			//025
			pWiFiData->InsecureOOBAccessEnabled = pWLANConfiguration->insecureOOBAccessEnabled;
			//026
			pWiFiData->BeaconAdvertisementEnabled = pWLANConfiguration->beaconAdvertisementEnabled;
			//027
			pWiFiData->SSIDAdvertisementEnabled = pWLANConfiguration->SSIDAdvertisementEnabled;
			//028
			pWiFiData->RadioEnabled = pWLANConfiguration->radioEnabled;
			//029
			STRNCPY(pWiFiData->TransmitPowerSupported, pWLANConfiguration->transmitPowerSupported, sizeof(pWiFiData->TransmitPowerSupported)-1);
			//030
			pWiFiData->TransmitPower = pWLANConfiguration->transmitPower;
			//031
			pWiFiData->AutoRateFallBackEnabled = pWLANConfiguration->autoRateFallBackEnabled;
			//032
			STRNCPY(pWiFiData->LocationDescription, pWLANConfiguration->locationDescription, sizeof(pWiFiData->LocationDescription)-1);
			//033
			STRNCPY(pWiFiData->RegulatoryDomain, pWLANConfiguration->regulatoryDomain, sizeof(pWiFiData->RegulatoryDomain)-1);
			//034
			pWiFiData->TotalPSKFailures = pWLANConfiguration->totalPSKFailures;
			//035
			pWiFiData->TotalIntegrityFailures = pWLANConfiguration->totalIntegrityFailures;
			//036
			STRNCPY(pWiFiData->ChannelsInUse, pWLANConfiguration->channelsInUse, sizeof(pWiFiData->ChannelsInUse)-1);
			//037
			STRNCPY(pWiFiData->DeviceOperationMode, pWLANConfiguration->deviceOperationMode, sizeof(pWiFiData->DeviceOperationMode)-1);
			//038
			pWiFiData->DistanceFromRoot = pWLANConfiguration->distanceFromRoot;
			//039
			STRNCPY(pWiFiData->PeerBSSID, pWLANConfiguration->peerBSSID, sizeof(pWiFiData->PeerBSSID)-1);
			//040
			STRNCPY(pWiFiData->AuthenticationServiceMode, pWLANConfiguration->authenticationServiceMode, sizeof(pWiFiData->AuthenticationServiceMode)-1);
			//041
			pWiFiData->WMMSupported = pWLANConfiguration->WMMSupported;
			//042
			pWiFiData->UAPSDSupported = pWLANConfiguration->UAPSDSupported;
			//043
			pWiFiData->WMMEnable = pWLANConfiguration->WMMEnable;
			//044
			pWiFiData->UAPSDEnable = pWLANConfiguration->UAPSDEnable;
			//045
			pWiFiData->TotalBytesSent = pWLANConfiguration->totalBytesSent;
			//046
			pWiFiData->TotalBytesReceived = pWLANConfiguration->totalBytesReceived;
			//047
			pWiFiData->TotalPacketsSent = pWLANConfiguration->totalPacketsSent;
			//048
			pWiFiData->TotalPacketsReceived = pWLANConfiguration->totalPacketsReceived;
			//049
			pWiFiData->TotalAssociations = pWLANConfiguration->totalAssociations;
			//x01
			pWiFiData->extPreSharedKeyIndex = pWLANConfiguration->X_ACTIONTEC_PreSharedKeyIndex;
			//x02
			pWiFiData->extGroupKeyUpdateInterval = pWLANConfiguration->X_ACTIONTEC_GroupKeyUpdateInterval;
			//x03
			STRNCPY(pWiFiData->ext8021xServerAddress, pWLANConfiguration->X_ACTIONTEC_8021XServerAddress, sizeof(pWiFiData->ext8021xServerAddress)-1);
			//x04
			pWiFiData->ext8021xServerPort = pWLANConfiguration->X_ACTIONTEC_8021XServerPort;
			//x05
			STRNCPY(pWiFiData->ext8021xServerSecret, pWLANConfiguration->X_ACTIONTEC_8021XServerSecret, sizeof(pWiFiData->ext8021xServerSecret)-1);
			//x06
			pWiFiData->extBeaconInterval = pWLANConfiguration->X_ACTIONTEC_BeaconInterval;
			//x07
			pWiFiData->extDTIMInterval = pWLANConfiguration->X_ACTIONTEC_DTIMInterval;
			//x08
			pWiFiData->extFragmentationThreshold = pWLANConfiguration->X_ACTIONTEC_FragmentationThreshold;
			//x09
			pWiFiData->extRTSThreshold = pWLANConfiguration->X_ACTIONTEC_RTSThreshold;
			//x10
			STRNCPY(pWiFiData->extCTSProtectMode, pWLANConfiguration->X_ACTIONTEC_CTSProtectMode, sizeof(pWiFiData->extCTSProtectMode)-1);
			//x11
			pWiFiData->extMSDULimit = pWLANConfiguration->X_ACTIONTEC_MSDU;
			//x12
			pWiFiData->extMPDULimit = pWLANConfiguration->X_ACTIONTEC_MPDU;
			//x13
			pWiFiData->extMACPolicy = pWLANConfiguration->X_ACTIONTEC_MACPolicy;
			//x14
			STRNCPY(pWiFiData->extMACList, pWLANConfiguration->X_ACTIONTEC_MACList, sizeof(pWiFiData->extMACList)-1);
		}

		if(pStats){
		}

		if(pWPS){
			//061
			pWiFiData->WPS.Enable = pWPS->enable;
			//062
			STRNCPY(pWiFiData->WPS.DeviceName, pWPS->deviceName, sizeof(pWiFiData->WPS.DeviceName)-1);
			//063
			pWiFiData->WPS.DevicePassword = pWPS->devicePassword;
			//064
			STRNCPY(pWiFiData->WPS.UUID, pWPS->UUID, sizeof(pWiFiData->WPS.UUID)-1);
			//065
			pWiFiData->WPS.Version = pWPS->version;
			//066
			STRNCPY(pWiFiData->WPS.ConfigMethodsSupported, pWPS->configMethodsSupported, sizeof(pWiFiData->WPS.ConfigMethodsSupported)-1);
			//067
			STRNCPY(pWiFiData->WPS.ConfigMethodsEnabled, pWPS->configMethodsEnabled, sizeof(pWiFiData->WPS.ConfigMethodsEnabled)-1);
			//068
			STRNCPY(pWiFiData->WPS.SetupLockedState, pWPS->setupLockedState, sizeof(pWiFiData->WPS.SetupLockedState)-1);
			//069
			pWiFiData->WPS.SetupLock = pWPS->setupLock;
			//070
			STRNCPY(pWiFiData->WPS.ConfigurationState, pWPS->configurationState, sizeof(pWiFiData->WPS.ConfigurationState)-1);
			//071
			STRNCPY(pWiFiData->WPS.LastConfigurationError, pWPS->lastConfigurationError, sizeof(pWiFiData->WPS.LastConfigurationError)-1);
			//072
			pWiFiData->WPS.RegistrarNumberOfEntries = pWPS->registrarNumberOfEntries;
			//073
			pWiFiData->WPS.RegistrarEstablished = pWPS->registrarEstablished;
		}

		if(pRegistrar){
			for(i=0; i<MAX_COUNT_WPS_REGISTRAR; i++){
				if(pRegistrar[i]){
					//074
					pWiFiData->WPS.Registrar[i].Enable = pRegistrar[i]->enable;
					//075
					STRNCPY(pWiFiData->WPS.Registrar[i].UUID, pRegistrar[i]->UUID, sizeof(pWiFiData->WPS.Registrar[i].UUID)-1);
					//076
					STRNCPY(pWiFiData->WPS.Registrar[i].DeviceName, pRegistrar[i]->deviceName, sizeof(pWiFiData->WPS.Registrar[i].DeviceName)-1);
				}
			}
		}

		if(pAssociatedDevice){
			for(i=0; i<MAX_COUNT_ASSOCIATED_DEVICES; i++){
				if(pAssociatedDevice[i]){
					//077
					STRNCPY(pWiFiData->AssociatedDevice[i].AssociatedDeviceMACAddress, pAssociatedDevice[i]->associatedDeviceMACAddress, sizeof(pWiFiData->AssociatedDevice[i].AssociatedDeviceMACAddress)-1);
					//078
					STRNCPY(pWiFiData->AssociatedDevice[i].AssociatedDeviceIPAddress, pAssociatedDevice[i]->associatedDeviceIPAddress, sizeof(pWiFiData->AssociatedDevice[i].AssociatedDeviceIPAddress)-1);
					//079
					pWiFiData->AssociatedDevice[i].AssociatedDeviceAuthenticationState = pAssociatedDevice[i]->associatedDeviceAuthenticationState;
					//080
					STRNCPY(pWiFiData->AssociatedDevice[i].LastRequestedUnicastCipher, pAssociatedDevice[i]->lastRequestedUnicastCipher, sizeof(pWiFiData->AssociatedDevice[i].LastRequestedUnicastCipher)-1);
					//081
					STRNCPY(pWiFiData->AssociatedDevice[i].LastRequestedMulticastCipher, pAssociatedDevice[i]->lastRequestedMulticastCipher, sizeof(pWiFiData->AssociatedDevice[i].LastRequestedMulticastCipher)-1);
					//082
					STRNCPY(pWiFiData->AssociatedDevice[i].LastPMKId, pAssociatedDevice[i]->lastPMKId, sizeof(pWiFiData->AssociatedDevice[i].LastPMKId)-1);
					//083
					STRNCPY(pWiFiData->AssociatedDevice[i].LastDataTransmitRate, pAssociatedDevice[i]->lastDataTransmitRate, sizeof(pWiFiData->AssociatedDevice[i].LastDataTransmitRate)-1);
				}
			}
		}

		if(pWEPKey){
			for(i=0; i<MAX_COUNT_WEPKEYS; i++){
				if(pWEPKey[i]){
					//084
					STRNCPY(pWiFiData->WEPKey[i].WEPKey, pWEPKey[i]->WEPKey, sizeof(pWiFiData->WEPKey[i].WEPKey)-1);
					//x29
					pWiFiData->WEPKey[i].WEPKeyType = pWEPKey[i]->X_ACTIONTEC_HexEncode;
				}
			}
		}

		if(pPreSharedKey){
			for(i=0; i<MAX_COUNT_PRESHAREDKEYS; i++){
				if(pPreSharedKey[i]){
					//085
					STRNCPY(pWiFiData->PreSharedKey[i].PreSharedKey, pPreSharedKey[i]->preSharedKey, sizeof(pWiFiData->PreSharedKey[i].PreSharedKey)-1);
					//086
					STRNCPY(pWiFiData->PreSharedKey[i].KeyPassphrase, pPreSharedKey[i]->keyPassphrase, sizeof(pWiFiData->PreSharedKey[i].KeyPassphrase)-1);
					//087
					STRNCPY(pWiFiData->PreSharedKey[i].AssociatedDeviceMACAddress, pPreSharedKey[i]->associatedDeviceMACAddress, sizeof(pWiFiData->PreSharedKey[i].AssociatedDeviceMACAddress)-1);
				}
			}
		}

		if(pAPWMMParameter){
			for(i=0; i<MAX_COUNT_WMM_ENTRIES; i++){
				if(pAPWMMParameter[i]){
					//088
					pWiFiData->APWMMParameter[i].AIFSN = pAPWMMParameter[i]->AIFSN;
					//089
					pWiFiData->APWMMParameter[i].ECWMin = pAPWMMParameter[i]->ECWMin;
					//090
					pWiFiData->APWMMParameter[i].ECWMax = pAPWMMParameter[i]->ECWMax;
					//091
					pWiFiData->APWMMParameter[i].TXOP = pAPWMMParameter[i]->TXOP;
					//092
					pWiFiData->APWMMParameter[i].AckPolicy = pAPWMMParameter[i]->ackPolicy;
				}
			}
		}
	} while(0);
	
	return ret;
}

static inline int tr098_wifi_data_store(PWLANConfiguration pWiFiData, 
	LanWlanObject *pWLANConfiguration, 
	LanWlanStatsObject* pStats, 
	LanWlanWPSObject* pWPS, 
	LanWlanWPSRegistrarObject* pRegistrar[MAX_COUNT_WPS_REGISTRAR], 
	LanWlanAssociatedDeviceEntryObject* pAssociatedDevice[MAX_COUNT_ASSOCIATED_DEVICES], 
	LanWlanWepKeyObject* pWEPKey[MAX_COUNT_WEPKEYS], 
	LanWlanPreSharedKeyObject* pPreSharedKey[MAX_COUNT_PRESHAREDKEYS], 
	APWMMParameterObject* pAPWMMParameter[MAX_COUNT_WMM_ENTRIES])
{
	int ret = 0;
	char buff[1024] = {0};
	int i = 0;
	
	do {
		if(NULL==pWiFiData){
			ret = -1;
			break;
		}

		if(pWLANConfiguration){
			//001
			pWLANConfiguration->enable = pWiFiData->Enable;
			//002
			CTLMEM_REPLACE_STRING(pWLANConfiguration->status, pWiFiData->Status);
			//003
			CTLMEM_REPLACE_STRING(pWLANConfiguration->name, pWiFiData->Name);
			//004 
			CTLMEM_REPLACE_STRING(pWLANConfiguration->BSSID, pWiFiData->BSSID);
			//005
			CTLMEM_REPLACE_STRING(pWLANConfiguration->maxBitRate, pWiFiData->MaxBitRate);
			//006
			pWLANConfiguration->channel = pWiFiData->Channel; 	
			//007
			pWLANConfiguration->autoChannelEnable = pWiFiData->AutoChannelEnable;
            
            pWLANConfiguration->X_ACTIONTEC_COM_AutoChannelRefresh = pWiFiData->X_ACTIONTEC_COM_AutoChannelRefresh;
			//008
			CTLMEM_REPLACE_STRING(pWLANConfiguration->SSID, pWiFiData->SSID);
			//009
			CTLMEM_REPLACE_STRING(pWLANConfiguration->beaconType, pWiFiData->BeaconType);
			//010
			pWLANConfiguration->MACAddressControlEnabled = pWiFiData->MACAddressControlEnabled;
			//011
			CTLMEM_REPLACE_STRING(pWLANConfiguration->standard, pWiFiData->Standard);
			//012
			pWLANConfiguration->WEPKeyIndex = pWiFiData->WEPKeyIndex;
			//013
			CTLMEM_REPLACE_STRING(pWLANConfiguration->keyPassphrase, pWiFiData->KeyPassphrase);
			//014
			CTLMEM_REPLACE_STRING(pWLANConfiguration->WEPEncryptionLevel, pWiFiData->WEPEncryptionLevel);
			//015
			CTLMEM_REPLACE_STRING(pWLANConfiguration->basicEncryptionModes, pWiFiData->BasicEncryptionModes);
			//016
			CTLMEM_REPLACE_STRING(pWLANConfiguration->basicAuthenticationMode, pWiFiData->BasicAuthenticationMode);
			//017
			CTLMEM_REPLACE_STRING(pWLANConfiguration->WPAEncryptionModes, pWiFiData->WPAEncryptionModes);
			//018
			CTLMEM_REPLACE_STRING(pWLANConfiguration->WPAAuthenticationMode, pWiFiData->WPAAuthenticationMode);
			//019
			CTLMEM_REPLACE_STRING(pWLANConfiguration->IEEE11iEncryptionModes, pWiFiData->IEEE11iEncryptionModes);
			//020
			CTLMEM_REPLACE_STRING(pWLANConfiguration->IEEE11iAuthenticationMode, pWiFiData->IEEE11iAuthenticationMode);
			//021
			CTLMEM_REPLACE_STRING(pWLANConfiguration->possibleChannels, pWiFiData->PossibleChannels);
			//022
			CTLMEM_REPLACE_STRING(pWLANConfiguration->basicDataTransmitRates, pWiFiData->BasicDataTransmitRates);
			//023
			CTLMEM_REPLACE_STRING(pWLANConfiguration->operationalDataTransmitRates, pWiFiData->OperationalDataTransmitRates);
			//024
			CTLMEM_REPLACE_STRING(pWLANConfiguration->possibleDataTransmitRates, pWiFiData->PossibleDataTransmitRates);
			//025
			pWLANConfiguration->insecureOOBAccessEnabled = pWiFiData->InsecureOOBAccessEnabled;
			//026
			pWLANConfiguration->beaconAdvertisementEnabled = pWiFiData->BeaconAdvertisementEnabled;
			//027
			pWLANConfiguration->SSIDAdvertisementEnabled = pWiFiData->SSIDAdvertisementEnabled;
			//028
			pWLANConfiguration->radioEnabled = pWiFiData->RadioEnabled;
			//029
			CTLMEM_REPLACE_STRING(pWLANConfiguration->transmitPowerSupported, pWiFiData->TransmitPowerSupported);
			//030
			pWLANConfiguration->transmitPower = pWiFiData->TransmitPower;
			//031
			pWLANConfiguration->autoRateFallBackEnabled = pWiFiData->AutoRateFallBackEnabled;
			//032
			CTLMEM_REPLACE_STRING(pWLANConfiguration->locationDescription, pWiFiData->LocationDescription);
			//033
			CTLMEM_REPLACE_STRING(pWLANConfiguration->regulatoryDomain, pWiFiData->RegulatoryDomain);
			//034
			pWLANConfiguration->totalPSKFailures = pWiFiData->TotalPSKFailures;
			//035
			pWLANConfiguration->totalIntegrityFailures = pWiFiData->TotalIntegrityFailures;
			//036
			CTLMEM_REPLACE_STRING(pWLANConfiguration->channelsInUse, pWiFiData->ChannelsInUse);
			//037
			CTLMEM_REPLACE_STRING(pWLANConfiguration->deviceOperationMode, pWiFiData->DeviceOperationMode);
			//038
			pWLANConfiguration->distanceFromRoot = pWiFiData->DistanceFromRoot;
			//039
			CTLMEM_REPLACE_STRING(pWLANConfiguration->peerBSSID, pWiFiData->PeerBSSID);
			//040
			CTLMEM_REPLACE_STRING(pWLANConfiguration->authenticationServiceMode, pWiFiData->AuthenticationServiceMode);
			//041
			pWLANConfiguration->WMMSupported = pWiFiData->WMMSupported;
			//042
			pWLANConfiguration->UAPSDSupported = pWiFiData->UAPSDSupported;
			//043
			pWLANConfiguration->WMMEnable = pWiFiData->WMMEnable;
			//044
			pWLANConfiguration->UAPSDEnable = pWiFiData->UAPSDEnable;
			//045
			pWLANConfiguration->totalBytesSent = pWiFiData->TotalBytesSent;
			//046
			pWLANConfiguration->totalBytesReceived = pWiFiData->TotalBytesReceived;
			//047
			pWLANConfiguration->totalPacketsSent = pWiFiData->TotalPacketsSent;
			//048
			pWLANConfiguration->totalPacketsReceived = pWiFiData->TotalPacketsReceived;
			//049
			pWLANConfiguration->totalAssociations = pWiFiData->TotalAssociations;
			
			//x01
			pWLANConfiguration->X_ACTIONTEC_PreSharedKeyIndex = pWiFiData->extPreSharedKeyIndex;
			//x02
			pWLANConfiguration->X_ACTIONTEC_GroupKeyUpdateInterval = pWiFiData->extGroupKeyUpdateInterval;
			//x03
			CTLMEM_REPLACE_STRING(pWLANConfiguration->X_ACTIONTEC_8021XServerAddress, pWiFiData->ext8021xServerAddress);
			//x04
			pWLANConfiguration->X_ACTIONTEC_8021XServerPort = pWiFiData->ext8021xServerPort;
			//x05
			CTLMEM_REPLACE_STRING(pWLANConfiguration->X_ACTIONTEC_8021XServerSecret, pWiFiData->ext8021xServerSecret);
			//x06
			pWLANConfiguration->X_ACTIONTEC_BeaconInterval = pWiFiData->extBeaconInterval;
			//x07
			pWLANConfiguration->X_ACTIONTEC_DTIMInterval = pWiFiData->extDTIMInterval;
			//x08
			pWLANConfiguration->X_ACTIONTEC_FragmentationThreshold = pWiFiData->extFragmentationThreshold;
			//x09
			pWLANConfiguration->X_ACTIONTEC_RTSThreshold = pWiFiData->extRTSThreshold;
			//x10
			CTLMEM_REPLACE_STRING(pWLANConfiguration->X_ACTIONTEC_CTSProtectMode, pWiFiData->extCTSProtectMode);
			//x11
			pWLANConfiguration->X_ACTIONTEC_MSDU = pWiFiData->extMSDULimit;
			//x12
			pWLANConfiguration->X_ACTIONTEC_MPDU = pWiFiData->extMPDULimit;
			//x13
			pWLANConfiguration->X_ACTIONTEC_MACPolicy = pWiFiData->extMACPolicy;
			//x14
			CTLMEM_REPLACE_STRING(pWLANConfiguration->X_ACTIONTEC_MACList, pWiFiData->extMACList);
		}

		if(pStats){
			//050
			pStats->errorsSent = pWiFiData->Stats.ErrorsSent; 
			//051
			pStats->errorsReceived = pWiFiData->Stats.ErrorsReceived; 
			//052
			pStats->unicastPacketsSent = pWiFiData->Stats.UnicastPacketsSent; 
			//053
			pStats->unicastPacketsReceived = pWiFiData->Stats.UnicastPacketsReceived; 
			//054
			pStats->discardPacketsSent = pWiFiData->Stats.DiscardPacketsSent; 
			//055
			pStats->discardPacketsReceived = pWiFiData->Stats.DiscardPacketsReceived; 
			//056
			pStats->multicastPacketsSent = pWiFiData->Stats.MulticastPacketsSent; 
			//057
			pStats->multicastPacketsReceived = pWiFiData->Stats.MulticastPacketsReceived; 
			//058
			pStats->broadcastPacketsSent = pWiFiData->Stats.BroadcastPacketsSent; 
			//059
			pStats->broadcastPacketsReceived = pWiFiData->Stats.BroadcastPacketsReceived; 
			//060
			pStats->unknownProtoPacketsReceived = pWiFiData->Stats.UnknownProtoPacketsReceived; 
		}

		if(pWPS){
			//061
			pWPS->enable = pWiFiData->WPS.Enable;
			//062
			CTLMEM_REPLACE_STRING(pWPS->deviceName, pWiFiData->WPS.DeviceName);
			//063
			pWPS->devicePassword = pWiFiData->WPS.DevicePassword;
			//064
			CTLMEM_REPLACE_STRING(pWPS->UUID, pWiFiData->WPS.UUID);
			//065
			pWPS->version = pWiFiData->WPS.Version;
			//066
			CTLMEM_REPLACE_STRING(pWPS->configMethodsSupported, pWiFiData->WPS.ConfigMethodsSupported);
			//067
			CTLMEM_REPLACE_STRING(pWPS->configMethodsEnabled, pWiFiData->WPS.ConfigMethodsEnabled);
			//068
			CTLMEM_REPLACE_STRING(pWPS->setupLockedState, pWiFiData->WPS.SetupLockedState);
			//069
			pWPS->setupLock = pWiFiData->WPS.SetupLock;
			//070
			CTLMEM_REPLACE_STRING(pWPS->configurationState, pWiFiData->WPS.ConfigurationState);
			//071
			CTLMEM_REPLACE_STRING(pWPS->lastConfigurationError, pWiFiData->WPS.LastConfigurationError);
			//072
			pWPS->registrarNumberOfEntries = pWiFiData->WPS.RegistrarNumberOfEntries;
			//073
			pWPS->registrarEstablished = pWiFiData->WPS.RegistrarEstablished;
		}

		if(pRegistrar){
			for(i=0; i<MAX_COUNT_WPS_REGISTRAR; i++){
				if(pRegistrar[i]){
					//074
					pRegistrar[i]->enable = pWiFiData->WPS.Registrar[i].Enable;
					//075
					CTLMEM_REPLACE_STRING(pRegistrar[i]->UUID, pWiFiData->WPS.Registrar[i].UUID);
					//076
					CTLMEM_REPLACE_STRING(pRegistrar[i]->deviceName, pWiFiData->WPS.Registrar[i].DeviceName);
				}
			}
		}

		if(pAssociatedDevice){
			for(i=0; i<MAX_COUNT_ASSOCIATED_DEVICES; i++){
				if(pAssociatedDevice[i]){
					//077
					CTLMEM_REPLACE_STRING(pAssociatedDevice[i]->associatedDeviceMACAddress, pWiFiData->AssociatedDevice[i].AssociatedDeviceMACAddress);
					//078
					CTLMEM_REPLACE_STRING(pAssociatedDevice[i]->associatedDeviceIPAddress, pWiFiData->AssociatedDevice[i].AssociatedDeviceIPAddress);
					//079
					pAssociatedDevice[i]->associatedDeviceAuthenticationState = pWiFiData->AssociatedDevice[i].AssociatedDeviceAuthenticationState;
					//080
					CTLMEM_REPLACE_STRING(pAssociatedDevice[i]->lastRequestedUnicastCipher, pWiFiData->AssociatedDevice[i].LastRequestedUnicastCipher);
					//081
					CTLMEM_REPLACE_STRING(pAssociatedDevice[i]->lastRequestedMulticastCipher, pWiFiData->AssociatedDevice[i].LastRequestedMulticastCipher);
					//082
					CTLMEM_REPLACE_STRING(pAssociatedDevice[i]->lastPMKId, pWiFiData->AssociatedDevice[i].LastPMKId);
					//083
					CTLMEM_REPLACE_STRING(pAssociatedDevice[i]->lastDataTransmitRate, pWiFiData->AssociatedDevice[i].LastDataTransmitRate);
				}
			}
		}

		if(pWEPKey){
			for(i=0; i<MAX_COUNT_WEPKEYS; i++){
				if(pWEPKey[i]){
					//084
					CTLMEM_REPLACE_STRING(pWEPKey[i]->WEPKey, pWiFiData->WEPKey[i].WEPKey);
					//x29
					pWEPKey[i]->X_ACTIONTEC_HexEncode = pWiFiData->WEPKey[i].WEPKeyType;
				}
			}
		}

		if(pPreSharedKey){
			for(i=0; i<MAX_COUNT_PRESHAREDKEYS; i++){
				if(pPreSharedKey[i]){
					//085
					CTLMEM_REPLACE_STRING(pPreSharedKey[i]->preSharedKey, pWiFiData->PreSharedKey[i].PreSharedKey);
					//086
					CTLMEM_REPLACE_STRING(pPreSharedKey[i]->keyPassphrase, pWiFiData->PreSharedKey[i].KeyPassphrase);
					//087
					CTLMEM_REPLACE_STRING(pPreSharedKey[i]->associatedDeviceMACAddress, pWiFiData->PreSharedKey[i].AssociatedDeviceMACAddress);
				}
			}
		}

		if(pAPWMMParameter){
			for(i=0; i<MAX_COUNT_WMM_ENTRIES; i++){
				if(pAPWMMParameter[i]){
					//088
					pAPWMMParameter[i]->AIFSN = pWiFiData->APWMMParameter[i].AIFSN;
					//089
					pAPWMMParameter[i]->ECWMin = pWiFiData->APWMMParameter[i].ECWMin;
					//090
					pAPWMMParameter[i]->ECWMax = pWiFiData->APWMMParameter[i].ECWMax;
					//091
					pAPWMMParameter[i]->TXOP = pWiFiData->APWMMParameter[i].TXOP;
					//092
					pAPWMMParameter[i]->ackPolicy = pWiFiData->APWMMParameter[i].AckPolicy;
				}
			}
		}
	} while(0);
	
	return ret;
}
#endif

#ifdef SUPPORTED_TR181
static inline int tr181_wifi_data_load(PDEVICE_WIFI pWiFiData,
	WIFIObject* pWiFi, WIFIRadioObject* pRadio[MAX_NUMBER_OF_ENTRIES_RADIO], 
	WIFISSIDObject* pSSID[MAX_NUMBER_OF_ENTRIES_SSID], 
	WIFIAccessPointObject* pAP[MAX_NUMBER_OF_ENTRIES_ACCESSPOINT],
	WIFIAccessPointSecurityObject* pAPSecurity[MAX_NUMBER_OF_ENTRIES_ACCESSPOINT])
{
	int ret = 0;
	int i;
	
	do {
		if(NULL==pWiFiData){
			ret = -1;
			break;
		}
		
		memset(pWiFiData, 0, sizeof(DEVICE_WIFI));
		
		if(pWiFi){
			//001
			pWiFiData->RadioNumberOfEntries = pWiFi->radioNumberOfEntries;
			//002
			pWiFiData->SSIDNumberOfEntries = pWiFi->SSIDNumberOfEntries;
			//003
			pWiFiData->AccessPointNumberOfEntries = pWiFi->accessPointNumberOfEntries;
		}

		if(pRadio){
			for(i=0; i<MAX_NUMBER_OF_ENTRIES_RADIO; i++){
				if(pRadio[i]){
					//005
					pWiFiData->Radio[i].Enable = pRadio[i]->enable;
					//008
					STRNCPY(pWiFiData->Radio[i].Name, pRadio[i]->name, sizeof(pWiFiData->Radio[i].Name)-1);
					//010 
					STRNCPY(pWiFiData->Radio[i].LowerLayers, pRadio[i]->lowerLayers, sizeof(pWiFiData->Radio[i].LowerLayers)-1);
					//014 
					STRNCPY(pWiFiData->Radio[i].OperatingFrequencyBand, pRadio[i]->operatingFrequencyBand, sizeof(pWiFiData->Radio[i].OperatingFrequencyBand)-1);
					//016
					STRNCPY(pWiFiData->Radio[i].OperatingStandards, pRadio[i]->operatingStandards, sizeof(pWiFiData->Radio[i].OperatingStandards)-1);
					//019
					pWiFiData->Radio[i].Channel = pRadio[i]->channel;
					//021
					pWiFiData->Radio[i].AutoChannelEnable = pRadio[i]->autoChannelEnable;

                    pWiFiData->Radio[i].X_ACTIONTEC_COM_AutoChannelRefresh = pRadio[i]->X_ACTIONTEC_COM_AutoChannelRefresh;
					//023
					STRNCPY(pWiFiData->Radio[i].OperatingChannelBandwidth, pRadio[i]->operatingChannelBandwidth, sizeof(pWiFiData->Radio[i].OperatingChannelBandwidth)-1);
					//024
					STRNCPY(pWiFiData->Radio[i].ExtensionChannel, pRadio[i]->extensionChannel, sizeof(pWiFiData->Radio[i].ExtensionChannel)-1);
					//025
					STRNCPY(pWiFiData->Radio[i].GuardInterval, pRadio[i]->guardInterval, sizeof(pWiFiData->Radio[i].GuardInterval)-1);
					//028 
					pWiFiData->Radio[i].TransmitPower = pRadio[i]->transmitPower;
				}
			}
		}

		if(pSSID){
			for(i=0; i<MAX_NUMBER_OF_ENTRIES_SSID; i++){
				if(pSSID[i]){
					//040
					pWiFiData->SSID[i].Enable = pSSID[i]->enable;
					//043
					STRNCPY(pWiFiData->SSID[i].Name, pSSID[i]->name, sizeof(pWiFiData->SSID[i].Name)-1);
					//045
					STRNCPY(pWiFiData->SSID[i].LowerLayers, pSSID[i]->lowerLayers, sizeof(pWiFiData->SSID[i].LowerLayers)-1);
					//048
					STRNCPY(pWiFiData->SSID[i].SSID, pSSID[i]->SSID, sizeof(pWiFiData->SSID[i].SSID)-1);
				}
			}
		}

		if(pAP){
			for(i=0; i<MAX_NUMBER_OF_ENTRIES_ACCESSPOINT; i++){
				if(pAP[i]){
					//064
					pWiFiData->AccessPoint[i].Enable = pAP[i]->enable;
					//067
					STRNCPY(pWiFiData->AccessPoint[i].SSIDReference, pAP[i]->SSIDReference, sizeof(pWiFiData->AccessPoint[i].SSIDReference)-1);
					//068
					pWiFiData->AccessPoint[i].SSIDAdvertisementEnabled = pAP[i]->SSIDAdvertisementEnabled;
					//072
					pWiFiData->AccessPoint[i].WMMEnable = pAP[i]->WMMEnable;
					//073
					pWiFiData->AccessPoint[i].UAPSDEnable = pAP[i]->UAPSDEnable;
				}
			}
		}

		if(pAPSecurity){
			for(i=0; i<MAX_NUMBER_OF_ENTRIES_ACCESSPOINT; i++){
				if(pAPSecurity[i]){
					//079
					STRNCPY(pWiFiData->AccessPoint[i].Security.ModeEnabled, pAPSecurity[i]->modeEnabled, sizeof(pWiFiData->AccessPoint[i].Security.ModeEnabled)-1);
					//080
					STRNCPY(pWiFiData->AccessPoint[i].Security.WEPKey, pAPSecurity[i]->WEPKey, sizeof(pWiFiData->AccessPoint[i].Security.WEPKey)-1);
					//081
					STRNCPY(pWiFiData->AccessPoint[i].Security.PreSharedKey, pAPSecurity[i]->preSharedKey, sizeof(pWiFiData->AccessPoint[i].Security.PreSharedKey)-1);
					//084
					STRNCPY(pWiFiData->AccessPoint[i].Security.RadiusServerIPAddr, pAPSecurity[i]->radiusServerIPAddr, sizeof(pWiFiData->AccessPoint[i].Security.RadiusServerIPAddr)-1);
					//086
					pWiFiData->AccessPoint[i].Security.RadiusServerPort = pAPSecurity[i]->radiusServerPort;
					//088
					STRNCPY(pWiFiData->AccessPoint[i].Security.RadiusSecret, pAPSecurity[i]->radiusSecret, sizeof(pWiFiData->AccessPoint[i].Security.RadiusSecret)-1);
					//x01
					pWiFiData->AccessPoint[i].Security.extRadiusEnabled = pAPSecurity[i]->X_ACTIONTEC_COM_RadiusEnabled;
					//x02
					STRNCPY(pWiFiData->AccessPoint[i].Security.extWEPAuthenticationMode, pAPSecurity[i]->X_ACTIONTEC_COM_WEPAuthenticationMode, sizeof(pWiFiData->AccessPoint[i].Security.extWEPAuthenticationMode)-1);
					//x03
					STRNCPY(pWiFiData->AccessPoint[i].Security.extWPAEncryptionMode, pAPSecurity[i]->X_ACTIONTEC_COM_WPAEncryptionMode, sizeof(pWiFiData->AccessPoint[i].Security.extWPAEncryptionMode)-1);
					//x04
					STRNCPY(pWiFiData->AccessPoint[i].Security.extWPA2EncryptionMode, pAPSecurity[i]->X_ACTIONTEC_COM_WPA2EncryptionMode, sizeof(pWiFiData->AccessPoint[i].Security.extWPA2EncryptionMode)-1);
				}
			}
		}
	} while(0);
	
	return ret;
}

static inline int tr181_wifi_data_store(PDEVICE_WIFI pWiFiData,
	WIFIObject* pWiFi, WIFIRadioObject* pRadio[MAX_NUMBER_OF_ENTRIES_RADIO], 
	WIFISSIDObject* pSSID[MAX_NUMBER_OF_ENTRIES_SSID], 
	WIFIAccessPointObject* pAP[MAX_NUMBER_OF_ENTRIES_ACCESSPOINT],
	WIFIAccessPointSecurityObject* pAPSecurity[MAX_NUMBER_OF_ENTRIES_ACCESSPOINT],
	WIFIAccessPointAssociatedDeviceObject* pAPAssociatedDevice[MAX_NUMBER_OF_ENTRIES_ACCESSPOINT][MAX_NUMBER_OF_ENTRIES_ASSOCIATEDDEVICES])
{
	int ret = 0;
	int i = 0;
	int j = 0;
	
	do {
		if(NULL==pWiFiData){
			ret = -1;
			break;
		}

		if(pWiFi){
			//001
			pWiFi->radioNumberOfEntries = pWiFiData->RadioNumberOfEntries;
			//002
			pWiFi->SSIDNumberOfEntries = pWiFiData->SSIDNumberOfEntries;
			//003
			pWiFi->accessPointNumberOfEntries = pWiFiData->AccessPointNumberOfEntries;
			//x00
			pWiFi->X_ACTIONTEC_COM_WiFiRestoreDefault = 0;
		}

		if(pRadio){
			for(i=0; i<MAX_NUMBER_OF_ENTRIES_RADIO; i++){
				if(pRadio[i]){
					//005
					pRadio[i]->enable = pWiFiData->Radio[i].Enable;
					//006
					CTLMEM_REPLACE_STRING(pRadio[i]->status, pWiFiData->Radio[i].Status);
					//008
					CTLMEM_REPLACE_STRING(pRadio[i]->name, pWiFiData->Radio[i].Name);
					//009
					pRadio[i]->lastChange = pWiFiData->Radio[i].LastChange;
					//010 
					CTLMEM_REPLACE_STRING(pRadio[i]->lowerLayers, pWiFiData->Radio[i].LowerLayers);
					//011
					pRadio[i]->upstream = pWiFiData->Radio[i].Upstream;
					//014 
					CTLMEM_REPLACE_STRING(pRadio[i]->operatingFrequencyBand, pWiFiData->Radio[i].OperatingFrequencyBand);
					//016
					CTLMEM_REPLACE_STRING(pRadio[i]->operatingStandards, pWiFiData->Radio[i].OperatingStandards);
					//019
					pRadio[i]->channel = pWiFiData->Radio[i].Channel;
					//021
					pRadio[i]->autoChannelEnable = pWiFiData->Radio[i].AutoChannelEnable;

                    pRadio[i]->X_ACTIONTEC_COM_AutoChannelRefresh = pWiFiData->Radio[i].X_ACTIONTEC_COM_AutoChannelRefresh;
					//023
					CTLMEM_REPLACE_STRING(pRadio[i]->operatingChannelBandwidth, pWiFiData->Radio[i].OperatingChannelBandwidth);
					//024
					CTLMEM_REPLACE_STRING(pRadio[i]->extensionChannel, pWiFiData->Radio[i].ExtensionChannel);
					//025
					CTLMEM_REPLACE_STRING(pRadio[i]->guardInterval, pWiFiData->Radio[i].GuardInterval);
					//028 
					pRadio[i]->transmitPower = pWiFiData->Radio[i].TransmitPower;
				}
			}
		}

		if(pSSID){
			for(i=0; i<MAX_NUMBER_OF_ENTRIES_SSID; i++){
				if(pSSID[i]){
					//040
					pSSID[i]->enable = pWiFiData->SSID[i].Enable;
					//041
					CTLMEM_REPLACE_STRING(pSSID[i]->status, pWiFiData->SSID[i].Status);
					//043
					CTLMEM_REPLACE_STRING(pSSID[i]->name, pWiFiData->SSID[i].Name);
					//044
					pSSID[i]->lastChange = pWiFiData->SSID[i].LastChange;
					//045
					CTLMEM_REPLACE_STRING(pSSID[i]->lowerLayers, pWiFiData->SSID[i].LowerLayers);
					//046
					CTLMEM_REPLACE_STRING(pSSID[i]->BSSID, pWiFiData->SSID[i].BSSID);
					//047
					CTLMEM_REPLACE_STRING(pSSID[i]->MACAddress, pWiFiData->SSID[i].MACAddress);
					//048
					CTLMEM_REPLACE_STRING(pSSID[i]->SSID, pWiFiData->SSID[i].SSID);
				}
			}
		}

		if(pAP){
			for(i=0; i<MAX_NUMBER_OF_ENTRIES_ACCESSPOINT; i++){
				if(pAP[i]){
					//064
					pAP[i]->enable = pWiFiData->AccessPoint[i].Enable;
					//065
					CTLMEM_REPLACE_STRING(pAP[i]->status, pWiFiData->AccessPoint[i].Status);
					//067
					CTLMEM_REPLACE_STRING(pAP[i]->SSIDReference, pWiFiData->AccessPoint[i].SSIDReference);
					//068
					pAP[i]->SSIDAdvertisementEnabled = pWiFiData->AccessPoint[i].SSIDAdvertisementEnabled;
					//072
					pAP[i]->WMMEnable = pWiFiData->AccessPoint[i].WMMEnable;
					//073
					pAP[i]->UAPSDEnable = pWiFiData->AccessPoint[i].UAPSDEnable;
					//074
					pAP[i]->associatedDeviceNumberOfEntries = pWiFiData->AccessPoint[i].AssociatedDeviceNumberOfEntries;
				}
			}
		}

		if(pAPSecurity){
			for(i=0; i<MAX_NUMBER_OF_ENTRIES_ACCESSPOINT; i++){
				if(pAPSecurity[i]){
					//078
					CTLMEM_REPLACE_STRING(pAPSecurity[i]->modesSupported, pWiFiData->AccessPoint[i].Security.ModesSupported);
					//079
					CTLMEM_REPLACE_STRING(pAPSecurity[i]->modeEnabled, pWiFiData->AccessPoint[i].Security.ModeEnabled);
					//080
					CTLMEM_REPLACE_STRING(pAPSecurity[i]->WEPKey, pWiFiData->AccessPoint[i].Security.WEPKey);
					//081
					CTLMEM_REPLACE_STRING(pAPSecurity[i]->preSharedKey, pWiFiData->AccessPoint[i].Security.PreSharedKey);
					//084
					CTLMEM_REPLACE_STRING(pAPSecurity[i]->radiusServerIPAddr, pWiFiData->AccessPoint[i].Security.RadiusServerIPAddr);
					//086
					pAPSecurity[i]->radiusServerPort = pWiFiData->AccessPoint[i].Security.RadiusServerPort;
					//088
					CTLMEM_REPLACE_STRING(pAPSecurity[i]->radiusSecret, pWiFiData->AccessPoint[i].Security.RadiusSecret);
					//x01
					pAPSecurity[i]->X_ACTIONTEC_COM_RadiusEnabled = pWiFiData->AccessPoint[i].Security.extRadiusEnabled;
					//x02
					CTLMEM_REPLACE_STRING(pAPSecurity[i]->X_ACTIONTEC_COM_WEPAuthenticationMode, pWiFiData->AccessPoint[i].Security.extWEPAuthenticationMode);
					//x03
					CTLMEM_REPLACE_STRING(pAPSecurity[i]->X_ACTIONTEC_COM_WPAEncryptionMode, pWiFiData->AccessPoint[i].Security.extWPAEncryptionMode);
					//x04
					CTLMEM_REPLACE_STRING(pAPSecurity[i]->X_ACTIONTEC_COM_WPA2EncryptionMode, pWiFiData->AccessPoint[i].Security.extWPA2EncryptionMode);
				}
			}
		}

		if(pAPAssociatedDevice){
			for(i=0; i<MAX_NUMBER_OF_ENTRIES_ACCESSPOINT; i++){
				for(j=0; j<MAX_NUMBER_OF_ENTRIES_ASSOCIATEDDEVICES; j++){
					if(pAPAssociatedDevice[i][j]){
						//101
						CTLMEM_REPLACE_STRING(pAPAssociatedDevice[i][j]->MACAddress, pWiFiData->AccessPoint[i].AssociatedDevice[j].MACAddress);
						//102
						pAPAssociatedDevice[i][j]->authenticationState = pWiFiData->AccessPoint[i].AssociatedDevice[j].AuthenticationState;
						//105
						pAPAssociatedDevice[i][j]->signalStrength = pWiFiData->AccessPoint[i].AssociatedDevice[j].SignalStrength;
						//107
						pAPAssociatedDevice[i][j]->active = pWiFiData->AccessPoint[i].AssociatedDevice[j].Active;
						//x05
						pAPAssociatedDevice[i][j]->X_COMCAST_COM_LinkQuality = pWiFiData->AccessPoint[i].AssociatedDevice[j].extLinkQuality;
					}
				}
			}
		}
		
	} while(0);
	
	return ret;
}
#endif

#endif 
