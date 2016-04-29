#ifndef LIBTR69_FUNC_H
#define LIBTR69_FUNC_H

/**************************************************************************
 *
 *      Following Function only used in tr69_func functions 
 *
 **************************************************************************/

#define TR69_NODE_TYPE_OBJECT                   0x11
#define TR69_NODE_TYPE_INSTANCE                 0x22
#define TR69_NODE_TYPE_LEAF                     0x33

#define TR69_NODE_LEAF_ACL_READONLY             0x41
#define TR69_NODE_LEAF_ACL_WRITEONLY            0x42
#define TR69_NODE_LEAF_ACL_READWRITE            0x43
#define TR69_NODE_LEAF_ACL2_NOSAVE				0x44

#define TR69_NODE_LEAF_UNNOTIFICATION           0x0
#define TR69_NODE_LEAF_NOTIFICATION             0x1
#define TR69_NODE_LEAF_ACTIVENOTIFICATION       0x2

#define TR69_NODE_LEAF_TYPE_STRING              0x51
#define TR69_NODE_LEAF_TYPE_UINT                0x52
#define TR69_NODE_LEAF_TYPE_INT                 0x53
#define TR69_NODE_LEAF_TYPE_BOOL                0x54
#define TR69_NODE_LEAF_TYPE_DATE                0x55

#define FULL_NAME_SIZE 256

typedef struct tr69_node_s{
        char full_name[FULL_NAME_SIZE];
        int  node_type;
        int  notification;
        int  acl;
}tr69_node_t;

typedef struct tr69_notification_s{
        char full_name[FULL_NAME_SIZE];
        char tap[8];
}tr69_notification_t;

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_get_obj_data
 *
 *	[DESCRIPTION]:
 *	        get object data, please refer tr69_func.h for structure details
 *
 *	[ARGUMENT]:
 *	        char *p_name
 *	        void **pp_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/
int tr69_get_obj_data(char *p_name, void **pp_data);

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_get_unfresh_obj_data
 *
 *	[DESCRIPTION]:
 *	        get object data without data refresh, please refer tr69_func.h for structure details
 *
 *	[ARGUMENT]:
 *	        char *p_name
 *	        void **pp_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/
int tr69_get_unfresh_obj_data(char *p_name, void **pp_data);

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_free_obj_data
 *
 *	[DESCRIPTION]:
 *	        free object data, please refer tr69_func.h for structure details
 *
 *	[ARGUMENT]:
 *	        char *p_name
 *	        void **pp_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/
int tr69_free_obj_data(char *p_name, void **pp_data);


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_set_obj_data
 *
 *	[DESCRIPTION]:
 *	        set object data, please refer tr69_func.h for structure details
 *
 *	[ARGUMENT]:
 *	        char *p_name
 *	        void **pp_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/
