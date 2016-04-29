#include "tsl_common.h"
#include "tsl_socket.h"
#include "libtr69_client.h"
#include "libtr69_imp.h"

tsl_int_t tr69_get_leaf_data(tsl_char_t *p_name, tsl_void_t **pp_data, tsl_int_t *type)
{
    tsl_int_t g_sock = -1;
    tsl_int_t sid = TR69_FUNC_GET_LEAF_DATA;
    tsl_int_t name_size;
    tsl_int_t data_size;
    tsl_int_t rtype;
    tsl_void_t *p_data = NULL;
    tsl_int_t data = 0;
    tsl_int_t ret = -1;

    TSL_VASSERT(p_name != NULL);
    TSL_VASSERT(pp_data != NULL);
    TSL_VASSERT(type != NULL);
    ////Coverity CID 16102: Side effect in assertion (ASSERT_SIDE_EFFECT)
#ifdef TR69_SOCK_PORT
    g_sock = tsl_socket_tcp_open(TR69_SERVER_IP, TR69_SERVER_PORT);
#else        
    g_sock = tsl_socket_local_open(TR69_SERVER_FILE);
#endif
    TSL_VASSERT(g_sock >= 0);

   
    //Coverity CID 17265: Resource leak
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&sid, sizeof(sid), 1) > 0), tsl_socket_tcp_close(g_sock));

    name_size = strlen(p_name);
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&name_size, sizeof(name_size), 1) > 0), tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_name, name_size, 1) > 0), tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&ret, sizeof(ret), 1) > 0), tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_RV_ACT(ret == TSL_RV_SUC, ret, tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&rtype, sizeof(rtype), 1) > 0), tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&data_size, sizeof(data_size), 1) > 0), tsl_socket_tcp_close(g_sock));

    *type = rtype;

    if (rtype == TR69_NODE_LEAF_TYPE_STRING || rtype == TR69_NODE_LEAF_TYPE_DATE){
        if (data_size != 0){
            TSL_SMALLOC_RV_ACT(p_data, char, data_size, TSL_RV_FAIL_MEM, tsl_socket_tcp_close(g_sock));
            TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)p_data, data_size, 1) > 0), if(p_data){TSL_FREE(p_data);}; *pp_data = NULL;tsl_socket_tcp_close(g_sock));
            *pp_data = p_data;
        }else {
            *pp_data = NULL;
        }
    }else {
        TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&data, data_size, 1) > 0), tsl_socket_tcp_close(g_sock));
        *pp_data = (void *)data;
    }

    tsl_socket_tcp_close(g_sock);

    return ret;
}


tsl_int_t tr69_get_unfresh_leaf_data(tsl_char_t *p_name, tsl_void_t **pp_data, tsl_int_t *type)
{
    tsl_int_t g_sock = -1;
    tsl_int_t sid = TR69_FUNC_GET_UNFRESH_LEAF_DATA;
    tsl_int_t name_size;
    tsl_int_t data_size;
    tsl_int_t rtype;
    tsl_void_t *p_data = NULL;
    tsl_int_t data = 0;
    tsl_int_t ret = -1;


    TSL_VASSERT(p_name != NULL);
    TSL_VASSERT(pp_data != NULL);
    TSL_VASSERT(type != NULL);

    //Coverity CID 16106: Side effect in assertion (ASSERT_SIDE_EFFECT)
#ifdef TR69_SOCK_PORT
    g_sock = tsl_socket_tcp_open(TR69_SERVER_IP, TR69_SERVER_PORT);
#else        
    g_sock = tsl_socket_local_open(TR69_SERVER_FILE);
#endif
    TSL_VASSERT(g_sock >= 0);

        
    //Coverity CID 17270: Resource leak 
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&sid, sizeof(sid), 1) > 0), tsl_socket_tcp_close(g_sock));

    name_size = strlen(p_name);
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&name_size, sizeof(name_size), 1) > 0), tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_name, name_size, 1) > 0), tsl_socket_tcp_close(g_sock));	

    TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&ret, sizeof(ret), 1) > 0), tsl_socket_tcp_close(g_sock)); 
    TSL_VASSERT_RV_ACT(ret == TSL_RV_SUC, ret, tsl_socket_tcp_close(g_sock));

    TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&rtype, sizeof(rtype), 1) > 0), tsl_socket_tcp_close(g_sock)); 
    TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&data_size, sizeof(data_size), 1) > 0), tsl_socket_tcp_close(g_sock));

    *type = rtype;

    if (rtype == TR69_NODE_LEAF_TYPE_STRING || rtype == TR69_NODE_LEAF_TYPE_DATE){
    if (data_size != 0){
        TSL_SMALLOC_RV_ACT(p_data, char, data_size, TSL_RV_FAIL_MEM, tsl_socket_tcp_close(g_sock));
        TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)p_data, data_size, 1) > 0), if(p_data){TSL_FREE(p_data);}; *pp_data = NULL;tsl_socket_tcp_close(g_sock));
        *pp_data = p_data;
    }else {
        *pp_data = NULL;
    }
    }else {
        TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&data, data_size, 1) > 0), tsl_socket_tcp_close(g_sock));
        *pp_data = (void *)data;
    }

    tsl_socket_tcp_close(g_sock);

    return ret;
}


