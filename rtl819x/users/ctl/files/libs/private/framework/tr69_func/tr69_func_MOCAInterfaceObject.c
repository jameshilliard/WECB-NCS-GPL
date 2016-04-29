/*./auto/tr69_func_MOCAInterfaceObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"
#include "tr69_cms_object.h"
#include "act_moca.h"
#include "tsl_strconv.h"
#include "act_intf_stack.h"
#ifdef AEI_CTL_BRIDGE
#include "act_dhcpv4.h"
#endif

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_MOCAInterfaceObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.MoCA.Interface
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_MOCAInterfaceObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_MOCAInterfaceObject_value(tsl_char_t *p_oid_name, _MOCAInterfaceObject *p_cur_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

	CTLMEM_REPLACE_STRING(p_cur_data->status, get_moca_status());

	get_moca_maxbw(&(p_cur_data->maxIngressBW), &(p_cur_data->maxEgressBW));
	get_moca_nodeinfo(&(p_cur_data->nodeID),  &(p_cur_data->networkCoordinator),  &(p_cur_data->backupNC));

	CALC_LASTCHANGE( p_cur_data );
	rv = TR69_RT_SUCCESS_VALUE_CHANGED;
	return rv;
}



static void set_moca_led(LED_STATE flag)
{
    char buff[128] = "";
    
    snprintf(buff, sizeof(buff), "echo 0x%x > /proc/act_config/MoCALED", flag);
    system(buff);
}


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_set_MOCAInterfaceObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.MoCA.Interface
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_MOCAInterfaceObject_t *p_cur_data
 *	        st_MOCAInterfaceObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_MOCAInterfaceObject_value(tsl_char_t *p_oid_name, _MOCAInterfaceObject *p_cur_data, _MOCAInterfaceObject *p_new_data)
{
	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_CHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

	if (ENABLE_NEW_OR_ENABLE_EXISTING(p_new_data, p_cur_data)) {
		char mac[18]="00:00:00:00:00:00";
		get_moca_mac(mac, sizeof(mac));

		CTLMEM_REPLACE_STRING(p_new_data->MACAddress, mac);
		set_moca_channel(p_new_data->freqCurrentMaskSetting);
		set_moca_password(p_new_data->keyPassphrase);
		set_moca_swconfig(p_new_data->privacyEnabledSetting, p_new_data->preferredNC, p_new_data->freqCurrentMaskSetting);
		set_moca_BeaconPwrLevel(p_new_data->beaconPowerLimit);	
		set_moca_TxPowerLimit(p_new_data->txPowerLimit);
        set_moca_LastOperFreq(p_new_data->lastOperFreq);

		write_moca_conf(CLINK_CONF_FILE);

		if( launch_moca_daemon(CLINK_CONF_FILE, p_new_data->MACAddress,CLINK_DEV) <0 ) {
            set_moca_led(MOCA_START_UP_FAIL_LED);
			UPDATE_INTF_STATUS_EX_NOCB( p_oid_name, p_cur_data, p_new_data, CTLVS_DOWN);
		}else{
            set_moca_led(MOCA_START_UP_SUCCESS_LED);
        } 
	} else if (POTENTIAL_CHANGE_OF_EXISTING(p_new_data, p_cur_data)) {
		int par_changed = 0;
		if (tsl_strcmp(p_new_data->freqCurrentMaskSetting, p_cur_data->freqCurrentMaskSetting)!=0)  {
			par_changed = 1;
			set_moca_channel(p_new_data->freqCurrentMaskSetting);
			set_moca_swconfig(p_new_data->privacyEnabledSetting, p_new_data->preferredNC, p_new_data->freqCurrentMaskSetting);
			CTLMEM_REPLACE_STRING(p_new_data->freqCurrentMask,p_new_data->freqCurrentMaskSetting);

                        p_new_data->lastOperFreq = find_moca_channel(p_new_data->freqCurrentMaskSetting);
                        if(p_new_data->lastOperFreq == 0)
                        {
                            p_new_data->lastOperFreq = -1;
                        }

                        set_moca_LastOperFreq(p_new_data->lastOperFreq);
		}

		if (tsl_strcmp(p_new_data->keyPassphrase, p_cur_data->keyPassphrase)!=0)   {
			par_changed = 1;
			set_moca_password(p_new_data->keyPassphrase);
		}

		if (p_new_data->privacyEnabledSetting!=p_cur_data->privacyEnabledSetting)  {
			par_changed = 1;
			set_moca_swconfig(p_new_data->privacyEnabledSetting, p_new_data->preferredNC, p_new_data->freqCurrentMaskSetting);
			p_new_data->privacyEnabled = p_new_data->privacyEnabledSetting;
		}

		if (p_new_data->preferredNC!=p_cur_data->preferredNC)  {
			par_changed = 1;
			set_moca_swconfig(p_new_data->privacyEnabledSetting, p_new_data->preferredNC, p_new_data->freqCurrentMaskSetting);
		}

		if (p_new_data->beaconPowerLimit!=p_cur_data->beaconPowerLimit)  {
			par_changed = 1;
			set_moca_BeaconPwrLevel(p_new_data->beaconPowerLimit);	
		}

		if (p_new_data->txPowerLimit!=p_cur_data->txPowerLimit)	{
			par_changed = 1;			
			set_moca_TxPowerLimit(p_new_data->txPowerLimit);
		}

		if (par_changed)
		{
			write_moca_conf(CLINK_CONF_FILE);			
			reconfig_moca_daemon();
		}

		if ( tsl_strcmp(p_new_data->X_ACTIONTEC_COM_Status, p_cur_data->X_ACTIONTEC_COM_Status)!=0) {
            
			if( tsl_strcmp(p_new_data->X_ACTIONTEC_COM_Status, MOCA_STATUS_UP) ==0 ) {
                set_moca_led(MOCA_LINK_UP_LED);
    			//struct timespec ts;
    			int curchan = find_moca_channel(p_new_data->freqCurrentMaskSetting);
    			if (curchan > 0)
    				p_new_data->currentOperFreq = curchan;

    			UPDATE_INTF_STATUS_EX_NOCB( p_oid_name, p_cur_data, p_new_data, CTLVS_UP );
            } else if( tsl_strcmp(p_cur_data->X_ACTIONTEC_COM_Status, MOCA_STATUS_UP) ==0 ) {
                // MoCA down
                set_moca_led(MOCA_LINK_DOWN_LED);
#ifdef AEI_CTL_BRIDGE
                triggerDHCPv4Cient();
#endif
            }
        }

	} else if (DELETE_OR_DISABLE_EXISTING(p_new_data, p_cur_data)) {
		stop_moca_daemon();
        set_moca_led(MOCA_CLINKD_STOP);
		if( DISABLE_EXISTING( p_new_data, p_cur_data ) ) {
			UPDATE_INTF_STATUS_EX_NOCB( p_oid_name, p_cur_data, p_new_data, CTLVS_DOWN);
		}
	}

	return rv;
}

