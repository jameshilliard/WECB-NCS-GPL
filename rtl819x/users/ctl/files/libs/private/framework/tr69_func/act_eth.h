#ifndef __ACT_ETH_H__
#define __ACT_ETH_H__

#include "tsl_common.h"

#define CTL_UPDATE_ETH_INFO_MIN_INTERNAL     (5) // in seconds

tsl_rv_t GetEthMACAddr( const tsl_char_t *name, tsl_char_t * mac, tsl_int_t size );


#endif

