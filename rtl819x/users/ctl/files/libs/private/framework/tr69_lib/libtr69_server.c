#include "tsl_common.h"
#include "tsl_socket.h"
#include "libtr69.h"
#include "libtr69_imp.h"
#include "tsl_msg.h"
#include "ctl_log.h"
#include "tr69_tree.h"
#include "accesscfg.h"

tsl_int_t server_tr69_get_leaf_data(tsl_int_t sock)
{
	tsl_int_t  name_size;
	tsl_int_t  data_size = 0;
	tsl_char_t name[256] = "\0";
	tsl_void_t *p_data = NULL;
	tsl_int_t  ret = -1;
	tsl_int_t  type = 0;


	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&name_size, sizeof(name_size), 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)name, name_size, 1) > 0);

	ret = tr69_get_leaf_data(name, &p_data, &type);

	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)&ret, sizeof(ret), 1) > 0);
	TSL_VASSERT(ret == TSL_RV_SUC);

	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)&type, sizeof(type), 1) > 0);

	if (type == TR69_NODE_LEAF_TYPE_STRING || type == TR69_NODE_LEAF_TYPE_DATE){
		if (p_data != NULL){
			data_size = strlen((tsl_char_t*)p_data) + 1;
			TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)&data_size, sizeof(data_size), 1) > 0);
			TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)p_data, data_size, 1) > 0);

			TSL_FREE(p_data);
		}else {
			data_size = 0;
			TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)&data_size, sizeof(data_size), 1) > 0);
		}
	}else {
		data_size = sizeof(int);
		TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)&data_size, sizeof(data_size), 1) > 0);
		TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)&p_data, data_size, 1) > 0);
	}

	return TSL_RV_SUC;
}

tsl_int_t server_tr69_get_unfresh_leaf_data(tsl_int_t sock)
{
	tsl_int_t  name_size;
	tsl_int_t  data_size = 0;
	tsl_char_t name[256] = "\0";
	tsl_void_t *p_data = NULL;
	tsl_int_t  ret = -1;
	tsl_int_t  type = 0;


	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&name_size, sizeof(name_size), 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)name, name_size, 1) > 0);

	ret = tr69_get_unfresh_leaf_data(name, &p_data, &type);

	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)&ret, sizeof(ret), 1) > 0);
	TSL_VASSERT(ret == TSL_RV_SUC);

	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)&type, sizeof(type), 1) > 0);

	if (type == TR69_NODE_LEAF_TYPE_STRING || type == TR69_NODE_LEAF_TYPE_DATE){
		if (p_data != NULL){
			data_size = strlen((tsl_char_t*)p_data) + 1;

			TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)&data_size, sizeof(data_size), 1) > 0);
			TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)p_data, data_size, 1) > 0);

			TSL_FREE(p_data);
		}else {
			data_size = 0;
			TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)&data_size, sizeof(data_size), 1) > 0);
		}
	}else {
		data_size = sizeof(int);

		TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)&data_size, sizeof(data_size), 1) > 0);
		TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)&p_data, data_size, 1) > 0);
	}

	return TSL_RV_SUC;
}

tsl_int_t server_tr69_set_leaf_data(tsl_int_t sock)
{
	tsl_int_t  name_size;
	tsl_int_t  data_size = 0;
	tsl_char_t name[256] = "\0";
	tsl_void_t *p_data = NULL;
	tsl_int_t  data = 0;
	tsl_int_t  ret = -1;
	tsl_int_t  type = 0;


	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&name_size, sizeof(name_size), 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)name, name_size, 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&type, sizeof(type), 1) > 0);

	if (type == TR69_NODE_LEAF_TYPE_STRING || type == TR69_NODE_LEAF_TYPE_DATE){
		TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&data_size, sizeof(data_size), 1) > 0);
		TSL_SMALLOC(p_data, char, data_size);
		TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)p_data, data_size, 1) > 0);
		ret = tr69_set_leaf_data(name, p_data, type);

		TSL_FREE(p_data);
	}else {
		TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&data_size, sizeof(data_size), 1) > 0);
		TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&data, data_size, 1) > 0);
		ret = tr69_set_leaf_data(name, &data, type);
	}


	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)&ret, sizeof(ret), 1) > 0);
	TSL_VASSERT(ret == TSL_RV_SUC);

	return TSL_RV_SUC;
}

