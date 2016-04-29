/**************************************************************************
 *	
 *	        wifi_adapter_driver.h
 *
 **************************************************************************/
#ifndef WIFI_ADAPTER_DRIVER_H
#define WIFI_ADAPTER_DRIVER_H

#define WIFI_DRIVER_REALTEK
/**************************************************************************
 *	
 *	        constant definition
 *
 **************************************************************************/
#define WLAN_DEVICE_NAME_DELIMITER	" "
#define WLAN_ACL_MAC_LIST_DELIMITER	" "
#define WLAN_ACL_MAC_LIST_COMMENT_DELIMITER	" "

#define MAX_COUNT_ASSOCIATED_DEVICES	8

typedef enum _DEVICE_STATUS{
	DEVICE_STATUS_Error,
	DEVICE_STATUS_Up,
	DEVICE_STATUS_Down, 
	DEVICE_STATUS_Unknown, 
	DEVICE_STATUS_Dormant, 
	DEVICE_STATUS_NotPresent, 
	DEVICE_STATUS_LowerLayerDown		
}DEVICE_STATUS;
typedef enum _CHANNEL_WIDTH{
	CHANNEL_WIDTH_20MHZ,
	CHANNEL_WIDTH_40MHZ,
	CHANNEL_WIDTH_AUTO
}CHANNEL_WIDTH;
typedef enum _EXTENSION_CHANNEL{
	CONTROL_CHANNEL_ABOVE,
	CONTROL_CHANNEL_BELOW,
	CONTROL_CHANNEL_AUTO
}EXTENSION_CHANNEL;
typedef enum _GUARD_INTERVAL{
	GUARD_INTERVAL_400,
	GUARD_INTERVAL_800,
	GUARD_INTERVAL_AUTO
}GUARD_INTERVAL;
typedef enum _DEVICE_OPERATION_MODE{
	DEVICE_OPERATION_MODE_ACCESSPOINT,
	DEVICE_OPERATION_MODE_STATION,
	DEVICE_OPERATION_MODE_BRIDGE,
	DEVICE_OPERATION_MODE_REPEATER
}DEVICE_OPERATION_MODE;
typedef enum _AUTHENTICATION_MODE{
	AUTHENTICATION_MODE_OPEN = 1,
	AUTHENTICATION_MODE_SHARED,
	AUTHENTICATION_MODE_EAP,
	AUTHENTICATION_MODE_AUTO,
	AUTHENTICATION_MODE_WPA
}AUTHENTICATION_MODE;
typedef enum _WEPKEY_TYPE{
	WEPKEY_TYPE_ASCII,
	WEPKEY_TYPE_HEX
}WEPKEY_TYPE;
typedef enum _WPA_MODE{
	WPA_MODE_NONE,
	WPA_MODE_WPA,
	WPA_MODE_WPA2,
	WPA_MODE_MIXED
}WPA_MODE;
typedef enum _WPA_ENCRYPTION_MODES{
	WPA_ENCRYPTION_MODES_TKIP = 1,
	WPA_ENCRYPTION_MODES_AES,
	WPA_ENCRYPTION_MODES_TKIP_AES
}WPA_ENCRYPTION_MODES;

typedef enum _WMM_CLASS_NUMBER{
	WMM_CLASS_NUMBER_BEST_EFFORT,
	WMM_CLASS_NUMBER_BACKGROUND,
	WMM_CLASS_NUMBER_VIDEO,
	WMM_CLASS_NUMBER_VOICE,
	WMM_CLASS_NUMBER_COUNT
}WMM_CLASS_NUMBER;

#define BAND_2G	(0x1 << 0)
#define BAND_5G	(0x1 << 1)

#define MODE_80211_A	(0x1 << 0)
#define MODE_80211_B	(0x1 << 1)
#define MODE_80211_G	(0x1 << 2)
#define MODE_80211_N	(0x1 << 3)

#define SECURITY_MODE_NONE				(0x1 << 0)
#define SECURITY_MODE_WEP64				(0x1 << 1)
#define SECURITY_MODE_WEP128			(0x1 << 2)
#define SECURITY_MODE_PERSONAL_WPA		(0x1 << 3)
#define SECURITY_MODE_PERSONAL_WPA2		(0x1 << 4)
#define SECURITY_MODE_PERSONAL_MIXED	(0x1 << 5)
#define SECURITY_MODE_ENTERPRISE_WPA	(0x1 << 6)
#define SECURITY_MODE_ENTERPRISE_WPA2	(0x1 << 7)
#define SECURITY_MODE_ENTERPRISE_MIXED	(0x1 << 8)

#define CTS_PROTECT_MODE_NONE		0
#define CTS_PROTECT_MODE_CTSONLY	1
#define CTS_PROTECT_MODE_RTSCTS		2



#define WPS_STATE_DISABLED 			0
#define WPS_STATE_NOT_CONFIGURED 	1
#define WPS_STATE_CONFIGURED 		2

#define WPS_CONFIG_METHODS_FLAG_USBFLASHDRIVER 			(0x1 << 0)
#define WPS_CONFIG_METHODS_FLAG_ETHERNET 				(0x1 << 1)
#define WPS_CONFIG_METHODS_FLAG_LABEL 					(0x1 << 2)
#define WPS_CONFIG_METHODS_FLAG_DISPLAY 				(0x1 << 3)
#define WPS_CONFIG_METHODS_FLAG_EXTERNEL_NFC_TOKEN 		(0x1 << 4)
#define WPS_CONFIG_METHODS_FLAG_INTEGRATED_NFC_TOKEN 	(0x1 << 5)
#define WPS_CONFIG_METHODS_FLAG_NFC_INTERFACE 			(0x1 << 6)
#define WPS_CONFIG_METHODS_FLAG_PUSHBUTTON 				(0x1 << 7)
#define WPS_CONFIG_METHODS_FLAG_PIN		 				(0x1 << 8)

