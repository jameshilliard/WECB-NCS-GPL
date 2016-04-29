#ifndef TR69_H
#define TR69_H

#include "tr69_tree.h"

#if 0
enum TIMEOUT_REASON{
    TIMEOUT_REASON_NONE,
    TIMEOUT_REASON_DELAYNOTIF,
    TIMEOUT_REASON_DELINST
};

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
#endif

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tr69_register
 *
 *	[DESCRIPTION]:
 *	        register NumberOfEntries table and RegisterFunc table
 *              which exist in autocreate files tr69_inst.h and tr69_st.h.
 *              make sure call this function before calling tr69_init
 *	[ARGUMENT]:
 *	        void *p_inst_tb
 *	        int inst_tb_count
 *              void *p_reg_tb
 *              int reg_tb_count
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/
int tr69_register(void *p_inst_tb, int inst_tb_count, void *p_reg_tb, int reg_tb_count);


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tr69_init
 *
 *	[DESCRIPTION]:
 *	        initiate tr69, make sure called tr69_register.
 *
 *	[ARGUMENT]:
 *	        char *p_protype_xml
 *	        char *p_cfg_xml
 *	        char *p_ori_cfg_xml
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/
int tr69_init(char *p_protype_xml, char *p_cfg_xml, char *p_ori_cfg_xml);


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tr69_cleanup
 *
 *	[DESCRIPTION]:
 *	        cleanup tr69
 *
 *	[ARGUMENT]:
 *
 *
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/
void tr69_cleanup();


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_save_xml
 *
 *	[DESCRIPTION]:
 *	        save tr69 trees into xml
 *
 *	[ARGUMENT]:
 *	        char *p_file_name
 *
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/
int tr69_save_xml(char *p_file_name);


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
 *              whole node tree loops.
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


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_get_notification
 *
 *	[DESCRIPTION]:
 *	        get notificationed parameters, only used for tr69 application
 *              who cared about noactive or active notification leaf parameter
 *
 *	[ARGUMENT]:
 *	        tr69_notification_t **pp_notf_tb
 *	        int *p_tb_count
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/
int tr69_get_notification(tr69_notification_t **pp_notf_tb, int *p_tb_count);


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
 *	        libtr69_server
 *
 *	[DESCRIPTION]:
 *	        only used for data_center and save cfg.xml in default
 *              periodic time
 *
 *	[ARGUMENT]:
 *	        char *p_save_xml
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/
int libtr69_server(char *p_save_xml, int def_wait_timeout);


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        libtr69_server_stop
 *
 *	[DESCRIPTION]:
 *	        only used for data_center to stop service
 *
 *
 *	[ARGUMENT]:
 *	        char *p_save_xml
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/
void libtr69_server_stop();

#endif

