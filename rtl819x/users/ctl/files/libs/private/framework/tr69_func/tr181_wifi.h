/**************************************************************************
 *	
 *	        tr181_wifi.h according to TR-181_Issue-2_Amendment-5.pdf
 *		http://www.broadband-forum.org/cwmp/tr-181-2-5-0.html#D.Device:2.Device.WiFi.
 *
 **************************************************************************/
#ifndef TR181_WIFI_H
#define TR181_WIFI_H
/**************************************************************************
 *	
 *	        constant definition
 *
 **************************************************************************/
 
#define _DEBUG

#define SIZE_8 					8
#define SIZE_10                 10
#define SIZE_16 				16
#define SIZE_20 				20
#define SIZE_26 				26
#define SIZE_32 				32
#define SIZE_48 				48
#define SIZE_64 				64
#define SIZE_128 				128

#define MAX_NUMBER_OF_ENTRIES_RADIO			2
#define MAX_NUMBER_OF_ENTRIES_SSID			8
#define MAX_NUMBER_OF_ENTRIES_ACCESSPOINT	8
#define MAX_NUMBER_OF_ENTRIES_ENDPOINT		2
#define MAX_NUMBER_OF_ENTRIES_PROFILE		4
#define MAX_NUMBER_OF_ENTRIES_ASSOCIATEDDEVICES		8

#define OBJ_FLAG_WIFI								(0x1 << 0)
#define OBJ_FLAG_WIFI_RADIO							(0x1 << 1)
#define OBJ_FLAG_WIFI_RADIO_STATISTICS				(0x1 << 2)
#define OBJ_FLAG_WIFI_SSID							(0x1 << 3)
#define OBJ_FLAG_WIFI_SSID_STATISTICS				(0x1 << 4)
#define OBJ_FLAG_WIFI_ACCESSPOINT					(0x1 << 5)
#define OBJ_FLAG_WIFI_ACCESSPOINT_SECURITY			(0x1 << 6)
#define OBJ_FLAG_WIFI_ACCESSPOINT_ACCOUNTING		(0x1 << 7)
#define OBJ_FLAG_WIFI_ACCESSPOINT_WPS				(0x1 << 8)
#define OBJ_FLAG_WIFI_ACCESSPOINT_ASSOCIATEDDEVICE	(0x1 << 9)
#define OBJ_FLAG_WIFI_ENDPOINT						(0x1 << 10)
#define OBJ_FLAG_WIFI_ENDPOINT_STATISTICS			(0x1 << 11)
#define OBJ_FLAG_WIFI_ENDPOINT_SECURITY				(0x1 << 12)
#define OBJ_FLAG_WIFI_ENDPOINT_PROFILE				(0x1 << 13)
#define OBJ_FLAG_WIFI_ENDPOINT_PROFILE_SECURITY		(0x1 << 14)
#define OBJ_FLAG_WIFI_ENDPOINT_WPS					(0x1 << 15)
#define OBJ_FLAG_MASK								(0x0000FFFF)

#define WLAN1_VA0_IF_NAME "wlan1_va0"

/**************************************************************************
 *	
 *			structure definition
 *
 **************************************************************************/

/*Device.WiFi.EndPoint.{i}.WPS.*/
typedef struct _ENDPOINT_WPS {
	unsigned int Enable;				   //129
	char ConfigMethodsSupported[SIZE_128]; //130
	char ConfigMethodsEnabled [SIZE_32];   //131
}ENDPOINT_WPS, *PENDPOINT_WPS;
	
/*Device.WiFi.EndPoint.{i}.Profile.{i}.Security.*/
typedef struct _PROFILE_SECURITY {
	char ModeEnabled[SIZE_20];	   			//125
	unsigned char WEPKey[SIZE_16]; 			//126
	unsigned char PreSharedKey[SIZE_32];	//127
	char KeyPassphrase[SIZE_64];			//128
}PROFILE_SECURITY, *PPROFILE_SECURITY;