#define WMM_PARAM_NAME_AIFS			"aifs"
#define WMM_PARAM_NAME_CWMIN		"cwmin"
#define WMM_PARAM_NAME_CWMAX		"cwmax"
#define WMM_PARAM_NAME_TXOPLIMIT	"txoplimit"
#define WMM_PARAM_NAME_NOACKPOLICY	"noackpolicy"



/**************************************************************************
 *	
 *	        structure definition
 *
 **************************************************************************/

/**************************************************************************
 *	
 *	        function declaration
 *
 **************************************************************************/

/*
 * wifi_destroy_ssid_device
 *		This function destroies a ssid device specified by devName.
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_destroy_ssid_device(char* devName);
/*
 * wifi_create_ssid_device
 *		This function creates a ssid device on a radio device specified by radioName.
 * Parameters
 *	radioName
 *		[in] Name of radio device which the ssid device created on.
 *	devOperationMode
 *		[in] specify the operation mode for ssid device.
 *	devName
 *		[out] Name of the created ssid device.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_create_ssid_device(char* radioName, DEVICE_OPERATION_MODE devOperationMode, char* devName);
/*
 * wifi_close_device
 *		This function closes the device specified by devName.
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_close_device(char* devName);
/*
 * wifi_open_device
 *		This function opens the device specified by devName.
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_open_device(char* devName);
/*
 * wifi_destroy_radio_device
 *		This function destroies a radio device.
 * Parameters
 *	devName
 *		[in] The name of the radio device.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_destroy_radio_device(char* devName);
/*
 * wifi_create_radio_device
 *		This function creates a radio device.
 * Parameters
 *	FrequencyBand
 *		[in] The frequency band of the radio device. bit0 - 2GHz, bit1 - 5GHz
 *	devName
 *		[out] The name of the radio device.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_create_radio_device(unsigned int FrequencyBand, char* devName);
/*
 * wifi_close_radio
 *		This function close a radio interface specified by radioName.
 * Parameters
 *	radioName
 *		[in] Name of interface.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_close_radio(char* radioName);
/*
 * wifi_open_radio
 *		This function open a radio interface specified by radioName.
 * Parameters
 *	radioName
 *		[in] Name of interface.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_open_radio(char* radioName);
/*
 * wifi_get_RadioNumberOfEntries
 *		This function retrieves the number of radio entries.
 * Parameters
 *	None
 * Return Values
 *	Return the number of radio entries.
 */
unsigned int wifi_get_RadioNumberOfEntries(void);
/*
 * wifi_get_SSIDNumberOfEntries
 *		This function retrieves the number of SSID entries.
 * Parameters
 *	None
 * Return Values
 *	Return the number of SSID entries.
 */
unsigned int wifi_get_SSIDNumberOfEntries(void);
/*
 * wifi_get_AccessPointNumberOfEntries
 *		This function retrieves the number of AccessPoint entries.
 * Parameters
 *	None
 * Return Values
 *	Return the number of AccessPoint entries.
 */
unsigned int wifi_get_AccessPointNumberOfEntries(void);
/*
 * wifi_get_EndPointNumberOfEntries
 *		This function retrieves the number of EndPoint entries.
 * Parameters
 *	None
 * Return Values
 *	Return the number of EndPoint entries.
 */
unsigned int wifi_get_EndPointNumberOfEntries(void);
/*
 * wifi_get_device_status
 *		This function retrieves the device status.
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Return device's status enumeration of DEVICE_STATUS.
 */
DEVICE_STATUS wifi_get_device_status(char *devName);
/*
 * wifi_get_radio_SupportedFrequencyBands
 *		This function retrieves the frequency bands at which the radio can operate.
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Return the frequency bands supported by this interface (bit0 - 2.4GHz, bit1 - 5GHz).
 */
unsigned int wifi_get_radio_SupportedFrequencyBands(char *devName);
/*
 * wifi_get_radio_MaxBitRate
 *		This function retrieves the maximum PHY bit rate supported by this interface.
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Return device's maximum PHY bit rate (expressed in Mbps).
 */
int wifi_get_radio_MaxBitRate(char *devName);
/*
 * wifi_get_radio_SupportedStandards
 *		This function retrieves which IEEE 802.11 standards this Radio instance can support .
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Return the IEEE 802.11 standards supported by this interface (bit0 - a, bit1 - b, bit2 - g, bit3 - n).
 */
unsigned int wifi_get_radio_SupportedStandards(char *devName);
/*
 * wifi_set_radio_standards
 *		This function sets standard parameter of the radio device.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	standard
 *		[in] 802.11 standards. bit0 - 802.11a, bit1 - 802.11b, bit2 - 802.11g, bit3 - 802.11n
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_radio_standards(char* devName, unsigned int standard);
/*
 * wifi_get_radio_PossibleChannels
 *		This function retrieves availiable possible channels list currently .
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	availableChannelList
 *		[out] Comma-separated list (maximum length 64) of strings,Ranges in the form "n-m" are permitted.
 *	bufLen
 *		[in] size of availableChannelList for receiving output string.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_get_radio_PossibleChannels(char* devName, char* availableChannelList, unsigned int bufLen);
/*
 * wifi_get_radio_ChannelsInUse
 *		This function retrieves channels in use currently .
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	channelsInUse
 *		[out] Comma-separated list (maximum length 64) of strings,Ranges in the form "n-m" are permitted.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_get_radio_ChannelsInUse(char* devName, char* channelsInUse);
/*
 * wifi_get_radio_channel
 *		This function retrieves current channel.
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Return the current channel [1:255].
 */
