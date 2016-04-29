#ifndef TR069_TREE_H
#define TR069_TREE_H

#include "tsl_list.h"
#include "tsl_common.h"

enum TIMEOUT_REASON{
    TIMEOUT_REASON_NONE,
    TIMEOUT_REASON_DELAYNOTIF,
    TIMEOUT_REASON_DELINST
};

#define TR69_SUPPORT_INSTANCE_NOSAVE // Jean, added for Instance nosave
#define TR69_SUPPORT_HIDE_OBJ_FROM_ACS // Beside hide param, also should support hide obj/inst


#define TR69_NODE_TYPE_OBJECT                   0x11
#define TR69_NODE_TYPE_INSTANCE                 0x22
#define TR69_NODE_TYPE_LEAF                     0x33

#define TR69_NODE_LEAF_ACL_READONLY             0x41
#define TR69_NODE_LEAF_ACL_WRITEONLY            0x42
#define TR69_NODE_LEAF_ACL_READWRITE            0x43
#define TR69_NODE_LEAF_ACL2_NOSAVE				0x44

#define TR69_NODE_LEAF_TYPE_STRING              0x51
#define TR69_NODE_LEAF_TYPE_UINT                0x52
#define TR69_NODE_LEAF_TYPE_INT                 0x53
#define TR69_NODE_LEAF_TYPE_BOOL                0x54
#define TR69_NODE_LEAF_TYPE_DATE                0x55


#define TR69_NODE_LEAF_UNNOTIFICATION           0x0
#define TR69_NODE_LEAF_NOTIFICATION             0x1
#define TR69_NODE_LEAF_ACTIVENOTIFICATION       0x2

#define TR69_NODE_LEAF_ATTR_HIDDEN              0x71

#define TR69_RT_SUCCESS_VALUE_UNCHANGED         0x101
#define TR69_RT_SUCCESS_VALUE_CHANGED           0x102

#define TR69_ERROR_BAD_ACL                      0x201

#define TR69_OID_COMM_NAME_SIZE                 128
#define TR69_OID_FULL_NAME_SIZE                 256
#define TR69_DEF_VALUE_SIZE                     256

typedef tsl_rv_t (*tr69_data_func_get_obj_t)(tsl_char_t *p_oid_name, tsl_void_t *p_cur_data);
typedef tsl_rv_t (*tr69_data_func_set_obj_t)(tsl_char_t *p_oid_name, tsl_void_t *p_cur_data, tsl_void_t *p_new_data);

typedef struct tr69_info_object_s{
        tsl_char_t name[TR69_OID_COMM_NAME_SIZE];
        tsl_char_t short_name[TR69_OID_COMM_NAME_SIZE];
        tsl_void_t *p_data;
        tsl_void_t *p_commit_data;
        tsl_uint_t data_size;
#ifdef TR69_SUPPORT_HIDE_OBJ_FROM_ACS
        tsl_int_t attr;
        tsl_bool_t hide;
#endif
        tr69_data_func_get_obj_t get_func;
        tr69_data_func_set_obj_t set_func;
}tr69_info_object_t;

typedef struct tr69_info_leaf_s{
        tsl_int_t acl;
        tsl_int_t type;
        tsl_char_t name[TR69_OID_COMM_NAME_SIZE];
        tsl_char_t def_value[TR69_DEF_VALUE_SIZE];
        tsl_int_t  notification;
        tsl_uint_t data_offset;
        tsl_int_t attr;
        tsl_uint_t min;
        tsl_uint_t max;
		tsl_int_t nosave;
#ifdef AEI_PARAM_FORCEUPDATE
        tsl_int_t forceupdate;
#endif
#ifdef TR69_SUPPORT_HIDE_OBJ_FROM_ACS
        tsl_bool_t hide;
#endif
}tr69_info_leaf_t;

typedef struct tr69_info_instance_s{
        tsl_int_t size;
        tsl_int_t acl;
        tsl_void_t *p_data;
        tsl_void_t *p_commit_data;
        tsl_uint_t data_size;
#ifdef TR69_SUPPORT_HIDE_OBJ_FROM_ACS
        tsl_int_t attr;
        tsl_bool_t hide;
#endif
		tsl_int_t nosave;
}tr69_info_instance_t;

