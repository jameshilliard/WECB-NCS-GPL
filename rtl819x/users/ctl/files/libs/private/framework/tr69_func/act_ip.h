#ifndef __ACT_IP_H__
#define __ACT_IP_H__

#include "tsl_common.h"

#define CTL_UPDATE_IP_INFO_MIN_INTERNAL     (5) // in seconds
#define CTL_QUERY_IP_INFO_MAX_NUMB          (32)

tsl_uint_t updateIPv6AddressInstUnderIPIntf( tsl_char_t * oid );

#endif

