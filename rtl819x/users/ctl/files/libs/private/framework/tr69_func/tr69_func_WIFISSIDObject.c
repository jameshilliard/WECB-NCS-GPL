/*./auto/tr69_func_WIFISSIDObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"
#include "wifi_data_openwrt.h"

#include "act_intf_stack.h"

static time_t last_access[MAX_NUMBER_OF_ENTRIES_SSID] = {0};
extern unsigned int object_has_been_created;
/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_WIFISSIDObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.WiFi.SSID
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_WIFISSIDObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_WIFISSIDObject_value(tsl_char_t *p_oid_name, _WIFISSIDObject *p_cur_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;
	tr69_oid_stack_id iidThisStack = EMPTY_INSTANCE_ID_STACK;
	int instance_number = 0;
	PDEVICE_WIFI pCurDevData = NULL;
	WIFISSIDObject* pSSID[MAX_NUMBER_OF_ENTRIES_SSID] = {0};
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
		pCurDevData = calloc(1, sizeof(DEVICE_WIFI));
		if(NULL==pCurDevData){
			ctllog_error("calloc failed for DEVICE_WIFI!\n");
			break;
		}
		//SSID
		pSSID[instance_number-1] = p_cur_data;
		tr181_wifi_data_load(pCurDevData, 
			NULL, 
			NULL, 
			pSSID, 
			NULL, 
			NULL);
		if(0==tr181WiFiObjectGet(p_oid_name, pCurDevData)){
			tr181_wifi_data_store(pCurDevData, 
				NULL, 
				NULL, 
				pSSID, 
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


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_set_WIFISSIDObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.WiFi.SSID
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_WIFISSIDObject_t *p_cur_data
 *	        st_WIFISSIDObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_WIFISSIDObject_value(tsl_char_t *p_oid_name, _WIFISSIDObject *p_cur_data, _WIFISSIDObject *p_new_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;
	tr69_oid_stack_id iidThisStack = EMPTY_INSTANCE_ID_STACK;
	tr69_oid_stack_id iidTempStack = EMPTY_INSTANCE_ID_STACK;
	PDEVICE_WIFI pCurDevData = NULL;
	PDEVICE_WIFI pNewDevData = NULL;
	tr69_oid_stack_id iidStack_Radio[MAX_NUMBER_OF_ENTRIES_RADIO] = {EMPTY_INSTANCE_ID_STACK};
	WIFIRadioObject* pRadio[MAX_NUMBER_OF_ENTRIES_RADIO] = {0};
	WIFISSIDObject* pSSID[MAX_NUMBER_OF_ENTRIES_SSID] = {0};
	tr69_oid_stack_id iidStack_AP[MAX_NUMBER_OF_ENTRIES_ACCESSPOINT] = {EMPTY_INSTANCE_ID_STACK};
	WIFIAccessPointObject* pAP[MAX_NUMBER_OF_ENTRIES_ACCESSPOINT] = {0};
	int instance_number = 0;
	int i;
	unsigned int enabled = 0;

	ctllog_debug("p_oid_name=%s p_cur_data=%p p_new_data=%p \n", p_oid_name, p_cur_data, p_new_data);
	do{
        UPDATE_INTF_STACK( p_oid_name, p_cur_data, p_new_data );

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
		//SSID
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
		
		pSSID[instance_number-1] = p_cur_data;
		tr181_wifi_data_load(pCurDevData, 
			NULL, 
			pRadio, 
			pSSID, 
			pAP, 
			NULL);
		pSSID[instance_number-1] = p_new_data;
		tr181_wifi_data_load(pNewDevData, 
			NULL, 
			pRadio, 
			pSSID, 
			pAP, 
			NULL);
		enabled = pAP[instance_number-1]->enable;
		if(0==tr181WiFiObjectModify(p_oid_name, pCurDevData, pNewDevData)){
			tr181_wifi_data_store(pNewDevData, 
				NULL, 
				NULL, 
				pSSID, 
				pAP, 
				NULL, 
				NULL);

			for(i=0; i<MAX_NUMBER_OF_ENTRIES_ACCESSPOINT; i++){
				if(!STRCMP(pAP[i]->SSIDReference, p_oid_name)){
					if(pAP[i]->enable!=enabled){
						tr69_set_obj_by_oid(TR69_OID_WIFI_ACCESS_POINT, pAP[i], &iidStack_AP[i]);
					}
				}
			}

			rv = TR69_RT_SUCCESS_VALUE_CHANGED;
		}
	}while(0);

	for(i=0; i<MAX_NUMBER_OF_ENTRIES_RADIO; i++){
		if(NULL!=pRadio[i]){
			tr69_free_obj_by_oid(TR69_OID_WIFI_RADIO, &iidStack_Radio[i], (void **) &pRadio[i]);
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

