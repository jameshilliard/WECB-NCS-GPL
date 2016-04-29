#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>     /* for isDigit, really should be in oal_strconv.c */
#include <sys/stat.h>  /* this really should be in oal_strconv.c */
#include <arpa/inet.h> /* for inet_aton */


#include "tsl_common.h"
#include "ctl_log.h"
//#include "ctl_util.h"
//#include "oal.h"

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

char *tsl_strncpy_n(char *dest, const char *src, tsl_int_t dlen, size_t n)
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
        /*ignore the return value*/
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
