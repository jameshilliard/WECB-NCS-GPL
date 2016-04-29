#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>     /* for isDigit, really should be in oal_strconv.c */
#include <sys/stat.h>  /* this really should be in oal_strconv.c */
#include <arpa/inet.h> /* for inet_aton */
#include <ctl.h>
#include "ctl_objectid.h"
#include "ctl_log.h"
#include "tr69_strconv.h"
#include "ctl_validstrings.h"
#include "ctl_mem.h"
#include "sys_log.h"
#include <dirent.h>

#define MEM_FREE_BUF_AND_NULL_PTR(p) \                                                                      
    do { \                                                                                                   
        if ((p) != NULL) {free((p)); (p) = NULL;}   \                                                         
    } while (0) 

#define isspace(c) ((c) == ' ' || (c) == '\t' || (c) == '\r' || (c) == '\n')
tsl_bool_t cmsUtl_isValidVpiVci(tsl_32_t vpi, tsl_32_t vci)
{
   if (vpi >= VPI_MIN && vpi <= VPI_MAX && vci >= VCI_MIN && vci <= VCI_MAX)
   {
      return TSL_B_TRUE;
   }
   
   ctllog_error("invalid vpi/vci %d/%d", vpi, vci);
   return TSL_B_FALSE;
}

tsl_rv_t cmsUtl_atmVpiVciStrToNum(const char *vpiVciStr, tsl_32_t *vpi, tsl_32_t *vci)
{
   char *pSlash;
   char vpiStr[BUFLEN_256];
   char vciStr[BUFLEN_256];
   char *prefix;
   
   *vpi = *vci = -1;   
   if (vpiVciStr == NULL)
   {
      ctllog_error("vpiVciStr is NULL");
      return CMSRET_INVALID_ARGUMENTS;
   }      

   strncpy(vpiStr, vpiVciStr, sizeof(vpiStr));
   /* Coverity #10298 */
   vpiStr[sizeof(vpiStr)-1]='\0';

   if (strstr(vpiStr, DSL_LINK_DESTINATION_PREFIX_SVC))
   {
      ctllog_error("DesitinationAddress string %s with %s is not supported yet.", vpiStr, DSL_LINK_DESTINATION_PREFIX_SVC);
      return CMSRET_INVALID_PARAM_VALUE;
   }

   if ((prefix = strstr(vpiStr, DSL_LINK_DESTINATION_PREFIX_PVC)) == NULL)
   {
      ctllog_error("Invalid DesitinationAddress string %s", vpiStr);
      return CMSRET_INVALID_PARAM_VALUE;
   }
 
   /* skip the prefix */
#if 0
   prefix += sizeof(DSL_LINK_DESTINATION_PREFIX_PVC);
#endif
   prefix += strlen(DSL_LINK_DESTINATION_PREFIX_PVC);
   /* skip the blank if there is one */
   if (*prefix == ' ')
   {
      prefix += 1;
   }

   pSlash = (char *) strchr(prefix, '/');
   if (pSlash == NULL)
   {
      ctllog_error("vpiVciStr %s is invalid", vpiVciStr);
      return CMSRET_INVALID_ARGUMENTS;
   }
   //Coverity CID 10298: Buffer not null terminated
   tsl_strncpy(vciStr, (pSlash + 1), sizeof(vciStr));
   *pSlash = '\0';       
   *vpi = atoi(prefix);
   *vci = atoi(vciStr);
   if (cmsUtl_isValidVpiVci(*vpi, *vci) == TSL_B_FALSE)
   {
      return CMSRET_INVALID_PARAM_VALUE;
   }     

   return TSL_RV_SUC;
   
}


tsl_rv_t cmsUtl_atmVpiVciNumToStr(const tsl_32_t vpi, const tsl_32_t vci, char *vpiVciStr)
{
   if (vpiVciStr == NULL)
   {
      ctllog_error("vpiVciStr is NULL");
      return CMSRET_INVALID_ARGUMENTS;
   }         
   if (cmsUtl_isValidVpiVci(vpi, vci) == TSL_B_FALSE)
   {
      return CMSRET_INVALID_PARAM_VALUE;
   }     

   sprintf(vpiVciStr, "%s %d/%d", DSL_LINK_DESTINATION_PREFIX_PVC, vpi, vci);

   return TSL_RV_SUC;
   
}