unsigned int wifi_get_radio_channel(char* devName);
/*
 * wifi_set_radio_channel_info
 *		This function sets the channel informations for special interface.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	channelBandwidth
 *		[in] Enumeration of CHANNEL_WIDTH.
 *	extensionChannel
 *		[in]  Enumeration of EXTENSION_CHANNEL.
 *	autoChannelEnable
 *		[in]  Enable or disable automatic channel selection.
 *	autoChannelRefreshPeriod
 *		[in]  The time period in seconds between two consecutive automatic channel selections. A value of 0 means that the automatic channel selection is done only at boot time.
 *	channel
 *		[in] The current radio channel used by the connection. 
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_radio_channel_info(char* devName, CHANNEL_WIDTH channelBandwidth, EXTENSION_CHANNEL extensionChannel,
	unsigned int autoChannelEnable, unsigned int autoChannelRefreshPeriod, unsigned int channel);
/*
 * wifi_get_radio_AutoChannelSupported
 *		This function retrieves the capability whether supports auto cnannel selection or not
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Nonzero if the interface support auto channel selection; otherwise returns zero.
 */
unsigned int wifi_get_radio_AutoChannelSupported(char* devName);
/*
 * wifi_set_radio_GuardInterval
 *		This function sets guard interval parameter of the radio device.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	guardInterval
 *		[in] Enumeration of GUARD_INTERVAL.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_radio_GuardInterval(char* devName, GUARD_INTERVAL guardInterval);
/*
 * wifi_set_radio_MCS
 *		This function sets MCS parameter of the radio device.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	MCS
 *		[in] The Modulation Coding Scheme index (applicable to 802.11n specifications only). Values from 0 to 15 MUST be supported ([802.11n-2009]). A value of -1 indicates automatic selection of the MCS index..
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_radio_MCS(char* devName, int MCS);
/*
 * wifi_get_radio_TransmitPowerSupported
 *		This function retrieves supported transmit power list .
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	transmitPowerSupported
 *		[out] Comma-separated list (maximum length 64) of integers (value -1 to 100). List items represent supported transmit power levels as percentage of full power. For example, "0,25,50,75,100". 
 *	bufLen
 *		[in] size of received buffer.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_get_radio_TransmitPowerSupported(char* devName, char* transmitPowerSupported, unsigned int bufLen);
/*
 * wifi_set_radio_TransmitPower
 *		This function sets transmit power parameter of the radio device.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	transmitPower
 *		[in] Indicates the current transmit power level as a percentage of full power. A value of -1 indicates auto mode
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_radio_TransmitPower(char* devName, int transmitPower);
/*
 * wifi_get_radio_IEEE80211hSupported
 *		This function retrieves the capability whether supports IEEE802.11h or not
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Nonzero if the interface support IEEE802.11h; otherwise returns zero.
 */
unsigned int wifi_get_radio_IEEE80211hSupported(char* devName);
/*
 * wifi_set_radio_IEEE80211hEnabled
 *		This function enables or disables IEEE802.11h functions of the radio device.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	IEEE80211hEnabled
 *		[in] Indicates whether IEEE 802.11h functionality is enabled on this radio.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_radio_IEEE80211hEnabled(char* devName, unsigned int IEEE80211hEnabled);
/*
 * wifi_set_radio_RegulatoryDomain
 *		This function sets regulatory domain of the radio device.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	RegulatoryDomain
 *		[in] The 802.11d Regulatory Domain. First two octets are [ISO3166-1] two-character country code. The third octet is either " " (all environments), "O" (outside) or "I" (inside). Possible patterns:[A-Z][A-Z][ OI] .
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_radio_RegulatoryDomain(char* devName, char* RegulatoryDomain);
/*
 * wifi_get_BytesSent
 *		This function gets the total number of bytes transmitted out of the interface, including framing characters.
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Return the total number of bytes transmitted out of the interface.
 */
unsigned int wifi_get_BytesSent(char* devName);
/*
 * wifi_get_BytesReceived
 *		This function gets the total number of bytes received on the interface, including framing characters.
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Return the total number of bytes received on the interface.
 */
unsigned int wifi_get_BytesReceived(char* devName);
/*
 * wifi_get_PacketsSent
 *		This function gets the total number of packets transmitted out of the interface.
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Return the total number of packets transmitted out of the interface.
 */
unsigned int wifi_get_PacketsSent(char* devName);
/*
 * wifi_get_PacketsReceived
 *		This function gets the total number of packets received on the interface.
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Return the total number of packets received on the interface.
 */
unsigned int wifi_get_PacketsReceived(char* devName);
/*
 * wifi_get_ErrorsSent
 *		This function gets the total number of outbound packets that could not be transmitted because of errors.
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Return the total number of outbound packets that could not be transmitted because of errors.
 */
unsigned int wifi_get_ErrorsSent(char* devName);
/*
 * wifi_get_ErrorsReceived
 *		This function gets the total number of inbound packets that contained errors preventing them from being delivered to a higher-layer protocol.
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Return the total number of inbound packets that contained errors preventing them from being delivered to a higher-layer protocol.
 */
