/*./auto/tr69_func_WIFIRadioObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"
#include "wifi_data_openwrt.h"


static time_t last_access[MAX_NUMBER_OF_ENTRIES_RADIO] = {0};
extern unsigned int object_has_been_created;
/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_WIFIRadioObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.WiFi.Radio
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_WIFIRadioObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_WIFIRadioObject_value(tsl_char_t *p_oid_name, _WIFIRadioObject *p_cur_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;
	tr69_oid_stack_id iidThisStack = EMPTY_INSTANCE_ID_STACK;
	int instance_number = 0;
	PDEVICE_WIFI pCurDevData = NULL;
	WIFIRadioObject* pRadio[MAX_NUMBER_OF_ENTRIES_RADIO] = {0};
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
		//Radio
		pRadio[instance_number-1] = p_cur_data;
		tr181_wifi_data_load(pCurDevData, 
			NULL, 
			pRadio, 
			NULL, 
			NULL, 
			NULL);
		if(0==tr181WiFiObjectGet(p_oid_name, pCurDevData)){
			tr181_wifi_data_store(pCurDevData, 
				NULL, 
				pRadio, 
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


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_set_WIFIRadioObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.WiFi.Radio
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_WIFIRadioObject_t *p_cur_data
 *	        st_WIFIRadioObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_WIFIRadioObject_value(tsl_char_t *p_oid_name, _WIFIRadioObject *p_cur_data, _WIFIRadioObject *p_new_data)
{
	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;
	WIFIRadioObject* pRadio[MAX_NUMBER_OF_ENTRIES_RADIO] = {0};
	tr69_oid_stack_id iidThisStack = EMPTY_INSTANCE_ID_STACK;
	//tr69_oid_stack_id iidTempStack = EMPTY_INSTANCE_ID_STACK;
	PDEVICE_WIFI pCurDevData = NULL;
	PDEVICE_WIFI pNewDevData = NULL;
	//int i;
	int instance_number = 0;
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
		if(NULL!=p_cur_data&&NULL==p_new_data){//destroy object
			//Radio
			pCurDevData = calloc(1, sizeof(DEVICE_WIFI));
			if(NULL==pCurDevData){
				ctllog_error("calloc failed for DEVICE_WIFI!\n");
				break;
			}
			
			pRadio[instance_number-1] = p_cur_data;
			tr181_wifi_data_load(pCurDevData, 
				NULL, 
				pRadio, 
				NULL, 
				NULL, 
				NULL);
			if(0==tr181WiFiObjectDestroy(p_oid_name, pCurDevData)){
				rv = TR69_RT_SUCCESS_VALUE_CHANGED;
			}
			break;
		}
		
		if(NULL==p_cur_data&&NULL!=p_new_data){//create object
			//Radio
			pNewDevData = calloc(1, sizeof(DEVICE_WIFI));
			if(NULL==pNewDevData){
				ctllog_error("calloc failed for DEVICE_WIFI!\n");
				break;
			}
			
			pRadio[instance_number-1] = p_new_data;
			tr181_wifi_data_load(pNewDevData, 
				NULL, 
				pRadio, 
				NULL, 
				NULL, 
				NULL);
			
			if(0==tr181WiFiObjectCreate(p_oid_name, pNewDevData)){
				tr181_wifi_data_store(pNewDevData, 
					NULL, 
					pRadio, 
					NULL, 
					NULL, 
					NULL, 
					NULL);
			
				rv = TR69_RT_SUCCESS_VALUE_CHANGED;
			}
			break;
		}
		
		//modify object 
		//Radio
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
		
		pRadio[instance_number-1] = p_cur_data;
		tr181_wifi_data_load(pCurDevData, 
			NULL, 
			pRadio, 
			NULL, 
			NULL, 
			NULL);
		pRadio[instance_number-1] = p_new_data;
		tr181_wifi_data_load(pNewDevData, 
			NULL, 
			pRadio, 
			NULL, 
			NULL, 
			NULL);
		
		if(0==tr181WiFiObjectModify(p_oid_name, pCurDevData, pNewDevData)){
			tr181_wifi_data_store(pNewDevData, 
				NULL, 
				pRadio, 
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

	if(NULL!=pNewDevData){
		free(pNewDevData);
		pNewDevData = NULL;
	}

	if(TR69_RT_SUCCESS_VALUE_CHANGED==rv){
		last_access[instance_number-1] = 0;
	}

    // SW-Task #101601: Always set X_ACTIONTEC_COM_AutoChannelRefres as 0
    if(p_new_data != NULL)
    {
        p_new_data->X_ACTIONTEC_COM_AutoChannelRefresh = 0;
    }
	return rv;
}