tsl_int_t server_tr69_commit_set_leaf_data(tsl_int_t sock)
{
	tsl_int_t  name_size;
	tsl_int_t  data_size = 0;
	tsl_char_t name[256] = "\0";
	tsl_void_t *p_data = NULL;
	tsl_int_t  data = 0;
	tsl_int_t  ret = -1;
	tsl_int_t  type = 0;


	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&name_size, sizeof(name_size), 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)name, name_size, 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&type, sizeof(type), 1) > 0);

	if (type == TR69_NODE_LEAF_TYPE_STRING || type == TR69_NODE_LEAF_TYPE_DATE){
		TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&data_size, sizeof(data_size), 1) > 0);
		TSL_SMALLOC(p_data, char, data_size);
		TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)p_data, data_size, 1) > 0);
		ret = tr69_commit_set_leaf_data(name, p_data, type);

		TSL_FREE(p_data);
	}else {
		TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&data_size, sizeof(data_size), 1) > 0);
		TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&data, data_size, 1) > 0);
		ret = tr69_commit_set_leaf_data(name, &data, type);
	}


	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)&ret, sizeof(ret), 1) > 0);
	TSL_VASSERT(ret == TSL_RV_SUC);

	return TSL_RV_SUC;
}

tsl_int_t server_tr69_commit_end(tsl_int_t sock)
{
	tsl_int_t  name_size;
	tsl_char_t name[256] = "\0";
	tsl_int_t  ret = -1;


	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&name_size, sizeof(name_size), 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)name, name_size, 1) > 0);

	ret = tr69_commit_set_leaf_data_end(name);

	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)&ret, sizeof(ret), 1) > 0);
	TSL_VASSERT(ret == TSL_RV_SUC);

	return TSL_RV_SUC;
}

tsl_int_t server_tr69_set_unfresh_leaf_data(tsl_int_t sock)
{
	tsl_int_t  name_size;
	tsl_int_t  data_size = 0;
	tsl_char_t name[256] = "\0";
	tsl_void_t *p_data = NULL;
	tsl_int_t  data = 0;
	tsl_int_t  ret = -1;
	tsl_int_t  type = 0;


	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&name_size, sizeof(name_size), 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)name, name_size, 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&type, sizeof(type), 1) > 0);

	if (type == TR69_NODE_LEAF_TYPE_STRING || type == TR69_NODE_LEAF_TYPE_DATE){
		TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&data_size, sizeof(data_size), 1) > 0);
		TSL_SMALLOC(p_data, char, data_size);
		TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)p_data, data_size, 1) > 0);
		ret = tr69_set_unfresh_leaf_data(name, p_data, type);

		TSL_FREE(p_data);
	}else {
		TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&data_size, sizeof(data_size), 1) > 0);
		TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&data, data_size, 1) > 0);
		ret = tr69_set_unfresh_leaf_data(name, &data, type);
	}


	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)&ret, sizeof(ret), 1) > 0);
	TSL_VASSERT(ret == TSL_RV_SUC);

	return TSL_RV_SUC;
}


tsl_int_t server_tr69_acl_set_leaf_data(tsl_int_t sock)
{
	tsl_int_t  name_size;
	tsl_int_t  data_size = 0;
	tsl_char_t name[256] = "\0";
	tsl_void_t *p_data = NULL;
	tsl_int_t  data = 0;
	tsl_int_t  ret = -1;
	tsl_int_t  type = 0;


	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&name_size, sizeof(name_size), 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)name, name_size, 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&type, sizeof(type), 1) > 0);

	if (type == TR69_NODE_LEAF_TYPE_STRING || type == TR69_NODE_LEAF_TYPE_DATE){
		TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&data_size, sizeof(data_size), 1) > 0);
		TSL_SMALLOC(p_data, char, data_size);
		TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)p_data, data_size, 1) > 0);
		ret = tr69_acl_set_leaf_data(name, p_data, type);

		TSL_FREE(p_data);
	}else {
		TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&data_size, sizeof(data_size), 1) > 0);
		TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&data, data_size, 1) > 0);
		ret = tr69_acl_set_leaf_data(name, &data, type);
	}


	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)&ret, sizeof(ret), 1) > 0);
	TSL_VASSERT(ret == TSL_RV_SUC);

	return TSL_RV_SUC;
}