unsigned int wifi_get_ErrorsReceived(char* devName);
/*
 * wifi_get_DiscardPacketsSent
 *		This function gets the total number of outbound packets which were chosen to be discarded even though no errors had been detected to prevent their being transmitted. One possible reason for discarding such a packet could be to free up buffer space.
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Return the total number of outbound packets which were chosen to be discarded even though no errors had been detected to prevent their being transmitted. One possible reason for discarding such a packet could be to free up buffer space.
 */
unsigned int wifi_get_DiscardPacketsSent(char* devName);
/*
 * wifi_get_DiscardPacketsReceived
 *		This function gets the total number of inbound packets which were chosen to be discarded even though no errors had been detected to prevent their being delivered. One possible reason for discarding such a packet could be to free up buffer space.
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Return the total number of inbound packets which were chosen to be discarded even though no errors had been detected to prevent their being delivered. One possible reason for discarding such a packet could be to free up buffer space.
 */
unsigned int wifi_get_DiscardPacketsReceived(char* devName);
/*
 * wifi_get_BSSID
 *		This function gets the BSSID of the interface.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	bssid
 *		[out] the BSSID of the interface
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_get_BSSID(char* devName, char* bssid);
/*
 * wifi_set_bssid
 *		This function sets the BSSID of the interface.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	bssid
 *		[in] the BSSID of the interface
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_bssid(char* devName, char* bssid);
/*
 * wifi_get_MACAddress
 *		This function gets the MAC address of the interface.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	mac
 *		[out] the MAC address of the interface
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_get_MACAddress(char *devName, char *mac);
/*
 * wifi_set_ssid
 *		This function sets the ssid of the device.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	ssid
 *		[in] a new ssid for the device.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_ssid(char* devName, char* ssid);
/*
 * wifi_get_UnicastPacketsSent
 *		This function gets the total number of packets requested for transmission which were not addressed to a multicast or broadcast address at this layer, including those that were discarded or not sent.
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Return the total number of packets requested for transmission which were not addressed to a multicast or broadcast address at this layer, including those that were discarded or not sent.
 */
unsigned int wifi_get_UnicastPacketsSent(char* devName);
/*
 * wifi_get_UnicastPacketsReceived
 *		This function gets the total number of received packets, delivered by this layer to a higher layer, which were not addressed to a multicast or broadcast address at this layer.
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Return the total number of received packets, delivered by this layer to a higher layer, which were not addressed to a multicast or broadcast address at this layer.
 */
unsigned int wifi_get_UnicastPacketsReceived(char* devName);
/*
 * wifi_get_MulticastPacketsSent
 *		This function gets the total number of packets that higher-level protocols requested for transmission and which were addressed to a multicast address at this layer, including those that were discarded or not sent.
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Return the total number of packets that higher-level protocols requested for transmission and which were addressed to a multicast address at this layer, including those that were discarded or not sent.
 */
unsigned int wifi_get_MulticastPacketsSent(char* devName);
/*
 * wifi_get_MulticastPacketsReceived
 *		This function gets the total number of received packets, delivered by this layer to a higher layer, which were addressed to a multicast address at this layer.
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Return the total number of received packets, delivered by this layer to a higher layer, which were addressed to a multicast address at this layer.
 */
unsigned int wifi_get_MulticastPacketsReceived(char* devName);
/*
 * wifi_get_BroadcastPacketsSent
 *		This function gets the total number of packets that higher-level protocols requested for transmission and which were addressed to a broadcast address at this layer, including those that were discarded or not sent.
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Return the total number of packets that higher-level protocols requested for transmission and which were addressed to a broadcast address at this layer, including those that were discarded or not sent.
 */
unsigned int wifi_get_BroadcastPacketsSent(char* devName);
/*
 * wifi_get_BroadcastPacketsReceived
 *		This function gets the total number of received packets, delivered by this layer to a higher layer, which were addressed to a broadcast address at this layer.
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Return the total number of received packets, delivered by this layer to a higher layer, which were addressed to a broadcast address at this layer.
 */
unsigned int wifi_get_BroadcastPacketsReceived(char* devName);
/*
 * wifi_get_UnknownProtoPacketsReceived
 *		This function gets the total number of packets received via the interface which were discarded because of an unknown or unsupported protocol.
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Return the total number of packets received via the interface which were discarded because of an unknown or unsupported protocol.
 */
unsigned int wifi_get_UnknownProtoPacketsReceived(char* devName);
/*
 * wifi_set_SSIDAdvertisementEnabled
 *		This function enables or disables SSID Advertisement.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	SSIDAdvertisementEnabled
 *		[in] A value of true means that SSID Advertisement feature will be enabled.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_SSIDAdvertisementEnabled(char* devName, unsigned int SSIDAdvertisementEnabled);
/*
 * wifi_set_ShortRetryLimit
 *		This function sets the maximum number of retransmission for a packet.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	RetryLimit
 *		[in] The maximum number of retransmission for a packet. This corresponds to IEEE 802.11 parameter dot11ShortRetryLimit.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_ShortRetryLimit(char* devName, unsigned int RetryLimit);
/*
 * wifi_get_WMMSupported
 *		This function retrieves the capability whether supports WMM or not
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Nonzero if the interface support WMM; otherwise returns zero.
 */