tsl_int_t tr69_set_leaf_data(tsl_char_t *p_name, tsl_void_t *p_data, tsl_int_t type)
{
    tsl_int_t g_sock = -1;
    tsl_int_t sid = TR69_FUNC_SET_LEAF_DATA;
    tsl_int_t name_size;
    tsl_int_t data_size;
    tsl_int_t ret = -1;

    TSL_VASSERT(p_name != NULL);
    TSL_VASSERT(p_data != NULL);
    TSL_VASSERT(type == TR69_NODE_LEAF_TYPE_STRING || 
        type == TR69_NODE_LEAF_TYPE_INT    ||
        type == TR69_NODE_LEAF_TYPE_UINT   ||
        type == TR69_NODE_LEAF_TYPE_BOOL   ||
        type == TR69_NODE_LEAF_TYPE_DATE);
    //Coverity CID 16108: Side effect in assertion (ASSERT_SIDE_EFFECT) 
#ifdef TR69_SOCK_PORT
    g_sock = tsl_socket_tcp_open(TR69_SERVER_IP, TR69_SERVER_PORT);
#else        
    g_sock = tsl_socket_local_open(TR69_SERVER_FILE);
#endif
    TSL_VASSERT(g_sock >= 0);

    //Coverity CID 17272: Resource leak
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&sid, sizeof(sid), 1) > 0), tsl_socket_tcp_close(g_sock));

    name_size = strlen(p_name);
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&name_size, sizeof(name_size), 1) > 0), tsl_socket_tcp_close(g_sock));

    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_name, name_size, 1) > 0), tsl_socket_tcp_close(g_sock));

    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&type, sizeof(type), 1) > 0), tsl_socket_tcp_close(g_sock)); 


    if (type == TR69_NODE_LEAF_TYPE_STRING || type == TR69_NODE_LEAF_TYPE_DATE){
        data_size = strlen(p_data) + 1;
    }else {
        data_size = sizeof(int);
    }

    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&data_size, sizeof(data_size), 1) > 0), tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_data, data_size, 1) > 0), tsl_socket_tcp_close(g_sock));

    TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&ret, sizeof(ret), 1) > 0), tsl_socket_tcp_close(g_sock)); 
    TSL_VASSERT_RV_ACT(ret == TSL_RV_SUC, ret, tsl_socket_tcp_close(g_sock));

    tsl_socket_tcp_close(g_sock);

    return ret;
}

tsl_int_t tr69_commit_set_leaf_data(tsl_char_t *p_name, tsl_void_t *p_data, tsl_int_t type)
{
        tsl_int_t g_sock = -1;
        tsl_int_t sid = TR69_FUNC_COMMIT_SET_LEAF_DATA;
        tsl_int_t name_size;
        tsl_int_t data_size;
        tsl_int_t ret = -1;
        
        TSL_VASSERT(p_name != NULL);
        TSL_VASSERT(p_data != NULL);
        TSL_VASSERT(type == TR69_NODE_LEAF_TYPE_STRING || 
                    type == TR69_NODE_LEAF_TYPE_INT    ||
                    type == TR69_NODE_LEAF_TYPE_UINT   ||
                    type == TR69_NODE_LEAF_TYPE_BOOL   ||
                    type == TR69_NODE_LEAF_TYPE_DATE);

        //Coverity CID 18268: Side effect in assertion (ASSERT_SIDE_EFFECT)
#ifdef TR69_SOCK_PORT
        g_sock = tsl_socket_tcp_open(TR69_SERVER_IP, TR69_SERVER_PORT);
#else        
        g_sock = tsl_socket_local_open(TR69_SERVER_FILE);
#endif
        TSL_VASSERT(g_sock >= 0);
        //Coverity CID 19776: Resource leak (RESOURCE_LEAK) 
        TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&sid, sizeof(sid), 1) > 0), tsl_socket_tcp_close(g_sock));
        
        name_size = strlen(p_name);
        TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&name_size, sizeof(name_size), 1) > 0), tsl_socket_tcp_close(g_sock));
        TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_name, name_size, 1) > 0), tsl_socket_tcp_close(g_sock));
        TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&type, sizeof(type), 1) > 0), tsl_socket_tcp_close(g_sock));
        
        if (type == TR69_NODE_LEAF_TYPE_STRING || type == TR69_NODE_LEAF_TYPE_DATE){
                data_size = strlen(p_data) + 1;
        }else {
                data_size = sizeof(int);
        }
        TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&data_size, sizeof(data_size), 1) > 0), tsl_socket_tcp_close(g_sock));
        TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_data, data_size, 1) > 0), tsl_socket_tcp_close(g_sock));
        TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&ret, sizeof(ret), 1) > 0), tsl_socket_tcp_close(g_sock));
        
        TSL_VASSERT_RV_ACT(ret == TSL_RV_SUC, ret, tsl_socket_tcp_close(g_sock));
        
        tsl_socket_tcp_close(g_sock);

        return ret;
}

tsl_int_t tr69_commit_set_leaf_data_end(tsl_char_t *p_name)
{
        tsl_int_t g_sock = -1;
        tsl_int_t sid = TR69_FUNC_COMMIT_END;
        tsl_int_t name_size;
        tsl_int_t ret = -1;
        
        TSL_VASSERT(p_name != NULL);

         //Coverity CID 18269: Side effect in assertion (ASSERT_SIDE_EFFECT)
#ifdef TR69_SOCK_PORT
        g_sock = tsl_socket_tcp_open(TR69_SERVER_IP, TR69_SERVER_PORT);
#else        
        g_sock = tsl_socket_local_open(TR69_SERVER_FILE);
#endif
        TSL_VASSERT(g_sock >= 0);

        //Coverity CID 19777: Resource leak (RESOURCE_LEAK)
        TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&sid, sizeof(sid), 1) > 0), tsl_socket_tcp_close(g_sock));
        
        name_size = strlen(p_name);
        TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&name_size, sizeof(name_size), 1) > 0), tsl_socket_tcp_close(g_sock));
        TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_name, name_size, 1) > 0), tsl_socket_tcp_close(g_sock));

        TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&ret, sizeof(ret), 1) > 0), tsl_socket_tcp_close(g_sock));

        TSL_VASSERT_RV_ACT(ret == TSL_RV_SUC, ret, tsl_socket_tcp_close(g_sock));
        
        tsl_socket_tcp_close(g_sock);

        return ret;
}