tsl_int_t server_tr69_get_node(tsl_int_t sock)
{
	tsl_int_t  name_size;
	tsl_char_t name[256] = "\0";
	tr69_node_t *p_data = NULL;
	tsl_int_t  ret = -1;


	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&name_size, sizeof(name_size), 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)name, name_size, 1) > 0);

	ret = tr69_get_node(name, &p_data);

	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)&ret, sizeof(ret), 1) > 0);
	TSL_VASSERT(ret == TSL_RV_SUC);

	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)p_data, sizeof(tr69_node_t), 1) > 0);
	TSL_FREE(p_data);

	return TSL_RV_SUC;
}

tsl_int_t server_tr69_get_next_node(tsl_int_t sock)
{
	tsl_int_t  name_size;
	tsl_char_t name[256] = "\0";
	tsl_int_t data_size = 0;
	tr69_node_t *p_data = NULL;
	tsl_int_t  ret = -1;


	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&name_size, sizeof(name_size), 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)name, name_size, 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&data_size, sizeof(data_size), 1) > 0);

	if (data_size){
		TSL_MALLOC(p_data, tr69_node_t);
		TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)p_data, sizeof(tr69_node_t), 1) > 0);
	}

	ret = tr69_get_next_node(name, &p_data);

	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)&ret, sizeof(ret), 1) > 0);
	TSL_VASSERT(ret == TSL_RV_SUC);

	printf("full_name:%s\n", p_data->full_name);

	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)p_data, sizeof(tr69_node_t), 1) > 0);
	TSL_FREE(p_data);

	return TSL_RV_SUC;
}

tsl_int_t server_tr69_get_next_node2(tsl_int_t sock)
{
	tsl_int_t name_size = 0;
	tsl_char_t name[256] = "\0";
	tsl_int_t data_size = 0;
	tr69_node_t *p_data = NULL;
	tsl_int_t ret = -1;

	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t *)&name_size, sizeof(name_size), 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t *)name, name_size, 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t *)&data_size, sizeof(data_size), 1) > 0);

	if (data_size) {
		TSL_MALLOC(p_data, tr69_node_t);
		TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t *)p_data, sizeof(tr69_node_t), 1) > 0);
	}

	ret = tr69_get_next_node2(name, &p_data);

	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t *)&ret, sizeof(ret), 1) > 0);
	TSL_VASSERT(ret == TSL_RV_SUC);

	printf("full_name:%s\n", p_data->full_name);
	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t *)p_data, sizeof(tr69_node_t), 1) > 0);
	TSL_FREE(p_data);

	return TSL_RV_SUC;
}

tsl_int_t server_tr69_get_next_child_node(tsl_int_t sock)
{
	tsl_int_t name_size;
	tsl_char_t name[256] = "\0";
	tsl_int_t data_size = 0;
	tr69_node_t *p_data = NULL;
	tsl_int_t ret = -1;

	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t *)&name_size, sizeof(name_size), 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t *)name, name_size, 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t *)&data_size, sizeof(data_size), 1) > 0);

	if (data_size){
		TSL_MALLOC(p_data, tr69_node_t);
		TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t *)p_data, sizeof(tr69_node_t), 1) > 0);
	}
	ret = tr69_get_next_child_node(name, &p_data);

	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t *)&ret, sizeof(ret), 1) > 0);
	TSL_VASSERT(ret == TSL_RV_SUC);

	printf("full_name:%s\n", p_data->full_name);

	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t *)p_data, sizeof(tr69_node_t), 1) > 0);
	TSL_FREE(p_data);

	return TSL_RV_SUC;
}