unsigned int wifi_get_WMMSupported(char* devName);
/*
 * wifi_get_UAPSDSupported
 *		This function retrieves the capability whether supports UAPSD or not
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Nonzero if the interface support UAPSD; otherwise returns zero.
 */
unsigned int wifi_get_UAPSDSupported(char* devName);
/*
 * wifi_set_WMMEnable
 *		This function enables or disables 802.11n WMM feature.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	WMMEnable
 *		[in] A value of true means that WMM feature will be enabled.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
 int wifi_set_WMMEnable(char* devName, unsigned int WMMEnable);
/*
 * wifi_set_UAPSDEnable
 *		This function enables or disables 802.11n UAPSD (Unscheduled Automatic Power Save Delivery) feature. UAPSD is also known as WMM power save.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	WMMEnable
 *		[in] A value of true means that UAPSD feature will be enabled.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_UAPSDEnable(char* devName, unsigned int UAPSDEnable);
/*
 * wifi_get_associated_device_count
 *		This function gets the count of the devices currently associated with the access point.
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Return the count of the devices currently associated with the access point.
 */
unsigned int wifi_get_associated_device_count(char* devName);
/*
 * wifi_set_MaxAssociatedDevices
 *		This function sets the maximum number of devices that can simultaneously be connected to the access point.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	MaxAssociatedDevices
 *		[in] The maximum number of devices that can simultaneously be connected to the access point.A value of 0 means that there is no specific limit.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_MaxAssociatedDevices(char* devName, unsigned int MaxAssociatedDevices);
/*
 * wifi_set_IsolationEnable
 *		This function enables or disables device isolation.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	IsolationEnable
 *		[in] A value of true means that the devices connected to the Access Point are isolated from all other devices within the home network (as is typically the case for a Wireless Hotspot).
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_IsolationEnable(char* devName, unsigned int IsolationEnable);
/*
 * wifi_get_SecurityModesSupported
 *		This function gets the security modes this AccessPoint instance is capable of supporting.
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Return the the security modes this AccessPoint instance is capable of supporting.
 *	bit0-None, bit1-WEP-64, bit2-WEP-128, bit3-WPA-Personal, bit4- WPA2-Personal, bit5-WPA-WPA2-Personal, bit6-WPA-Enterprise, bit7-WPA2-Enterprise, bit8-WPA-WPA2-Enterprise
 */
unsigned int wifi_get_SecurityModesSupported(char* devName);
/*
 * wifi_set_security_off
 *		This function set the security mode of special interface to None.
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_security_off(char* devName);
/*
 * wifi_set_security_8021x
 *		This function set the security mode of special interface to 802.1x.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	server
 *		[in]  The IP Address of the RADIUS server used for WLAN security.
 *	port
 *		[in] The port number of the RADIUS server used for WLAN security. 
 *	secret
 *		[in] The secret used for handshaking with the RADIUS server.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_security_8021x(char* devName, char* server, unsigned int port, char* secret);
/*
 * wifi_set_security_wep_only
 *		This function set the security mode of special interface to WEP mode.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	WEPKeyType
 *		[in] Enumeration of WEPKEY_TYPE.
 *	WEPKey
 *		[in]  Pointer to four WEP keys.
 *	WEPKeyIndex
 *		[in] Specify which WEP key is used.
 *	customedWEPKey
 *		[in]  A customed WEP key and prior to WEPKey.
 *	authenticationMode
 *		[in] Enumeration of AUTHENTICATION_MODE.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_security_wep_only(char* devName, WEPKEY_TYPE WEPKeyType[4], char* WEPKey[4], unsigned int WEPKeyIndex, char* customedWEPKey, AUTHENTICATION_MODE authenticationMode);
/*
 * wifi_set_security_wep_8021x
 *		This function set the security mode of special interface to WEP+802.1x.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	server
 *		[in]  The IP Address of the RADIUS server used for WLAN security.
 *	port
 *		[in] The port number of the RADIUS server used for WLAN security. 
 *	secret
 *		[in] The secret used for handshaking with the RADIUS server.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_security_wep_8021x(char* devName, char* server, unsigned int port, char* secret);
/*
 * wifi_set_security_wpa_psk
 *		This function set the security mode of special interface to wpa personal pre-shared-key.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	wpa
 *		[in] Enumeration of WPA mode.
 *	wpaEncryptionModes
 *		[in]  Encryption mode used for WPA.
 *	wpa2EncryptionModes
 *		[in] Encryption mode used for WPA2.
 *	preSharedKey
 *		[in]  A literal PreSharedKey (PSK) expressed as a hexadecimal string.
 *	RekeyingInterval
 *		[in] The interval (expressed in seconds) in which the keys are re-generated.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_security_wpa_psk(char* devName, WPA_MODE wpa, WPA_ENCRYPTION_MODES wpaEncryptionModes, WPA_ENCRYPTION_MODES wpa2EncryptionModes, char* preSharedKey, unsigned int RekeyingInterval);
/*
 * wifi_set_security_wpa_eap
 *		This function set the security mode of special interface to wpa enterprise.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	wpa
 *		[in] Enumeration of WPA mode.
 *	wpaEncryptionModes
 *		[in]  Encryption mode used for WPA.
 *	wpa2EncryptionModes
 *		[in] Encryption mode used for WPA2.
 *	server
 *		[in]  The IP Address of the RADIUS server used for WLAN security.
 *	port
 *		[in] The port number of the RADIUS server used for WLAN security. 
 *	secret
 *		[in] The secret used for handshaking with the RADIUS server.
 *	RekeyingInterval
 *		[in] The interval (expressed in seconds) in which the keys are re-generated.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_security_wpa_eap(char* devName, WPA_MODE wpa, WPA_ENCRYPTION_MODES wpaEncryptionModes, WPA_ENCRYPTION_MODES wpa2EncryptionModes, char* server, unsigned int port, char* secret, unsigned int RekeyingInterval);
/*
 * wifi_set_acl
 *		This function enables or disables MAC Address Control function of the AP.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	enabled
 *		[in] Indicates whether MAC Address Control is enabled or not on this interface.
 *	macList
 *		[in] MAC addresses list.
 *	comment
 *		[in] Comment for MAC.
 *	policyDeny
 *		[in]  Indicates the acl policy is deny or allow.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_acl(char* devName, unsigned int enabled, char* macList, char* comment, unsigned int policyDeny);
/*
 * wifi_device_is_exist
 *		This function checks the device is exist or not.
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	Nonzero if the device exists; otherwise zero.
 */
