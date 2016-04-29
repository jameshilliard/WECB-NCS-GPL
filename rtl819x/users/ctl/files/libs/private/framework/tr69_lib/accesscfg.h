#ifndef _ACCESS_CFG_H
#define _ACCESS_CFG_H

#include "ctl_msg.h"
#include "dbussend_msg.h"
#include "dbus_define.h"

#ifndef	SUPPORT_PK5000
#define _ZIP_CTL_LAYER_CFG
#endif

void cfgtest(void);
tsl_rv do_cfg_access( DBusConnection *connection, DBusMessage *message, void *user_data, CtlMsgHeader * p_msg,
		const tsl_char_t * methodType, tsl_bool bMethodCall );
tsl_rv cfg_save();

//////////////////////////////////////////////////////////////////////
// special implementation for diff platform
#if !defined(SUPPORT_BHR3) && !defined(SUPPORT_REVJ)
tsl_rv cfg_write_data( tsl_char_t * pStrData );
tsl_rv cfg_read_data(tsl_char_t ** ppStrData );
#endif
//
//////////////////////////////////////////////////////////////////////

#endif //_ACCESS_CFG_H