tsl_int_t server_tr69_get_next_obj_node(tsl_int_t sock)
{
	tsl_int_t  name_size = 0;
	tsl_char_t name[256] = "\0";
	tsl_int_t data_size = 0;
	tr69_node_t *p_data = NULL;
	tsl_int_t  ret = -1;


	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&name_size, sizeof(name_size), 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)name, name_size, 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&data_size, sizeof(data_size), 1) > 0);

	if (data_size){
		TSL_MALLOC(p_data, tr69_node_t);
		TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)p_data, sizeof(tr69_node_t), 1) > 0);
	}

	ret = tr69_get_next_obj_node(name, &p_data);

	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)&ret, sizeof(ret), 1) > 0);
	TSL_VASSERT(ret == TSL_RV_SUC);

	printf("full_name:%s\n", p_data->full_name);

	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)p_data, sizeof(tr69_node_t), 1) > 0);
	TSL_FREE(p_data);

	return TSL_RV_SUC;
}

tsl_int_t server_tr69_get_next_inst_node(tsl_int_t sock)
{
	tsl_int_t name_size = 0;
	tsl_char_t name[256] = "\0";
	tsl_int_t data_size = 0;
	tr69_node_t *p_data = NULL;
	tsl_int_t ret = -1;

	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t *)&name_size, sizeof(name_size), 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t *)name, name_size, 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t *)&data_size, sizeof(data_size), 1) > 0);

	if (data_size){
		TSL_MALLOC(p_data, tr69_node_t);
		TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t *)p_data, sizeof(tr69_node_t), 1) > 0);
	}
	ret = tr69_get_next_inst_node(name, &p_data);

	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t *)&ret, sizeof(ret), 1) > 0);
	TSL_VASSERT(ret == TSL_RV_SUC);

	printf("full_name:%s\n", p_data->full_name);

	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t *)p_data, sizeof(tr69_node_t), 1) > 0);
	TSL_FREE(p_data);

	return TSL_RV_SUC;
}

tsl_int_t server_tr69_get_next_samedepth_node(tsl_int_t sock)
{
	tsl_int_t name_size = 0;
	tsl_char_t name[256] = "\0";
	tsl_int_t data_size = 0;
	tr69_node_t *p_data = NULL;
	tsl_int_t ret = -1;

	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t *)&name_size, sizeof(name_size), 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t *)name, name_size, 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t *)&data_size, sizeof(data_size), 1) > 0);

	if (data_size){
		TSL_MALLOC(p_data, tr69_node_t);
		TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t *)p_data, data_size, 1) > 0);
	}

	ret = tr69_get_next_samedepth_node(name, &p_data);

	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t *)&ret, sizeof(ret), 1) > 0);
	TSL_VASSERT(ret == TSL_RV_SUC);

	printf("full_name:%s\n", p_data->full_name);

	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t *)p_data, sizeof(tr69_node_t), 1) > 0);
	TSL_FREE(p_data);

	return TSL_RV_SUC;
}

tsl_int_t server_tr69_get_obj_data_size(tsl_int_t sock)
{
	tsl_int_t name_size = 0;
	tsl_char_t name[256] = "\0";
	tsl_int_t obj_data_size = 0;
	tsl_int_t ret = -1;

	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t *)&name_size, sizeof(name_size), 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t *)name, name_size, 1) > 0);

	ret = tr69_get_obj_data_size(name, &obj_data_size);

	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t *)&ret, sizeof(ret), 1) > 0);
	TSL_VASSERT(ret == TSL_RV_SUC);

	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t *)&obj_data_size, sizeof(obj_data_size), 1) > 0);

	return TSL_RV_SUC;
}

tsl_int_t server_tr69_set_node(tsl_int_t sock)
{
	tsl_int_t  name_size;
	tsl_char_t name[256] = "\0";
	tsl_int_t  ret = -1;
	tr69_node_t node;


	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&name_size, sizeof(name_size), 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)name, name_size, 1) > 0);

	memset(&node, 0, sizeof(tr69_node_t));
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&node, sizeof(tr69_node_t), 1) > 0);
	ret = tr69_set_node(name, &node);

	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)&ret, sizeof(ret), 1) > 0);
	TSL_VASSERT(ret == TSL_RV_SUC);

	return TSL_RV_SUC;
}