unsigned int wifi_device_is_exist(char* devName);
/*
 * wifi_get_default_ssid
 *		This function get the default SSID name for given interface.
 * Parameters
 *	radioDevName
 *		[in] Name of radio interface.
 *	devName
 *		[in] Name of virtual access point interface.
 *	ssid
 *		[out]  The default ssid name returned.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_get_default_ssid(char* radioDevName, char *devName, char *ssid);
/*
 * wifi_get_default_key
 *		This function get the default security key (WEPKey and PreSharedKey) for given interface.
 * Parameters
 *	devName
 *		[in] Name of virtual access point interface.
 *	isWEPKey128
 *		[in]  Specify the default WEPKey is 128bits or 64bits.
 *	sizePreSharedKey
 *		[in]  The default PreSharedKey size in bytes.
 *	defaultWEPKey
 *		[out]  The default WEPKey returned.
 *	defaultPreSharedKey
 *		[out]  The default PreSharedKey returned.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_get_default_key(char *devName, unsigned int isWEPKey128, unsigned int sizePreSharedKey, char *defaultWEPKey, char *defaultPreSharedKey);
/*
 * wifi_get_wps_ConfigMethodsSupported
 *		This function gets the supported wps config methods by given interface.
 * Parameters
 *	devName
 *		[in] Name of virtual access point interface.
 * Return Values
 *	bit0-USBFlashDrive, bit1-Ethernet, bit2-Label, bit3-Display, bit4-ExternalNFCToken, bit5-IntegratedNFCToken, bit6-NFCInterface, bit7-PushButton, bit8-PIN.
 */
unsigned int wifi_get_wps_ConfigMethodsSupported(char* devName);
/*
 * wifi_set_BeaconInterval
 *		This function sets the Beacon interval for given interface.
 * Parameters
 *	devName
 *		[in] Name of virtual access point interface.
 *	beaconInterval
 *		[in] The Beacon Interval field represents the number of time units (TUs) between target beacon transmission times (TBTTs).
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_BeaconInterval(char* devName, unsigned int beaconInterval);
/*
 * wifi_set_DTIMInterval
 *		This function sets the DTIM interval for given interface.
 * Parameters
 *	devName
 *		[in] Name of virtual access point interface.
 *	DTIMInterval
 *		[in] The DTIM Interval field represents the number of time units (TUs) between delivery traffic indication messages.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_DTIMInterval(char* devName, unsigned int DTIMInterval);
/*
 * wifi_set_CTSProtectMode
 *		This function sets the RTS/CTS protection mode for given interface.
 * Parameters
 *	devName
 *		[in] Name of virtual access point interface.
 *	CTSProtectMode
 *		[in] Indicates RTS/CTS protection is used .
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_CTSProtectMode(char* devName, unsigned int CTSProtectMode);
/*
 * wifi_set_FragmentationThreshold
 *		This function sets the dot11FragmentationThreshold for given interface.
 * Parameters
 *	devName
 *		[in] Name of virtual access point interface.
 *	fragmentationThreshold
 *		[in] dot11FragmentationThreshold.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_FragmentationThreshold(char* devName, unsigned int fragmentationThreshold);
/*
 * wifi_set_RTSThreshold
 *		This function sets the dot11RTSThreshold for given interface.
 * Parameters
 *	devName
 *		[in] Name of virtual access point interface.
 *	RTSThreshold
 *		[in] dot11RTSThreshold.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_RTSThreshold(char* devName, unsigned int RTSThreshold);
/*
 * wifi_set_MSDULimit
 *		This function sets the AMSDU limatation for given interface.
 * Parameters
 *	devName
 *		[in] Name of virtual access point interface.
 *	MSDULimit
 *		[in] AMSDU limatation.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_MSDULimit(char* devName, unsigned int MSDULimit);
/*
 * wifi_set_MPDULimit
 *		This function sets the AMPDU limatation for given interface.
 * Parameters
 *	devName
 *		[in] Name of virtual access point interface.
 *	MPDULimit
 *		[in] AMPDU limatation.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_MPDULimit(char* devName, unsigned int MPDULimit);
/*
 * wifi_set_wds
 *		This function enable/disable the wds function for given interface.
 * Parameters
 *	devName
 *		[in] Name of virtual access point interface.
 *	wds
 *		[in] 0 - enable, other - disable.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_wds(char* devName, unsigned int wds);
int wifi_commit(char* devName);
int wifi_set_PreambleType(char* devName, unsigned int preambleType);
int wifi_set_IAPPEnable(char* devName, unsigned int enable);
int wifi_set_SpaceTimeBlockCoding(char* devName, unsigned int STBC);

/*
**************************************************************************
*	TR-098 
**************************************************************************
*/

