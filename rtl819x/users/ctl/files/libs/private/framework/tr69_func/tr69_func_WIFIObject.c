/*./auto/tr69_func_WIFIObject.c*/
#include <signal.h>

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"
#include "wifi_data_openwrt.h"
#include "libtr69_func.h"

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_WIFIObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.WiFi
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_WIFIObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_WIFIObject_value(tsl_char_t *p_oid_name, _WIFIObject *p_cur_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;
	PDEVICE_WIFI pCurDevData = NULL;
#ifndef SUPPORTED_TR181
	return TR69_RT_SUCCESS_VALUE_UNCHANGED;
#endif

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);
	do{
		//WiFi
		pCurDevData = calloc(1, sizeof(DEVICE_WIFI));
		if(NULL==pCurDevData){
			ctllog_error("calloc failed for DEVICE_WIFI!\n");
			break;
		}
		tr181_wifi_data_load(pCurDevData, 
			p_cur_data, 
			NULL, 
			NULL, 
			NULL, 
			NULL);
		if(0==tr181WiFiObjectGet(p_oid_name, pCurDevData)){
			tr181_wifi_data_store(pCurDevData, 
				p_cur_data, 
				NULL, 
				NULL, 
				NULL, 
				NULL, 
				NULL);
		
			rv = TR69_RT_SUCCESS_VALUE_CHANGED;
		}
	}while(0);

	if(NULL!=pCurDevData){
		free(pCurDevData);
		pCurDevData = NULL;
	}

	return rv;
}

static void* routine_thread(void* param)
{
	sleep(1);
	ctllog_error("----> before tr69_restore_default_node.\n");
	tr69_restore_default_node("Device.WiFi");
	ctllog_error("----> after tr69_restore_default_node.\n");
	return NULL;
}

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_set_WIFIObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.WiFi
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_WIFIObject_t *p_cur_data
 *	        st_WIFIObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_WIFIObject_value(tsl_char_t *p_oid_name, _WIFIObject *p_cur_data, _WIFIObject *p_new_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;
	pthread_t handle_thread = 0;
#ifndef SUPPORTED_TR181
	return TR69_RT_SUCCESS_VALUE_UNCHANGED;
#endif
	ctllog_error("************************************************************************\n");
	ctllog_error("----> enter %s.\n", __func__);
	do{
		if(NULL==p_cur_data&&NULL==p_new_data){//error
			break;
		}
		if(NULL!=p_cur_data&&NULL==p_new_data){//destroy object
			break;
		}
		if(NULL==p_cur_data&&NULL!=p_new_data){//create object
			break;
		}
		//modify object 
		if(p_new_data->X_ACTIONTEC_COM_WiFiRestoreDefault){
			p_new_data->X_ACTIONTEC_COM_WiFiRestoreDefault = 0;
			rv = TR69_RT_SUCCESS_VALUE_CHANGED;
			pthread_create(&handle_thread, NULL, routine_thread, NULL);
		}
	}while(0);
	ctllog_error("----> leave %s.\n", __func__);

	return rv;
}