tsl_int_t tr69_set_unfresh_leaf_data(tsl_char_t *p_name, tsl_void_t *p_data, tsl_int_t type)
{
    tsl_int_t g_sock = -1;
    tsl_int_t sid = TR69_FUNC_SET_UNFRESH_LEAF_DATA;
    tsl_int_t name_size;
    tsl_int_t data_size;
    tsl_int_t ret = -1;

    TSL_VASSERT(p_name != NULL);
    TSL_VASSERT(p_data != NULL);
    TSL_VASSERT(type == TR69_NODE_LEAF_TYPE_STRING || 
            type == TR69_NODE_LEAF_TYPE_INT    ||
            type == TR69_NODE_LEAF_TYPE_UINT   ||
            type == TR69_NODE_LEAF_TYPE_BOOL   ||
            type == TR69_NODE_LEAF_TYPE_DATE);

    //Coverity CID 16110: Side effect in assertion (ASSERT_SIDE_EFFECT) 
#ifdef TR69_SOCK_PORT
    g_sock = tsl_socket_tcp_open(TR69_SERVER_IP, TR69_SERVER_PORT);
#else
    g_sock = tsl_socket_local_open(TR69_SERVER_FILE);
#endif
    TSL_VASSERT(g_sock >= 0);

    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&sid, sizeof(sid), 1) > 0), tsl_socket_tcp_close(g_sock));

    name_size = strlen(p_name);
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&name_size, sizeof(name_size), 1) > 0), tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_name, name_size, 1) > 0), tsl_socket_tcp_close(g_sock));

    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&type, sizeof(type), 1) > 0), tsl_socket_tcp_close(g_sock)); 

    if (type == TR69_NODE_LEAF_TYPE_STRING || type == TR69_NODE_LEAF_TYPE_DATE){
        data_size = strlen(p_data) + 1;
    }else {
        data_size = sizeof(int);
    }

    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&data_size, sizeof(data_size), 1) > 0), tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_data, data_size, 1) > 0), tsl_socket_tcp_close(g_sock));

    TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&ret, sizeof(ret), 1) > 0), tsl_socket_tcp_close(g_sock)); 
    TSL_VASSERT_RV_ACT(ret == TSL_RV_SUC, ret, tsl_socket_tcp_close(g_sock));

    tsl_socket_tcp_close(g_sock);

    return ret;
}


tsl_int_t tr69_acl_set_leaf_data(tsl_char_t *p_name, tsl_void_t *p_data, tsl_int_t type)
{
    tsl_int_t g_sock = -1;
    tsl_int_t sid = TR69_FUNC_ACL_SET_LEAF_DATA;
    tsl_int_t name_size;
    tsl_int_t data_size;
    tsl_int_t ret = -1;

    TSL_VASSERT(p_name != NULL);
    TSL_VASSERT(p_data != NULL);
    TSL_VASSERT(type == TR69_NODE_LEAF_TYPE_STRING || 
            type == TR69_NODE_LEAF_TYPE_INT    ||
            type == TR69_NODE_LEAF_TYPE_UINT   ||
            type == TR69_NODE_LEAF_TYPE_BOOL   ||
            type == TR69_NODE_LEAF_TYPE_DATE);

    //Coverity CID 16097: Side effect in assertion (ASSERT_SIDE_EFFECT) 
#ifdef TR69_SOCK_PORT
    g_sock = tsl_socket_tcp_open(TR69_SERVER_IP, TR69_SERVER_PORT);
#else
    g_sock = tsl_socket_local_open(TR69_SERVER_FILE);
#endif
    TSL_VASSERT(g_sock >= 0);


	//Coverity CID 17260: Resource leak
	TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&sid, sizeof(sid), 1) > 0), tsl_socket_tcp_close(g_sock));
        
        name_size = strlen(p_name);
	TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&name_size, sizeof(name_size), 1) > 0), tsl_socket_tcp_close(g_sock));
	TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_name, name_size, 1) > 0), tsl_socket_tcp_close(g_sock));
       
	TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&type, sizeof(type), 1) > 0), tsl_socket_tcp_close(g_sock)); 
        
    if (type == TR69_NODE_LEAF_TYPE_STRING || type == TR69_NODE_LEAF_TYPE_DATE){
        data_size = strlen(p_data) + 1;
    }else {
        data_size = sizeof(int);
    }

    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&data_size, sizeof(data_size), 1) > 0), tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_data, data_size, 1) > 0), tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&ret, sizeof(ret), 1) > 0), tsl_socket_tcp_close(g_sock)); 
    TSL_VASSERT_RV_ACT(ret == TSL_RV_SUC, ret, tsl_socket_tcp_close(g_sock));

    tsl_socket_tcp_close(g_sock);

    return ret;
}


tsl_int_t tr69_get_node(tsl_char_t *p_name, tr69_node_t **pp_node)
{
    tsl_int_t g_sock = -1;
    tsl_int_t sid = TR69_FUNC_GET_NODE;
    tr69_node_t *p_data = NULL;
    tsl_int_t name_size;
    tsl_int_t ret = -1;

    TSL_VASSERT(p_name != NULL);
    TSL_VASSERT(pp_node != NULL);

    //Coverity CID 16104: Side effect in assertion (ASSERT_SIDE_EFFECT) 
#ifdef TR69_SOCK_PORT
    g_sock = tsl_socket_tcp_open(TR69_SERVER_IP, TR69_SERVER_PORT);
#else
    g_sock = tsl_socket_local_open(TR69_SERVER_FILE);
#endif
    TSL_VASSERT(g_sock >= 0);

    //Coverity CID 17268: Resource leak
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&sid, sizeof(sid), 1) > 0), tsl_socket_tcp_close(g_sock));

    name_size = strlen(p_name);
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&name_size, sizeof(name_size), 1) > 0), tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_name, name_size, 1) > 0), tsl_socket_tcp_close(g_sock));

    TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&ret, sizeof(ret), 1) > 0), tsl_socket_tcp_close(g_sock));  
    TSL_VASSERT_ACT(ret == TSL_RV_SUC, tsl_socket_tcp_close(g_sock);*pp_node = NULL);

    TSL_MALLOC_ACT(p_data, tr69_node_t, tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)p_data, sizeof(tr69_node_t), 1) > 0), if(p_data){TSL_FREE(p_data);};*pp_node=NULL;tsl_socket_tcp_close(g_sock));

    *pp_node = p_data;

    tsl_socket_tcp_close(g_sock);

    return ret;
}

