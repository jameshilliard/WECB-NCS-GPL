#ifndef __ACT_RLOGIN_H
#define __ACT_RLOGIN_H

#define SSHD_NAME    "dropbear"

tsl_rv_t stop_rlogin( _RemoteLoginObject *p_cur_data, _RemoteLoginObject *p_new_data );
tsl_rv_t start_rlogin( _RemoteLoginObject *p_cur_data, _RemoteLoginObject *p_new_data );
tsl_rv_t restart_rlogin( _RemoteLoginObject *p_cur_data, _RemoteLoginObject *p_new_data );
tsl_void_t triggerZconfExt( tsl_void_t );
tsl_bool_t rlogin_changed( _RemoteLoginObject *p_cur_data, _RemoteLoginObject *p_new_data );

#endif