tsl_rv_t ctlUtl_macStrToNum(const char *macStr, tsl_u8_t *macNum) 
{
   char *pToken = NULL;
   char *pLast = NULL;
   char *buf;
   tsl_32_t i;
   
   if (macNum == NULL || macStr == NULL) 
   {
      ctllog_error("Invalid macNum/macStr %p/%p", macNum, macStr);
      return CMSRET_INVALID_ARGUMENTS;
   }    
   
   if (ctlUtl_isValidMacAddress(macStr) == TSL_B_FALSE)
   {
      return CMSRET_INVALID_PARAM_VALUE;
   }   
   
   if ((buf = (char *) calloc(MAC_STR_LEN+1, 1)) == NULL)
   {
      ctllog_error("alloc of %d bytes failed", MAC_STR_LEN+1);
      return CMSRET_RESOURCE_EXCEEDED;
   }

   /* need to copy since strtok_r updates string */
   strcpy(buf, macStr);

   /* Mac address has the following format
    * xx:xx:xx:xx:xx:xx where x is hex number 
    */
   pToken = strtok_r(buf, ":", &pLast);
   macNum[0] = (tsl_u8_t) strtol(pToken, (char **)NULL, 16);
   for (i = 1; i < MAC_ADDR_LEN; i++) 
   {
      pToken = strtok_r(NULL, ":", &pLast);
      macNum[i] = (tsl_u8_t) strtol(pToken, (char **)NULL, 16);
   }

   CTLMEM_FREE_BUF_AND_NULL_PTR(buf);

   return TSL_RV_SUC;
   
}

tsl_rv_t ctlUtl_macNumToStr(const tsl_u8_t *macNum, char *macStr) 
{
   if (macNum == NULL || macStr == NULL) 
   {
      ctllog_error("Invalid macNum/macStr %p/%p", macNum, macStr);
      return CMSRET_INVALID_ARGUMENTS;
   }  

   sprintf(macStr, "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
           (tsl_u8_t) macNum[0], (tsl_u8_t) macNum[1], (tsl_u8_t) macNum[2],
           (tsl_u8_t) macNum[3], (tsl_u8_t) macNum[4], (tsl_u8_t) macNum[5]);

   return TSL_RV_SUC;
}

tsl_rv_t cmsUtl_strtol(const char *str, char **endptr, tsl_32_t base, tsl_32_t *val)
{
   return(oal_strtol(str, endptr, base, val));
}


tsl_rv_t ctlUtl_strtoul(const char *str, char **endptr, tsl_32_t base, tsl_u32_t *val)
{
   return(oal_strtoul(str, endptr, base, val));
}


tsl_rv_t cmsUtl_strtol64(const char *str, char **endptr, tsl_32_t base, tsl_64_t *val)
{
   return(oal_strtol64(str, endptr, base, val));
}


tsl_rv_t cmsUtl_strtoul64(const char *str, char **endptr, tsl_32_t base, tsl_u64_t *val)
{
   return(oal_strtoul64(str, endptr, base, val));
}


