/*./auto/tr69_func_WIFIAccessPointSecurityObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"
#include "wifi_data_openwrt.h"


static time_t last_access[MAX_NUMBER_OF_ENTRIES_ACCESSPOINT] = {0};
extern unsigned int object_has_been_created;
/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_WIFIAccessPointSecurityObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.WiFi.AccessPoint.i.Security
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_WIFIAccessPointSecurityObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_WIFIAccessPointSecurityObject_value(tsl_char_t *p_oid_name, _WIFIAccessPointSecurityObject *p_cur_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;
	tr69_oid_stack_id iidThisStack = EMPTY_INSTANCE_ID_STACK;
	tr69_oid_stack_id iidTempStack = EMPTY_INSTANCE_ID_STACK;
	int instance_number = 0;
	PDEVICE_WIFI pCurDevData = NULL;
	WIFIAccessPointSecurityObject* pAPSecurity[MAX_NUMBER_OF_ENTRIES_ACCESSPOINT] = {0};
	tr69_oid_stack_id iidStack_AP[MAX_NUMBER_OF_ENTRIES_ACCESSPOINT] = {EMPTY_INSTANCE_ID_STACK};
	WIFIAccessPointObject* pAP[MAX_NUMBER_OF_ENTRIES_ACCESSPOINT] = {0};
	tr69_oid_stack_id iidStack_SSID[MAX_NUMBER_OF_ENTRIES_SSID] = {EMPTY_INSTANCE_ID_STACK};
	WIFISSIDObject* pSSID[MAX_NUMBER_OF_ENTRIES_SSID] = {0};
	int i;
#ifndef SUPPORTED_TR181
	return TR69_RT_SUCCESS_VALUE_UNCHANGED;
#endif

	do
	{
		if(object_has_been_created<MAX_NUMBER_OF_ENTRIES_ACCESSPOINT){
			break;
		}
		
		oid_to_iidstack(p_oid_name, &iidThisStack);
		instance_number = iidThisStack.instance[iidThisStack.currentDepth-1];
		if(DIFF_TIME(last_access[instance_number-1])<MIN_OBJ_GET_INTERVAL){
			break;
		}
		last_access[instance_number-1] = CURRENT_TIME;

		ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);
		//AP
		i = 0;
		memset(&iidTempStack, 0, sizeof(iidTempStack));
		while (i<MAX_NUMBER_OF_ENTRIES_ACCESSPOINT&&tr69_get_next_sub_obj_by_oid(TR69_OID_WIFI_ACCESS_POINT,
			&iidThisStack, &iidTempStack, (void **)&pAP[i]) == TSL_RV_SUC){
			iidStack_AP[i] = iidTempStack;
			i++;
		}
		if (0==i){
			ctllog_error("WIFIAccessPointObject not found!\n");
			break;
		}
		//SSID
		i = 0;
		memset(&iidTempStack, 0, sizeof(iidTempStack));
		while (i<MAX_NUMBER_OF_ENTRIES_SSID&&tr69_get_next_sub_obj_by_oid(TR69_OID_WIFISSID,
			&iidThisStack, &iidTempStack, (void **)&pSSID[i]) == TSL_RV_SUC){
			iidStack_SSID[i] = iidTempStack;
			i++;
		}
		if (0==i){
			ctllog_error("WIFISSIDObject not found!\n");
			break;
		}
		pCurDevData = calloc(1, sizeof(DEVICE_WIFI));
		if(NULL==pCurDevData){
			ctllog_error("calloc failed for DEVICE_WIFI!\n");
			break;
		}
		//APSecurity
		pAPSecurity[instance_number-1] = p_cur_data;
		tr181_wifi_data_load(pCurDevData, 
			NULL, 
			NULL, 
			pSSID, 
			pAP, 
			pAPSecurity);
		if(0==tr181WiFiObjectGet(p_oid_name, pCurDevData)){
			tr181_wifi_data_store(pCurDevData, 
				NULL, 
				NULL, 
				NULL, 
				NULL, 
				pAPSecurity,
				NULL);
		
			rv = TR69_RT_SUCCESS_VALUE_CHANGED;
		}
	}while(0);

	for(i=0; i<MAX_NUMBER_OF_ENTRIES_ACCESSPOINT; i++){
		if(NULL!=pAP[i]){
			tr69_free_obj_by_oid(TR69_OID_WIFI_ACCESS_POINT, &iidStack_AP[i], (void **) &pAP[i]);
		}
	}

	for(i=0; i<MAX_NUMBER_OF_ENTRIES_SSID; i++){
		if(NULL!=pSSID[i]){
			tr69_free_obj_by_oid(TR69_OID_WIFISSID, &iidStack_SSID[i], (void **) &pSSID[i]);
		}
	}

	if(NULL!=pCurDevData){
		free(pCurDevData);
		pCurDevData = NULL;
	}

	return rv;
}


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_set_WIFIAccessPointSecurityObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.WiFi.AccessPoint.i.Security
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_WIFIAccessPointSecurityObject_t *p_cur_data
 *	        st_WIFIAccessPointSecurityObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_WIFIAccessPointSecurityObject_value(tsl_char_t *p_oid_name, _WIFIAccessPointSecurityObject *p_cur_data, _WIFIAccessPointSecurityObject *p_new_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;
	tr69_oid_stack_id iidThisStack = EMPTY_INSTANCE_ID_STACK;
	tr69_oid_stack_id iidTempStack = EMPTY_INSTANCE_ID_STACK;
	PDEVICE_WIFI pCurDevData = NULL;
	PDEVICE_WIFI pNewDevData = NULL;
	WIFIAccessPointSecurityObject* pAPSecurity[MAX_NUMBER_OF_ENTRIES_ACCESSPOINT] = {0};
	tr69_oid_stack_id iidStack_SSID[MAX_NUMBER_OF_ENTRIES_SSID] = {EMPTY_INSTANCE_ID_STACK};
	WIFISSIDObject* pSSID[MAX_NUMBER_OF_ENTRIES_SSID] = {0};
	tr69_oid_stack_id iidStack_AP[MAX_NUMBER_OF_ENTRIES_ACCESSPOINT] = {EMPTY_INSTANCE_ID_STACK};
	WIFIAccessPointObject* pAP[MAX_NUMBER_OF_ENTRIES_ACCESSPOINT] = {0};
	int i;
	int instance_number = 0;

#ifndef SUPPORTED_TR181
	return TR69_RT_SUCCESS_VALUE_UNCHANGED;
#endif
	do{
		ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);
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
		oid_to_iidstack(p_oid_name, &iidThisStack);
		instance_number = iidThisStack.instance[iidThisStack.currentDepth-1];
		
		if(object_has_been_created<MAX_NUMBER_OF_ENTRIES_ACCESSPOINT){
			rv = TR69_RT_SUCCESS_VALUE_CHANGED;
			break;
		}
		
		//SSID
		i = 0;
		memset(&iidTempStack, 0, sizeof(iidTempStack));
		while (i<MAX_NUMBER_OF_ENTRIES_SSID&&tr69_get_next_sub_obj_by_oid(TR69_OID_WIFISSID,
			&iidThisStack, &iidTempStack, (void **)&pSSID[i]) == TSL_RV_SUC){
			iidStack_SSID[i] = iidTempStack;
			i++;
		}
		if (0==i){
			ctllog_error("WIFISSIDObject not found!\n");
			break;
		}
		//AP
		i = 0;
		memset(&iidTempStack, 0, sizeof(iidTempStack));
		while (i<MAX_NUMBER_OF_ENTRIES_ACCESSPOINT&&tr69_get_next_sub_obj_by_oid(TR69_OID_WIFI_ACCESS_POINT,
			&iidThisStack, &iidTempStack, (void **)&pAP[i]) == TSL_RV_SUC){
			iidStack_AP[i] = iidTempStack;
			i++;
		}
		if (0==i){
			ctllog_error("WIFIAccessPointObject not found!\n");
			break;
		}

		pCurDevData = calloc(1, sizeof(DEVICE_WIFI));
		if(NULL==pCurDevData){
			ctllog_error("calloc failed for DEVICE_WIFI!\n");
			break;
		}
		pNewDevData = calloc(1, sizeof(DEVICE_WIFI));
		if(NULL==pNewDevData){
			ctllog_error("calloc failed for DEVICE_WIFI!\n");
			break;
		}
		//AccessPointSecurity
		pAPSecurity[instance_number-1] = p_cur_data;
		tr181_wifi_data_load(pCurDevData, 
			NULL, 
			NULL, 
			pSSID, 
			pAP, 
			pAPSecurity);
		pAPSecurity[instance_number-1] = p_new_data;
		tr181_wifi_data_load(pNewDevData, 
			NULL, 
			NULL, 
			pSSID, 
			pAP, 
			pAPSecurity);

		if(0==tr181WiFiObjectModify(p_oid_name, pCurDevData, pNewDevData)){
			tr181_wifi_data_store(pNewDevData, 
				NULL, 
				NULL, 
				NULL, 
				NULL, 
				pAPSecurity,
				NULL);

			 rv = TR69_RT_SUCCESS_VALUE_CHANGED;
		}
	}while(0);

	for(i=0; i<MAX_NUMBER_OF_ENTRIES_SSID; i++){
		if(NULL!=pSSID[i]){
			tr69_free_obj_by_oid(TR69_OID_WIFISSID, &iidStack_SSID[i], (void **) &pSSID[i]);
		}
	}

	for(i=0; i<MAX_NUMBER_OF_ENTRIES_ACCESSPOINT; i++){
		if(NULL!=pAP[i]){
			tr69_free_obj_by_oid(TR69_OID_WIFI_ACCESS_POINT, &iidStack_AP[i], (void **) &pAP[i]);
		}
	}

	if(NULL!=pCurDevData){
		free(pCurDevData);
		pCurDevData = NULL;
	}
	
	if(NULL!=pNewDevData){
		free(pNewDevData);
		pNewDevData = NULL;
	}
	
	if(TR69_RT_SUCCESS_VALUE_CHANGED==rv){
		last_access[instance_number-1] = 0;
	}
	
	return rv;
}

