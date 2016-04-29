#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>     /* for isDigit, really should be in oal_strconv.c */
#include <sys/stat.h>  /* this really should be in oal_strconv.c */
#include <arpa/inet.h> /* for inet_aton */
#include "tsl_common.h"
#include "ctl_log.h"

tsl_int_t tsl_strcmp(const char *s1, const char *s2) 
{
   char emptyStr = '\0';
   char *str1 = (char *) s1;
   char *str2 = (char *) s2;

   if (str1 == NULL)
   {
      str1 = &emptyStr;
   }
   if (str2 == NULL)
   {
      str2 = &emptyStr;
   }

   return strcmp(str1, str2);
}


tsl_int_t tsl_strcasecmp(const char *s1, const char *s2) 
{
   char emptyStr = '\0';
   char *str1 = (char *) s1;
   char *str2 = (char *) s2;

   if (str1 == NULL)
   {
      str1 = &emptyStr;
   }
   if (str2 == NULL)
   {
      str2 = &emptyStr;
   }

   return strcasecmp(str1, str2);
}


tsl_int_t tsl_strncmp(const char *s1, const char *s2, tsl_int_t n) 
{
   char emptyStr = '\0';
   char *str1 = (char *) s1;
   char *str2 = (char *) s2;

   if (str1 == NULL)
   {
      str1 = &emptyStr;
   }
   if (str2 == NULL)
   {
      str2 = &emptyStr;
   }

   return strncmp(str1, str2, n);
}


tsl_int_t tsl_strncasecmp(const char *s1, const char *s2, tsl_int_t n) 
{
   char emptyStr = '\0';
   char *str1 = (char *) s1;
   char *str2 = (char *) s2;

   if (str1 == NULL)
   {
      str1 = &emptyStr;
   }
   if (str2 == NULL)
   {
      str2 = &emptyStr;
   }

   return strncasecmp(str1, str2, n);
}


char *tsl_strstr(const char *s1, const char *s2) 
{
   char emptyStr = '\0';
   char *str1 = (char *)s1;
   char *str2 = (char *)s2;

   if (str1 == NULL)
   {
      str1 = &emptyStr;
   }
   if (str2 == NULL)
   {
      str2 = &emptyStr;
   }

   return strstr(str1, str2);
}

char *tsl_strncpy(char *dest, const char *src, tsl_int_t dlen)
{

    if((dest == NULL))
    {
        return NULL;
    }	

    if(src == NULL)
    {
        dest[0]='\0';
        return dest; 
    }

    if( strlen(src)+1 > dlen )
    {
        ctllog_notice("truncating:src string length > dest buffer");
        strncpy(dest,src,dlen-1);
        dest[dlen-1] ='\0';
    }
    else
    {
        strcpy(dest,src);
    }

    return dest;
} 

char *tsl_strncpy_n(char *dest, const char *src, tsl_int_t  dlen, size_t n)
{
    if(dest == NULL){
        return NULL;
    }

    if(src == NULL){
        dest[0]='\0';
        return dest;
    }

    if( n+1 > dlen ){
        strncpy(dest,src,dlen-1);
        dest[dlen-1] ='\0';
    }else{
        strncpy(dest,src,n);
        dest[n]='\0';
    }

    return dest;
}

tsl_int_t tsl_strlen(const char *src)
{
   char emptyStr = '\0';
   char *str = (char *)src;
   
   if(src == NULL)
   {
      str = &emptyStr;
   }	

   return strlen(str);
} 

char *tsl_strncat(char *dest, const char *src, tsl_int_t dst_len)
{
    size_t len;
    int rst;
    if(dest == NULL || src == NULL)
    {
        return dest;
    }

    len=strlen(dest);
    if(dst_len>len)
    {
        rst = snprintf(dest+len, dst_len-len, "%s", src);
    }

    return dest;
}

char* tsl_strncat_n(char *dest, const char *src, tsl_int_t dst_len, size_t n)
{
    if(dest == NULL || src == NULL)
    {
        return dest;
    }

    size_t len = strlen(dest);
    int j = dst_len - len;
    if(j>0)
    {
        if(j <= n)
        {
            snprintf(dest+len, j, "%s", src);
        }
        else
        {
            snprintf(dest+len, n+1, "%s", src);
        }
    }

    return dest;
}

tsl_bool_t cmsUtl_isValidMacAddress(const char* input)
{
   tsl_bool_t ret =  TSL_B_TRUE;
   char *pToken = NULL;
   char *pLast = NULL;
   char buf[32];
   tsl_int_t i, num;

#define MAC_STR_LEN 17
   if (input == NULL || strlen(input) != MAC_STR_LEN)
   {
      return TSL_B_FALSE;
   }

   /* need to copy since strtok_r updates string */
   strcpy(buf, input);

   /* Mac address has the following format
       xx:xx:xx:xx:xx:xx where x is hex number */
   pToken = strtok_r(buf, ":", &pLast);
   if ((strlen(pToken) != 2) )
   {
      ret = TSL_B_FALSE;
   }
   else
   {
      for ( i = 0; i < 5; i++ )
      {
         pToken = strtok_r(NULL, ":", &pLast);
         if ((strlen(pToken) != 2))
         {
            ret = TSL_B_FALSE;
            break;
         }
      }
   }

   return ret;
}

tsl_bool_t cmsUtl_macStrToNum(const char *macStr, unsigned char *macNum)
{
   char *pToken = NULL;
   char *pLast = NULL;
   char *buf;
   tsl_int_t i;
   
   if (macNum == NULL || macStr == NULL) 
   {
      ctllog_error("Invalid macNum/macStr %p/%p", macNum, macStr);
      return TSL_B_FALSE;
   }    
   
   if (cmsUtl_isValidMacAddress(macStr) == TSL_B_FALSE)
   {
      return TSL_B_FALSE;
   }   
   
#define MAC_STR_LEN 17
#define MAC_ADDR_LEN 6
   if ((buf = (char *) calloc(MAC_STR_LEN+1, 1)) == NULL)
   {
      ctllog_error("alloc of %d bytes failed", MAC_STR_LEN+1);
      return TSL_B_FALSE;
   }

   /* need to copy since strtok_r updates string */
   strcpy(buf, macStr);

   /* Mac address has the following format
    * xx:xx:xx:xx:xx:xx where x is hex number 
    */
   pToken = strtok_r(buf, ":", &pLast);
   macNum[0] = (unsigned char) strtol(pToken, (char **)NULL, 16);
   for (i = 1; i < MAC_ADDR_LEN; i++) 
   {
      pToken = strtok_r(NULL, ":", &pLast);
      macNum[i] = (unsigned char) strtol(pToken, (char **)NULL, 16);
   }

  free(buf);

   return TSL_B_TRUE;
   
}