tsl_int_t tr69_get_next_node(tsl_char_t *p_name, tr69_node_t **pp_node)
{
    tsl_int_t g_sock = -1;
    tsl_int_t sid = TR69_FUNC_GET_NEXT_NODE;
    tr69_node_t *p_data = NULL;
    tsl_int_t data_size = 0;
    tsl_int_t name_size;
    tsl_int_t ret = -1;
    
    TSL_VASSERT(p_name != NULL);
    TSL_VASSERT(pp_node != NULL);

//Coverity CID 16103: Side effect in assertion (ASSERT_SIDE_EFFECT)        
#ifdef TR69_SOCK_PORT
    g_sock = tsl_socket_tcp_open(TR69_SERVER_IP, TR69_SERVER_PORT);
#else
    g_sock = tsl_socket_local_open(TR69_SERVER_FILE);
#endif
    TSL_VASSERT(g_sock >= 0);

    //Coverity CID 17266: Resource leak
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&sid, sizeof(sid), 1) > 0), tsl_socket_tcp_close(g_sock));

    name_size = strlen(p_name);
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&name_size, sizeof(name_size), 1) > 0), tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_name, name_size, 1) > 0), tsl_socket_tcp_close(g_sock));

    if (*pp_node != NULL){
        data_size = sizeof(tr69_node_t);
        p_data = *pp_node;
        TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&data_size, sizeof(data_size), 1) > 0), tsl_socket_tcp_close(g_sock));
        TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_data, sizeof(tr69_node_t), 1) > 0), tsl_socket_tcp_close(g_sock));
    }else {
        data_size = 0;
        TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&data_size, sizeof(data_size), 1) > 0), tsl_socket_tcp_close(g_sock));
        TSL_MALLOC_ACT(p_data, tr69_node_t, tsl_socket_tcp_close(g_sock));
    }
       
	//Coverity CID 17267: Resource leak
    TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&ret, sizeof(ret), 1) > 0), if(p_data){TSL_FREE(p_data);}; *pp_node = NULL; tsl_socket_tcp_close(g_sock)); 
    TSL_VASSERT_ACT(ret == TSL_RV_SUC, if(p_data){TSL_FREE(p_data);}; *pp_node = NULL; tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)p_data, sizeof(tr69_node_t), 1) > 0), if(p_data){TSL_FREE(p_data);}; *pp_node = NULL; tsl_socket_tcp_close(g_sock));

    *pp_node = p_data;

    tsl_socket_tcp_close(g_sock);

    return ret;
}

tsl_int_t tr69_get_next_node2(tsl_char_t *p_name, tr69_node_t **pp_node)
{
	tsl_int_t g_sock = -1;
	tsl_int_t sid = TR69_FUNC_GET_NEXT_NODE2;
	tr69_node_t *p_data = NULL;
	tsl_int_t data_size = 0;
	tsl_int_t name_size = 0;
	tsl_int_t ret = -1;

	TSL_VASSERT(p_name != NULL);
	TSL_VASSERT(pp_node != NULL);

    //Coverity CID 18272: Side effect in assertion (ASSERT_SIDE_EFFECT)
#ifdef TR69_SOCK_PORT
    g_sock = tsl_socket_tcp_open(TR69_SERVER_IP, TR69_SERVER_PORT);
#else
    g_sock = tsl_socket_local_open(TR69_SERVER_FILE);
#endif
    TSL_VASSERT(g_sock >= 0);

    //Coverity CID 19782: Resource leak (RESOURCE_LEAK)
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&sid, sizeof(sid), 1) > 0), tsl_socket_tcp_close(g_sock));

	name_size = strlen(p_name);
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&name_size, sizeof(name_size), 1) > 0), tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_name, name_size, 1) > 0), tsl_socket_tcp_close(g_sock));

	if (*pp_node != NULL) {
		data_size = sizeof(tr69_node_t);
		p_data = *pp_node;
        TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&data_size, sizeof(data_size), 1) > 0), tsl_socket_tcp_close(g_sock));
        TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_data, data_size, 1) > 0), tsl_socket_tcp_close(g_sock));	
	}else {
		data_size = 0;
		TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&data_size, sizeof(data_size), 1) > 0), tsl_socket_tcp_close(g_sock));
		TSL_MALLOC(p_data, tr69_node_t);
	}

    //Coverity CID 19783: Resource leak (RESOURCE_LEAK)
    TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&ret, sizeof(ret), 1) > 0), if (p_data){TSL_FREE(p_data);}; *pp_node = NULL; tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT(ret == TSL_RV_SUC, if (p_data){TSL_FREE(p_data);}; *pp_node = NULL; tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)p_data, sizeof(tr69_node_t), 1) > 0), if (p_data){TSL_FREE(p_data);}; *pp_node = NULL; tsl_socket_tcp_close(g_sock));

	*pp_node = p_data;
	
	tsl_socket_tcp_close(g_sock);
	
	return ret;
}

tsl_int_t tr69_get_next_child_node(tsl_char_t *p_name, tr69_node_t **pp_node)
{
	tsl_int_t g_sock = -1;
	tsl_int_t sid = TR69_FUNC_GET_NEXT_CHILD_NODE;
	tr69_node_t *p_data = NULL;
	tsl_int_t data_size = 0;
	tsl_int_t name_size = 0;
	tsl_int_t ret = -1;

	TSL_VASSERT(p_name != NULL);
	TSL_VASSERT(pp_node != NULL);

    //Coverity CID 18270: Side effect in assertion (ASSERT_SIDE_EFFECT)
#ifdef TR69_SOCK_PORT
    g_sock = tsl_socket_tcp_open(TR69_SERVER_IP, TR69_SERVER_PORT);
#else
    g_sock = tsl_socket_local_open(TR69_SERVER_FILE);
#endif
    TSL_VASSERT(g_sock >= 0);

    //Coverity CID 19778: Resource leak (RESOURCE_LEAK) 
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&sid, sizeof(sid), 1) > 0), tsl_socket_tcp_close(g_sock));

	name_size = strlen(p_name);
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&name_size, sizeof(name_size), 1) > 0), tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_name, name_size, 1) > 0), tsl_socket_tcp_close(g_sock));

	if (*pp_node != NULL){
		data_size = sizeof(tr69_node_t);
		p_data = *pp_node;
        TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&data_size, sizeof(data_size), 1) > 0), tsl_socket_tcp_close(g_sock));
        TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_data, sizeof(tr69_node_t), 1) > 0), tsl_socket_tcp_close(g_sock));	
	}else {
		data_size = 0;
        TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&data_size, sizeof(data_size), 1) > 0), tsl_socket_tcp_close(g_sock));
		TSL_MALLOC(p_data, tr69_node_t);
	}
    
    //Coverity CID 19779: Resource leak (RESOURCE_LEAK)
    TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&ret, sizeof(ret), 1) > 0), if (p_data){TSL_FREE(p_data);}; *pp_node = NULL; tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT(ret == TSL_RV_SUC, if (p_data){TSL_FREE(p_data);}; *pp_node = NULL; tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)p_data, sizeof(tr69_node_t), 1) > 0), if (p_data){TSL_FREE(p_data);}; *pp_node = NULL; tsl_socket_tcp_close(g_sock));

	*pp_node = p_data;

	tsl_socket_tcp_close(g_sock);

	return ret;
}

