/**************************************************************************
 *	
 *	        wifi_adapter_driver.c
 *
 **************************************************************************/
#include "wifi_adapter_driver.h"

#ifdef WIFI_DRIVER_MADWIFI
#include "wifi_adapter_driver_madwifi.c"
#elif defined WIFI_DRIVER_REALTEK
#include "wifi_adapter_driver_realtek.c"
#elif defined WIFI_DRIVER_MARVELL
#include "wifi_adapter_driver_marvell.c"
#elif defined WIFI_DRIVER_BROADCOM
#include "wifi_adapter_driver_broadcom.c"
#else
#error "No wifi driver defined."
#endif


