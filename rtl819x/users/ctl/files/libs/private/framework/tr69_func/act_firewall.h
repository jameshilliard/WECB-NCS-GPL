#ifndef __ACT_FIREWALL_H
#define __ACT_FIREWALL_H


#include "tr69_func_common.h"

#ifdef AEI_FIREWALL
#define FWI_ADD_ACCEPT_RULE(protocal, dport) \
        DO_SYSTEM( "iptables -A INPUT -p %s --dport %d -j ACCEPT", protocal, dport )
#define FWI_DEL_ACCEPT_RULE(protocal, dport) \
        DO_SYSTEM( "iptables -D INPUT -p %s --dport %d -j ACCEPT", protocal, dport )
#else
#define FWI_ADD_ACCEPT_RULE(protocal, dport)
#define FWI_DEL_ACCEPT_RULE(protocal, dport)
#endif

#endif