tsl_int_t server_tr69_add_instance(tsl_int_t sock)
{
	tsl_int_t  name_size;
	tsl_char_t name[256] = "\0";
	tsl_int_t  ret = -1;
	tsl_int_t  inst_numb = 0;

	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&name_size, sizeof(name_size), 1) > 0);

	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)name, name_size, 1) > 0);

	ret = tr69_add_instance(name, &inst_numb);

	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)&ret, sizeof(ret), 1) > 0);
	TSL_VASSERT(ret == TSL_RV_SUC);

	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)&inst_numb, sizeof(inst_numb), 1) > 0);

	return TSL_RV_SUC;
}

tsl_int_t server_tr69_del2_instance(tsl_int_t sock)
{
	tsl_int_t  name_size;
	tsl_char_t name[256] = "\0";
	tsl_int_t  ret = -1;


	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&name_size, sizeof(name_size), 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)name, name_size, 1) > 0);

	ret = tr69_del2_instance(name);

	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)&ret, sizeof(ret), 1) > 0);
	TSL_VASSERT(ret == TSL_RV_SUC);

	return TSL_RV_SUC;
}

tsl_int_t server_tr69_del_instance(tsl_int_t sock)
{
	tsl_int_t  name_size;
	tsl_char_t name[256] = "\0";
	tsl_int_t  ret = -1;
	tsl_int_t  inst = 0;

	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&name_size, sizeof(name_size), 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)name, name_size, 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&inst, sizeof(inst), 1) > 0);

	ret = tr69_del_instance(name, inst);

	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)&ret, sizeof(ret), 1) > 0);
	TSL_VASSERT(ret == TSL_RV_SUC);

	return TSL_RV_SUC;
}

tsl_int_t server_tr69_restore_default_node(tsl_int_t sock)
{
	tsl_int_t  name_size;
	tsl_char_t name[256] = "\0";
	tsl_int_t  ret = -1;


	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&name_size, sizeof(name_size), 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)name, name_size, 1) > 0);

	ret = tr69_restore_default_node(name);

	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)&ret, sizeof(ret), 1) > 0);
	TSL_VASSERT(ret == TSL_RV_SUC);

	return TSL_RV_SUC;
}

tsl_int_t server_tr69_move_instance(tsl_int_t sock)
{
	tsl_int_t name_dst_len = 0;
	tsl_char_t name_dst[MAX_NAME_SIZE] = "\0";
	tsl_int_t name_src_len = 0;
	tsl_char_t name_src[MAX_NAME_SIZE] = "\0";
	tsl_int_t inst_numb = -1;
	tsl_int_t  ret = -1;

	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&name_dst_len, sizeof(name_dst_len), 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)name_dst, name_dst_len, 1) > 0);

	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)&name_src_len, sizeof(name_src_len), 1) > 0);
	TSL_VASSERT(tsl_socket_tcp_read(sock, (tsl_char_t*)name_src, name_src_len, 1) > 0);

	ret = tr69_move_instance(name_dst, name_src, &inst_numb);
	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)&ret, sizeof(ret), 1) > 0);
	TSL_VASSERT(ret == TSL_RV_SUC);

	TSL_VASSERT(tsl_socket_tcp_write(sock, (tsl_char_t*)&inst_numb, sizeof(inst_numb), 1) > 0);

	return TSL_RV_SUC;
}

