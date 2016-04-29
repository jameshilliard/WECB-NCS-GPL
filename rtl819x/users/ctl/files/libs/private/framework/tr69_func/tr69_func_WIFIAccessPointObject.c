/*./auto/tr69_func_WIFIAccessPointObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"
#include "wifi_data_openwrt.h"


static time_t last_access[MAX_NUMBER_OF_ENTRIES_ACCESSPOINT] = {0};
unsigned int object_has_been_created = 0;
/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_WIFIAccessPointObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.WiFi.AccessPoint
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_WIFIAccessPointObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_WIFIAccessPointObject_value(tsl_char_t *p_oid_name, _WIFIAccessPointObject *p_cur_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;
	tr69_oid_stack_id iidThisStack = EMPTY_INSTANCE_ID_STACK;
	tr69_oid_stack_id iidTempStack = EMPTY_INSTANCE_ID_STACK;
	int instance_number = 0;
	PDEVICE_WIFI pCurDevData = NULL;
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
		//AP
		pAP[instance_number-1] = p_cur_data;
		tr181_wifi_data_load(pCurDevData, 
			NULL, 
			NULL, 
			pSSID, 
			pAP, 
			NULL);
		if(0==tr181WiFiObjectGet(p_oid_name, pCurDevData)){
			tr181_wifi_data_store(pCurDevData, 
				NULL, 
				NULL, 
				NULL, 
				pAP, 
				NULL, 
				NULL);
		
			rv = TR69_RT_SUCCESS_VALUE_CHANGED;
		}
	}while(0);

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
 *	        tf69_func_set_WIFIAccessPointObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.WiFi.AccessPoint
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_WIFIAccessPointObject_t *p_cur_data
 *	        st_WIFIAccessPointObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_WIFIAccessPointObject_value(tsl_char_t *p_oid_name, _WIFIAccessPointObject *p_cur_data, _WIFIAccessPointObject *p_new_data)
{
		tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;
		tr69_oid_stack_id iidStack_Radio[MAX_NUMBER_OF_ENTRIES_RADIO] = {EMPTY_INSTANCE_ID_STACK};
		WIFIRadioObject* pRadio[MAX_NUMBER_OF_ENTRIES_RADIO] = {0};
		tr69_oid_stack_id iidStack_SSID[MAX_NUMBER_OF_ENTRIES_SSID] = {EMPTY_INSTANCE_ID_STACK};
		WIFISSIDObject* pSSID[MAX_NUMBER_OF_ENTRIES_SSID] = {0};
		WIFIAccessPointObject* pAP[MAX_NUMBER_OF_ENTRIES_ACCESSPOINT] = {0};
		tr69_oid_stack_id iidStack_APSecurity[MAX_NUMBER_OF_ENTRIES_ACCESSPOINT] = {EMPTY_INSTANCE_ID_STACK};
		WIFIAccessPointSecurityObject* pAPSecurity[MAX_NUMBER_OF_ENTRIES_ACCESSPOINT] = {0};
		tr69_oid_stack_id iidThisStack = EMPTY_INSTANCE_ID_STACK;
		tr69_oid_stack_id iidTempStack = EMPTY_INSTANCE_ID_STACK;
		//tr69_oid_stack_id iidAPStack = EMPTY_INSTANCE_ID_STACK;
		PDEVICE_WIFI pCurDevData = NULL;
		PDEVICE_WIFI pNewDevData = NULL;
		int i;
		int instance_number = 0;
		int instance_number2 = 0;
		unsigned int enabled = 0;
		char name[32] = {0};
#ifndef SUPPORTED_TR181
		return TR69_RT_SUCCESS_VALUE_UNCHANGED;
#endif
		ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);
		do{
			if(NULL==p_cur_data&&NULL==p_new_data){//error
				break;
			}
			oid_to_iidstack(p_oid_name, &iidThisStack);
			instance_number = iidThisStack.instance[iidThisStack.currentDepth-1];
			//Radio
			i = 0;
			memset(&iidTempStack, 0, sizeof(iidTempStack));
			while (i<MAX_NUMBER_OF_ENTRIES_RADIO&&tr69_get_next_sub_obj_by_oid(TR69_OID_WIFI_RADIO,
				&iidThisStack, &iidTempStack, (void **)&pRadio[i]) == TSL_RV_SUC){
				iidStack_Radio[i] = iidTempStack;
				i++;
			}
			if (0==i){
				ctllog_error("WIFIRadioObject not found!\n");
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
			//APSecurity
			memset(&iidTempStack, 0, sizeof(iidTempStack));
			if(TSL_RV_SUC!=tr69_get_next_sub_obj_by_oid(TR69_OID_WIFI_ACCESS_POINT_SECURITY,
				&iidThisStack, &iidTempStack, (void **)&pAPSecurity[instance_number-1])){
				ctllog_error("WIFIAccessPointSecurityObject not found!\n");
				break;
			}
			iidStack_APSecurity[instance_number-1] = iidTempStack;

			if(NULL!=p_cur_data&&NULL==p_new_data){//destroy object
				//AP
				pCurDevData = calloc(1, sizeof(DEVICE_WIFI));
				if(NULL==pCurDevData){
					ctllog_error("calloc failed for DEVICE_WIFI!\n");
					break;
				}
				
				pAP[instance_number-1] = p_cur_data;
				tr181_wifi_data_load(pCurDevData, 
					NULL, 
					pRadio, 
					pSSID, 
					pAP, 
					pAPSecurity);
				if(0==tr181WiFiObjectDestroy(p_oid_name, pCurDevData)){
					rv = TR69_RT_SUCCESS_VALUE_CHANGED;
					object_has_been_created = 0;
				}
				break;
			}
			
			if(NULL==p_cur_data&&NULL!=p_new_data){//create object
				//AP
				pNewDevData = calloc(1, sizeof(DEVICE_WIFI));
				if(NULL==pNewDevData){
					ctllog_error("calloc failed for DEVICE_WIFI!\n");
					break;
				}
				
				pAP[instance_number-1] = p_new_data;
				tr181_wifi_data_load(pNewDevData, 
					NULL, 
					pRadio, 
					pSSID, 
					pAP, 
					pAPSecurity);
				
				if(0==tr181WiFiObjectCreate(p_oid_name, pNewDevData)){
					tr181_wifi_data_store(pNewDevData, 
						NULL, 
						NULL, 
						pSSID, 
						pAP, 
						pAPSecurity,
						NULL);
					tr69_set_obj_by_oid(TR69_OID_WIFI_ACCESS_POINT_SECURITY, pAPSecurity[instance_number-1], &iidStack_APSecurity[instance_number-1]);
					oid_to_iidstack(p_new_data->SSIDReference, &iidTempStack);
					instance_number2 = iidTempStack.instance[iidTempStack.currentDepth-1];
					tr69_set_obj_by_oid(TR69_OID_WIFISSID, pSSID[instance_number2-1], &iidStack_SSID[instance_number2-1]);
				
					rv = TR69_RT_SUCCESS_VALUE_CHANGED;
				}
				object_has_been_created ++;
				if(MAX_NUMBER_OF_ENTRIES_ACCESSPOINT==object_has_been_created){			
					ctllog_error("************************************************************************\n");
				}
				break;
			}

			//modify object 
			//AP
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
			
			pAP[instance_number-1] = p_cur_data;
			tr181_wifi_data_load(pCurDevData, 
				NULL, 
				pRadio, 
				pSSID, 
				pAP, 
				NULL);
			pAP[instance_number-1] = p_new_data;
			tr181_wifi_data_load(pNewDevData, 
				NULL, 
				pRadio, 
				pSSID, 
				pAP, 
				NULL);
			
			oid_to_iidstack(p_new_data->SSIDReference, &iidTempStack);
			instance_number2 = iidTempStack.instance[iidTempStack.currentDepth-1];
			enabled = pSSID[instance_number2-1]->enable;
			strncpy(name, pSSID[instance_number2-1]->name, sizeof(name)-1);
			if(0==tr181WiFiObjectModify(p_oid_name, pCurDevData, pNewDevData)){
				tr181_wifi_data_store(pNewDevData, 
					NULL, 
					NULL, 
					pSSID, 
					pAP, 
					NULL, 
					NULL);
				
				if(pSSID[instance_number2-1]->enable!=enabled || strncmp(pSSID[instance_number2-1]->name, name, sizeof(name)-1)){
					tr69_set_obj_by_oid(TR69_OID_WIFISSID, pSSID[instance_number2-1], &iidStack_SSID[instance_number2-1]);
				}
				
				if(STRCMP(p_new_data->SSIDReference, p_cur_data->SSIDReference)){
					oid_to_iidstack(p_cur_data->SSIDReference, &iidTempStack);
					instance_number2 = iidTempStack.instance[iidTempStack.currentDepth-1];
					tr69_set_obj_by_oid(TR69_OID_WIFISSID, pSSID[instance_number2-1], &iidStack_SSID[instance_number2-1]);
				}
			
				rv = TR69_RT_SUCCESS_VALUE_CHANGED;
			}
		}while(0);
		
		for(i=0; i<MAX_NUMBER_OF_ENTRIES_RADIO; i++){
			if(NULL!=pRadio[i]){
				tr69_free_obj_by_oid(TR69_OID_WIFI_RADIO, &iidStack_Radio[i], (void **) &pRadio[i]);
			}
		}
		for(i=0; i<MAX_NUMBER_OF_ENTRIES_SSID; i++){
			if(NULL!=pSSID[i]){
				tr69_free_obj_by_oid(TR69_OID_WIFISSID, &iidStack_SSID[i], (void **) &pSSID[i]);
			}
		}
		for(i=0; i<MAX_NUMBER_OF_ENTRIES_ACCESSPOINT; i++){
			if(NULL!=pAPSecurity[i]){
				tr69_free_obj_by_oid(TR69_OID_WIFI_ACCESS_POINT_SECURITY, &iidStack_APSecurity[i], (void **) &pAPSecurity[i]);
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