/*Device.WiFi.EndPoint.{i}. Profile.{i}.*/
typedef struct _ENDPOINT_PROFILE {
	unsigned int Enable;	//119
	char Status[SIZE_20];	//120
	char Alias[SIZE_64];	//121
	char SSID[SIZE_32]; 	//122
	char Location[SIZE_32]; //123
	unsigned int Priority;	//124	 	unsignedInt[:255]
	PROFILE_SECURITY Security;
}ENDPOINT_PROFILE, *PENDPOINT_PROFILE;
	
/*Device.WiFi.EndPoint.{i}.Security.*/
typedef struct _ENDPOINT_SECURITY {
	char ModesSupported[SIZE_128]; //118
}ENDPOINT_SECURITY, *PENDPOINT_SECURITY;

/*Device.WiFi.EndPoint.{i}.Stats.*/
typedef struct _ENDPOINT_STATISTICS {
	unsigned int LastDataDownlinkRate;	//114		[1000:600000]
	unsigned int LastDataUplinkRate;	//115		[1000:600000]
	int SignalStrength;  				//116	    	int[-200:0]
	unsigned int Retransmissions;  		//117	    	unsignedInt[0:100]
}ENDPOINT_STATISTICS, *PENDPOINT_STATISTICS;

/*Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.*/
typedef struct _ACCESSPOINT_ASSOCIATEDDEVICE {
	char MACAddress[SIZE_20]; 		   	//101
	unsigned int AuthenticationState;	//102
	unsigned int LastDataDownlinkRate;	//103		[1000:600000]
	unsigned int LastDataUplinkRate;	//104		[1000:600000]
	int SignalStrength;  				//105	    	int[-200:0]
	unsigned int Retransmissions;  		//106	    	unsignedInt[0:100]
	unsigned int Active;  				//107
	unsigned int extLinkQuality;		//x05		unsignedInt[0:100]
}ACCESSPOINT_ASSOCIATEDDEVICE, *PACCESSPOINT_ASSOCIATEDDEVICE;

/*Device.WiFi.AccessPoint.{i}.WPS.*/
typedef struct _ACCESSPOINT_WPS {
	unsigned int Enable;				   //098
	char ConfigMethodsSupported[SIZE_128]; //099
	char ConfigMethodsEnabled [SIZE_32];   //100
}ACCESSPOINT_WPS, *PACCESSPOINT_WPS;

/*Device.WiFi.AccessPoint.{i}.Accounting.*/
typedef struct _ACCESSPOINT_ACCOUNTING {
	unsigned int Enable;		   //090
	char ServerIPAddr[SIZE_48]; 		   //091
	char SecondaryServerIPAddr [SIZE_48];  //092
	unsigned int ServerPort;			   //093
	unsigned int SecondaryServerPort;	   //094
	char Secret[SIZE_64+1];				   //095
	char SecondarySecret[SIZE_64+1]; //096
	unsigned int InterimInterval;  //097	   unsignedInt[0, 60:]
}ACCESSPOINT_ACCOUNTING, *PACCESSPOINT_ACCOUNTING;

/*Device.WiFi.AccessPoint.{i}.Security.*/
typedef struct _ACCESSPOINT_SECURITY {
	unsigned int Reset; 		   //077
	char ModesSupported[SIZE_128]; //078
	char ModeEnabled[SIZE_20];	   //079
	char WEPKey[SIZE_26+1]; //080
	char PreSharedKey[SIZE_64+1];		   //081
	char KeyPassphrase[SIZE_64];				   //082
	unsigned int RekeyingInterval;				   //083
	char RadiusServerIPAddr[SIZE_48];			   //084
	char SecondaryRadiusServerIPAddr [SIZE_48];    //085
	unsigned int RadiusServerPort;			   //086
	unsigned int SecondaryRadiusServerPort;    //087
	char RadiusSecret[SIZE_64+1]; 			   //088
	char SecondaryRadiusSecret[SIZE_64+1];	   //089
	unsigned int extRadiusEnabled;			//x01
	char extWEPAuthenticationMode[SIZE_32];	//x02
	char extWPAEncryptionMode[SIZE_32];		//x03
	char extWPA2EncryptionMode[SIZE_32];	//x04
}ACCESSPOINT_SECURITY, *PACCESSPOINT_SECURITY;

