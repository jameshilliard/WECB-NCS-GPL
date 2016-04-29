#ifndef TR069_TREE_H
#define TR069_TREE_H

#include "tsl_list.h"
#include "tsl_common.h"

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

#define TR69_NODE_LEAF_UNNOTIFICATION           0x61
#define TR69_NODE_LEAF_NOTIFICATION             0x62
#define TR69_NODE_LEAF_ACTIVENOTIFICATION       0x63

#define TR69_NODE_LEAF_ATTR_HIDDEN              0x71

#define TR69_RT_SUCCESS_VALUE_UNCHANGED         0x101
#define TR69_RT_SUCCESS_VALUE_CHANGED           0x102


#define TR69_ERROR_BAD_ACL                      0x201


typedef tsl_rv_t (*tr69_data_func_get_obj_t)(tsl_char_t *p_oid_name, tsl_void_t *p_cur_data);
typedef tsl_rv_t (*tr69_data_func_set_obj_t)(tsl_char_t *p_oid_name, tsl_void_t *p_cur_data, tsl_void_t *p_new_data);

typedef struct tr69_info_object_s{
        tsl_char_t name[MAX_NAME_SIZE];
        tsl_char_t short_name[MAX_NAME_SIZE];
        tsl_char_t profile[MAX_NAME_SIZE];
        tsl_void_t *p_data;
        tsl_uint_t data_size;
        tr69_data_func_get_obj_t get_func;
        tr69_data_func_set_obj_t set_func;
}tr69_info_object_t;

typedef struct tr69_info_leaf_s{
        tsl_int_t acl;		
        tsl_int_t type;
        tsl_char_t name[MAX_NAME_SIZE];
        tsl_char_t def_value[MAX_NAME_SIZE];
        tsl_int_t  notification;
        tsl_uint_t data_offset;
        tsl_int_t attr;
        tsl_int_t min;
        tsl_u64_t max;
		tsl_int_t nosave;
#ifdef AEI_PARAM_FORCEUPDATE
        tsl_int_t forceupdate;
#endif
#ifdef GEN_PROTYPE
		tsl_int_t has_hideParameterFromAcs;
		tsl_int_t has_range;
		tsl_int_t has_nosave;
		tsl_int_t has_spec_source;
		tsl_int_t has_profile;
		tsl_int_t has_maxLength;
		tsl_char_t spec_source[MAX_NAME_SIZE];
		tsl_char_t profile[MAX_NAME_SIZE];
		tsl_char_t typestr[MAX_NAME_SIZE];
#endif
}tr69_info_leaf_t;

typedef struct tr69_info_instance_s{
        tsl_int_t size;
        tsl_int_t acl;
        tsl_void_t *p_data;
        tsl_uint_t data_size;
		tsl_int_t nosave;
}tr69_info_instance_t;

typedef struct tr69_info_tree_s{
        sem_t mutex_sem;
        
        tsl_char_t full_name[MAX_NAME_SIZE];
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
        tsl_char_t full_name[MAX_NAME_SIZE];
        tsl_int_t  node_type;
        tsl_int_t  notification;
        tsl_int_t  acl;
}tr69_node_t;


#endif