tsl_int_t tr69_get_next_obj_node(tsl_char_t *p_name, tr69_node_t **pp_node)
{
        tsl_int_t g_sock = -1;
        tsl_int_t sid = TR69_FUNC_GET_NEXT_OBJ_NODE;
        tr69_node_t *p_data = NULL;
        tsl_int_t data_size = 0;
        tsl_int_t name_size;
        tsl_int_t ret = -1;
        
        TSL_VASSERT(p_name != NULL);
        TSL_VASSERT(pp_node != NULL);

        //Coverity CID 18273: Side effect in assertion (ASSERT_SIDE_EFFECT)
#ifdef TR69_SOCK_PORT
        g_sock = tsl_socket_tcp_open(TR69_SERVER_IP, TR69_SERVER_PORT);
#else        
        g_sock = tsl_socket_local_open(TR69_SERVER_FILE);
#endif
        TSL_VASSERT(g_sock >= 0);

        //Coverity CID 19784: Resource leak (RESOURCE_LEAK) 
        TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&sid, sizeof(sid), 1) > 0), tsl_socket_tcp_close(g_sock));

        name_size = strlen(p_name);
        TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&name_size, sizeof(name_size), 1) > 0), tsl_socket_tcp_close(g_sock));
        TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_name, name_size, 1) > 0), tsl_socket_tcp_close(g_sock));

        
        if (*pp_node != NULL){
                data_size = sizeof(tr69_node_t);
                p_data = *pp_node;
                TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&data_size, sizeof(data_size), 1) > 0), tsl_socket_tcp_close(g_sock));
                TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_data, sizeof(tr69_node_t), 1) > 0), tsl_socket_tcp_close(g_sock));
        }else {
                data_size = 0;
                TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&data_size, sizeof(data_size), 1) > 0), tsl_socket_tcp_close(g_sock));
                TSL_MALLOC(p_data, tr69_node_t);
        }
        //Coverity CID 19785: Resource leak (RESOURCE_LEAK)
        TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&ret, sizeof(ret), 1) > 0), if (p_data){TSL_FREE(p_data);}; *pp_node = NULL; tsl_socket_tcp_close(g_sock));
        TSL_VASSERT_ACT(ret == TSL_RV_SUC, if(p_data){TSL_FREE(p_data);}; *pp_node = NULL; tsl_socket_tcp_close(g_sock));
        TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)p_data, sizeof(tr69_node_t), 1) > 0), if (p_data){TSL_FREE(p_data);}; *pp_node = NULL; tsl_socket_tcp_close(g_sock));

        *pp_node = p_data;
        
        tsl_socket_tcp_close(g_sock);

        return ret;
}

tsl_int_t tr69_get_next_inst_node(tsl_char_t *p_name, tr69_node_t **pp_node)
{
	tsl_int_t g_sock = -1;
	tsl_int_t sid = TR69_FUNC_GET_NEXT_INST_NODE;
	tr69_node_t *p_data = NULL;
	tsl_int_t data_size = 0;
	tsl_int_t name_size = 0;
	tsl_int_t ret = -1;

	TSL_VASSERT(p_name != NULL);
	TSL_VASSERT(pp_node != NULL);

    //Coverity CID 18271: Side effect in assertion (ASSERT_SIDE_EFFECT)
#ifdef TR69_SOCK_PORT
    g_sock = tsl_socket_tcp_open(TR69_SERVER_IP, TR69_SERVER_PORT);
#else
    g_sock = tsl_socket_local_open(TR69_SERVER_FILE);

#endif
    TSL_VASSERT(g_sock >= 0);

    //Coverity CID 19780: Resource leak (RESOURCE_LEAK)
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&sid, sizeof(sid), 1) > 0), tsl_socket_tcp_close(g_sock));

	name_size = strlen(p_name);
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&name_size, sizeof(name_size), 1) > 0), tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_name, name_size, 1) > 0), tsl_socket_tcp_close(g_sock));

	if (*pp_node != NULL){
		data_size = sizeof(tr69_node_t);
		p_data = *pp_node;
        TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&data_size, sizeof(data_size), 1) > 0), tsl_socket_tcp_close(g_sock));
        TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_data, sizeof(tr69_node_t), 1) > 0), tsl_socket_tcp_close(g_sock));		
	}else {
		data_size = 0;
        TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&data_size, sizeof(data_size), 1) > 0), tsl_socket_tcp_close(g_sock));
		TSL_MALLOC(p_data, tr69_node_t);
	}

    //Coverity CID 19781: Resource leak (RESOURCE_LEAK) 
    TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&ret, sizeof(ret), 1) > 0), if (p_data){TSL_FREE(p_data);}; *pp_node = NULL; tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT(ret == TSL_RV_SUC, if (p_data){TSL_FREE(p_data);}; *pp_node = NULL; tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)p_data, sizeof(tr69_node_t), 1) > 0), if (p_data){TSL_FREE(p_data);}; *pp_node = NULL; tsl_socket_tcp_close(g_sock));
	
	*pp_node = p_data;

	tsl_socket_tcp_close(g_sock);

	return ret;
}