void cmsUtl_strToLower(char *string)
{
   char *ptr = string;
   for (ptr = string; *ptr; ptr++)
   {
       *ptr = tolower(*ptr);
   }
}
tsl_rv_t cmsUtl_parseUrl(const char *url, UrlProto *proto, char **addr, tsl_u16_t *port, char **path)
{
   int n = 0;
   char *p = NULL;
   char protocol[BUFLEN_16];
   char host[BUFLEN_1024];
   char uri[BUFLEN_1024];

   if (url == NULL)
   {
      ctllog_debug("url is NULL");
      return CMSRET_INVALID_ARGUMENTS;
   }

  *port = 0;
   protocol[0] = host[0]  = uri[0] = '\0';

   /* proto */
   p = (char *) url;
   if ((p = strchr(url, ':')) == NULL) 
   {
      return CMSRET_INVALID_ARGUMENTS;
   }
   n = p - url;
   strncpy(protocol, url, n);
   protocol[n] = '\0';

   if (!strcmp(protocol, "http"))
   {
      *proto = URL_PROTO_HTTP;
   }
   else if (!strcmp(protocol, "https"))
   {
      *proto = URL_PROTO_HTTPS;
   }
   else if (!strcmp(protocol, "ftp"))
   {
      *proto = URL_PROTO_FTP;
   }
   else if (!strcmp(protocol, "tftp"))
   {
      *proto = URL_PROTO_TFTP;
   }
   else
   {
      ctllog_error("unrecognized proto in URL %s", url);
      return CMSRET_INVALID_ARGUMENTS;
   }

   /* skip "://" */
   if (*p++ != ':') return CMSRET_INVALID_ARGUMENTS;
   if (*p++ != '/') return CMSRET_INVALID_ARGUMENTS;
   if (*p++ != '/') return CMSRET_INVALID_ARGUMENTS;

   /* host */
   {
      char *pHost = host;
    
      while (*p && *p != ':' && *p != '/') 
      {
         *pHost++ = *p++;
      }
      *pHost = '\0';
   }
   if (strlen(host) != 0)
   {
      *addr = strdup(host);
   }
   else
   {
      ctllog_error("unrecognized host in URL %s", url);
      return CMSRET_INVALID_ARGUMENTS;
   }

   /* end */
   if (*p == '\0') 
   {
      *path = strdup("/");
       return TSL_RV_SUC;
   }

   /* port */
   if (*p == ':') 
   {
      char buf[BUFLEN_16];
      char *pBuf = buf;

      p++;
      while (isdigit(*p)) 
      {
         *pBuf++ = *p++;
      }
      *pBuf = '\0';
      if (strlen(buf) == 0)
      {
         CTLMEM_FREE_BUF_AND_NULL_PTR(*addr);
         ctllog_error("unrecognized port in URL %s", url);
         return CMSRET_INVALID_ARGUMENTS;
      }
      *port = atoi(buf);
   }
  
   /* path */
   if (*p == '/') 
   {
      char *pUri = uri;

      while ((*pUri++ = *p++));
      *path = strdup(uri);  
   }
   else
   {
      *path = strdup("/");
   }

   return TSL_RV_SUC;
}

tsl_rv_t cmsUtl_getBaseDir(char *pathBuf, tsl_u32_t pathBufLen)
{
   tsl_u32_t rc;

   rc = snprintf(pathBuf, pathBufLen, "/var");

   if (rc >= pathBufLen)
   {
      return CMSRET_RESOURCE_EXCEEDED;
   }

   return TSL_RV_SUC;
}
tsl_rv_t ctlUtl_parseDNS(const char *inDsnServers, char *outDnsPrimary, char *outDnsSecondary)
{
   tsl_int_t ret = TSL_RV_SUC;
   char *tmpBuf;
   char *separator;
   tsl_u32_t len;

   if (inDsnServers == NULL)
   {
      //return tsl_int_t_INVALID_ARGUMENTS;
      return TSL_RV_FAIL;
   }      
   

   //ctllog_debug("entered: DDNSservers=>%s<=", inDsnServers);

   if (outDnsPrimary)
   {
      strcpy(outDnsPrimary, "0.0.0.0");
   }

   if (outDnsSecondary)
   {
      strcpy(outDnsSecondary, "0.0.0.0");
   }
   
   len = strlen(inDsnServers);

   if ((tmpBuf = calloc(len+1, 1)) == NULL)
   {
      ctllog_error("alloc of %d bytes failed", len);
      //ret = tsl_int_t_INTERNAL_ERROR;
      ret = TSL_RV_FAIL;
   }
   else
   {
      sprintf(tmpBuf, "%s", inDsnServers);
      separator = strstr(tmpBuf, ",");
      if (separator != NULL)
      {
         /* break the string into two strings */
         *separator = 0;
         separator++;
         while ((isspace(*separator)) && (*separator != 0))
         {
            /* skip white space after comma */
            separator++;
         }

         if (outDnsSecondary != NULL)
         {
            if (ctlUtl_isValidIpv4Address(separator))
            {
               strcpy(outDnsSecondary, separator);
            }
            //ctllog_debug("dnsSecondary=%s", outDnsSecondary);
         }
      }

      if (outDnsPrimary != NULL)
      {
         if (ctlUtl_isValidIpv4Address(tmpBuf))
         {
            strcpy(outDnsPrimary, tmpBuf);
         }
         //ctllog_debug("dnsPrimary=%s", outDnsPrimary);
      }

      CTLMEM_FREE_BUF_AND_NULL_PTR(tmpBuf);
   }

   return ret;
   
}
tsl_32_t cmsUtl_syslogModeToNum(const char *modeStr)
{
   tsl_32_t mode=1;

   /*
    * These values are hard coded in httpd/html/logconfig.html.
    * Any changes to these values must also be reflected in that file.
    */
   if (!strcmp(modeStr, CTLVS_LOCAL_BUFFER))
   {
      mode = 1;
   }
   else if (!strcmp(modeStr, CTLVS_REMOTE))
   {
      mode = 2;
   }
   else if (!strcmp(modeStr, CTLVS_LOCAL_BUFFER_AND_REMOTE))
   {
      mode = 3;
   }
   else 
   {
      ctllog_error("unsupported mode string %s, default to mode=%d", modeStr, mode);
   }

   /*
    * The data model also specifies LOCAL_FILE and LOCAL_FILE_AND_REMOTE,
    * but its not clear if syslogd actually supports local file mode.
    */

   return mode;
}


