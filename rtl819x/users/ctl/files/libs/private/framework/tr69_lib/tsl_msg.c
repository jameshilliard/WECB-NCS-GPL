#include "tsl_msg.h"
#include "tsl_common.h"
#include "tsl_list.h"
#include "tsl_socket.h"

tsl_rv_t tsl_msg_client_send(tsl_char_t *p_sock_file, tsl_int_t event)
{
        tsl_int_t g_sock = -1;
        
        TSL_VASSERT(p_sock_file != NULL);

	//Coverity CID 16112: Side effect in assertion (ASSERT_SIDE_EFFECT) 
	g_sock = tsl_socket_local_open(p_sock_file);
       TSL_VASSERT(g_sock !=-1);

	//Coverity CID: 17276 Resource leak
	TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&event, sizeof(event), 1) > 0), tsl_socket_tcp_close(g_sock));
        tsl_socket_tcp_close(g_sock);
 
        return TSL_RV_SUC;
}

tsl_msg_t *tsl_msg_server_create(tsl_char_t *p_sock_file, tsl_msg_func cb_func)
{
        tsl_msg_t *p_msg = NULL;

        TSL_MALLOC_RV(p_msg, tsl_msg_t, NULL);
        
        sprintf(p_msg->sock_file, "%s", p_sock_file);
        p_msg->cb_func = cb_func;
        p_msg->quit = 0;
        
        return p_msg;
}

tsl_void_t tsl_msg_server_cleanup(tsl_msg_t *p_msg)
{
        p_msg->quit = 1;
        tsl_socket_tcp_close(p_msg->master_id);
        sleep(3);
        TSL_FREE(p_msg);
}

tsl_rv_t tsl_msg_server_service(tsl_msg_t *p_msg)
{
        tsl_int_t               m_socket_id = -1;
	tsl_int_t 		c_socket_id = -1;
        struct sockaddr_in  	c_sin ;
	tsl_int_t		c_sin_len   = 0;
        fd_set                  set;
        tsl_int_t               sid = 0;

	//Coverity CID 16113: Side effect in assertion (ASSERT_SIDE_EFFECT) 
	 m_socket_id = tsl_socket_local_bind(p_msg->sock_file);
        TSL_FVASSERT_ACT( (m_socket_id>= 0), tsl_printf("please set ipaddress as \"192.168.0.2\"\n");exit(-1));
        p_msg->master_id = m_socket_id;

        while(!p_msg->quit){
                FD_ZERO(&set);
                FD_SET(m_socket_id, &set);
                tsl_printf("msg selecting...\n");

		  /* coverity #16304 */
                if (select(m_socket_id+1, &set, NULL, NULL, NULL) > 0)  	{                
	                if (FD_ISSET(m_socket_id, &set)){
	                        tsl_printf("accepit....%d\n", m_socket_id);
	                        c_socket_id = accept(m_socket_id, (struct sockaddr *)&c_sin, (socklen_t *)&c_sin_len);
	                        
	                        if (c_socket_id < 0){
	                            continue;
	                        }else if (p_msg->quit != 0){
	                                break;
	                        }

				   /* Coverity #16305 */
	                        if (tsl_socket_tcp_read(c_socket_id,  (tsl_char_t*)&sid, sizeof(sid), 1) > 0) {  
		                        if (p_msg->cb_func){
		                                p_msg->cb_func(c_socket_id, sid);
		                        }
	                        }

	                        tsl_socket_tcp_close(c_socket_id);
	                }
                }
				
        }
        
        return TSL_RV_SUC;
}