tsl_int_t tr69_get_next_samedepth_node(tsl_char_t *p_name, tr69_node_t **pp_node)
{
	tsl_int_t g_sock = -1;
	tsl_int_t sid = TR69_FUNC_GET_SAME_DEPTH_NODE;
	tr69_node_t *p_data = NULL;
	tsl_int_t data_size = 0;
	tsl_int_t name_size = 0;
	tsl_int_t ret = -1;

	TSL_VASSERT(p_name != NULL);
	TSL_VASSERT(pp_node != NULL);

    //Coverity CID 18274: Side effect in assertion (ASSERT_SIDE_EFFECT)
#ifdef TR69_SOCK_PORT
    g_sock = tsl_socket_tcp_open(TR69_SERVER_IP, TR69_SERVER_PORT);
#else
    g_sock = tsl_socket_local_open(TR69_SERVER_FILE);
#endif
    TSL_VASSERT(g_sock >= 0);

    //Coverity CID 19786: Resource leak (RESOURCE_LEAK) 
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&sid, sizeof(sid), 1) > 0), tsl_socket_tcp_close(g_sock));

	name_size = strlen(p_name);
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&name_size, sizeof(name_size), 1) > 0), tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_name, name_size, 1) > 0), tsl_socket_tcp_close(g_sock));

	if (*pp_node != NULL){
		data_size = sizeof(tr69_node_t);
		p_data = *pp_node;
        TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&data_size, sizeof(data_size), 1) > 0), tsl_socket_tcp_close(g_sock));
        TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_data, data_size, 1) > 0), tsl_socket_tcp_close(g_sock));
	}else {
		data_size = 0;
        TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&data_size, sizeof(data_size), 1) > 0), tsl_socket_tcp_close(g_sock));
		TSL_MALLOC(p_data, tr69_node_t);
	}

    //Coverity CID 19787: Resource leak (RESOURCE_LEAK)
    TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&ret, sizeof(ret), 1) > 0), if (p_data){TSL_FREE(p_data);}; *pp_node = NULL; tsl_socket_tcp_close(g_sock));
	TSL_VASSERT_ACT(ret == TSL_RV_SUC, if (p_data){TSL_FREE(p_data);}; *pp_node = NULL; tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)p_data, sizeof(tr69_node_t), 1) > 0), if (p_data){TSL_FREE(p_data);}; *pp_node = NULL; tsl_socket_tcp_close(g_sock));
	
	*pp_node = p_data;

	tsl_socket_tcp_close(g_sock);
	
	return ret;
}

tsl_int_t tr69_get_obj_data_size(tsl_char_t *p_name, tsl_int_t *p_obj_data_size)
{
	tsl_int_t g_sock = -1;
	tsl_int_t sid = TR69_FUNC_GET_OBJ_DATA_SIZE;
	tsl_int_t name_size = 0;
	tsl_int_t obj_data_size = 0;
	tsl_int_t ret = -1;

	TSL_VASSERT(p_name != NULL);
	TSL_VASSERT(p_obj_data_size != NULL);

    //Coverity CID 19788: Resource leak (RESOURCE_LEAK)
#ifdef TR69_SOCK_PORT
        g_sock = tsl_socket_tcp_open(TR69_SERVER_IP, TR69_SERVER_PORT);
#else
        g_sock = tsl_socket_local_open(TR69_SERVER_FILE);
#endif
    TSL_VASSERT(g_sock >= 0);

    //Coverity CID 18275: Side effect in assertion (ASSERT_SIDE_EFFECT)
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&sid, sizeof(sid), 1) > 0), tsl_socket_tcp_close(g_sock));

	name_size = strlen(p_name) + 1;
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&name_size, sizeof(name_size), 1) > 0), tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_name, name_size, 1) > 0), tsl_socket_tcp_close(g_sock));

    TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&ret, sizeof(ret), 1) > 0), tsl_socket_tcp_close(g_sock));

	TSL_VASSERT_RV_ACT(ret == TSL_RV_SUC, ret, tsl_socket_tcp_close(g_sock));

    TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&obj_data_size, sizeof(obj_data_size), 1) > 0), tsl_socket_tcp_close(g_sock));

	*p_obj_data_size = obj_data_size;

	tsl_socket_tcp_close(g_sock);

	return ret;
}

tsl_int_t tr69_set_node(tsl_char_t *p_name, tr69_node_t *p_node)
{
    tsl_int_t g_sock = -1;
    tsl_int_t sid = TR69_FUNC_SET_NODE;
    tsl_int_t name_size;
    tsl_int_t ret = -1;
    
    TSL_VASSERT(p_name != NULL);
    TSL_VASSERT(p_node != NULL);
	
	//Coverity CID 16109: Side effect in assertion (ASSERT_SIDE_EFFECT)
#ifdef TR69_SOCK_PORT
    g_sock = tsl_socket_tcp_open(TR69_SERVER_IP, TR69_SERVER_PORT);
#else
    g_sock = tsl_socket_local_open(TR69_SERVER_FILE);
#endif
    TSL_VASSERT(g_sock >= 0);


	//Coverity CID 17273: Resource leak
	TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&sid, sizeof(sid), 1) > 0), tsl_socket_tcp_close(g_sock));
        
        name_size = strlen(p_name);
	TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&name_size, sizeof(name_size), 1) > 0), tsl_socket_tcp_close(g_sock));
	TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_name, name_size, 0) > 0), tsl_socket_tcp_close(g_sock));
	TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_node, sizeof(tr69_node_t), 1) > 0), tsl_socket_tcp_close(g_sock));
       
	TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&ret, sizeof(ret), 1) > 0), tsl_socket_tcp_close(g_sock)); 
        TSL_VASSERT_RV_ACT(ret == TSL_RV_SUC, ret, tsl_socket_tcp_close(g_sock));
        
        tsl_socket_tcp_close(g_sock);

        return ret;
}