char * cmsUtl_numToSyslogModeString(tsl_32_t mode)
{
   char *modeString = CTLVS_LOCAL_BUFFER;

   /*
    * These values are hard coded in httpd/html/logconfig.html.
    * Any changes to these values must also be reflected in that file.
    */
   switch(mode)
   {
   case 1:
      modeString = CTLVS_LOCAL_BUFFER;
      break;

   case 2:
      modeString = CTLVS_REMOTE;
      break;

   case 3:
      modeString = CTLVS_LOCAL_BUFFER_AND_REMOTE;
      break;

   default:
      ctllog_error("unsupported mode %d, default to %s", mode, modeString);
      break;
   }

   /*
    * The data model also specifies LOCAL_FILE and LOCAL_FILE_AND_REMOTE,
    * but its not clear if syslogd actually supports local file mode.
    */

   return modeString;
}


tsl_bool_t cmsUtl_isValidSyslogMode(const char * modeStr)
{
   tsl_u32_t mode;

   if (ctlUtl_strtoul(modeStr, NULL, 10, &mode) != TSL_RV_SUC) 
   {
      return TSL_B_FALSE;
   }

   return ((mode >= 1) && (mode <= 3));
}


tsl_32_t cmsUtl_syslogLevelToNum(const char *levelStr)
{
   tsl_32_t level=3; /* default all levels to error */

   /*
    * These values are from /usr/include/sys/syslog.h.
    */
   if (!strcmp(levelStr, CTLVS_EMERGENCY))
   {
      level = 0;
   }
   else if (!strcmp(levelStr, CTLVS_ALERT))
   {
      level = 1;
   }
   else if (!strcmp(levelStr, CTLVS_CRITICAL))
   {
      level = 2;
   }
   else if (!strcmp(levelStr, CTLVS_ERROR))
   {
      level = 3;
   }
   else if (!strcmp(levelStr, CTLVS_WARNING))
   {
      level = 4;
   }
   else if (!strcmp(levelStr, CTLVS_NOTICE))
   {
      level = 5;
   }
   else if (!strcmp(levelStr, CTLVS_INFORMATIONAL))
   {
      level = 6;
   }
   else if (!strcmp(levelStr, CTLVS_DEBUG))
   {
      level = 7;
   }
   else 
   {
      ctllog_error("unsupported level string %s, default to level=%d", levelStr, level);
   }

   return level;
}


char * cmsUtl_numToSyslogLevelString(tsl_32_t level)
{
   char *levelString = CTLVS_ERROR;

   /*
    * These values come from /usr/include/sys/syslog.h.
    */
   switch(level)
   {
   case 0:
      levelString = CTLVS_EMERGENCY;
      break;

   case 1:
      levelString = CTLVS_ALERT;
      break;

   case 2:
      levelString = CTLVS_CRITICAL;
      break;

   case 3:
      levelString = CTLVS_ERROR;
      break;

   case 4:
      levelString = CTLVS_WARNING;
      break;

   case 5:
      levelString = CTLVS_NOTICE;
      break;

   case 6:
      levelString = CTLVS_INFORMATIONAL;
      break;

   case 7:
      levelString = CTLVS_DEBUG;
      break;

   default:
      ctllog_error("unsupported level %d, default to %s", level, levelString);
      break;
   }

   return levelString;
}


