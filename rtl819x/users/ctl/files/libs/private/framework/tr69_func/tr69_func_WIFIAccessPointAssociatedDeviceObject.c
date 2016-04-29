/*./auto/tr69_func_WIFIAccessPointAssociatedDeviceObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"
#include "wifi_data_openwrt.h"


static time_t last_access[MAX_NUMBER_OF_ENTRIES_ACCESSPOINT][MAX_NUMBER_OF_ENTRIES_ASSOCIATEDDEVICES] = {{0}};
extern unsigned int object_has_been_created;
/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_WIFIAccessPointAssociatedDeviceObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.WiFi.AccessPoint.i.AssociatedDevice
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_WIFIAccessPointAssociatedDeviceObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_WIFIAccessPointAssociatedDeviceObject_value(tsl_char_t *p_oid_name, _WIFIAccessPointAssociatedDeviceObject *p_cur_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;
	tr69_oid_stack_id iidThisStack = EMPTY_INSTANCE_ID_STACK;
	tr69_oid_stack_id iidTempStack = EMPTY_INSTANCE_ID_STACK;
	PDEVICE_WIFI pCurDevData = NULL;
	WIFIAccessPointAssociatedDeviceObject* pAPAssociatedDevice[MAX_NUMBER_OF_ENTRIES_ACCESSPOINT][MAX_NUMBER_OF_ENTRIES_ASSOCIATEDDEVICES];
	int i, j, k;
	tr69_oid_stack_id iidStack_AP[MAX_NUMBER_OF_ENTRIES_ACCESSPOINT] = {EMPTY_INSTANCE_ID_STACK};
	WIFIAccessPointObject* pAP[MAX_NUMBER_OF_ENTRIES_ACCESSPOINT] = {0};
	tr69_oid_stack_id iidStack_SSID[MAX_NUMBER_OF_ENTRIES_SSID] = {EMPTY_INSTANCE_ID_STACK};
	WIFISSIDObject* pSSID[MAX_NUMBER_OF_ENTRIES_SSID] = {0};
#ifndef SUPPORTED_TR181
	return TR69_RT_SUCCESS_VALUE_UNCHANGED;
#endif

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);
	do{
		if(object_has_been_created<MAX_NUMBER_OF_ENTRIES_ACCESSPOINT){
			break;
		}
		oid_to_iidstack(p_oid_name, &iidThisStack);
		i = iidThisStack.instance[iidThisStack.currentDepth-2]-1;
		j = iidThisStack.instance[iidThisStack.currentDepth-1]-1;
		if(DIFF_TIME(last_access[i][j])<MIN_OBJ_GET_INTERVAL){
			break;
		}
		last_access[i][j] = CURRENT_TIME;
		
		//AP
		k = 0;
		memset(&iidTempStack, 0, sizeof(iidTempStack));
		while (k<MAX_NUMBER_OF_ENTRIES_ACCESSPOINT&&tr69_get_next_sub_obj_by_oid(TR69_OID_WIFI_ACCESS_POINT,
			&iidThisStack, &iidTempStack, (void **)&pAP[k]) == TSL_RV_SUC){
			iidStack_AP[k] = iidTempStack;
			k++;
		}
		if (0==k){
			ctllog_error("WIFIAccessPointObject not found!\n");
			break;
		}
		//SSID
		k = 0;
		memset(&iidTempStack, 0, sizeof(iidTempStack));
		while (k<MAX_NUMBER_OF_ENTRIES_SSID&&tr69_get_next_sub_obj_by_oid(TR69_OID_WIFISSID,
			&iidThisStack, &iidTempStack, (void **)&pSSID[k]) == TSL_RV_SUC){
			iidStack_SSID[k] = iidTempStack;
			k++;
		}
		if (0==k){
			ctllog_error("WIFISSIDObject not found!\n");
			break;
		}
		//WiFi
		pCurDevData = calloc(1, sizeof(DEVICE_WIFI));
		if(NULL==pCurDevData){
			ctllog_error("calloc failed for DEVICE_WIFI!\n");
			break;
		}
		
		memset(pAPAssociatedDevice, 0, sizeof(pAPAssociatedDevice));
		pAPAssociatedDevice[i][j] = p_cur_data;
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
				NULL, 
				NULL, 
				pAPAssociatedDevice);
		
			rv = TR69_RT_SUCCESS_VALUE_CHANGED;
		}
	}while(0);

	for(k=0; k<MAX_NUMBER_OF_ENTRIES_ACCESSPOINT; k++){
		if(NULL!=pAP[k]){
			tr69_free_obj_by_oid(TR69_OID_WIFI_ACCESS_POINT, &iidStack_AP[k], (void **) &pAP[k]);
		}
	}
	for(k=0; k<MAX_NUMBER_OF_ENTRIES_SSID; k++){
		if(NULL!=pSSID[k]){
			tr69_free_obj_by_oid(TR69_OID_WIFISSID, &iidStack_SSID[k], (void **) &pSSID[k]);
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
 *	        tf69_func_set_WIFIAccessPointAssociatedDeviceObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.WiFi.AccessPoint.i.AssociatedDevice
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_WIFIAccessPointAssociatedDeviceObject_t *p_cur_data
 *	        st_WIFIAccessPointAssociatedDeviceObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_WIFIAccessPointAssociatedDeviceObject_value(tsl_char_t *p_oid_name, _WIFIAccessPointAssociatedDeviceObject *p_cur_data, _WIFIAccessPointAssociatedDeviceObject *p_new_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

	return rv;
}

