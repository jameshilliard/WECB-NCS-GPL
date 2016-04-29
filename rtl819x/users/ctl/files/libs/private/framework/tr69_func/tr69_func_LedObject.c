/*./auto/tr69_func_LedObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"
#include "act_moca.h"
#include "ctl_mem.h"
#include "act_intf_stack.h"
#include <linux/wireless.h> 
#include <sys/ioctl.h>
#include "act_led.h"

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        wlioctl_get_wifi_led_status
 *
 *	[DESCRIPTION]:
 *	        get wireless led status
 *
 *	[PARAMETER]:
 *	        char* interface
 *          char* status
 *
 *	[RETURN]
 *           >= 0 Success
 *           < 0 Failed
 **************************************************************************/
int wlioctl_get_wifi_led_status(char* interface, char* status)
{
    int fd;
    struct iwreq wrq;
    struct _ioctl_led_status led_status;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0)
    {
        ctllog_error("FUNCTION LINE  %s %d: fd alloc error\n", __func__, __LINE__);
        return -1;
    }

    memset(&wrq, 0, sizeof(wrq));
    tsl_strncpy(wrq.ifr_name, interface, sizeof(wrq.ifr_name) - 1);
    wrq.u.data.pointer = &led_status;
    wrq.u.data.length = sizeof(struct _ioctl_led_status);
    if(ioctl(fd, SIOWIFILEDSTATUS, &wrq) < 0)
    {
        close(fd);
        ctllog_error("FUNCTION LINE  %s %d: ioctl error \n",  __FUNCTION__, __LINE__); 
        return -1;
    }
    close(fd);

    ctllog_debug("FUNCTION LINE  %s %d: mode = %d, cycle = %d on = %d, off = %d \n",  __FUNCTION__, __LINE__, 
            led_status.led_mode, led_status.led_cycles, led_status.led_on_interval, 
            led_status.led_off_interval);

    switch(led_status.led_mode)
    {
        case LED_MODE_OFF:
            strcpy(status, "Off");
            break;
        case LED_MODE_ON:
            strcpy(status, "On");
            break;
        case LED_MODE_BLINK:
            if(led_status.led_on_interval > BLINK_INTERVAL_JIFFIES(FAST_BLINK_INTERVAL_ON) &&
                    led_status.led_off_interval > BLINK_INTERVAL_JIFFIES(FAST_BLINK_INTERVAL_OFF))
            {
                strcpy(status, "SlowBlink");
            }
            else
            {
                strcpy(status, "FastBlink");
            }
            break;
        default:
            break;
    }
    return 0;
}

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        get_moca_led_status
 *
 *	[DESCRIPTION]:
 *	        get led status: On/Off/Connected_Below_Threshold
 *
 *	[PARAMETER]:
 *	        char* status
 *
 *	[RETURN]
 *           char* status
 **************************************************************************/
void get_moca_led_status(char* status)
{
    int moca_led = get_moca_led();

    if(moca_led & LED_BLINK)
    {
        strcpy(status, "Blink");
    }
    else if(moca_led & LED_SLOW_BLINK)
    {
#if defined(AEI_WECB_CUSTOMER_VERIZON)
    strcpy(status, "Connected_Below_Threshold");
#else
    strcpy(status, "SlowBlink");
#endif
    }
    else if(moca_led & LED_FAST_BLINK)
    {
        strcpy(status, "FastBlink");
    }
    else if(moca_led & LED_ON)
    {
        strcpy(status, "On");
    }
    else if(moca_led & LED_OFF)
    {
        strcpy(status, "Off");
    }
    else
    {
        strcpy(status, "Error");
    }

    return;
}

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_LedObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.X_ACTIONTEC_COM_LED
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_LedObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_LedObject_value(tsl_char_t *p_oid_name, _LedObject *p_cur_data)
{
    tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;
    char  status[64] = {0};

    get_moca_led_status(status);
    CTLMEM_REPLACE_STRING(p_cur_data->LED_MoCA_Status, status);


    wlioctl_get_wifi_led_status(INTERFACE_WIFI2G, status);
    CTLMEM_REPLACE_STRING(p_cur_data->LED_WiFi2G_Status, status);

    wlioctl_get_wifi_led_status(INTERFACE_WIFI5G, status);
    CTLMEM_REPLACE_STRING(p_cur_data->LED_WiFi5G_Status, status);

    rv = TR69_RT_SUCCESS_VALUE_CHANGED;
    return rv;
}


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_set_LedObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.X_ACTIONTEC_COM_LED
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_LedObject_t *p_cur_data
 *	        st_LedObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_LedObject_value(tsl_char_t *p_oid_name, _LedObject *p_cur_data, _LedObject *p_new_data)
{
    tsl_int_t rv = TR69_RT_SUCCESS_VALUE_CHANGED;

    ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

    return rv;
}