tsl_bool_t cmsUtl_isValidSyslogLevel(const char *levelStr)
{
   tsl_u32_t level;

   if (ctlUtl_strtoul(levelStr, NULL, 10, &level) != TSL_RV_SUC) 
   {
      return TSL_B_FALSE;
   }

   return (level <= 7);
}


tsl_bool_t cmsUtl_isValidSyslogLevelString(const char *levelStr)
{
   if ((!strcmp(levelStr, CTLVS_EMERGENCY)) ||
       (!strcmp(levelStr, CTLVS_ALERT)) ||
       (!strcmp(levelStr, CTLVS_CRITICAL)) ||
       (!strcmp(levelStr, CTLVS_ERROR)) ||
       (!strcmp(levelStr, CTLVS_WARNING)) ||
       (!strcmp(levelStr, CTLVS_NOTICE)) ||
       (!strcmp(levelStr, CTLVS_INFORMATIONAL)) ||
       (!strcmp(levelStr, CTLVS_DEBUG)))
   {
      return TSL_B_TRUE;
   }
   else
   {
      return TSL_B_FALSE;
   }
}
tsl_32_t cmsUtl_pppAuthToNum(const char *authStr)
{
    tsl_32_t authNum = PPP_AUTH_METHOD_AUTO;  /* default is auto  */

    if (!strcmp(authStr, CTLVS_AUTO_AUTH))
    {
        authNum = PPP_AUTH_METHOD_AUTO;
    }
    else if (!strcmp(authStr, CTLVS_PAP))
    {      
        authNum = PPP_AUTH_METHOD_PAP;
    }
    else if (!strcmp(authStr, CTLVS_CHAP))
    {
        authNum = PPP_AUTH_METHOD_CHAP;
    }
    else if (!strcmp(authStr, CTLVS_MS_CHAP))
    {
        authNum = PPP_AUTH_METHOD_MSCHAP;
    }
    else
    {
        ctllog_error("unsupported auth string %s, default to auto=%d", authStr, authNum);
    }

    return authNum;

}
char * cmsUtl_numToPppAuthString(tsl_32_t authNum)
{
   char *authStr = CTLVS_AUTO_AUTH;   /* default to auto */

   switch(authNum)
   {
   case PPP_AUTH_METHOD_AUTO:
      authStr = CTLVS_AUTO_AUTH;
      break;

   case PPP_AUTH_METHOD_PAP:
      authStr = CTLVS_PAP;
      break;

   case PPP_AUTH_METHOD_CHAP:
      authStr = CTLVS_CHAP;
      break;

   case PPP_AUTH_METHOD_MSCHAP:
      authStr = CTLVS_MS_CHAP; 
      break;

   default:
      ctllog_error("unsupported authNum %d, default to %s", authNum, authStr);
      break;
   }

   return authStr;
   
}

