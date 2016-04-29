/*./auto/tr69_func_LANObject.c*/

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"
#include "ctl.h"
#include "tr69_cms_object.h"
#include "tsl_msg.h"
#include "tsl_strconv.h"

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_LANObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.LAN
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_LANObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_LANObject_value(tsl_char_t *p_oid_name, _LANObject *p_cur_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

	return rv;
}


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_set_LANObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.LAN
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_LANObject_t *p_cur_data
 *	        st_LANObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_LANObject_value(tsl_char_t *p_oid_name, _LANObject *p_cur_data, _LANObject *p_new_data)
{
	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_CHANGED;
	int s;
	struct ifreq buffer;
	char mac[18]={0};

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

	if (p_new_data!=NULL && p_cur_data!=NULL)	// modify object
	{
		if ( tsl_strcmp(p_new_data->IPAddress , p_cur_data->IPAddress)!=0 &&
				tsl_strcmp(p_new_data->IPAddress,"0.0.0.0")!=0 )
		{
			if ((s=socket(PF_INET, SOCK_DGRAM, 0))!=-1)
			{
				memset(&buffer, 0x00, sizeof(buffer));
				strcpy(buffer.ifr_name, "br0");
				/* coverity #29513 */
				if (ioctl(s, SIOCGIFHWADDR, &buffer) != -1)
				{				
					snprintf(mac, sizeof(mac),"%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
						(unsigned char)buffer.ifr_hwaddr.sa_data[0],
						(unsigned char)buffer.ifr_hwaddr.sa_data[1],
						(unsigned char)buffer.ifr_hwaddr.sa_data[2],
						(unsigned char)buffer.ifr_hwaddr.sa_data[3],
						(unsigned char)buffer.ifr_hwaddr.sa_data[4],
						(unsigned char)buffer.ifr_hwaddr.sa_data[5] );	
				
					CTLMEM_REPLACE_STRING(p_new_data->MACAddress, mac);	
				}
				
				close(s);
			}

		}
	}

	return rv;
}

