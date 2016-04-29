#include "ctl.h"

#include "act_eth.h"


#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>

tsl_rv_t GetEthMACAddr( const tsl_char_t *name, tsl_char_t * mac, tsl_int_t size )
{//done
	tsl_rv_t rv = TSL_RV_FAIL;
	int s = -1;
	struct ifreq ifr;

	do{
        //ctllog_notice( "name = %s", name );
        if( (NULL == mac ) || (size < 18) ) {
            ctllog_warn( "No enough space to save Ethernet MAC address!" );
            break;
        }
		s=socket(PF_INET, SOCK_DGRAM, 0);
		if(s<0){
			perror("socket");
			break;
		}
		memset(&ifr, 0x00, sizeof(ifr));
		snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", name);
		if (ioctl(s, SIOCGIFHWADDR, &ifr)<0){
			perror("ioctl");
			break;
		}
		snprintf(mac, size, "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
			(unsigned char)ifr.ifr_hwaddr.sa_data[0],
			(unsigned char)ifr.ifr_hwaddr.sa_data[1],
			(unsigned char)ifr.ifr_hwaddr.sa_data[2],
			(unsigned char)ifr.ifr_hwaddr.sa_data[3],
			(unsigned char)ifr.ifr_hwaddr.sa_data[4],
			(unsigned char)ifr.ifr_hwaddr.sa_data[5] );
        //ctllog_notice( "mac = %s", mac );
		rv = TSL_RV_SUC;
	}while(0);

	if(-1!=s){
		close(s);
	}

	return rv;
}