tsl_int_t tr69_add_instance(tsl_char_t *p_name, tsl_int_t *p_inst_numb)
{
    tsl_int_t g_sock = -1;
    tsl_int_t sid = TR69_FUNC_ADD_INSTANCE;
    tsl_int_t name_size;
    tsl_int_t ret = -1;
    tsl_int_t inst_numb = 0;

    TSL_VASSERT(p_name != NULL);
    TSL_VASSERT(p_inst_numb != NULL);
   
//Coverity CID 16098: Side effect in assertion (ASSERT_SIDE_EFFECT) 
#ifdef TR69_SOCK_PORT
    g_sock = tsl_socket_tcp_open(TR69_SERVER_IP, TR69_SERVER_PORT);
#else
    g_sock = tsl_socket_local_open(TR69_SERVER_FILE);
#endif
    TSL_VASSERT(g_sock >= 0);

       
	//Coverity CID 17261: Resource leak
	TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&sid, sizeof(sid), 1) > 0), tsl_socket_tcp_close(g_sock)); 
        
        name_size = strlen(p_name);
	TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&name_size, sizeof(name_size), 1) > 0), tsl_socket_tcp_close(g_sock));
	TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_name, name_size, 1) > 0), tsl_socket_tcp_close(g_sock));
       
	TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&ret, sizeof(ret), 1) > 0), tsl_socket_tcp_close(g_sock)); 
        TSL_VASSERT_RV_ACT(ret == TSL_RV_SUC, ret, tsl_socket_tcp_close(g_sock));
       
	TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&inst_numb, sizeof(inst_numb), 1) > 0), tsl_socket_tcp_close(g_sock)); 
        *p_inst_numb = inst_numb;

        tsl_socket_tcp_close(g_sock);

        return ret;
}


tsl_int_t tr69_del2_instance(tsl_char_t *p_name)
{
    tsl_int_t g_sock = -1;
    tsl_int_t sid = TR69_FUNC_DEL2_INSTANCE;
    tsl_int_t name_size;
    tsl_int_t ret = -1;

    TSL_VASSERT(p_name != NULL);

    //Coverity CID 16100: Side effect in assertion (ASSERT_SIDE_EFFECT)  
#ifdef TR69_SOCK_PORT
    g_sock = tsl_socket_tcp_open(TR69_SERVER_IP, TR69_SERVER_PORT);
#else
    g_sock = tsl_socket_local_open(TR69_SERVER_FILE);
#endif
    TSL_VASSERT(g_sock >= 0);

        
	//Coverity CID 17263: Resource leak
	TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&sid, sizeof(sid), 1) > 0), tsl_socket_tcp_close(g_sock));
        
    name_size = strlen(p_name);
	TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&name_size, sizeof(name_size), 1) > 0), tsl_socket_tcp_close(g_sock));
	TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_name, name_size, 1) > 0), tsl_socket_tcp_close(g_sock));
       
    TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&ret, sizeof(ret), 1) > 0), tsl_socket_tcp_close(g_sock)); 
    TSL_VASSERT_RV_ACT(ret == TSL_RV_SUC, ret, tsl_socket_tcp_close(g_sock));

    tsl_socket_tcp_close(g_sock);

    return ret;
}


tsl_int_t tr69_restore_default_node(tsl_char_t *p_name)
{
    tsl_int_t g_sock = -1;
    tsl_int_t sid = TR69_FUNC_RESTORE_DEFUALT_NODE;
    tsl_int_t name_size;
    tsl_int_t ret = -1;

    TSL_VASSERT(p_name != NULL);

    //Coverity CID 18277: Side effect in assertion (ASSERT_SIDE_EFFECT)
#ifdef TR69_SOCK_PORT
    g_sock = tsl_socket_tcp_open(TR69_SERVER_IP, TR69_SERVER_PORT);
#else
    g_sock = tsl_socket_local_open(TR69_SERVER_FILE);
#endif
    TSL_VASSERT(g_sock >= 0);

    //Coverity CID 19790: Resource leak (RESOURCE_LEAK)
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&sid, sizeof(sid), 1) > 0), tsl_socket_tcp_close(g_sock));

    name_size = strlen(p_name);
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&name_size, sizeof(name_size), 1) > 0), tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_name, name_size, 1) > 0), tsl_socket_tcp_close(g_sock));

    TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&ret, sizeof(ret), 1) > 0), tsl_socket_tcp_close(g_sock));

    TSL_VASSERT_RV_ACT(ret == TSL_RV_SUC, ret, tsl_socket_tcp_close(g_sock));

    tsl_socket_tcp_close(g_sock);

    return ret;
}




tsl_int_t tr69_del_instance(tsl_char_t *p_name, tsl_int_t inst)
{
    tsl_int_t g_sock = -1;
    tsl_int_t sid = TR69_FUNC_DEL_INSTANCE;
    tsl_int_t name_size;
    tsl_int_t ret = -1;

    TSL_VASSERT(p_name != NULL);

    //Coverity CID 16101: Side effect in assertion (ASSERT_SIDE_EFFECT) 
#ifdef TR69_SOCK_PORT
    g_sock = tsl_socket_tcp_open(TR69_SERVER_IP, TR69_SERVER_PORT);
#else
    g_sock = tsl_socket_local_open(TR69_SERVER_FILE);
#endif
    TSL_VASSERT(g_sock >= 0);

    //Coverity CID 17264: Resource leak
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&sid, sizeof(sid), 1) > 0), tsl_socket_tcp_close(g_sock));

    name_size = strlen(p_name);
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&name_size, sizeof(name_size), 1) > 0), tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_name, name_size, 1) > 0), tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&inst, sizeof(inst), 1) > 0), tsl_socket_tcp_close(g_sock));

    TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&ret, sizeof(ret), 1) > 0), tsl_socket_tcp_close(g_sock)); 
    TSL_VASSERT_RV_ACT(ret == TSL_RV_SUC, ret, tsl_socket_tcp_close(g_sock));

    tsl_socket_tcp_close(g_sock);

    return ret;
}

