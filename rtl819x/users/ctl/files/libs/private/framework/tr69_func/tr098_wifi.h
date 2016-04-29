/**************************************************************************
 *	
 *	        tr098_wifi.h according to TR-98_Amendment_2.pdf
 *
 **************************************************************************/
#ifndef TR098_WIFI_H
#define TR098_WIFI_H
/**************************************************************************
 *	
 *	        constant definition
 *
 **************************************************************************/
#define SIZE_8 					8
#define SIZE_16 				16
#define SIZE_20 				20
#define SIZE_26 				26
#define SIZE_32 				32
#define SIZE_40 				40
#define SIZE_64 				64
#define SIZE_128 				128

#define LEN_MAC_ADDRESS			17
#define LEN_ACL_COMMENT			32

#define MAX_COUNT_WPS_REGISTRAR	8
#ifndef MAX_COUNT_ASSOCIATED_DEVICES
#define MAX_COUNT_ASSOCIATED_DEVICES	8
#endif
#define MAX_COUNT_WEPKEYS		4
#define MAX_COUNT_PRESHAREDKEYS	10
#define MAX_COUNT_WMM_ENTRIES	4
#define MAX_COUNT_MAC_LIST		32

#define OBJ_FLAG_WLAN				(0x1 << 0)
#define OBJ_FLAG_STATISTICS			(0x1 << 1)
#define OBJ_FLAG_WPS				(0x1 << 2)
#define OBJ_FLAG_ASSOCIATED_DEVICE	(0x1 << 3)
#define OBJ_FLAG_WEP_KEY			(0x1 << 4)
#define OBJ_FLAG_PRE_SHARED_KEY		(0x1 << 5)
#define OBJ_FLAG_AP_WMM_PARAMETER	(0x1 << 6)
#define OBJ_FLAG_STA_WMM_PARAMETER	(0x1 << 7)
#define OBJ_FLAG_WPS_REGISTRAR		(0x1 << 8)
/**************************************************************************
 *	
 *	        structure definition
 *
 **************************************************************************/
/*InternetGatewayDevice.LANDevice.0.WLANConfiguration.0.Stats*/
typedef struct _WLANStats {
	 unsigned int ErrorsSent;				//050
	 unsigned int ErrorsReceived;			//051
	 unsigned int UnicastPacketsSent;		//052
	 unsigned int UnicastPacketsReceived;	//053
	 unsigned int DiscardPacketsSent;		//054
	 unsigned int DiscardPacketsReceived;	//055
	 unsigned int MulticastPacketsSent;			//056
	 unsigned int MulticastPacketsReceived;		//057
	 unsigned int BroadcastPacketsSent;			//058
	 unsigned int BroadcastPacketsReceived;		//059
	 unsigned int UnknownProtoPacketsReceived;	//060
}WLANStats, *PWLANStats;

/*InternetGatewayDevice.LANDevice.0.WLANConfiguration.0.WPS.Registrar*/
typedef struct _WLANRegistrar {
	 unsigned int Enable;		//074
	 char UUID[SIZE_40];		//075
	 char DeviceName[SIZE_32];	//076
}WLANRegistrar, *PWLANRegistrar;

/*InternetGatewayDevice.LANDevice.0.WLANConfiguration.0.WPS*/
typedef struct _WLANWPS {
	 unsigned int Enable;			//061
	 char DeviceName[SIZE_32];		//062
	 unsigned int DevicePassword;	//063
	 char UUID[SIZE_40];			//064
	 unsigned int Version;			//065
	 char ConfigMethodsSupported[SIZE_128];	//066
	 char ConfigMethodsEnabled[SIZE_64];	//067
	 char SetupLockedState[SIZE_32];		//068
	 unsigned int SetupLock;				//069
	 char ConfigurationState[SIZE_16];		//070
	 char LastConfigurationError[SIZE_32];	//071
	 unsigned int RegistrarNumberOfEntries;	//072
	 unsigned int RegistrarEstablished;		//073
	 WLANRegistrar Registrar[MAX_COUNT_WPS_REGISTRAR];
}WLANWPS, *PWLANWPS;