tsl_bool_t ctlUtl_isValidIpAddress(tsl_32_t af, const char* address)
{
#ifdef DMP_X_ACTIONTEC_COM_IPV6_1 /* aka SUPPORT_IPV6 */
   if (af == AF_INET6)
   {
      struct in6_addr in6Addr;
      tsl_u32_t plen;
      char   addr[BUFLEN_40];

      if (ctlUtl_parsePrefixAddress(address, addr, &plen) != tsl_int_t_SUCCESS)
      {
         ctllog_error("Invalid ipv6 address=%s", address);
         return TSL_B_FALSE;
      }

      if (inet_pton(AF_INET6, addr, &in6Addr) <= 0)
      {
         ctllog_error("Invalid ipv6 address=%s", address);
         return TSL_B_FALSE;
      }

      if ( in6Addr.s6_addr32[0] == 0 && in6Addr.s6_addr32[1] == 0 && in6Addr.s6_addr32[2] == 0 &&
          (in6Addr.s6_addr32[3] == 0 || in6Addr.s6_addr32[3] == 1))
      {
         ctllog_error("Unspecified or loopback ipv6 address=%s", address);
         return TSL_B_FALSE;
      }

      return TSL_B_TRUE;
   }
   else
#endif
   {
      if (af == AF_INET)
      {
         return ctlUtl_isValidIpv4Address(address);
      }
      else
      {
         return TSL_B_FALSE;
      }
   }
}  /* End of ctlUtl_isValidIpAddress() */
tsl_bool_t ctlUtl_isValidIpv4Address(const char* input)
{
   tsl_bool_t ret = TSL_B_TRUE;
   char *pToken = NULL;
   char *pLast = NULL;
   char buf[BUFLEN_16];
   tsl_u32_t i, num;

   if (input == NULL || strlen(input) < 7 || strlen(input) > 15)
   {
      return TSL_B_FALSE;
   }

   /* need to copy since strtok_r updates string */
   strcpy(buf, input);

   /* IP address has the following format
      xxx.xxx.xxx.xxx where x is decimal number */
   pToken = strtok_r(buf, ".", &pLast);
   if ((ctlUtl_strtoul(pToken, NULL, 10, &num) != TSL_RV_SUC) ||
       (num > 255))
   {
      ret = TSL_B_FALSE;
   }
   else
   {
      for ( i = 0; i < 3; i++ )
      {
         pToken = strtok_r(NULL, ".", &pLast);

         if ( (NULL == pToken) || (ctlUtl_strtoul(pToken, NULL, 10, &num) != TSL_RV_SUC) ||
             (num > 255))
         {
            ret = TSL_B_FALSE;
            break;
         }
      }
   }

   return ret;
}
tsl_bool_t ctlUtl_isValidMacAddress(const char* input)
{
   tsl_bool_t ret =  TSL_B_TRUE;
   char *pToken = NULL;
   char *pLast = NULL;
   char buf[BUFLEN_32];
   tsl_u32_t i, num;

   if (input == NULL || strlen(input) != MAC_STR_LEN)
   {
      return TSL_B_FALSE;
   }

   /* need to copy since strtok_r updates string */
   strcpy(buf, input);

   /* Mac address has the following format
       xx:xx:xx:xx:xx:xx where x is hex number */
   pToken = strtok_r(buf, ":", &pLast);
   if ((strlen(pToken) != 2) ||
       (ctlUtl_strtoul(pToken, NULL, 16, &num) != TSL_RV_SUC))
   {
      ret = TSL_B_FALSE;
   }
   else
   {
      for ( i = 0; i < 5; i++ )
      {
         pToken = strtok_r(NULL, ":", &pLast);
         if ((strlen(pToken) != 2) ||
             (ctlUtl_strtoul(pToken, NULL, 16, &num) != TSL_RV_SUC))
         {
            ret = TSL_B_FALSE;
            break;
         }
      }
   }

   return ret;
}


tsl_bool_t cmsUtl_isValidPortNumber(const char * portNumberStr)
{
   tsl_u32_t portNum;

   if (ctlUtl_strtoul(portNumberStr, NULL, 10, &portNum) != TSL_RV_SUC) 
   {
      return TSL_B_FALSE;
   }

   return (portNum < (64 * 1024));
}


/* ACTIONTEC */
tsl_bool_t cmsUtl_getValidPortNumber(const char * portNumberStr, tsl_u32_t *portNum)
{
   if (ctlUtl_strtoul(portNumberStr, NULL, 10, portNum) != TSL_RV_SUC) 
   {
      return TSL_B_FALSE;
   }

   return ((*portNum) < (64 * 1024));
}
tsl_32_t cmsUtl_fwSecurityLevelToNum(const char *levelStr)
{
    tsl_32_t level=2;

    if (!tsl_strcmp(levelStr, CTLVS_LOW)) 
    {   
        level = 1;
    }   
    else if (!tsl_strcmp(levelStr, CTLVS_MEDIUM)) 
    {   
        level = 2;
    }   
    else if (!tsl_strcmp(levelStr, CTLVS_HIGH))
    {   
        level = 3;
    }   
    else 
    {   
        ctllog_error("unsupported level string %s, default to level=%d", levelStr, level);
    }   

    return level;
}
char * cmsUtl_numToFwSecurityLevelString(tsl_32_t level) 
{
    char *levelString = CTLVS_MEDIUM;                                                                             

    switch(level)
    {     
        case 1:
            levelString = CTLVS_LOW;      
            break;

        case 2:
            levelString = CTLVS_MEDIUM;
            break;

        case 3:
            levelString = CTLVS_HIGH;
            break;

        default:
            ctllog_error("unsupported level %d, default to %s", level, levelString);
            break;
    }

    return levelString;
}