tsl_rv_t tr69_get_notification(tr69_notification_t **pp_notf_tb, tsl_int_t *p_tb_count)
{
    tsl_int_t g_sock = -1;
    tsl_int_t sid = TR69_FUNC_GET_NOTIFICATION;
    tsl_int_t ret = -1;
    tsl_int_t count = 0;
    tr69_notification_t *p_notf_tb = NULL;
    
    TSL_VASSERT(pp_notf_tb != NULL);
    TSL_VASSERT(p_tb_count != NULL);

//Coverity CID 16105: Side effect in assertion (ASSERT_SIDE_EFFECT)
#ifdef TR69_SOCK_PORT
    g_sock = tsl_socket_tcp_open(TR69_SERVER_IP, TR69_SERVER_PORT);
#else
    g_sock = tsl_socket_local_open(TR69_SERVER_FILE);
#endif
    TSL_VASSERT(g_sock >= 0);

	//Coverity CID 17269: Resource leak
	TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&sid, sizeof(sid), 1) > 0), tsl_socket_tcp_close(g_sock));
       
	TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&ret, sizeof(ret), 1) > 0), tsl_socket_tcp_close(g_sock)); 
        TSL_VASSERT_RV_ACT(ret == TSL_RV_SUC, ret, tsl_socket_tcp_close(g_sock));
       
	TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&count, sizeof(count), 1) > 0), tsl_socket_tcp_close(g_sock)); 
        
    *pp_notf_tb = NULL;
    *p_tb_count = 0;
    
    if (count){
        TSL_SMALLOC_RV_ACT(p_notf_tb, tr69_notification_t, count*sizeof(tr69_notification_t), TSL_RV_FAIL_MEM, tsl_socket_tcp_close(g_sock));
        TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)p_notf_tb, sizeof(tr69_notification_t)*count, 1) > 0), if(p_notf_tb){TSL_FREE(p_notf_tb);};*pp_notf_tb=NULL;tsl_socket_tcp_close(g_sock));
        *pp_notf_tb = p_notf_tb;
        *p_tb_count = count;
    }
    
    tsl_socket_tcp_close(g_sock);

    return ret;
}

tsl_int_t tr69_save_now()
{
    tsl_int_t g_sock = -1;
    tsl_int_t sid = TR69_FUNC_SAVE_NOW;
    //Coverity CID 16107: Side effect in assertion (ASSERT_SIDE_EFFECT) 
#ifdef TR69_SOCK_PORT
    g_sock = tsl_socket_tcp_open(TR69_SERVER_IP, TR69_SERVER_PORT);
#else
    g_sock = tsl_socket_local_open(TR69_SERVER_FILE);
#endif
    TSL_VASSERT(g_sock >= 0);

    //Coverity CID 17271: Resource leak
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&sid, sizeof(sid), 1) > 0), tsl_socket_tcp_close(g_sock));

    tsl_socket_tcp_close(g_sock);

    return TSL_RV_SUC;
}
tsl_int_t tr69_clear_notification()
{
    tsl_int_t g_sock = -1;
    tsl_int_t sid = TR69_FUNC_CLEAR_NOTIFICATION;
    //Coverity CID 16107: Side effect in assertion (ASSERT_SIDE_EFFECT) 
#ifdef TR69_SOCK_PORT
    g_sock = tsl_socket_tcp_open(TR69_SERVER_IP, TR69_SERVER_PORT);
#else
    g_sock = tsl_socket_local_open(TR69_SERVER_FILE);
#endif
    TSL_VASSERT(g_sock >= 0);

    //Coverity CID 17271: Resource leak
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&sid, sizeof(sid), 1) > 0), tsl_socket_tcp_close(g_sock));

    tsl_socket_tcp_close(g_sock);

    return TSL_RV_SUC;
}


tsl_int_t tr69_move_instance(tsl_char_t *p_name_dstobj, tsl_char_t *p_name_srcinst, tsl_int_t *p_inst_numb)
{
    tsl_int_t g_sock = -1;
    tsl_int_t sid = TR69_FUNC_MOVE_INSTANCE;
    tsl_int_t name_dst_len = 0;
    tsl_int_t name_src_len = 0;
    tsl_int_t inst_numb = -1;
    tsl_int_t ret = -1;

    TSL_VASSERT(p_name_dstobj != NULL);
    TSL_VASSERT(p_name_srcinst != NULL);
    TSL_VASSERT(p_inst_numb != NULL);

    //Coverity CID 18276: Side effect in assertion (ASSERT_SIDE_EFFECT)
#ifdef TR69_SOCK_PORT
    g_sock = tsl_socket_tcp_open(TR69_SERVER_IP, TR69_SERVER_PORT);
#else
    g_sock = tsl_socket_local_open(TR69_SERVER_FILE);
#endif
    TSL_VASSERT(g_sock >= 0);

    //Coverity CID 19789: Resource leak (RESOURCE_LEAK)
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&sid, sizeof(sid), 1) > 0), tsl_socket_tcp_close(g_sock));

    name_dst_len = strlen(p_name_dstobj);
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&name_dst_len, sizeof(name_dst_len), 1) > 0), tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_name_dstobj, name_dst_len, 1) > 0), tsl_socket_tcp_close(g_sock));

    name_src_len = strlen(p_name_srcinst);
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)&name_src_len, sizeof(name_src_len), 1) > 0), tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT((tsl_socket_tcp_write(g_sock, (tsl_char_t *)p_name_srcinst, name_src_len, 1) > 0), tsl_socket_tcp_close(g_sock));
    TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&ret, sizeof(ret), 1) > 0), tsl_socket_tcp_close(g_sock)); 

    TSL_VASSERT_RV_ACT(ret == TSL_RV_SUC, ret, tsl_socket_tcp_close(g_sock));

    TSL_VASSERT_ACT((tsl_socket_tcp_read(g_sock, (tsl_char_t *)&inst_numb, sizeof(inst_numb), 1) > 0), tsl_socket_tcp_close(g_sock));

    *p_inst_numb = inst_numb;
        
    tsl_socket_tcp_close(g_sock);

    return ret;
}

