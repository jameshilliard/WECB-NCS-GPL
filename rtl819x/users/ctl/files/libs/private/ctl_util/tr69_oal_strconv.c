#include "tr69_oal.h"
tsl_rv_t oal_strtol(const char *str, char **endptr, tsl_32_t base, tsl_32_t *val)
{
   tsl_rv_t ret=TSL_RV_SUC;
   char *localEndPtr=NULL;

   errno = 0;  /* set to 0 so we can detect ERANGE */

   *val = strtol(str, &localEndPtr, base);

   if ((errno != 0) || (*localEndPtr != '\0'))
   {
      *val = 0;
      ret = CMSRET_INVALID_ARGUMENTS;
   }

   if (endptr != NULL)
   {
      *endptr = localEndPtr;
   }

   return ret;
}

tsl_int_t oal_strtoul(const char *str, char **endptr, tsl_32_t base, tsl_u32_t *val)
{
   char *localEndPtr=NULL;

   /*
    * Linux strtoul allows a minus sign in front of the number.
    * This seems wrong to me.  Specifically check for this and reject
    * such strings.
    */
   while (isspace(*str))
   {
      str++;
   }
   if (*str == '-')
   {
      if (endptr)
      {
         *endptr = (char *) str;
      }
      *val = 0;
      //return CMSRET_INVALID_ARGUMENTS;
        return TSL_RV_FAIL;
   }

   errno = 0;  /* set to 0 so we can detect ERANGE */

   *val = strtoul(str, &localEndPtr, base);

   if ((errno != 0) || (*localEndPtr != '\0'))
   {
      *val = 0;
      //ret = CMSRET_INVALID_ARGUMENTS;
      return TSL_RV_FAIL;
   }

   if (endptr != NULL)
   {
      *endptr = localEndPtr;
   }

   return TSL_RV_SUC;
}
tsl_rv_t oal_strtol64(const char *str, char **endptr, tsl_32_t base, tsl_64_t *val)
{
   tsl_rv_t ret=TSL_RV_SUC;
   char *localEndPtr=NULL;

   errno = 0;  /* set to 0 so we can detect ERANGE */

   *val = strtoll(str, &localEndPtr, base);

   if ((errno != 0) || (*localEndPtr != '\0'))
   {
      *val = 0;
      ret = CMSRET_INVALID_ARGUMENTS;
   }

   if (endptr != NULL)
   {
      *endptr = localEndPtr;
   }

   return ret;
}


tsl_rv_t oal_strtoul64(const char *str, char **endptr, tsl_32_t base, tsl_u64_t *val)
{
   tsl_rv_t ret=TSL_RV_SUC;
   char *localEndPtr=NULL;

   /*
    * Linux strtoul allows a minus sign in front of the number.
    * This seems wrong to me.  Specifically check for this and reject
    * such strings.
    */
   while (isspace(*str))
   {
      str++;
   }
   if (*str == '-')
   {
      if (endptr)
      {
         *endptr = (char *) str;
      }
      *val = 0;
      return CMSRET_INVALID_ARGUMENTS;
   }

   errno = 0;  /* set to 0 so we can detect ERANGE */

   *val = strtoull(str, &localEndPtr, base);

   if ((errno != 0) || (*localEndPtr != '\0'))
   {
      *val = 0;
      ret = CMSRET_INVALID_ARGUMENTS;
   }

   if (endptr != NULL)
   {
      *endptr = localEndPtr;
   }

   return ret;
}