ConnectionModeType cmsUtl_connectionModeStrToNum(const char *connModeStr)
{
    ConnectionModeType connMode = CMS_CONNECTION_MODE_DEFAULT;
    if (connModeStr == NULL)
    {
        ctllog_error("connModeStr is NULL");
        return connMode;
    }

    if (tsl_strcmp(connModeStr, CTLVS_VLANMUXMODE) == 0)
    {
        connMode = CMS_CONNECTION_MODE_VLANMUX;
    }
    return connMode;

}
/******************end porting by Michelle***************/

void rut_doSystemAction(const char* from, char *cmd)
{
   ctllog_debug("%s -- %s", from, cmd);
   if(strstr(cmd, "iptables") != NULL)
   {
       INFO_FW_SYSLOG("%s", cmd);
   }
#ifndef DESKTOP_LINUX   
   system(cmd);
#endif   
}

/** Free all the arg buffers in the argv, and the argv array itself.
 *
 */
void AEI_freeArgs(char **argv)
{
    int i = 0;

    while (argv[i] != NULL) {
        MEM_FREE_BUF_AND_NULL_PTR(argv[i]);
        i++;
    }

    MEM_FREE_BUF_AND_NULL_PTR(argv);
}

/** Give a single string, allocate and fill in an array of char *'s
 * each pointing to an individually malloc'd buffer containing a single
 * argument in the string; the array will end with a char * slot containing NULL.
 *
 * This array can be passed to execv.
 * This array must be freed by calling freeArgs.
 */
int AEI_parseArgs(const char *cmd, const char *args, char ***argv)
{
    int numArgs = 3, i, len, argIndex = 0;
    int inSpace = 1;
    const char *cmdStr;
    char **array;

    len = (args == NULL) ? 0 : strlen(args);

    /*
     * First count the number of spaces to determine the number of args
     * there are in the string.
     */
    for (i = 0; i < len; i++) {
        if ((args[i] == ' ') && (!inSpace)) {
            numArgs++;
            inSpace = 1;
        } else {
            inSpace = 0;
        }
    }

    array = (char **)malloc((numArgs) * sizeof(char *));
    if (array == NULL) {
        ctllog_debug("malloc of %d failed\n", numArgs);
        return -1;
    }

    memset(array, 0x0, (numArgs) * sizeof(char *));
    /* locate the command name, last part of string */
    cmdStr = strrchr(cmd, '/');
    if (cmdStr == NULL) {
        cmdStr = cmd;
    } else {
        cmdStr++;               /* move past the / */
    }

    /* copy the command into argv[0] */
    array[argIndex] = malloc(strlen(cmdStr) + 1);
    if (array[argIndex] == NULL) {
        ctllog_debug("%malloc of %d failed\n", strlen(cmdStr) + 1);
        AEI_freeArgs(array);
        return -1;
    } else {
        strcpy(array[argIndex], cmdStr);
        argIndex++;
    }

    /*
     * Wow, this is going to be incredibly messy.  I have to malloc a buffer
     * for each arg and copy them in individually.
     */
    inSpace = 1;
    for (i = 0; i < len; i++) {
        if ((args[i] == ' ') && (!inSpace)) {
            numArgs++;
            inSpace = 1;
        } else if ((args[i] != ' ') && (inSpace)) {
            int startIndex, endIndex;

            /*
             * this is the first character I've seen after a space.
             * Figure out how many letters this arg is, malloc a buffer
             * to hold it, and copy it into the buffer.
             */
            startIndex = i;
            endIndex = i;
            while ((endIndex < len) && (args[endIndex] != ' ')) {
                endIndex++;
            }

            array[argIndex] = malloc(endIndex - startIndex + 1);
            memset(array[argIndex], 0, endIndex - startIndex + 1);
            if (array[argIndex] == NULL) {
                ctllog_debug("malloc of %d failed\n", endIndex - startIndex + 1);
                AEI_freeArgs(array);
                return -1;
            }

            memcpy(array[argIndex], &args[startIndex], endIndex - startIndex);

            ctllog_debug("index=%d len=%d (%s)\n", argIndex, endIndex - startIndex,
                    &args[startIndex]);

            argIndex++;

            inSpace = 0;
        }
    }

    /* check what we did */
    i = 0;
    while (array[i] != NULL) {
        ctllog_debug("argv[%d] = %s\n", i, array[i]);
        i++;
    }

    (*argv) = array;

    return 0;
}