typedef struct tr69_info_tree_s{
#if ( !defined( _BHR2 ) && !defined( SUPPORT_BHR1 ) )
        sem_t mutex_sem;
#endif
        tsl_char_t full_name[TR69_OID_FULL_NAME_SIZE];
        tsl_char_t node_type;

        union{
                tr69_info_object_t   info_obj;
                tr69_info_leaf_t     info_leaf;
                tr69_info_instance_t info_inst;
        }info;

        struct tr69_info_tree_s *p_parent;

        tsl_list_head_t list_head;
        tsl_list_head_t list_node;

}tr69_info_tree_t;

typedef struct tr69_node_s{
        tsl_char_t full_name[TR69_OID_FULL_NAME_SIZE];
        tsl_int_t  node_type;
        tsl_int_t  notification;
        tsl_int_t  acl;
}tr69_node_t;

typedef struct tr69_notification_s{
        tsl_char_t full_name[TR69_OID_FULL_NAME_SIZE];
        tsl_list_head_t list_node;
}tr69_notification_t;

typedef struct tr69_delaynotification_s{
        tsl_char_t full_name[TR69_OID_FULL_NAME_SIZE];
        time_t last_time;
        tsl_int_t active;
        tsl_int_t is_add;
}tr69_delaynotification_t;

typedef struct tr69_timeout_instance_s{
        tsl_char_t full_name[TR69_OID_FULL_NAME_SIZE];
        time_t timeout;
        tsl_list_head_t list_node;
}tr69_timeout_instance_t;

typedef tsl_int_t (*tr69_func_get_obj_t)(tsl_char_t *p_oid_name, tsl_void_t *p_cur_data);
typedef tsl_int_t (*tr69_func_set_obj_t)(tsl_char_t *p_oid_name, tsl_void_t *p_cur_data, tsl_void_t *p_new_data);

struct tr69_register_func{
        tsl_char_t oid_name[TR69_OID_FULL_NAME_SIZE];
	tr69_func_get_obj_t get_func;
	tr69_func_set_obj_t set_func;
};

struct tr69_inst_numb_of_entries_s{
	tsl_char_t numb_of_entries_oid[TR69_OID_FULL_NAME_SIZE];
	tsl_char_t entries_oid[TR69_OID_FULL_NAME_SIZE];
};


tsl_rv_t tr69_inline_timeout(tsl_int_t def_wait_timeout, tsl_int_t *p_timeout_reason);

tsl_rv_t tr69_get_obj_data_size(tsl_char_t *p_name, tsl_int_t *data_size);

tsl_rv_t tr69_get_obj_by_oid_flag(char* full_name, tsl_u32_t getFlags, void **obj);

tsl_rv_t tr69_set_unfresh_obj_data(tsl_char_t *p_name, tsl_void_t **pp_data);


tsl_rv_t tr69_commit_set_leaf_data(tsl_char_t *p_name, tsl_void_t *p_value, tsl_int_t type);
tsl_rv_t tr69_acl_commit_set_leaf_data(tsl_char_t *p_name, tsl_void_t *p_value, tsl_int_t type);
tsl_rv_t tr69_commit_set_leaf_data_end(tsl_char_t *p_name);
tsl_rv_t tr69_get_next_node2(tsl_char_t *p_root_name, tr69_node_t **pp_next_node);
tsl_rv_t tr69_get_next_child_node(tsl_char_t *p_root_name, tr69_node_t **pp_next_node);
tsl_rv_t tr69_get_next_obj_node(tsl_char_t *p_root_name, tr69_node_t **pp_next_node);
tsl_rv_t tr69_get_next_inst_node(tsl_char_t *p_root_name, tr69_node_t **pp_next_node);
tsl_rv_t tr69_get_next_samedepth_node(tsl_char_t *p_root_name, tr69_node_t **pp_next_node);
tsl_rv_t tr69_add_instance(tsl_char_t *p_name, tsl_int_t *p_inst_numb);
tsl_rv_t tr69_del_instance(tsl_char_t *p_name, tsl_int_t inst);
tsl_rv_t tr69_restore_default_node(tsl_char_t *p_name);
tsl_rv_t tr69_del2_instance(tsl_char_t *p_name);
tsl_rv_t tr69_clear_notification();
tsl_rv_t tr69_get_notification(tr69_notification_t **pp_notf_tb, tsl_int_t *p_tb_count);
tsl_rv_t tr69_add_instance_timeout(tsl_char_t *p_name, tsl_int_t timeout);
tsl_rv_t tr69_move_instance(tsl_char_t *p_name_dstobj, tsl_char_t *p_name_srcinst, tsl_int_t *p_inst_numb);


#endif