int tr69_set_obj_data(char *p_name, void **pp_data);


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_get_leaf_data
 *
 *	[DESCRIPTION]:
 *	        get leaf parameter value
 *
 *	[ARGUMENT]:
 *	        char *p_name
 *	        void **pp_data
 *              int  *type
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/
int tr69_get_leaf_data(char *p_name, void **pp_data, int *type);


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_set_leaf_data
 *
 *	[DESCRIPTION]:
 *	        set leaf parameter value without acl control
 *
 *	[ARGUMENT]:
 *	        char *p_name
 *	        void *p_value
 *              int  type
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/
int tr69_set_leaf_data(char *p_name, void *p_value, int type);


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_set_unfresh_leaf_data
 *
 *	[DESCRIPTION]:
 *	        set leaf parameter value without calling SET Function
 *
 *	[ARGUMENT]:
 *	        char *p_name
 *	        void *p_value
 *              int  type
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/
int tr69_set_unfresh_leaf_data(char *p_name, void *p_value, int type);


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_acl_set_leaf_data
 *
 *	[DESCRIPTION]:
 *	        set leaf parameter value with acl control, if a leaf's 
 *              attribute is read only, this function will return failed. 
 *
 *	[ARGUMENT]:
 *	        char *p_name
 *	        void *p_value
 *              int  type
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/
int tr69_acl_set_leaf_data(char *p_name, void *p_value, int type);


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_get_unfresh_leaf_data
 *
 *	[DESCRIPTION]:
 *	        get leaf parameter value without calling get_func() registered
 *              in tr69_st.h
 *
 *	[ARGUMENT]:
 *	        char *p_name
 *	        void **pp_data
 *              int  type
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/
int tr69_get_unfresh_leaf_data(char *p_name, void **pp_data, int *type);


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_get_node
 *
 *	[DESCRIPTION]:
 *	        get a leaf parameter's attribute
 *
 *	[ARGUMENT]:
 *	        char *p_name
 *	        tr69_node_t **pp_node
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/
int tr69_get_node(char *p_name, tr69_node_t **pp_node);


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_get_next_node
 *
 *	[DESCRIPTION]:
 *	        get next leaf parameter's attribute, usually used in traveling 
 *              whole node tree loops.(this function return with unhidden Node)
 *              for samples, please see tr69.c:GetParameterValue()  
 *
 *	[ARGUMENT]:
 *	        char *p_root_name
 *	        tr69_node_t **pp_next_node
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/
int tr69_get_next_node(char *p_root_name, tr69_node_t **pp_next_node);

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tr69_get_next_node2
 *
 *	[DESCRIPTION]:
 *	        get next leaf parameter's attribute, usually used in traveling 
 *              whole node tree loops.(this function return with hidden/unhidden Node)
 *              for samples, please see tr69.c:GetParameterValue()  
 *
 *	[ARGUMENT]:
 *	        char *p_root_name
 *	        tr69_node_t **pp_next_node
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/
int tr69_get_next_node2(char *p_root_name, tr69_node_t **pp_next_node);

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tr69_get_next_child_node
 *
 *	[DESCRIPTION]:
 *	        get next leaf parameter's attribute, usually used in traveling 
 *              whole node tree loops.(this function return with OBJECT/INSTANCE/LEAF Node)
 *              for samples, please see tr69.c:GetParameterValue()  
 *
 *	[ARGUMENT]:
 *	        char *p_root_name
 *	        tr69_node_t **pp_next_node
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/
int tr69_get_next_child_node(char *p_root_name, tr69_node_t **pp_next_node);

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_get_next_obj_node
 *
 *	[DESCRIPTION]:
 *	        get next OBJECT parameter's attribute, usually used in traveling 
 *              whole node tree loops.(compare to tr69_get_next_node(), this 
 *              function only return with OBJECT Node)
 *              for samples, please see tr69.c:GetParameterValue()  
 *
 *	[ARGUMENT]:
 *	        char *p_root_name
 *	        tr69_node_t **pp_next_node
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/
int tr69_get_next_obj_node(char *p_root_name, tr69_node_t **pp_next_node);

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_set_node
 *
 *	[DESCRIPTION]:
 *	        set a leaf parameter's attribute including acl and notification
 *
 *	[ARGUMENT]:
 *	        char *p_name
 *	        tr69_node_t *p_node
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/
int tr69_set_node(char *p_name, tr69_node_t *p_node);


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_add_instance
 *
 *	[DESCRIPTION]:
 *	        add an instance. Be notice the correspoding XXNumberofEntries 
 *              is automatically increased, a user does not need to care about 
 *              XXXNumberofEntries. 
 *
 *	[ARGUMENT]:
 *	        char *p_name
 *	        int *p_inst_numb
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/
int tr69_add_instance(char *p_name, int *p_inst_numb);


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_add_instance_timeout
 *
 *	[DESCRIPTION]:
 *	        The instance will be automatically deleted when the timeout 
 *              expired, sample used in PortMappingDurationExpire. So that  
 *              a user can feel comfortable when dealing with the instance 
 *              timeout issue.   
 *
 *	[ARGUMENT]:
 *	        char *p_name: oid_full_name
 *	        int timeout: timeout seconds
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/
int tr69_add_instance_timeout(char *p_name, int timeout);


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_del_instance
 *
 *	[DESCRIPTION]:
 *	        del an instance with an instance number.
 *              Be notice the correspoding XXNumberofEntries 
 *              is automatically decreased, a user does not need to care about 
 *              XXXNumberofEntries. 
 *
 *	[ARGUMENT]:
 *	        char *p_name
 *	        int inst
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/
int tr69_del_instance(char *p_name, int inst);


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_del2_instance
 *
 *	[DESCRIPTION]:
 *	        del instance with full path name
 *              Be notice the correspoding XXNumberofEntries 
 *              is automatically decreased, a user does not need to care about 
 *              XXXNumberofEntries. 
 *
 *	[ARGUMENT]:
 *	        char *p_name
 *	        
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/
int tr69_del2_instance(char *p_name);


tsl_rv_t tr69_restore_default_node(tsl_char_t *p_name);

tsl_rv_t tr69_get_obj_data_size(tsl_char_t *p_name, tsl_int_t *data_size);

tsl_rv_t tr69_get_next_inst_node(tsl_char_t *p_root_name, tr69_node_t **pp_next_node);

tsl_rv_t tr69_get_next_samedepth_node(tsl_char_t *p_root_name, tr69_node_t **pp_next_node);

tsl_rv_t tr69_get_obj_by_oid_flag(char* full_name, tsl_u32_t getFlags, void **obj);

tsl_rv_t tr69_set_unfresh_obj_data(tsl_char_t *p_name, tsl_void_t **pp_data);

#endif