tsl_int_t server_tr69_get_notification(tsl_int_t sock)
{
	tsl_int_t  ret = -1;
	tr69_notification_t *p_notf_tb=NULL;
	tsl_int_t tb_count = 0;

	ret = tr69_get_notification(&p_notf_tb, &tb_count);
	TSL_VASSERT_ACT((tsl_socket_tcp_write(sock, (tsl_char_t*)&ret, sizeof(ret), 1) > 0), if(p_notf_tb){TSL_FREE(p_notf_tb);});
	TSL_VASSERT_ACT((ret == TSL_RV_SUC), if(p_notf_tb){TSL_FREE(p_notf_tb);});
	TSL_VASSERT_ACT((tsl_socket_tcp_write(sock, (tsl_char_t*)&tb_count, sizeof(tb_count), 1) > 0), if(p_notf_tb){TSL_FREE(p_notf_tb);});

	if (tb_count){
		TSL_VASSERT_ACT((tsl_socket_tcp_write(sock, (tsl_char_t*)p_notf_tb, sizeof(tr69_notification_t) * tb_count, 1) > 0), if(p_notf_tb){TSL_FREE(p_notf_tb);});
	}

	if(p_notf_tb){
		TSL_FREE(p_notf_tb);
	}

	return TSL_RV_SUC;
}

tsl_rv_t tr69_inline_del_instance_timeout(tsl_int_t def_wait_timeout);
tsl_rv_t tr69_inline_clear_instance_timeout();
tsl_rv_t tr69_inline_clear_notification();

tsl_int_t m_socket_id = -1;
tsl_int_t g_quit = 0;

#ifdef TR69_SEM_MUTEX
sem_t global_sem;
#endif



tsl_void_t libtr69_server_stop()
{
	tr69_inline_clear_notification();
	tr69_inline_clear_instance_timeout();
	g_quit = 1;
	tsl_socket_tcp_close(m_socket_id);
}


#define PID_FILE_NAME "/var/data_center.pid"
static void write_pid_file()
{
	FILE* fd = fopen(PID_FILE_NAME, "w");
	if (!fd)
		return;

	fprintf(fd, "%d", getpid());
	fclose(fd);
}

static void unlink_pid_file()
{
	unlink(PID_FILE_NAME);
}