/*
 * wifi_set_radio_MaxBitRate
 *		This function sets the maximum upstream and downstream bit rate.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	maxBitRate
 *		[in] The maximum upstream and downstream bit rate available to this connection in Mbps. Either Auto, or the largest of the OperationalDataTransmitRates values.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_radio_MaxBitRate(char* devName, char* maxBitRate);
/*
 * wifi_set_radio_BasicDataTransmitRates
 *		This function sets the Maximum access point data transmit rates in Mbps for unicast, multicast and broadcast frames.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	basicDataTransmitRates
 *		[in] The Maximum access point data transmit rates in Mbps for unicast, multicast and broadcast frames.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_radio_BasicDataTransmitRates(char* devName, char* basicDataTransmitRates);
/*
 * wifi_set_radio_OperationalDataTransmitRates
 *		This function sets the Maximum access point data transmit rates in Mbps for unicast frames
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	operationalDataTransmitRates
 *		[in] The Maximum access point data transmit rates in Mbps for unicast frames
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_radio_OperationalDataTransmitRates(char* devName, char* operationalDataTransmitRates);
/*
 * wifi_get_radio_PossibleDataTransmitRates
 *		This function gets the Possible Data TransmitRates of the interface.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	bitrateList
 *		[out] Comma-separated list (maximum length 256) of strings. Data transmit rates for unicast frames at which the access point will permit a station to connect (a subset of OperationalDataTransmitRates).
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_get_radio_PossibleDataTransmitRates(char* devName, char* bitrateList);
/*
 * wifi_get_wmm_parameters
 *		This function gets the wmm parameters of the interface.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	isAP
 *		[in]  Indicates whether this interface is AP or Client.
 *	classNumber
 *		[in] Enumeration of WMM_CLASS_NUMBER.
 *	aifs
 *		[out]  Arbitration Inter Frame Spacing (Number). This is the number of time slots in the arbitration interframe space.
 *	cwmin
 *		[out]  Contention Window (Minimum). 
 *	cwmax
 *		[out]  Contention Window (Maximum). 
 *	txoplimit
 *		[out]  Transmit Opportunity, in microseconds.
 *	noackpolicy
 *		[out]  Ack Policy, where true=Do Not Acknowledge and false=Acknowledge.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_get_wmm_parameters(char* devName, WMM_CLASS_NUMBER classNumber, unsigned int* aifs, unsigned int* cwmin, unsigned int* cwmax, unsigned int* txoplimit, unsigned int* noackpolicy, unsigned int isAP);
/*
 * wifi_set_wmm_parameters
 *		This function sets the wmm parameters of the interface.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	isAP
 *		[in]  Indicates whether this interface is AP or Client.
 *	classNumber
 *		[in] Enumeration of WMM_CLASS_NUMBER.
 *	aifs
 *		[in]  Arbitration Inter Frame Spacing (Number). This is the number of time slots in the arbitration interframe space.
 *	cwmin
 *		[in]  Contention Window (Minimum). 
 *	cwmax
 *		[in]  Contention Window (Maximum). 
 *	txoplimit
 *		[in]  Transmit Opportunity, in microseconds.
 *	noackpolicy
 *		[in]  Ack Policy, where true=Do Not Acknowledge and false=Acknowledge.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_set_wmm_parameters(char* devName, WMM_CLASS_NUMBER classNumber, unsigned int aifs, unsigned int cwmin, unsigned int cwmax, unsigned int txoplimit, unsigned int noackpolicy, unsigned int isAP);
/*
 * wifi_get_associated_device_mac_address
 *		This function gets the mac address of the device associated with the interface.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	index
 *		[in]  Index of the device associated with the interface.
 *	macAddr
 *		[out]  The mac address of the device associated with the interface.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_get_associated_device_mac_address(char* devName, unsigned int index, char* macAddr);
/*
 * wifi_get_associated_device_authentication_state
 *		This function gets the authentication state of the device associated with the interface.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	macAddr
 *		[in]  The mac address of the device associated with the interface.
 * Return Values
 *	Whether an associated device has authenticated (true) or not (false).
 */
unsigned int wifi_get_associated_device_authentication_state(char* devName, char* macAddr);
/*
 * wifi_get_associated_device_LastDataDownlinkRate
 *		This function gets the data transmit rate in kbps that was most recently used for transmission from the access point to the associated device.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	macAddr
 *		[in]  The mac address of the device associated with the interface.
 * Return Values
 *	The data transmit rate in kbps that was most recently used for transmission from the access point to the associated device.[1000:600000].
 */
//wifi_get_associated_device_LastDataDownlinkRate : return value:[1000:600000]
unsigned int wifi_get_associated_device_LastDataDownlinkRate(char* devName, char* macAddr);
/*
 * wifi_get_associated_device_LastDataUplinkRate
 *		This function gets the data transmit rate in kbps that was most recently used for transmission from the associated device to the access point.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	macAddr
 *		[in]  The mac address of the device associated with the interface.
 * Return Values
 *	The data transmit rate in kbps that was most recently used for transmission from the associated device to the access point.[1000:600000].
 */
