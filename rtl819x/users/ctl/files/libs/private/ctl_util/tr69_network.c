

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>

#include "ctl.h"
#include "tr69_oal.h"
#include "tsl_common.h"
#include "tr69_strconv.h"
#include "tsl_strconv.h"
#include "ctl_log.h"


tsl_bool_t tr69_util_ip6_addr_equal(const char *ip6Addr1, const char *ip6Addr2)
{
   char address1[CTL_IPADDR_LENGTH];
   char address2[CTL_IPADDR_LENGTH];
   tsl_u32_t plen1 = 0;
   tsl_u32_t plen2 = 0;
   struct in6_addr   in6Addr1, in6Addr2;
   tsl_rv_t ret;

   if (IS_EMPTY_STRING(ip6Addr1) && IS_EMPTY_STRING(ip6Addr2))
   {
      return tsl_b_true;
   }
   if (ip6Addr1 == NULL || ip6Addr2 == NULL)
   {
      return tsl_b_false;
   }

   if ((ret = ctlUtl_parsePrefixAddress(ip6Addr1, address1, &plen1)) != tsl_rv_suc)
   {
      ctllog_error("cmsUtl_parsePrefixAddress returns error. ret=%d", ret);
      return tsl_b_false;
   }
   if ((ret = ctlUtl_parsePrefixAddress(ip6Addr2, address2, &plen2)) != tsl_rv_suc)
   {
      ctllog_error("cmsUtl_parsePrefixAddress returns error. ret=%d", ret);
      return tsl_b_false;
   }

   if (inet_pton(AF_INET6, address1, &in6Addr1) <= 0)
   {
      ctllog_error("Invalid address1=%s", address1);
      return tsl_b_false;
   }
   if (inet_pton(AF_INET6, address2, &in6Addr2) <= 0)
   {
      ctllog_error("Invalid address2=%s", address2);
      return tsl_b_false;
   }

   return ((memcmp(&in6Addr1, &in6Addr2, sizeof(struct in6_addr)) == 0) && (plen1 == plen2));

}  /* tr69_util_ip6_addr_equal() */