/*InternetGatewayDevice.LANDevice.0.WLANConfiguration.0.AssociatedDevice*/
typedef struct _WLANAssociatedDevice {
	 char AssociatedDeviceMACAddress[SIZE_20];	//077
	 char AssociatedDeviceIPAddress[SIZE_64];	//078
	 unsigned int AssociatedDeviceAuthenticationState;	//079
	 char LastRequestedUnicastCipher[SIZE_32];			//080
	 char LastRequestedMulticastCipher[SIZE_32];		//081
	 char LastPMKId[SIZE_32];							//082
	 char LastDataTransmitRate[SIZE_8];					//083
}WLANAssociatedDevice, *PWLANAssociatedDevice;

/*InternetGatewayDevice.LANDevice.0.WLANConfiguration.0.WEPKey*/
typedef struct _WLANWEPKey {
	 char WEPKey[SIZE_64+1];	//084	
	 unsigned int WEPKeyType;	//x29	0 - ASCII, 1 - HEX
}WLANWEPKey, *PWLANWEPKey;

/*InternetGatewayDevice.LANDevice.0.WLANConfiguration.0.PreSharedKey*/
typedef struct _WLANPreSharedKey {
	 char PreSharedKey[SIZE_64+1];				//085
	 char KeyPassphrase[SIZE_64+1];				//086
	 char AssociatedDeviceMACAddress[SIZE_20];	//087
}WLANPreSharedKey, *PWLANPreSharedKey;

/*InternetGatewayDevice.LANDevice.0.WLANConfiguration.0.APWMMParameter*/
typedef struct _WLANAPWMMParameter {
	 unsigned int AIFSN;	//088
	 unsigned int ECWMin;	//089
	 unsigned int ECWMax;	//090
	 unsigned int TXOP;		//091
	 unsigned int AckPolicy;//092
}WLANAPWMMParameter, *PWLANAPWMMParameter;

/*InternetGatewayDevice.LANDevice.0.WLANConfiguration.0.STAWMMParameter*/
typedef struct _WLANSTAWMMParameter {
	 unsigned int AIFSN;	//093
	 unsigned int ECWMin;	//094
	 unsigned int ECWMax;	//095
	 unsigned int TXOP;		//096
	 unsigned int AckPolicy;//097
}WLANSTAWMMParameter, *PWLANSTAWMMParameter;