unsigned int wifi_get_associated_device_LastDataUplinkRate(char* devName, char* macAddr);
/*
 * wifi_get_associated_device_SignalStrength
 *		This function gets the signal strength of the associated device.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	macAddr
 *		[in]  The mac address of the device associated with the interface.
 * Return Values
 *	An indicator of radio signal strength of the uplink from the associated device to the access point, measured in dBm, as an average of the last 100 packets received from the device.[-200:0].
 */
int wifi_get_associated_device_SignalStrength(char* devName, char* macAddr);
/*
 * wifi_get_associated_device_Retransmissions
 *		This function gets the number of packets that had to be re-transmitted of the associated device.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	macAddr
 *		[in]  The mac address of the device associated with the interface.
 * Return Values
 *	 [0:100] The number of packets that had to be re-transmitted, from the last 100 packets sent to the associated device. Multiple re-transmissions of the same packet count as one.
 */
unsigned int wifi_get_associated_device_Retransmissions(char* devName, char* macAddr);
/*
 * wifi_get_associated_device_Active
 *		This function gets the active state of the associated device.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	macAddr
 *		[in]  The mac address of the device associated with the interface.
 * Return Values
 *	 Whether or not this node is currently present in the WiFi AccessPoint network.
 */
unsigned int wifi_get_associated_device_Active(char* devName, char* macAddr);
/*
 * wifi_get_associated_device_LinkQuality
 *		This function gets the Link Quality of the associated device.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	macAddr
 *		[in]  The mac address of the device associated with the interface.
 * Return Values
 *	 Signal quality indication on a scale of 0 to 100. This parameter is similar to "Link Quality" of 'iwconfig' Linux utility.
 */
unsigned int wifi_get_associated_device_LinkQuality(char* devName, char* macAddr);
/*
 * wifi_get_associated_device_last_data_transmit_rate
 *		This function gets the data transmit rate that was most recently used for a station with a specified MAC address.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	macAddr
 *		[in]  The mac address of the device associated with the interface.
 *	lastDataTransmitRate
 *		[out]  The data transmit rate that was most recently used for a station with a specified MAC address. (string(4) in Mbps)
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_get_associated_device_last_data_transmit_rate(char* devName, char* macAddr, char* lastDataTransmitRate);
/*
 * wifi_get_wps_uuid
 *		This function gets the UUID of the device.
 * Parameters
 *	devName
 *		[in] Name of interface.
 *	uuid
 *		[out]  The UUID of the device
 *	bufLen
 *		[in] size of received buffer.
 * Return Values
 *	Zero if the function succeeds; otherwise nonzero.
 */
int wifi_get_wps_uuid(char* devName, char* uuid, unsigned int bufLen);
/*
 * wifi_get_wps_version
 *		This function gets the WPS version of the device.
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	The Wi-Fi Protected Setup version supported by the device.
 */
unsigned int wifi_get_wps_version(char* devName);

int wifi_config_wps(char* devName, unsigned int eap_server, unsigned int wps_state, char* device_name, unsigned int ap_setup_locked, unsigned int ap_pin, unsigned int enabled);
int wifi_set_wps_ap_pin_static(char* devName, unsigned int devicePassword, unsigned int timeout);
unsigned int wifi_set_wps_ap_pin_random(char* devName, unsigned int timeout);
int wifi_set_wps_push_button(char* devName);
int wifi_set_wps_pin(char* devName, unsigned int devicePassword, unsigned int timeout);
/*
 * wifi_get_device_index
 *		This function retrieves the device index from devName.
 * Parameters
 *	devName
 *		[in] Name of interface.
 * Return Values
 *	If the function succeeds; it returns the device index based zero, otherwise return -1.
 */
int wifi_get_device_index(char* devName);

unsigned int wifi_get_wps_ap_pin(char* devName);
///////////////////////////////////////////////////////////////////////////////
int wifi_get_wps_registrar_uuid(char* devName, unsigned int index, char* uuid);
int wifi_get_wps_registrar_device_name(char* devName, unsigned int index, char* registrarDeviceName);
unsigned int wifi_close_all_virtual_devices(char* wlanDevName);
void wifi_open_all_virtual_devices(char* wlanDevName, unsigned int flag);
int wifi_get_WEPEncryptionLevel(char* devName, char* supportedKeySizes);
int wifi_set_InsecureOOBAccessEnabled(char* devName, unsigned int insecureOOBAccessEnabled);
int wifi_set_BeaconAdvertisementEnabled(char* devName, unsigned int beaconAdvertisementEnabled);
int wifi_set_AutoRateFallBackEnabled(char* devName, unsigned int autoRateFallBackEnabled);
int wifi_set_LocationDescription(char* devName, char* locationDescription);
unsigned int wifi_get_TotalPSKFailures(char* devName);
unsigned int wifi_get_TotalIntegrityFailures(char* devName);
int wifi_set_DistanceFromRoot(char* devName, unsigned int distanceFromRoot);
int wifi_set_PeerBSSID(char* devName, char* peerBSSID);
int wifi_set_AuthenticationServiceMode(char* devName, char* authenticationServiceMode);
int wifi_get_associated_device_ip_address(char* devName, char* macAddr, char* ipAddr);
int wifi_get_associated_device_last_requested_unicast_cipher(char* devName, char* macAddr, char* lastRequestedUnicastCipher);
int wifi_get_associated_device_last_requested_multicast_cipher(char* devName, char* macAddr, char* lastRequestedMulticastCipher);
int wifi_get_associated_device_last_pairwise_master_key_id(char* devName, char* macAddr, char* lastPMKId);
///////////////////////////////////////////////////////////////////////////////

#endif