void AEI_close_open_fds()
{
    long fd, maxfd;
    char fdpath[PATH_MAX], *endp;
    struct dirent *dent;
    DIR *dirp;
    int len;

    /* Check for a /proc/$$/fd directory. */
    len = snprintf(fdpath, sizeof(fdpath), "/proc/%ld/fd", (long)getpid());
    if (len > 0 && (size_t) len <= sizeof(fdpath) && (dirp = opendir(fdpath))) {

        while ((dent = readdir(dirp)) != NULL) {

            fd = strtol(dent->d_name, &endp, 10);

            if (dent->d_name != endp && *endp == '\0' && fd != dirfd(dirp) && fd > 2) {
                (void)close((int)fd);
            }
        }
        (void)closedir(dirp);
    }
}

void AEI_tsl_child_dupfds(char *filename)
{
    int dev_null_fd, i;
    int fd = 0;
    dev_null_fd = open("/dev/null", O_RDWR);

    //Coverity CID 11917/11918: Resource leak
    if(dev_null_fd < 0){
        return;
    }

    if (filename != NULL)
        fd = open(filename, O_RDWR | O_CREAT | O_TRUNC);

    if (fd < 0) {
        close(dev_null_fd);
        return;
    }

    for (i = 0; i < 3; i++) {
        if (fd > 0) {
            dup2(fd, i);
            continue;
        }
        dup2(dev_null_fd, i);
    }
    if (fd > 0)
        close(fd);
    close(dev_null_fd);
}

int AEI_tsl_system(char *p_path, char *p_argv)
{
    pid_t pid;
    int ret;
    char **argv = NULL;
    int status;


    if ((ret = AEI_parseArgs(p_path, p_argv, &argv)) != 0) {
        ctllog_debug(">>>>>>>>>>>> parseArgs error\n");
        return ret;
    }

    ctllog_debug("p_path = %s , p_argv = %s\n", p_path, p_argv);

    pid = fork();
    if (pid == 0) {
        signal(SIGCHLD, SIG_IGN);
        if (fork() == 0) {
            AEI_tsl_child_dupfds(NULL);
            execv(p_path, argv);
            ctllog_debug("fork failed!!!!!\n");
            exit(-1);
        }
        exit(0);
    }

    AEI_freeArgs(argv);
    wait(&status);
    ctllog_debug("\nlaunch pid:%d \n", pid);
    return 0;
}

int AEI_tsl_system_pwd(char *p_path, char *p_argv, char *filename, char *fin_file)
{
    pid_t pid;
    int ret;
    char **argv = NULL;
    int status;
    char cmd[BUFLEN_128] = { 0 };


    if ((ret = AEI_parseArgs(p_path, p_argv, &argv)) != 0) {
        ctllog_debug("==>>>>>>>>>>>> parseArgs error\n");
        return ret;
    }

    ctllog_debug("p_path = %s , p_argv = %s\n", p_path, p_argv);

    pid = fork();
    if (pid == 0) {
        signal(SIGCHLD, SIG_IGN);
        if (fork() == 0) {
            if (fork() == 0) {
                AEI_close_open_fds();   // ex stdin stdout stderr
                AEI_tsl_child_dupfds(filename);
                execv(p_path, argv);
                ctllog_debug("fork failed!!!!!\n");
                exit(-1);
            }
            AEI_close_open_fds();
            wait(&status);
            sprintf(cmd, "(echo > %s)&", fin_file);
            system(cmd);
        }
        exit(0);
    }

    AEI_freeArgs(argv);

    wait(&status);

    ctllog_debug("\n%d \n", pid);

    return 0;
}