tsl_rv libtr69_server(tsl_char_t *p_save_xml, tsl_int_t def_wait_timeout)
{
	tsl_int_t 		c_socket_id = -1;
	struct sockaddr_in  	c_sin ;
	tsl_int_t		c_sin_len   = 0;
	fd_set                  set;
	tsl_int_t               sid = 0;
	struct timeval          tv;
	tsl_uint_t              wait_timeout = def_wait_timeout;
	tsl_rv                  rv = 0;
	tsl_int_t   timeout_reason = 0;

	//Coverity CID 16111: Side effect in assertion (ASSERT_SIDE_EFFECT)
#ifdef TR69_SOCK_PORT
	TSL_FVASSERT_ACT((m_socket_id = tsl_socket_tcp_bind(TR69_SERVER_PORT)) >= 0, ctllog_debug("please set ipaddress as \"192.168.0.2\"\n");exit(-1));
#else
	TSL_FVASSERT_ACT((m_socket_id = tsl_socket_local_bind(TR69_SERVER_FILE)) >= 0, ctllog_debug("please set ipaddress as \"192.168.0.2\"\n");exit(-1));
#endif

	wait_timeout = tr69_inline_del_instance_timeout(def_wait_timeout);

#ifdef TR69_SEM_MUTEX
	sem_init(&global_sem, 0, 1);
#endif

	write_pid_file();

	while(!g_quit){
		FD_ZERO(&set);
		FD_SET(m_socket_id, &set);

		tv.tv_sec = wait_timeout;
		tv.tv_usec = 0;
		printf("selecting... %u\n", (tsl_uint_t)tv.tv_sec);

		rv = select(m_socket_id+1, &set, NULL, NULL, &tv);

		if (FD_ISSET(m_socket_id, &set)){
			printf("accepit....%d\n", rv);
			c_socket_id = accept(m_socket_id, (struct sockaddr *)&c_sin, (socklen_t *)&c_sin_len);

			if (c_socket_id < 0){
				continue;
			}else if (g_quit != 0){
				break;
			}

#ifdef TR69_SEM_MUTEX
			sem_wait(&global_sem);
#endif

			tsl_socket_tcp_read(c_socket_id,
					(tsl_char_t*)&sid,
					sizeof(sid),
					1);

			switch(sid){
				case TR69_FUNC_GET_LEAF_DATA:
					server_tr69_get_leaf_data(c_socket_id);
					break;
				case TR69_FUNC_GET_UNFRESH_LEAF_DATA:
					server_tr69_get_unfresh_leaf_data(c_socket_id);
					break;
				case TR69_FUNC_SET_LEAF_DATA:
					server_tr69_set_leaf_data(c_socket_id);
					break;
				case TR69_FUNC_SET_UNFRESH_LEAF_DATA:
					server_tr69_set_unfresh_leaf_data(c_socket_id);
					break;
				case TR69_FUNC_ACL_SET_LEAF_DATA:
					server_tr69_acl_set_leaf_data(c_socket_id);
					break;
				case TR69_FUNC_GET_NODE:
					server_tr69_get_node(c_socket_id);
					break;
				case TR69_FUNC_GET_NEXT_NODE:
					server_tr69_get_next_node(c_socket_id);
					break;
				case TR69_FUNC_GET_NEXT_NODE2:
					server_tr69_get_next_node2(c_socket_id);
					break;
				case TR69_FUNC_GET_NEXT_CHILD_NODE:
					server_tr69_get_next_child_node(c_socket_id);
					break;
				case TR69_FUNC_GET_NEXT_OBJ_NODE:
					server_tr69_get_next_obj_node(c_socket_id);
					break;
				case TR69_FUNC_GET_NEXT_INST_NODE:
					server_tr69_get_next_inst_node(c_socket_id);
					break;
				case TR69_FUNC_GET_SAME_DEPTH_NODE:
					server_tr69_get_next_samedepth_node(c_socket_id);
					break;
				case TR69_FUNC_GET_OBJ_DATA_SIZE:
					server_tr69_get_obj_data_size(c_socket_id);
					break;
				case TR69_FUNC_SET_NODE:
					server_tr69_set_node(c_socket_id);
					break;
				case TR69_FUNC_ADD_INSTANCE:
					server_tr69_add_instance(c_socket_id);
					break;
				case TR69_FUNC_DEL_INSTANCE:
					server_tr69_del_instance(c_socket_id);
					break;
				case TR69_FUNC_DEL2_INSTANCE:
					server_tr69_del2_instance(c_socket_id);
					break;
				case TR69_FUNC_GET_NOTIFICATION:
					server_tr69_get_notification(c_socket_id);
					break;
				case TR69_FUNC_SAVE_NOW:
					tsl_socket_tcp_close(c_socket_id);
#ifdef TR69_SEM_MUTEX
					sem_post(&global_sem);
#endif
					goto save_now;

				case TR69_FUNC_COMMIT_SET_LEAF_DATA:
					server_tr69_commit_set_leaf_data(c_socket_id);
					break;

				case TR69_FUNC_COMMIT_END:
					server_tr69_commit_end(c_socket_id);
					break;

				case TR69_FUNC_RESTORE_DEFUALT_NODE:
					server_tr69_restore_default_node(c_socket_id);
					break;

				case TR69_FUNC_MOVE_INSTANCE:
					server_tr69_move_instance(c_socket_id);
					break;
				case TR69_FUNC_CLEAR_NOTIFICATION:
					tr69_inline_clear_notification();
					break;
				default:
					break;
			}

			tsl_socket_tcp_close(c_socket_id);
			wait_timeout = tr69_inline_timeout(def_wait_timeout, &timeout_reason);

#ifdef TR69_SEM_MUTEX
			sem_post(&global_sem);
#endif
		}
		else if (timeout_reason == TIMEOUT_REASON_DELAYNOTIF)
		{
			wait_timeout = tr69_inline_timeout(def_wait_timeout, &timeout_reason);
		}else {

save_now:

#ifdef TR69_SEM_MUTEX
			sem_wait(&global_sem);
#endif
			wait_timeout = tr69_inline_timeout(def_wait_timeout, &timeout_reason);

			if (tr69_save_xml(p_save_xml) == TSL_RV_SUC)
			{
#ifdef AEI_STORAGE_CFG_BY_DC
				cfg_save();
#endif
			}

#ifdef TR69_SEM_MUTEX
			sem_post(&global_sem);
#endif
		}
	}

	unlink_pid_file();
	return TSL_RV_SUC;

}

