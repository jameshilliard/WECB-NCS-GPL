

#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "ctl.h"
#include "tr69_oal.h"
#include "ctl_log.h"
#include "tsl_strconv.h"
#include "ctl_mem.h"
tsl_rv_t tr69_util_get_ip6_addr_info(const char *ifname, tsl_u32_t addrIdx,
                      char *ipAddr, tsl_u32_t *ifIndex, tsl_u32_t *prefixLen, tsl_u32_t *scope, tsl_u32_t *ifaFlags)
{
   tsl_rv_t   ret = tsl_rv_fail;
   FILE     *fp;
   tsl_32_t   count = 0;
   char     line[BUFLEN_64];

   *ipAddr = '\0';

   if ((fp = fopen("/proc/net/if_inet6", "r")) == NULL)
   {
      ctllog_error("failed to open /proc/net/if_inet6");
      return tsl_rv_err;
   }

   while (fgets(line, sizeof(line), fp) != NULL)
   {
      /* remove the carriage return char */
      line[strlen(line)-1] = '\0';

      if (strstr(line, ifname) != NULL)
      {
         char *addr, *ifidx, *plen, *scp, *flags, *devname; 
         char *nextToken = NULL;

         /* the first token in the line is the ip address */
         addr = strtok_r(line, " ", &nextToken);

         /* the second token is the Netlink device number (interface index) in hexadecimal */
         ifidx = strtok_r(NULL, " ", &nextToken);
         if (ifidx == NULL)
         {
            ctllog_error("Invalid /proc/net/if_inet6 line");
            ret = tsl_rv_err;
            break;
         }
            
         /* the third token is the Prefix length in hexadecimal */
         plen = strtok_r(NULL, " ", &nextToken);
         if (plen == NULL)
         {
            ctllog_error("Invalid /proc/net/if_inet6 line");
            ret = tsl_rv_err;
            break;
         }
            
         /* the forth token is the Scope value */
         scp = strtok_r(NULL, " ", &nextToken);
         if (scp == NULL)
         {
            ctllog_error("Invalid /proc/net/if_inet6 line");
            ret = tsl_rv_err;
            break;
         }
            
         /* the fifth token is the ifa flags */
         flags = strtok_r(NULL, " ", &nextToken);
         if (flags == NULL)
         {
            ctllog_error("Invalid /proc/net/if_inet6 line");
            ret = tsl_rv_err;
            break;
         }
            
         /* the sixth token is the device name */
         devname = strtok_r(NULL, " ", &nextToken);
         if (devname == NULL)
         {
            ctllog_error("Invalid /proc/net/if_inet6 line");
            ret = tsl_rv_err;
            break;
         }
         else
         {
            if (tsl_strcmp(devname, ifname) != 0)
            {
               continue;
            }
            else if (count == addrIdx)
            {
               tsl_32_t   i;
               char     *p1, *p2;

               *ifIndex   = strtoul(ifidx, NULL, 16);
               *prefixLen = strtoul(plen, NULL, 16);
               *scope     = strtoul(scp, NULL, 16);
               *ifaFlags  = strtoul(flags, NULL, 16);

               /* insert a colon every 4 digits in the address string */
               p2 = ipAddr;
               for (i = 0, p1 = addr; *p1 != '\0'; i++)
               {
                  if (i == 4)
                  {
                     i = 0;
                     *p2++ = ':';
                  }
                  *p2++ = *p1++;
               }
               *p2 = '\0';

               ret = tsl_rv_suc;
               break;   /* done */
            }
            else
            {
               count++;
            }
         }
      }
   }  /* while */

   fclose(fp);

   return ret;

}  /* End of oal_getIfAddr6() */


/* Get the existing interface names in the kernel, regardless they're active
 * or not. If success, the ifNameList will be assigned a new allocated string
 * containing names separated by commas. It may look like
 * "lo,dsl0,eth0,eth1,usb0,wl0".
 *
 * Caller should free ifNameList by cmsMem_free() after use.
 *
 * Return TSL_RV_SUC if success, error code otherwise.
 */
tsl_rv_t oalNet_getIfNameList(char **ifNameList)
{
   struct if_nameindex *ni_list = if_nameindex();
   struct if_nameindex *ni_list2 = ni_list;
   char buf[1024];
   char *pbuf = buf;
   int len;

   if (ni_list == NULL)
      return CMSRET_INTERNAL_ERROR;

   /* Iterate through the array of interfaces to concatenate interface
    * names, separated by commas */
   while(ni_list->if_index) {
      len = strlen(ni_list->if_name);
      memcpy(pbuf, ni_list->if_name, len);
      pbuf += len;
      *pbuf++ = ',';
      ni_list++;
   }
   len = pbuf - buf;
   buf[len-1] = 0;

   if_freenameindex(ni_list2);

   /* Allocate dynamic memory for interface name list */
   if ((*ifNameList = calloc(len, 1)) == NULL)
      return CMSRET_RESOURCE_EXCEEDED;
   memcpy(*ifNameList, buf, len);
   
   return TSL_RV_SUC;
}


tsl_rv_t oal_Net_getPersistentWanIfNameList(char **persistentWanIfNameList)
{
   tsl_32_t  skfd;
   struct ifreq ifr;

   if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
   {
      ctllog_error("Error openning socket for getting the enet WAN list");
      return CMSRET_INTERNAL_ERROR;
   }
   
    /* Get the name -> if_index mapping for ethswctl */
    strcpy(ifr.ifr_name, "bcmsw");
    if (ioctl(skfd, SIOCGIFINDEX, &ifr) < 0) 
    {
        close(skfd);
        ctllog_debug("bcmsw interface does not exist.  Error: %d", errno);
        return CMSRET_INTERNAL_ERROR;
    }

   /* Allocate dynamic memory to hold max interface names (eth0,eth1,..eth10<cr>)*/
   if ((*persistentWanIfNameList = calloc(((MAX_PERSISTENT_WAN_PORT * (IFNAMSIZ+1)) + 2), 1)) == NULL)
   {
      ctllog_error("Fail to alloc mem in getting the enet WAN list");
      close(skfd);      
      return CMSRET_RESOURCE_EXCEEDED;
   }

/* coverity #11300 */
   memset((void *) &ifr, 0, sizeof(ifr));
   ifr.ifr_data = *persistentWanIfNameList;
   if (ioctl(skfd, SIOCGWANPORT, &ifr) < 0)
   {
      ctllog_error("ioct error in getting the enet WAN list.  Error: %d", errno);
      close(skfd);
      CTLMEM_FREE_BUF_AND_NULL_PTR(*persistentWanIfNameList);
      return CMSRET_INTERNAL_ERROR;
   }

   close(skfd);

   ctllog_debug("WannEnetPortList=%s, strlen=%d", *persistentWanIfNameList, strlen(*persistentWanIfNameList));

   return TSL_RV_SUC;
   
}