/*Device.WiFi.SSID.{i}.Stats.*/
typedef struct _SSID_STATISTICS {
	unsigned int BytesSent;		//049
	unsigned int BytesReceived;	//050
	unsigned int PacketsSent;	   		//051
	unsigned int PacketsReceived;   	//052
	unsigned int ErrorsSent;	   		//053
	unsigned int ErrorsReceived;   		//054
	unsigned int UnicastPacketsSent;	//055
	unsigned int UnicastPacketsReceived;	//056
	unsigned int DiscardPacketsSent;		//057
	unsigned int DiscardPacketsReceived;	//058
	unsigned int MulticastPacketsSent;  	//059
	unsigned int MulticastPacketsReceived;	//060
	unsigned int BroadcastPacketsSent;		   	//061
	unsigned int BroadcastPacketsReceived;	   	//062
	unsigned int UnknownProtoPacketsReceived;	//063
}SSID_STATISTICS, *PSSID_STATISTICS;

/*Device.WiFi.Radio.{i}.Stats.*/
typedef struct _RADIO_STATISTICS {
	unsigned int BytesSent;  				//032
	unsigned int BytesReceived;  			//033
	unsigned int PacketsSent;  				//034
	unsigned int PacketsReceived;  			//035
	unsigned int ErrorsSent;  				//036
	unsigned int ErrorsReceived;  			//037
	unsigned int DiscardPacketsSent;  		//038
	unsigned int DiscardPacketsReceived;	//039
}RADIO_STATISTICS, *PRADIO_STATISTICS;

/*Device.WiFi.EndPoint.{i}.*/
typedef struct _WIFI_ENDPOINT {
	unsigned int Enable;	//108
	char Status[SIZE_20];	//109
	char Alias[SIZE_64];	//110
	char ProfileReference[SIZE_64];	   		//111		//string(256) in tr181
	char SSIDReference[SIZE_64];	   		//112		//string(256) in tr181
	unsigned int ProfileNumberOfEntries;	//113
	ENDPOINT_STATISTICS Stats;
	ENDPOINT_SECURITY Security;
	ENDPOINT_PROFILE Profile[MAX_NUMBER_OF_ENTRIES_PROFILE];
	ENDPOINT_WPS WPS;
}WIFI_ENDPOINT, *PWIFI_ENDPOINT;

/*Device.WiFi.AccessPoint.{i}.*/
typedef struct _WIFI_ACCESSPOINT {
	unsigned int Enable;	//064
	char Status[SIZE_20];	//065
	char Alias[SIZE_64];	   				//066
	char SSIDReference[SIZE_64];	   		//067		//string(256) in tr181
	unsigned int SSIDAdvertisementEnabled;	//068
	unsigned int RetryLimit;	   			//069		 	unsignedInt[0:7]
	unsigned int WMMCapability;	   			//070
	unsigned int UAPSDCapability;	   				//071
	unsigned int WMMEnable;	   						//072
	unsigned int UAPSDEnable;	   					//073
	unsigned int AssociatedDeviceNumberOfEntries;	//074
	unsigned int MaxAssociatedDevices;	   			//075
	unsigned int IsolationEnable;	//076
	ACCESSPOINT_SECURITY Security;
	ACCESSPOINT_ACCOUNTING Accounting;
	ACCESSPOINT_WPS WPS;
	ACCESSPOINT_ASSOCIATEDDEVICE AssociatedDevice[MAX_NUMBER_OF_ENTRIES_ASSOCIATEDDEVICES];
}WIFI_ACCESSPOINT, *PWIFI_ACCESSPOINT;

