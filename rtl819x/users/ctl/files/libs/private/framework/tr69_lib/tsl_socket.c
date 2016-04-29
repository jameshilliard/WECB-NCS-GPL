/**************************************************************************
 *
 *	版权:	
 *              
 *
 *	文件名:	
 *              tsl_socket.c
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
 *              
 *		
 **************************************************************************/
#include "tsl_common.h"
#include "tsl_socket.h"

/**************************************************************************
 *
 *              TCP/IP功能函数实现
 *
 *************************************************************************/

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
                              IN tsl_int_t  port)
{
	struct sockaddr_in 	address;
	tsl_int_t 		handle = -1;
//Coverity comment 17837
	memset(&address,0,sizeof(struct sockaddr_in));

#ifdef _WIN32
	WORD    dVer;
	WSADATA dData;
	dVer = MAKEWORD(2, 2);
	WSAStartup(dVer, &dData);
#endif 
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(ip);
	address.sin_port = htons((unsigned short)port);

	handle = socket(AF_INET, SOCK_STREAM, 0);

	//Coverity CID 16835: Argument cannot be negative 
	if(handle == -1){
		return TSL_RV_ERR;
	}
    
	if( connect(handle, (struct sockaddr *)&address, sizeof(address)) == -1) {
		//Coverity ID: 17280,Resource leak, need close socket here
		close(handle);
		return TSL_RV_ERR;
	}else {
		return handle;
	}
}

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
tsl_int_t tsl_socket_local_open(tsl_char_t *p_sockfile)
{
  	static struct sockaddr_un s_un;
	tsl_int_t                 sock_id;

	memset(&s_un, 0, sizeof(s_un));
	s_un.sun_family = AF_LOCAL;
        sprintf(s_un.sun_path, p_sockfile);
	//Coverity CID 16115: Side effect in assertion (ASSERT_SIDE_EFFECT)	
	sock_id = socket(AF_LOCAL, SOCK_STREAM, 0);
       TSL_FVASSERT(sock_id != -1);
	//Coverity ID:17278, Resource leak
	TSL_FVASSERT_ACT((connect(sock_id, (struct sockaddr *)&s_un, sizeof(s_un)) >= 0), close(sock_id));
	
	return sock_id;
}


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
tsl_rv tsl_socket_tcp_close(IN tsl_int_t  socket_id)
{
	return close(socket_id);
}

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
tsl_int_t tsl_socket_tcp_bind(tsl_int_t port)
{
	static struct sockaddr_in s_in;
	tsl_int_t sock_id;
	tsl_int_t opt_val   = 1;

	memset(&s_in, 0, sizeof(s_in));
	s_in.sin_family = AF_INET;

	s_in.sin_addr.s_addr = INADDR_ANY;
	s_in.sin_port = htons(port);

	//Coverity CID 16116: Side effect in assertion (ASSERT_SIDE_EFFECT) 
	sock_id = socket(AF_INET, SOCK_STREAM, 0);
       TSL_FVASSERT(sock_id != -1);

	//Coverity ID: 17279,  Resource leak 
	TSL_FVASSERT_ACT((setsockopt(sock_id, SOL_SOCKET, SO_REUSEADDR,  (char *)&opt_val, sizeof(int)) >= 0), close(sock_id));
	TSL_FVASSERT_ACT((bind(sock_id, (struct sockaddr *)&s_in, sizeof(s_in)) >= 0), close(sock_id));
	TSL_FVASSERT_ACT((listen(sock_id, 64 ) >= 0), close(sock_id));

	return sock_id;
}

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
tsl_int_t tsl_socket_local_bind(tsl_char_t *p_sockfile)
{
  	static struct sockaddr_un s_un;
	tsl_int_t                 sock_id;
	tsl_int_t                 opt_val   = 1;

	memset(&s_un, 0, sizeof(s_un));
	s_un.sun_family = AF_LOCAL;
        sprintf(s_un.sun_path, p_sockfile);
        
        unlink(p_sockfile);

	//Coverity CID 16114: Side effect in assertion (ASSERT_SIDE_EFFECT)
	sock_id = socket(AF_LOCAL, SOCK_STREAM, 0);
	TSL_FVASSERT(sock_id != -1);

	//Coverity CID 17277: Resource leak (RESOURCE_LEAK)
	TSL_FVASSERT_ACT((setsockopt(sock_id, SOL_SOCKET, SO_REUSEADDR,  (char *)&opt_val, sizeof(int)) >= 0), close(sock_id));
	TSL_FVASSERT_ACT((bind(sock_id, (struct sockaddr *)&s_un, sizeof(s_un)) >= 0), close(sock_id));
	TSL_FVASSERT_ACT((listen(sock_id, 64 ) >= 0), close(sock_id));
	return sock_id;
}


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
 *                        成功
 *              TSL_RV_ERR 失败
 *
**************************************************************************/
tsl_int_t tsl_socket_tcp_write(tsl_int_t sock_id, 
                               tsl_char_t *buf, 
                               tsl_int_t buf_size,
                               tsl_int_t flag)
{
        tsl_int_t       size = 0;
        tsl_int_t 	res  = 0;

        TSL_VASSERT(sock_id > 0);
	TSL_VASSERT(buf != NULL);
	TSL_VASSERT(buf_size > 0);
	
	while(size < buf_size){
        	res = send(sock_id, (char *)buf + size, buf_size - size, 0);
   		if (res <= 0){
			return -1;
		}
        	size += res;
		if (!flag){
			return size;
		}
        }
	
	return size;
	
}


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
                              INOUT tsl_char_t *buf, 
                              IN    tsl_int_t buf_size,
                              IN    tsl_int_t flag)
{
	tsl_int_t 	size = 0;
	tsl_int_t 	res  = 0;

        TSL_VASSERT(sock_id > 0);
	TSL_VASSERT(buf != NULL);
	TSL_VASSERT(buf_size > 0);
	
	while(size < buf_size){
		res = recv(sock_id, (char *)buf + size, buf_size - size, 0); 
        	if(res <= 0 ){
			return -1;
		}
                
		size += res;
		if (!flag){
			return size;
		}
	}
	
        return size;
}
