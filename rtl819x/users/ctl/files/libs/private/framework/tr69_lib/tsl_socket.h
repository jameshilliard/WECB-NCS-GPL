/**************************************************************************
 *
 *	版权:	
 *              
 *
 *	文件名:	
 *              tsl_socket.h
 *
 *	作者:	
 *              祝红亮 
 *
 *	功能描述:	
 *		
 *            
 *	函数列表:	
 *             
 *              
 *      历史:
 *              $Id: $ 
 *		
 **************************************************************************/
#ifndef TSL_SOCKET_H
#define TSL_SOCKET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "tsl_common.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>

/**************************************************************************
 *	[函数名]:
 *	        tsl_socket_tcp_open
 *
 *	[函数功能]:
 *              开启TCP/IP连接
 *
 *	[函数参数]
 *  		IN tsl_char_t ip    对方IP地址
 *              IN tsl_int_t  port  对方端口号
 *
 *	[函数返回值]
 *              >=1       TCP/IP连接句柄
 *              TSL_RV_ERR 失败
 *
 *************************************************************************/
tsl_int_t tsl_socket_tcp_open(IN tsl_char_t *ip,
                              IN tsl_int_t  port);

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tsl_socket_local_open
 *
 *	[DESCRIPTION]:
 *              Open AF_LOCAL socket
 *
 *	[ARGUMENT]
 *  		IN tsl_int_t port 
 *            
 *	[RETURN]
 *              >=1          SUCCESS
 *              TSL_RV_ERR   ERROR
 *
**************************************************************************/
tsl_int_t tsl_socket_local_open(tsl_char_t *p_sockfile);

/**************************************************************************
 *	[函数名]:
 *	        tsl_socket_tcp_close
 *
 *	[函数功能]:
 *              关闭TCP/IP连接
 *
 *	[函数参数]
 *  		IN tsl_int_t  socket_id 
 *
 *	[函数返回值]
 *              TSL_RV_SUC 成功
 *              TSL_RV_ERR 失败
 *
 *************************************************************************/
tsl_rv tsl_socket_tcp_close(IN tsl_int_t  socket_id);


/**************************************************************************
 *	[函数名]:
 *	        tsl_socket_tcp_bind
 *
 *	[函数功能]:
 *              监听TCP/IP端口
 *
 *	[函数参数]
 *  		IN tsl_int_t port 端口号
 *            
 *	[函数返回值]
 *              >=1         返回SOCKET句柄
 *              TSL_RV_ERR   失败
 *
**************************************************************************/
tsl_int_t tsl_socket_tcp_bind(tsl_int_t port);


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tsl_socket_local_bind
 *
 *	[DESCRIPTION]:
 *              Bind AF_LOCAL socket
 *
 *	[ARGUMENT]
 *  		IN tsl_int_t port 
 *            
 *	[RETURN]
 *              >=1          SUCCESS
 *              TSL_RV_ERR   ERROR
 *
**************************************************************************/
tsl_int_t tsl_socket_local_bind(tsl_char_t *p_sockfile);

/**************************************************************************
 *	[函数名]:
 *	        tsl_socket_tcp_write
 *
 *	[函数功能]:
 *              写TCP/IP数据
 *
 *	[函数参数]
 *  		IN tsl_int_t  socket_id   SOCKET句柄
 *              IN tsl_char_t *p_buf      传送字符串
 *              IN tsl_int_t  buf_size    字符串长度
 *              IN tsl_int_t  flag        1 表示循环读写直到满足长度 0表示只读写
 * 
 *	[函数返回值]
 *              >= 1       写入数据长度
 *              TSL_RV_ERR  失败
 *
**************************************************************************/
tsl_int_t tsl_socket_tcp_write(IN tsl_int_t sock_id, 
                               IN tsl_char_t *p_buf, 
                               IN tsl_int_t buf_size,
                               IN tsl_int_t flag);
        

/**************************************************************************
 *	[函数名]:
 *	        tsl_socket_tcp_read
 *
 *	[函数功能]:
 *              读取TCP/IP数据
 *
 *	[函数参数]
 *  		IN    tsl_int_t  socket_id   SOCKET句柄
 *              INOUT tsl_char_t *p_buf      读取字符串
 *              IN    tsl_int_t  buf_size    字符串长度
 *              IN tsl_int_t  flag           1 表示循环读写直到满足长度 0表示只读写
 *            
 *	[函数返回值]
 *              >= 1       读取数据长度
 *              TSL_RV_ERR  失败
 *
**************************************************************************/
tsl_int_t tsl_socket_tcp_read(IN    tsl_int_t sock_id,
                              INOUT tsl_char_t *p_buf, 
                              IN    tsl_int_t buf_size,
                              IN    tsl_int_t flag);
	
	
#ifdef __cplusplus
}
#endif

#endif