/*Device.WiFi.SSID.{i}.*/
typedef struct _WIFI_SSID {
	unsigned int Enable;	   //040
	char Status[SIZE_16];	   //041
	char Alias[SIZE_64];	   //042
	char Name[SIZE_64]; 	   //043
	unsigned int LastChange;   //044
	char LowerLayers[SIZE_64]; //045   //string(1024) in tr181
	char BSSID[SIZE_20];	   //046
	char MACAddress[SIZE_20];  //047
	char SSID[SIZE_32+1]; 	   //048 /* if ssid is 5G , ssid is ssid-5G(\0)*/
	SSID_STATISTICS Stats;
}WIFI_SSID, *PWIFI_SSID;

/*Device.WiFi.Radio.{i}.*/
typedef struct _WIFI_RADIO {
	unsigned int Enable;	   //005
	char Status[SIZE_16];	   //006
	char Alias[SIZE_64];	   //007
	char Name[SIZE_64]; 	   //008
	unsigned int LastChange;   //009
	char LowerLayers[SIZE_8]; //010   //string(1024) in tr181
	unsigned int Upstream;				   //011
	unsigned int MaxBitRate;			   //012
	char SupportedFrequencyBands[SIZE_16]; //013
	char OperatingFrequencyBand[SIZE_8];   //014
	char SupportedStandards[SIZE_8];	   //015
	char OperatingStandards[SIZE_8];   //016
	char PossibleChannels[SIZE_64];    //017   //string(1024) in tr181
	char ChannelsInUse[SIZE_64];	   //018   //string(1024) in tr181
	unsigned int Channel;			   //019
	unsigned int AutoChannelSupported; //020
	unsigned int AutoChannelEnable; 	   //021
    unsigned int X_ACTIONTEC_COM_AutoChannelRefresh;
	unsigned int AutoChannelRefreshPeriod; //022
	char OperatingChannelBandwidth[SIZE_8];//023
	char ExtensionChannel[SIZE_20]; 	   //024
	char GuardInterval[SIZE_8]; 		   //025
	int MCS;							   //026   //[-1:15, 16:31]
	char TransmitPowerSupported[SIZE_64];  //027
	int TransmitPower;					   //028   //[-1:100]
	unsigned int IEEE80211hSupported;	   //029
	unsigned int IEEE80211hEnabled; 	   //030
	char RegulatoryDomain[SIZE_8];		   //031
	RADIO_STATISTICS Stats;
}WIFI_RADIO, *PWIFI_RADIO;

/*Device.WiFi.*/
typedef struct _DEVICE_WIFI {
	unsigned int RadioNumberOfEntries;			//001
	unsigned int SSIDNumberOfEntries;			//002
	unsigned int AccessPointNumberOfEntries;	//003
	unsigned int EndPointNumberOfEntries;		//004
	WIFI_RADIO Radio[MAX_NUMBER_OF_ENTRIES_RADIO];
	WIFI_SSID SSID[MAX_NUMBER_OF_ENTRIES_SSID];
	WIFI_ACCESSPOINT AccessPoint[MAX_NUMBER_OF_ENTRIES_ACCESSPOINT];
	WIFI_ENDPOINT EndPoint[MAX_NUMBER_OF_ENTRIES_ENDPOINT];
}DEVICE_WIFI, *PDEVICE_WIFI;
/**************************************************************************
 *	
 *			function declaration
 *
 **************************************************************************/
int tr181WiFiObjectDestroy(char* objPathName, PDEVICE_WIFI pCurData);
int tr181WiFiObjectCreate(char* objPathName, PDEVICE_WIFI pNewData);
int tr181WiFiObjectModify(char* objPathName, PDEVICE_WIFI pCurData, PDEVICE_WIFI pNewData);
int tr181WiFiObjectGet(char* objPathName, PDEVICE_WIFI pCurData);

#endif