/*InternetGatewayDevice.LANDevice.0.WLANConfiguration*/
typedef struct _WLANConfiguration {
	 unsigned int Enable;		//001
	 char Status[SIZE_16];		//002
	 char Name[SIZE_16];		//003
	 char BSSID[SIZE_20];		//004 '00:03:7F:BE:F1:13'
	 char MaxBitRate[SIZE_8];	//005
	 unsigned int Channel;					//006
	 unsigned int AutoChannelEnable;		//007
     unsigned int X_ACTIONTEC_COM_AutoChannelRefresh;
	 char SSID[SIZE_32];					//008
	 char BeaconType[SIZE_16];				//009
	 unsigned int MACAddressControlEnabled;	//010
	 char Standard[SIZE_8];					//011
	 unsigned int WEPKeyIndex;				//012
	 char KeyPassphrase[SIZE_64];			//013
	 char WEPEncryptionLevel[SIZE_64];		//014
	 char BasicEncryptionModes[SIZE_16];	//015
	 char BasicAuthenticationMode[SIZE_32];		//016
	 char WPAEncryptionModes[SIZE_32];			//017
	 char WPAAuthenticationMode[SIZE_20];		//018
	 char IEEE11iEncryptionModes[SIZE_32];		//019
	 char IEEE11iAuthenticationMode[SIZE_32];	//020
	 char PossibleChannels[SIZE_64];			//021
	 char BasicDataTransmitRates[SIZE_64];		//022
	 char OperationalDataTransmitRates[SIZE_64];//023
	 char PossibleDataTransmitRates[SIZE_64];	//024
	 unsigned int InsecureOOBAccessEnabled;		//025
	 unsigned int BeaconAdvertisementEnabled;	//026
	 unsigned int SSIDAdvertisementEnabled;		//027
	 unsigned int RadioEnabled;					//028
	 char TransmitPowerSupported[SIZE_64];		//029
	 unsigned int TransmitPower;				//030
	 unsigned int AutoRateFallBackEnabled;	//031
	 char LocationDescription[SIZE_32];		//032
	 char RegulatoryDomain[SIZE_8];			//033
	 unsigned int TotalPSKFailures;			//034
	 unsigned int TotalIntegrityFailures;	//035
	 char ChannelsInUse[SIZE_64];				//036
	 char DeviceOperationMode[SIZE_32];			//037
	 unsigned int DistanceFromRoot;				//038
	 char PeerBSSID[SIZE_20];					//039
	 char AuthenticationServiceMode[SIZE_32];	//040
	 unsigned int WMMSupported;		//041
	 unsigned int UAPSDSupported;	//042
	 unsigned int WMMEnable;		//043
	 unsigned int UAPSDEnable;		//044
	 unsigned int TotalBytesSent;	//045
	 unsigned int TotalBytesReceived;	//046
	 unsigned int TotalPacketsSent;		//047
	 unsigned int TotalPacketsReceived;	//048
	 unsigned int TotalAssociations;	//049
	 
	 WLANStats	Stats;
	 WLANWPS	WPS;
	 WLANAssociatedDevice AssociatedDevice[MAX_COUNT_ASSOCIATED_DEVICES];
	 WLANWEPKey WEPKey[MAX_COUNT_WEPKEYS];
	 WLANPreSharedKey PreSharedKey[MAX_COUNT_PRESHAREDKEYS];
	 WLANAPWMMParameter APWMMParameter[MAX_COUNT_WMM_ENTRIES];
	 WLANSTAWMMParameter STAWMMParameter[MAX_COUNT_WMM_ENTRIES];
	 
	 unsigned int extPreSharedKeyIndex;		//x01
	 unsigned int extGroupKeyUpdateInterval;//x02
	 char ext8021xServerAddress[SIZE_16];	//x03
	 unsigned int ext8021xServerPort;		//x04
	 char ext8021xServerSecret[SIZE_64];	//x05
	 unsigned int extBeaconInterval;		//x06
	 unsigned int extDTIMInterval;			//x07
	 unsigned int extFragmentationThreshold;//x08
	 unsigned int extRTSThreshold;			//x09
	 char extCTSProtectMode[SIZE_8];		//x10
	 unsigned int extMSDULimit;									//x11
	 unsigned int extMPDULimit;									//x12
	 unsigned int extMACPolicy;									//x13
	 char extMACList[MAX_COUNT_MAC_LIST*(LEN_MAC_ADDRESS+1)];	//x14
	 char extRadioDevName[SIZE_16];								//x15
	 unsigned int extWDS;					//x16
	 unsigned int extAPBridge;				//x17
	 char extCustomedWEPKey[SIZE_64+1];		//x18
	 unsigned int extWPSApPinTimeout;		//x19
	 unsigned int extWPSApPinDefault;		//x20
	 unsigned int extWpsPinTimeout;			//x21
	 unsigned int extChannelWidth;			//x22
	 unsigned int extControlSideBand;		//x23
	 unsigned int extPreambleType;			//x24
	 unsigned int extIAPPEnable;			//x25
	 unsigned int extShortGuardInterval;			//x26
	 unsigned int extSpaceTimeBlockCoding;			//x27
	 //x29 for WEPKeyType
	 char extACLComment[MAX_COUNT_MAC_LIST*(LEN_ACL_COMMENT+1)];	//x30
}WLANConfiguration, *PWLANConfiguration;
/**************************************************************************
 *	
 *	        function declaration
 *
 **************************************************************************/
int tr098WiFiObjectDestroy(unsigned int objFlag, PWLANConfiguration pCurData);
int tr098WiFiObjectCreate(unsigned int objFlag, PWLANConfiguration pNewData);
int tr098WiFiObjectModify(unsigned int objFlag, PWLANConfiguration pCurData, PWLANConfiguration pNewData);
int tr098WiFiObjectGet(unsigned int objFlag, PWLANConfiguration pCurData);

#endif

