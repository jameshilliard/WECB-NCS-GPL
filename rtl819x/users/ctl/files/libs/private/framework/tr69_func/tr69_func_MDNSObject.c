/*./auto/tr69_func_MDNSObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"

#include "act_intf_stack.h"
#include "tr69_cms_object.h"
#include "ctl_validstrings.h"
#include "tsl_strconv.h"
#include "libtr69_func.h"

#include "ctl.h"
#include "tsl_msg.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <stdio.h>

void stopMDNSResponder()
{
    system("killall mDNSResponderPosix");
#ifdef AEI_FIREWALL
    system("iptables -D INPUT -i br0  -d 224.0.0.251 -p udp --dport 5353 -j ACCEPT");
#endif
}
void startMDNSResponder()
{
    FILE *pfp = NULL;
    tsl_char_t pipeline[128]={0};
    tsl_char_t cmdstr[128]={0};

#ifdef AEI_FIREWALL
    system("iptables -A INPUT -i br0  -d 224.0.0.251 -p udp --dport 5353 -j ACCEPT");
#endif
    snprintf(cmdstr, sizeof(cmdstr), "hostname");
    if ((pfp = popen(cmdstr, "r"))!=NULL) {
        if (fgets(pipeline, sizeof(pipeline)-1, pfp)) {		
            if(strlen(pipeline) && (pipeline[strlen(pipeline)-1]=='\n'))
            {
                pipeline[strlen(pipeline)-1]='\0';
            }
            DO_SYSTEM( "mDNSResponderPosix -n %s -t _WECB_AEI._tcp. -b",pipeline);
        }
        pclose(pfp);
    }
}
/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_MDNSObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.X_ACTIONTEC_COM_MDNS
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_MDNSObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_MDNSObject_value(tsl_char_t *p_oid_name, _MDNSObject *p_cur_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);
	return rv;
}


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_set_MDNSObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.X_ACTIONTEC_COM_MDNS
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_MDNSObject_t *p_cur_data
 *	        st_MDNSObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_MDNSObject_value(tsl_char_t *p_oid_name, _MDNSObject *p_cur_data, _MDNSObject *p_new_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_CHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);
    if(ENABLE_NEW_OR_ENABLE_EXISTING(p_new_data,p_cur_data))
    {
        startMDNSResponder();
    }
    else if (POTENTIAL_CHANGE_OF_EXISTING(p_new_data,p_cur_data))
    {
        if(p_new_data->X_ACTIONTEC_COM_Trigger==1 && p_cur_data->X_ACTIONTEC_COM_Trigger==0)
        {
            stopMDNSResponder();
            startMDNSResponder();
            p_new_data->X_ACTIONTEC_COM_Trigger=0;
        }
    }
    else if (DISABLE_EXISTING(p_new_data,p_cur_data) )
    {
        stopMDNSResponder();
    }
    
	return rv;
}

