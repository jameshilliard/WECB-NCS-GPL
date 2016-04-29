#include "tr69_tree.h"
#include "tsl_msg.h"
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <ctype.h>
#include "ctl_log.h"

extern struct tr69_inst_numb_of_entries_s tr69_inst_numb_tb[];
extern struct tr69_register_func tr69_regfunc_tb[];
tsl_int_t tr69_inst_numb_tb_count = 0;
tsl_int_t tr69_regfunc_tb_count = 0;
extern tsl_char_t st_version[];

tsl_int_t tr69_is_data_changed = 0;

tr69_info_tree_t *pg_origin_tree = NULL;
tr69_info_tree_t *pg_root_tree   = NULL;
tr69_info_tree_t *pg_ghost_tree  = NULL;
tr69_info_tree_t *pg_ghost_tree_malloc  = NULL;
tsl_list_head_t global_notification_list_head = {&global_notification_list_head, &global_notification_list_head};

tsl_list_head_t global_timeout_instance_list_head = {&global_timeout_instance_list_head, &global_timeout_instance_list_head};

#define NOTIF_DEVNUM_INDEX  0
tr69_delaynotification_t global_delaynotifications[1] = {
    {"InternetGatewayDevice.ManagementServer.ManageableDeviceNumberOfEntries", 0, 0, 0},
};

tsl_void_t *tr69_inline_dump_data(tr69_info_tree_t *p_tree);
tsl_rv_t tr69_inline_free_data(tr69_info_tree_t *p_tree, tsl_void_t **pp_data);
tsl_rv_t tr69_inline_get_node(tr69_info_tree_t *p_root, tsl_char_t *p_name, tr69_info_tree_t **pp_find_node);
tsl_rv_t tr69_inline_add_notification(tsl_char_t *p_oid_name, tsl_int_t active);
tsl_rv_t tr69_get_leaf_data(tsl_char_t *p_name, tsl_void_t **pp_data, tsl_int_t *type);
tsl_rv_t tr69_get_unfresh_leaf_data(tsl_char_t *p_name, tsl_void_t **pp_data, tsl_int_t *type);
tsl_rv_t tr69_set_leaf_data(tsl_char_t *p_name, tsl_void_t *p_value, tsl_int_t type);
tsl_rv_t tr69_set_unfresh_leaf_data(tsl_char_t *p_name, tsl_void_t *p_value, tsl_int_t type);
tsl_rv_t tr69_del2_instance(tsl_char_t *p_name);
tsl_rv_t tr69_inline_save_data(tr69_info_tree_t *p_tree, tsl_void_t *p_data);
xmlNode *tr69_func_inline_tree_to_xml(tr69_info_tree_t *p_root, xmlNode *p_parent_node);
tsl_rv_t tr69_intf_mod_instance(tsl_char_t *p_name, tsl_int_t rel);

tsl_char_t protype_xml_version[128] = "\0";
tsl_char_t cfg_xml_version[128] = "\0";

tsl_bool_t tr69_check_oid_is_default_inst(tsl_char_t *p_name)
{
    tsl_int_t i = 0;
	tsl_char_t oid_stack[32][128];
	tsl_int_t max_stack = 0;
    tsl_char_t *p_match = NULL;
    tsl_char_t *p_tmp = p_name;

    TSL_VASSERT(p_name != NULL);

	memset(oid_stack, 0, sizeof(oid_stack));

	//tr69_common_func_parse_oid(p_name, oid_stack, &max_stack);
    while ((p_match = strchr(p_tmp, '.')) != NULL){
        memcpy(oid_stack[max_stack++], p_tmp, p_match - p_tmp);
        p_tmp = p_match + 1;
    }
    memcpy(oid_stack[max_stack], p_tmp, p_name + strlen(p_name) - p_tmp);

	for (i = 0; i <= max_stack; i++){
        if (isdigit(oid_stack[i][0])){
            if (atoi(oid_stack[i]) == 0){
			    return tsl_b_true;
		    }else{
                return tsl_b_false;
            }
        }
	}

	return tsl_b_false;
}

tsl_int_t tr69_ghost_func_count_child_node(xmlNode *p_node, tsl_char_t *p_name, tsl_int_t *p_count)
{
    tsl_char_t type = 0;
    xmlChar *p_char  = NULL;
    xmlNode *p_child = NULL;
    tsl_int_t inst_size = 0;
    tsl_char_t oid[MAX_NAME_SIZE] = {0};
    tsl_char_t full_name[MAX_NAME_SIZE] = {0};

    TSL_VASSERT_RV(p_node != NULL, 0);
    TSL_VASSERT_RV(p_count != NULL, 0);

    if (!strcmp((char *)p_node->name, "Object")){
        type = TR69_NODE_TYPE_OBJECT;
    }else if (!strcmp((char *)p_node->name, "Instance")){
        type = TR69_NODE_TYPE_INSTANCE;
    }else if(!strcmp((char *)p_node->name, "Parameter")){
        type = TR69_NODE_TYPE_LEAF;
    }
    TSL_VASSERT_RV(type == TR69_NODE_TYPE_OBJECT ||
                   type == TR69_NODE_TYPE_INSTANCE ||
                   type == TR69_NODE_TYPE_LEAF, 0);
    switch(type){
        case TR69_NODE_TYPE_OBJECT:
            p_char = xmlGetProp(p_node, (xmlChar *)"name");
    	    if(p_name == NULL){
        		sprintf(full_name, "%s", p_char);
    	    }else {
        		sprintf(full_name, "%s.%s", p_name, p_char);
    	    }
    	    printf("OBJ:%s %s\n", p_node->name, p_char);
    	    xmlFree(p_char);
            break;
        case TR69_NODE_TYPE_INSTANCE:
            p_char = xmlGetProp(p_node, (xmlChar *)"size");
            inst_size = atoi((char *)p_char);
            xmlFree(p_char);

            if (p_name == NULL){
		        sprintf(full_name, "%d", inst_size);
    	    }else {
                sprintf(full_name, "%s.%d", p_name, inst_size);
    	    }

            sprintf(oid, "%s.%d", p_name, inst_size);
            if (tr69_check_oid_is_default_inst(oid) == tsl_b_false)
            {
        		return 0;
            }
            break;
        case TR69_NODE_TYPE_LEAF:
            p_char = xmlGetProp(p_node, (xmlChar *)"name");
            if (p_name == NULL){
    		    sprintf(full_name, "%s", p_char);
    	    }else {
    		    sprintf(full_name, "%s.%s", p_name, p_char);
    	    }
    	    printf("LEAF:%s %s\n", p_node->name, p_char);
    	    xmlFree(p_char);
            break;
    }
    *p_count += 1;

    if (p_node->type == XML_ELEMENT_NODE){
	    p_child = p_node->children;
	    while(p_child != NULL){
    		if (p_child->type == XML_ELEMENT_NODE){
    		    tr69_ghost_func_count_child_node(p_child, full_name, p_count);
    		}
    		p_child = p_child->next;
	    }
    }

    return *p_count;
}

tsl_int_t tr69_ghost_func_count_child_parameters(xmlNode *p_node, tsl_char_t *p_name, tsl_int_t *p_count)
{
    tsl_char_t type = 0;
    xmlChar *p_char  = NULL;
    xmlNode *p_child = NULL;
    tsl_int_t inst_size = 0;
    tsl_char_t oid[MAX_NAME_SIZE] = {0};
    tsl_char_t full_name[MAX_NAME_SIZE] = {0};

    TSL_VASSERT_RV(p_node != NULL, 0);
    TSL_VASSERT_RV(p_count != NULL, 0);

    if (!strcmp((char *)p_node->name, "Object")){
        type = TR69_NODE_TYPE_OBJECT;
    }else if (!strcmp((char *)p_node->name, "Instance")){
        type = TR69_NODE_TYPE_INSTANCE;
    }else if(!strcmp((char *)p_node->name, "Parameter")){
        type = TR69_NODE_TYPE_LEAF;
    }
    TSL_VASSERT_RV(type == TR69_NODE_TYPE_OBJECT ||
                   type == TR69_NODE_TYPE_INSTANCE ||
                   type == TR69_NODE_TYPE_LEAF, 0);
    switch(type){
        case TR69_NODE_TYPE_OBJECT:
            p_char = xmlGetProp(p_node, (xmlChar *)"name");
    	    if(p_name == NULL){
        		sprintf(full_name, "%s", p_char);
    	    }else {
        		sprintf(full_name, "%s.%s", p_name, p_char);
    	    }
    	    printf("OBJ:%s %s\n", p_node->name, p_char);
    	    xmlFree(p_char);
            break;
        case TR69_NODE_TYPE_INSTANCE:
            p_char = xmlGetProp(p_node, (xmlChar *)"size");
            inst_size = atoi((char *)p_char);
            xmlFree(p_char);

            if (p_name == NULL){
		        sprintf(full_name, "%d", inst_size);
    	    }else {
                sprintf(full_name, "%s.%d", p_name, inst_size);
    	    }

            sprintf(oid, "%s.%d", p_name, inst_size);
            if (tr69_check_oid_is_default_inst(oid) == tsl_b_false)
            {
        		return 0;
            }
            break;
        case TR69_NODE_TYPE_LEAF:
            p_char = xmlGetProp(p_node, (xmlChar *)"name");
            if (p_name == NULL){
    		    sprintf(full_name, "%s", p_char);
    	    }else {
    		    sprintf(full_name, "%s.%s", p_name, p_char);
    	    }
    	    printf("LEAF:%s %s\n", p_node->name, p_char);
    	    xmlFree(p_char);
            *p_count += 1;
            break;
    }

    if (p_node->type == XML_ELEMENT_NODE){
	    p_child = p_node->children;
	    while(p_child != NULL){
    		if (p_child->type == XML_ELEMENT_NODE){
    		    tr69_ghost_func_count_child_parameters(p_child, full_name, p_count);
    		}
    		p_child = p_child->next;
	    }
    }

    return *p_count;
}

#ifdef TR69_SUPPORT_HIDE_OBJ_FROM_ACS
tr69_info_tree_t *tr69_ghost_func_add_child_node(xmlNode *p_node, tr69_info_tree_t *p_tree_head, tsl_char_t *p_name, tsl_int_t *p_count, tsl_bool_t hide)
#else
tr69_info_tree_t *tr69_ghost_func_add_child_node(xmlNode *p_node, tr69_info_tree_t *p_tree_head, tsl_char_t *p_name, tsl_int_t *p_count)
#endif
{
        tr69_info_tree_t *p_tree  = NULL;
        tsl_char_t        type    = 0;
        xmlChar          *p_char  = NULL;
        xmlNode          *p_child = NULL;
		tsl_int_t        acl      = 0;
		tsl_int_t        nosave      = 0;
        static tsl_int_t sg_offset = 0;
        tsl_int_t        off      = 0;
        tsl_char_t oid[MAX_NAME_SIZE] = {0};

        TSL_VASSERT_RV(p_node != NULL, NULL);
        TSL_VASSERT_RV(p_count != NULL, NULL);

        if (!strcmp((char *)p_node->name, "Object")){
	    type = TR69_NODE_TYPE_OBJECT;
        }else if (!strcmp((char *)p_node->name, "Instance")){
	    type = TR69_NODE_TYPE_INSTANCE;
        }else if(!strcmp((char *)p_node->name, "Parameter")){
	    type = TR69_NODE_TYPE_LEAF;
        }
        TSL_VASSERT_RV(type == TR69_NODE_TYPE_OBJECT ||
                       type == TR69_NODE_TYPE_INSTANCE ||
                       type == TR69_NODE_TYPE_LEAF, NULL);


        //p_tree = (tr69_info_tree_t *)calloc(1, sizeof(tr69_info_tree_t));
		//memset(p_tree, 0x0, sizeof(tr69_info_tree_t));
		p_tree = &pg_ghost_tree_malloc[*p_count];
        if (p_tree == NULL){
            ctllog_debug("Why tree is NULL!(%d)\n", *p_count);
            return NULL;
        }
        //TSL_VASSERT_RV(p_tree != NULL, NULL);
        *p_count += 1;

        switch(type){
        case TR69_NODE_TYPE_OBJECT:

	    p_tree->node_type = type;
	    p_char = xmlGetProp(p_node, (xmlChar *)"name");
	    sprintf(p_tree->info.info_obj.name, "%s", p_char);
	    printf("OBJ:%s %s\n", p_node->name, p_char);

	    xmlFree(p_char);

	    if(p_name == NULL){
		sprintf(p_tree->full_name, "%s", p_tree->info.info_obj.name);
	    }else {
		sprintf(p_tree->full_name, "%s.%s", p_name, p_tree->info.info_obj.name);
	    }

            sg_offset = 0;

#ifdef TR69_SUPPORT_HIDE_OBJ_FROM_ACS
                p_tree->info.info_obj.hide = TSL_B_FALSE;
                if( hide ) {
                    p_tree->info.info_obj.hide = TSL_B_TRUE;
                }
                p_char = xmlGetProp(p_node, (xmlChar *)"hideObjectFromAcs");
                if (p_char){
                    if (!strcmp((tsl_char_t *)p_char, "true")){
                        p_tree->info.info_obj.attr = TR69_NODE_LEAF_ATTR_HIDDEN;
                        p_tree->info.info_obj.hide = TSL_B_TRUE;
                    }
                    xmlFree(p_char);
                }
#endif
            if (!strcmp(p_tree->info.info_obj.name, "Device")){
                    p_char = xmlGetProp(p_node, (xmlChar *)"framework_xml_version");
                    if (p_char){
                            sprintf(protype_xml_version, "%s", p_char);
                            printf("version:%s\n", p_char);
                            xmlFree(p_char);
                    }
            }

	    break;
        case TR69_NODE_TYPE_INSTANCE:
	    printf("INS:%s\n", p_node->name);
	    p_tree->node_type = type;

	    p_char = xmlGetProp(p_node, (xmlChar *)"size");
	    p_tree->info.info_inst.size = atoi((char *)p_char);
	    xmlFree(p_char);

        sprintf(oid, "%s.%d", p_name, p_tree->info.info_inst.size);
        if (tr69_check_oid_is_default_inst(oid) == tsl_b_false)
        {
            //free(p_tree);
    		//p_tree = NULL;
    		*p_count -= 1;
    		return NULL;
        }

	    p_char = xmlGetProp(p_node, (xmlChar *)"readable");
            acl = 10 * atoi((char *)p_char);
	    xmlFree(p_char);

	    p_char = xmlGetProp(p_node, (xmlChar *)"writable");
	    acl = acl + atoi((char *)p_char);
	    xmlFree(p_char);

	    if (acl == 10){
		p_tree->info.info_inst.acl = TR69_NODE_LEAF_ACL_READONLY;
	    }else if (acl == 11){
		p_tree->info.info_inst.acl = TR69_NODE_LEAF_ACL_READWRITE;
	    }

#ifdef TR69_SUPPORT_HIDE_OBJ_FROM_ACS
                p_tree->info.info_inst.hide = TSL_B_FALSE;
                if( hide ) {
                    p_tree->info.info_inst.hide = TSL_B_TRUE;
                }
                p_char = xmlGetProp(p_node, (xmlChar *)"hideObjectFromAcs");
                if (p_char){
                    if (!strcmp((tsl_char_t *)p_char, "true")){
                        p_tree->info.info_inst.attr = TR69_NODE_LEAF_ATTR_HIDDEN;
                        p_tree->info.info_inst.hide = TSL_B_TRUE;
                    }
                    xmlFree(p_char);
                }
#endif
		p_tree->info.info_inst.nosave = 0;
		p_char = xmlGetProp(p_node, (xmlChar *)"nosave");
		if (p_char != NULL){
		    nosave = atoi((char *)p_char);
		    xmlFree(p_char);
			if (nosave){
				p_tree->info.info_inst.nosave = TR69_NODE_LEAF_ACL2_NOSAVE;
			}
		}

	    if (p_name == NULL){
		sprintf(p_tree->full_name, "%d",p_tree->info.info_inst.size);
	    }else {
		sprintf(p_tree->full_name, "%s.%d", p_name, p_tree->info.info_inst.size);
	    }

            sg_offset = 0;

	    break;
        case TR69_NODE_TYPE_LEAF:
	    p_tree->node_type = type;

            p_char = xmlGetProp(p_node, (xmlChar *)"type");

            if (strstr((tsl_char_t *)p_char, "string")){
                    p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_STRING;
                    off = sizeof(tsl_char_t *);
            }else if (strstr((tsl_char_t *)p_char, "unsignedInt")){
                    p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_UINT;
                    off = sizeof(tsl_uint_t);
	    }else if (strstr((tsl_char_t *)p_char, "int")){
                    p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_INT;
                    off = sizeof(tsl_int_t);
	    }else if (!strcmp((tsl_char_t *)p_char, "boolean")){
                    p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_BOOL;
                    off = sizeof(tsl_int_t);
	    }else if (!strcmp((tsl_char_t *)p_char, "dateTime")){
                    p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_DATE;
                    off = sizeof(tsl_char_t *);
	    }
	    xmlFree(p_char);

	    p_char = xmlGetProp(p_node, (xmlChar *)"readable");
	    acl = 10 * atoi((char *)p_char);
	    xmlFree(p_char);

	    p_char = xmlGetProp(p_node, (xmlChar *)"writable");
	    acl = acl + atoi((char *)p_char);
	    xmlFree(p_char);

	    if (acl == 10){
		p_tree->info.info_leaf.acl = TR69_NODE_LEAF_ACL_READONLY;
	    }else if (acl == 11){
		p_tree->info.info_leaf.acl = TR69_NODE_LEAF_ACL_READWRITE;
	    }

		p_tree->info.info_leaf.nosave = 0;
		p_char = xmlGetProp(p_node, (xmlChar *)"nosave");
		if (p_char != NULL){
		    nosave = atoi((char *)p_char);
		    xmlFree(p_char);
			if (nosave){
				p_tree->info.info_leaf.nosave = TR69_NODE_LEAF_ACL2_NOSAVE;
			}
		}

	    p_char = xmlGetProp(p_node, (xmlChar *)"name");
	    sprintf(p_tree->info.info_leaf.name, "%s", p_char);
	    printf("LEAF:%s %s\n", p_node->name, p_char);
	    xmlFree(p_char);

	    if (p_name == NULL){
		sprintf(p_tree->full_name, "%s", p_tree->info.info_leaf.name);
	    }else {
		sprintf(p_tree->full_name, "%s.%s", p_name, p_tree->info.info_leaf.name);
	    }

            if (p_tree_head->node_type == TR69_NODE_TYPE_OBJECT){
                    if (sizeof(tsl_char_t *) == 8){
			// 64 Bit Machine
			p_tree_head->info.info_obj.data_size += sizeof(tsl_char_t *);
		    }else {
		    	p_tree_head->info.info_obj.data_size += off;
		    }
            }else if (p_tree_head->node_type == TR69_NODE_TYPE_INSTANCE){
		    if (sizeof(tsl_char_t *) == 8){
			// 64 Bit Machine
			p_tree_head->info.info_inst.data_size += sizeof(tsl_char_t *);
		    }else {
                    	p_tree_head->info.info_inst.data_size += off;
		    }
            }

            p_char = xmlGetProp(p_node, (xmlChar *)"default");
            if (strcmp((tsl_char_t *)p_char, "(null)")){
                    snprintf(p_tree->info.info_leaf.def_value, sizeof(p_tree->info.info_leaf.def_value), "%s", p_char);
            }
            xmlFree(p_char);

            p_char = xmlGetProp(p_node, (xmlChar *)"notification");
            if (atoi((tsl_char_t *)p_char)){
                    if ((atoi((tsl_char_t *)p_char)) == 1){
                            p_tree->info.info_leaf.notification = TR69_NODE_LEAF_NOTIFICATION;
                    }else if ((atoi((tsl_char_t *)p_char)) == 2){
                            p_tree->info.info_leaf.notification = TR69_NODE_LEAF_ACTIVENOTIFICATION;
                    }
            }else {
                    p_tree->info.info_leaf.notification = TR69_NODE_LEAF_UNNOTIFICATION;
            }
            xmlFree(p_char);

            if (sizeof(tsl_char_t *) == 8 &&
                (p_tree->info.info_leaf.type == TR69_NODE_LEAF_TYPE_STRING ||
                 p_tree->info.info_leaf.type == TR69_NODE_LEAF_TYPE_DATE)){
                    //64 Bit Machine
                    sg_offset = MEM_ALIGN(sg_offset, 8);
            }

            p_tree->info.info_leaf.data_offset = sg_offset;

            sg_offset += off;

           // ctllog_debug("%s offset %d\n", p_tree->full_name, p_tree->info.info_leaf.data_offset);

#ifdef TR69_SUPPORT_HIDE_OBJ_FROM_ACS
            p_tree->info.info_leaf.hide = TSL_B_FALSE;
            if( hide ) {
                p_tree->info.info_leaf.hide = TSL_B_TRUE;
            }
#endif
            p_char = xmlGetProp(p_node, (xmlChar *)"hideParameterFromAcs");
            if (p_char){
                    if (!strcmp((tsl_char_t *)p_char, "true")){
                            p_tree->info.info_leaf.attr = TR69_NODE_LEAF_ATTR_HIDDEN;
#ifdef TR69_SUPPORT_HIDE_OBJ_FROM_ACS
                        p_tree->info.info_leaf.hide = TSL_B_TRUE;
#endif
                    }
                    xmlFree(p_char);
            }

            p_tree->info.info_leaf.min = 0;
            p_tree->info.info_leaf.max = 0;

            p_char = xmlGetProp(p_node, (xmlChar *)"minValue");
            if (p_char){
                    p_tree->info.info_leaf.min = atoi((char *)p_char);
                    xmlFree(p_char);
            }

            p_char = xmlGetProp(p_node, (xmlChar *)"maxValue");
            if (p_char){
                    p_tree->info.info_leaf.max = atoi((char *)p_char);
                    xmlFree(p_char);
            }

            p_char = xmlGetProp(p_node, (xmlChar *)"maxLength");
            if (p_char){
                    p_tree->info.info_leaf.max = atoi((char *)p_char);
                    xmlFree(p_char);
            }

            break;
        }

        TSL_INIT_LIST_HEAD(&p_tree->list_head);
        TSL_INIT_LIST_HEAD(&p_tree->list_node);

        if (p_tree_head != NULL){
                tsl_list_add(&p_tree->list_node, &p_tree_head->list_head);
                p_tree->p_parent = p_tree_head;
        }

        if (p_node->type == XML_ELEMENT_NODE){
#ifdef TR69_SUPPORT_HIDE_OBJ_FROM_ACS
            tsl_bool_t hide_child_nodes = TSL_B_FALSE;
            if( (TR69_NODE_TYPE_OBJECT == type) &&
                    (TSL_B_TRUE == p_tree->info.info_obj.hide)) {
                hide_child_nodes = TSL_B_TRUE;
            } else if( (TR69_NODE_TYPE_INSTANCE == type) &&
                    (TSL_B_TRUE == p_tree->info.info_inst.hide)) {
                hide_child_nodes = TSL_B_TRUE;
            }
#endif
            p_child = p_node->children;
            while(p_child != NULL){
                if (p_child->type == XML_ELEMENT_NODE){
#ifdef TR69_SUPPORT_HIDE_OBJ_FROM_ACS
                    tr69_ghost_func_add_child_node(p_child, p_tree, p_tree->full_name, p_count, hide_child_nodes);
#else
                    tr69_ghost_func_add_child_node(p_child, p_tree, p_tree->full_name, p_count);
#endif
                }
                p_child = p_child->next;
            }
        }

        return p_tree;
}

tsl_int_t tr69_func_count_child_node(xmlNode *p_node, tsl_int_t *p_count)
{
    tsl_char_t        type    = 0;
    xmlNode          *p_child = NULL;

    TSL_VASSERT_RV(p_node != NULL, 0);
    TSL_VASSERT_RV(p_count != NULL, 0);

    if (!strcmp((char *)p_node->name, "Object")){
        type = TR69_NODE_TYPE_OBJECT;
    }else if (!strcmp((char *)p_node->name, "Instance")){
        type = TR69_NODE_TYPE_INSTANCE;
    }else if(!strcmp((char *)p_node->name, "Parameter")){
        type = TR69_NODE_TYPE_LEAF;
    }

    TSL_VASSERT_RV(type == TR69_NODE_TYPE_OBJECT ||
                   type == TR69_NODE_TYPE_INSTANCE ||
                   type == TR69_NODE_TYPE_LEAF, 0);

    *p_count += 1;

    if (p_node->type == XML_ELEMENT_NODE){
	    p_child = p_node->children;
        while(p_child != NULL){
        	if (p_child->type == XML_ELEMENT_NODE){
                tr69_func_count_child_node(p_child, p_count);
        	}
    	    p_child = p_child->next;
        }
    }

    return *p_count;
}

#ifdef TR69_SUPPORT_HIDE_OBJ_FROM_ACS
tr69_info_tree_t *tr69_func_add_child_node(xmlNode *p_node, tr69_info_tree_t *p_tree_head, tsl_char_t *p_name, tsl_int_t data_alloc, tsl_bool_t hide )
#else
tr69_info_tree_t *tr69_func_add_child_node(xmlNode *p_node, tr69_info_tree_t *p_tree_head, tsl_char_t *p_name, tsl_int_t data_alloc)
#endif
{
        tr69_info_tree_t *p_tree  = NULL;
        tsl_char_t        type    = 0;
        xmlChar          *p_char  = NULL;
        xmlNode          *p_child = NULL;
        static tsl_int_t s_offset = 0;
        tsl_int_t        off      = 0;

        TSL_VASSERT_RV(p_node != NULL, NULL);

        if (!strcmp((char *)p_node->name, "Object")){
	    type = TR69_NODE_TYPE_OBJECT;
        }else if (!strcmp((char *)p_node->name, "Instance")){
	    type = TR69_NODE_TYPE_INSTANCE;
        }else if(!strcmp((char *)p_node->name, "Parameter")){
	    type = TR69_NODE_TYPE_LEAF;
        }
        TSL_VASSERT_RV(type == TR69_NODE_TYPE_OBJECT ||
                       type == TR69_NODE_TYPE_INSTANCE ||
                       type == TR69_NODE_TYPE_LEAF, NULL);

        p_tree = (tr69_info_tree_t *)calloc(1, sizeof(tr69_info_tree_t));

        switch(type){
        case TR69_NODE_TYPE_OBJECT:
                p_tree->node_type = type;
                p_char = xmlGetProp(p_node, (xmlChar *)"name");
                sprintf(p_tree->info.info_obj.name, "%s", p_char);
                printf("OBJ:%s %s\n", p_node->name, p_char);
                xmlFree(p_char);

                if(p_name == NULL){
                        sprintf(p_tree->full_name, "%s", p_tree->info.info_obj.name);
                }else {
                        sprintf(p_tree->full_name, "%s.%s", p_name, p_tree->info.info_obj.name);
                }

                p_char = xmlGetProp(p_node, (xmlChar *)"data_size");
                if (p_char){
                        p_tree->info.info_obj.data_size = atoi((char *)p_char);
                        if (sizeof(tsl_char_t *) == 8){
                                //64 Bit Machine
                                p_tree->info.info_obj.data_size = p_tree->info.info_obj.data_size*2;
                        }
                }
                xmlFree(p_char);

                if (p_tree->info.info_obj.data_size && data_alloc){
                        p_tree->info.info_obj.p_data = calloc(1, p_tree->info.info_obj.data_size);
                }

                s_offset = 0;

#ifdef TR69_SUPPORT_HIDE_OBJ_FROM_ACS
                p_tree->info.info_obj.hide = TSL_B_FALSE;
                if( hide ) {
                    p_tree->info.info_obj.hide = TSL_B_TRUE;
                }
                p_char = xmlGetProp(p_node, (xmlChar *)"hideObjectFromAcs");
                if (p_char){
                    if (!strcmp((tsl_char_t *)p_char, "true")){
                        p_tree->info.info_obj.attr = TR69_NODE_LEAF_ATTR_HIDDEN;
                        p_tree->info.info_obj.hide = TSL_B_TRUE;
                    }
                    xmlFree(p_char);
                }
#endif

                if (!strcmp(p_tree->info.info_obj.name, "Device")){
                        p_char = xmlGetProp(p_node, (xmlChar *)"framework_xml_version");
                        if (p_char){
                                sprintf(cfg_xml_version, "%s", p_char);
                                printf("version:%s\n", p_char);
                                xmlFree(p_char);
                        }
                }

                break;
        case TR69_NODE_TYPE_INSTANCE:
                printf("INS:%s\n", p_node->name);
                p_tree->node_type = type;

                p_char = xmlGetProp(p_node, (xmlChar *)"size");
                p_tree->info.info_inst.size = atoi((char *)p_char);
                xmlFree(p_char);

                p_char = xmlGetProp(p_node, (xmlChar *)"acl");
                if (!strcmp((tsl_char_t *)p_char, "ReadOnly")){
                        p_tree->info.info_inst.acl = TR69_NODE_LEAF_ACL_READONLY;
                }else if (!strcmp((tsl_char_t *)p_char, "ReadWrite")){
                        p_tree->info.info_inst.acl = TR69_NODE_LEAF_ACL_READWRITE;
                }
                xmlFree(p_char);
#ifdef TR69_SUPPORT_HIDE_OBJ_FROM_ACS
                p_tree->info.info_inst.hide = TSL_B_FALSE;
                if( hide ) {
                    p_tree->info.info_inst.hide = TSL_B_TRUE;
                }
                p_char = xmlGetProp(p_node, (xmlChar *)"hideObjectFromAcs");
                if (p_char){
                    if (!strcmp((tsl_char_t *)p_char, "true")){
                        p_tree->info.info_inst.attr = TR69_NODE_LEAF_ATTR_HIDDEN;
                        p_tree->info.info_inst.hide = TSL_B_TRUE;
                    }
                    xmlFree(p_char);
                }
#endif

				p_tree->info.info_inst.nosave = 0;
				p_char = xmlGetProp(p_node, (xmlChar *)"nosave");
				if (p_char != NULL){
					if (atoi((char *)p_char)){
						p_tree->info.info_inst.nosave = TR69_NODE_LEAF_ACL2_NOSAVE;
					}
	                xmlFree(p_char);
				}


                if (p_name == NULL){
                        sprintf(p_tree->full_name, "%d",p_tree->info.info_inst.size);
                }else {
                        sprintf(p_tree->full_name, "%s.%d", p_name, p_tree->info.info_inst.size);
                }

                p_char = xmlGetProp(p_node, (xmlChar *)"data_size");
                if (p_char){
                        p_tree->info.info_inst.data_size = atoi((char *)p_char);

                        if (sizeof(tsl_char_t *) == 8){
                                //64 Bit Machine
                                p_tree->info.info_inst.data_size = p_tree->info.info_inst.data_size*2;
                        }
                }
                xmlFree(p_char);

                if (p_tree->info.info_inst.data_size && data_alloc){
                        p_tree->info.info_inst.p_data = calloc(1, p_tree->info.info_inst.data_size);
                }

                s_offset = 0;

                break;
        case TR69_NODE_TYPE_LEAF:
                p_tree->node_type = type;

                p_char = xmlGetProp(p_node, (xmlChar *)"type");

                if (strstr((tsl_char_t *)p_char, "string")){
                        p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_STRING;
                        off = sizeof(tsl_char_t *);
                }else if (strstr((tsl_char_t *)p_char, "unsignedInt")){
                        p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_UINT;
                        off = sizeof(tsl_uint_t);
                }else if (!strcmp((tsl_char_t *)p_char, "Int")){
                        p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_INT;
                        off = sizeof(tsl_int_t);
                }else if (!strcmp((tsl_char_t *)p_char, "boolean")){
                        p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_BOOL;
                        off = sizeof(tsl_int_t);
                }else if (!strcmp((tsl_char_t *)p_char, "dateTime")){
                        p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_DATE;
                        off = sizeof(tsl_char_t *);
                }
                xmlFree(p_char);

                p_char = xmlGetProp(p_node, (xmlChar *)"acl");
                if (!strcmp((tsl_char_t *)p_char, "ReadOnly")){
                        p_tree->info.info_leaf.acl = TR69_NODE_LEAF_ACL_READONLY;
                }else if (!strcmp((tsl_char_t *)p_char, "ReadWrite")){
                        p_tree->info.info_leaf.acl = TR69_NODE_LEAF_ACL_READWRITE;
                }
                xmlFree(p_char);

				p_tree->info.info_leaf.nosave = 0;
				p_char = xmlGetProp(p_node, (xmlChar *)"nosave");
				if (p_char != NULL){
					if (atoi((char *)p_char)){
						p_tree->info.info_leaf.nosave = TR69_NODE_LEAF_ACL2_NOSAVE;
					}
	                xmlFree(p_char);
				}

                p_char = xmlGetProp(p_node, (xmlChar *)"name");
                sprintf(p_tree->info.info_leaf.name, "%s", p_char);
                printf("LEAF:%s %s\n", p_node->name, p_char);
                xmlFree(p_char);

                p_char = xmlNodeGetContent(p_node);
                printf("LEAF value :%s %s\n", p_node->name, p_char);
				if (p_char){
                    snprintf(p_tree->info.info_leaf.def_value, sizeof(p_tree->info.info_leaf.def_value), "%s", p_char);
            	}

                if (p_name == NULL){
                        sprintf(p_tree->full_name, "%s", p_tree->info.info_leaf.name);
                }else {
                        sprintf(p_tree->full_name, "%s.%s", p_name, p_tree->info.info_leaf.name);
                }

                if (sizeof(tsl_char_t *) == 8 &&
                    (p_tree->info.info_leaf.type == TR69_NODE_LEAF_TYPE_STRING ||
                     p_tree->info.info_leaf.type == TR69_NODE_LEAF_TYPE_DATE)){
                        //64 Bit Machine
                        s_offset = MEM_ALIGN(s_offset, 8);
                }

                p_tree->info.info_leaf.data_offset = s_offset;
                s_offset += off;

                //ctllog_debug("%s offset %d\n", p_tree->full_name, p_tree->info.info_leaf.data_offset);

                if (p_char != NULL && strlen(p_char) > 0 && p_tree_head->node_type == TR69_NODE_TYPE_OBJECT && data_alloc){
                //if (p_tree_head->node_type == TR69_NODE_TYPE_OBJECT && data_alloc){
                        if (p_tree->info.info_leaf.type == TR69_NODE_LEAF_TYPE_STRING||
                            p_tree->info.info_leaf.type == TR69_NODE_LEAF_TYPE_DATE){
                                *((char **)((char *)(p_tree_head->info.info_obj.p_data)+ p_tree->info.info_leaf.data_offset)) = strdup((char *)p_char);
                        }else {
                                *((int *)((char *)(p_tree_head->info.info_obj.p_data)+ p_tree->info.info_leaf.data_offset)) = atoi((char *)p_char);
                        }

                //}else if (p_tree_head->node_type == TR69_NODE_TYPE_INSTANCE && data_alloc){
                }else if (p_char != NULL && strlen(p_char) > 0 && p_tree_head->node_type == TR69_NODE_TYPE_INSTANCE && data_alloc){
                        if (p_tree->info.info_leaf.type == TR69_NODE_LEAF_TYPE_STRING||
                            p_tree->info.info_leaf.type == TR69_NODE_LEAF_TYPE_DATE){
                                *((char **)((char *)(p_tree_head->info.info_inst.p_data)+ p_tree->info.info_leaf.data_offset)) = strdup((char *)p_char);
                        }else {
                                *((int *)((char *)(p_tree_head->info.info_inst.p_data)+ p_tree->info.info_leaf.data_offset)) = atoi((char *)p_char);
                        }
                }

                xmlFree(p_char);

                p_char = xmlGetProp(p_node, (xmlChar *)"notification");
                if (atoi((tsl_char_t *)p_char)){
                        if ((atoi((tsl_char_t *)p_char)) == 1){
                                p_tree->info.info_leaf.notification = TR69_NODE_LEAF_NOTIFICATION;
                        }else if ((atoi((tsl_char_t *)p_char)) == 2){
                                p_tree->info.info_leaf.notification = TR69_NODE_LEAF_ACTIVENOTIFICATION;
                        }
                }else {
                        p_tree->info.info_leaf.notification = TR69_NODE_LEAF_UNNOTIFICATION;
                }
                xmlFree(p_char);

#ifdef TR69_SUPPORT_HIDE_OBJ_FROM_ACS
                p_tree->info.info_leaf.hide = TSL_B_FALSE;
                if( hide ) {
                    p_tree->info.info_leaf.hide = TSL_B_TRUE;
                }
#endif
                p_char = xmlGetProp(p_node, (xmlChar *)"hideParameterFromAcs");
                if (p_char){
                    if (!strcmp((tsl_char_t *)p_char, "true")){
                        p_tree->info.info_leaf.attr = TR69_NODE_LEAF_ATTR_HIDDEN;
#ifdef TR69_SUPPORT_HIDE_OBJ_FROM_ACS
                        p_tree->info.info_leaf.hide = TSL_B_TRUE;
#endif
                    }
                    xmlFree(p_char);
                }

                p_tree->info.info_leaf.min = 0;
                p_tree->info.info_leaf.max = 0;

                p_char = xmlGetProp(p_node, (xmlChar *)"minValue");
                if (p_char){
                        p_tree->info.info_leaf.min = atoi((char *)p_char);
                        xmlFree(p_char);
                }

                p_char = xmlGetProp(p_node, (xmlChar *)"maxValue");
                if (p_char){
                        p_tree->info.info_leaf.max = atoi((char *)p_char);
                        xmlFree(p_char);
                }

                p_char = xmlGetProp(p_node, (xmlChar *)"maxLength");
                if (p_char){
                        p_tree->info.info_leaf.max = atoi((char *)p_char);
                        xmlFree(p_char);
                }

                break;
        }

        TSL_INIT_LIST_HEAD(&p_tree->list_head);
        TSL_INIT_LIST_HEAD(&p_tree->list_node);

        if (p_tree_head != NULL){
                tsl_list_add(&p_tree->list_node, &p_tree_head->list_head);
                p_tree->p_parent = p_tree_head;
        }

        if (p_node->type == XML_ELEMENT_NODE){
#ifdef TR69_SUPPORT_HIDE_OBJ_FROM_ACS
            tsl_bool_t hide_child_nodes = TSL_B_FALSE;
            if( (TR69_NODE_TYPE_OBJECT == type) &&
                    (TSL_B_TRUE == p_tree->info.info_obj.hide)) {
                hide_child_nodes = TSL_B_TRUE;
            } else if( (TR69_NODE_TYPE_INSTANCE == type) &&
                    (TSL_B_TRUE == p_tree->info.info_inst.hide)) {
                hide_child_nodes = TSL_B_TRUE;
            }
#endif
	    p_child = p_node->children;
        while(p_child != NULL){
            if (p_child->type == XML_ELEMENT_NODE){
#ifdef TR69_SUPPORT_HIDE_OBJ_FROM_ACS
                tr69_func_add_child_node(p_child, p_tree, p_tree->full_name, data_alloc, hide_child_nodes);
#else
                tr69_func_add_child_node(p_child, p_tree, p_tree->full_name, data_alloc);
#endif
            }
            p_child = p_child->next;
        }
        }

        return p_tree;
}


tsl_rv_t tr69_travel_tree(tr69_info_tree_t *p_root)
{
        tsl_list_head_t     *p_list = NULL;
        tr69_info_tree_t    *p_tree = NULL;

        if (p_root->node_type == TR69_NODE_TYPE_OBJECT ){
	    printf("OBJECT %s %s\n", p_root->info.info_obj.name, p_root->full_name);

        }else if (p_root->node_type == TR69_NODE_TYPE_INSTANCE){
	    printf("INSTANCE %d %s\n", p_root->info.info_inst.size, p_root->full_name);

	}else if (p_root->node_type == TR69_NODE_TYPE_LEAF){
                printf("PARAMETER %s %s\n", p_root->info.info_leaf.name, p_root->full_name);

                if (p_root->p_parent->node_type == TR69_NODE_TYPE_OBJECT){
                        printf("value :%s\n", *((char **)((char *)(p_root->p_parent->info.info_obj.p_data)+ p_root->info.info_leaf.data_offset)));
                }else if (p_root->p_parent->node_type == TR69_NODE_TYPE_INSTANCE){
                        printf("value :%s\n", *((char **)((char *)(p_root->p_parent->info.info_inst.p_data)+ p_root->info.info_leaf.data_offset)));
                }
        }

        for(p_list = p_root->list_head.prev; p_list != &p_root->list_head;){
	    p_tree = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
	    tr69_travel_tree(p_tree);
	    p_list = p_list->prev;
        }
        return TSL_RV_SUC;
}

tsl_rv_t tr69_init_tree(tr69_info_tree_t *p_root)
{
        tsl_list_head_t     *p_list = NULL;
        tr69_info_tree_t    *p_tree = NULL;
        tsl_void_t          *p_new_data = NULL;

        TSL_VASSERT(p_root->node_type == TR69_NODE_TYPE_OBJECT ||
                    p_root->node_type == TR69_NODE_TYPE_INSTANCE);

        p_new_data = tr69_inline_dump_data(p_root);
        if (p_root->node_type == TR69_NODE_TYPE_OBJECT){
                if (p_root->info.info_obj.data_size != 0 && p_root->info.info_obj.set_func != NULL){
                        p_root->info.info_obj.set_func(p_root->full_name, NULL, p_new_data);
                }

        }else if (p_root->node_type == TR69_NODE_TYPE_INSTANCE){
                if (p_root->p_parent->info.info_obj.set_func != NULL){
                        p_root->p_parent->info.info_obj.set_func(p_root->full_name, NULL, p_new_data);
                }
        }
        tr69_inline_save_data(p_root, p_new_data);
        tr69_inline_free_data(p_root, &p_new_data);

        for(p_list = p_root->list_head.prev; p_list != &p_root->list_head;){
	    p_tree = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
	    tr69_init_tree(p_tree);
	    p_list = p_list->prev;
        }
        return TSL_RV_SUC;
}

tsl_rv_t tr69_uninit_tree(tr69_info_tree_t *p_root)
{
    tsl_list_head_t *p_list = NULL;
    tr69_info_tree_t *p_tree = NULL;
    tsl_void_t *p_cur_data = NULL;

    TSL_VASSERT(p_root != NULL);
    TSL_VASSERT(p_root->node_type == TR69_NODE_TYPE_OBJECT ||
                p_root->node_type == TR69_NODE_TYPE_INSTANCE);

    for (p_list = p_root->list_head.prev; p_list != &p_root->list_head; p_list=p_list->prev){
        p_tree = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
        tr69_uninit_tree(p_tree);
    }

    p_cur_data = tr69_inline_dump_data(p_root);
    if (p_root->node_type == TR69_NODE_TYPE_OBJECT){
        if (p_root->info.info_obj.data_size != 0 && p_root->info.info_obj.set_func != NULL){
            p_root->info.info_obj.set_func(p_root->full_name, p_cur_data, NULL);
        }
    }else if (p_root->node_type == TR69_NODE_TYPE_INSTANCE){
        if (p_root->p_parent->info.info_obj.set_func != NULL){
            p_root->p_parent->info.info_obj.set_func(p_root->full_name, p_cur_data, NULL);
        }
    }
    tr69_inline_free_data(p_root, &p_cur_data);

    return TSL_RV_SUC;
}

tr69_info_tree_t *tr69_ghost_func_parse_xml(tsl_char_t *p_file_name)
{
    xmlNode          *root_node = NULL;
    xmlDoc           *doc = NULL;
	tr69_info_tree_t *p_root = NULL;
    tsl_int_t count = 0;
    tsl_int_t mem_index = 0;

    doc = xmlReadFile(p_file_name, NULL, 0);
    TSL_VASSERT_RV(doc != NULL, NULL);
    root_node = xmlDocGetRootElement(doc);

    tr69_ghost_func_count_child_node(root_node, NULL, &count);
    ctllog_debug("protype.xml all nodes count=%d\n", count);

    //count = 0;
    //tr69_ghost_func_count_child_parameters(root_node, NULL, &count);
    //ctllog_debug("protype.xml all parameters count=%d\n", count);

    pg_ghost_tree_malloc = (tr69_info_tree_t *)calloc(count+1, sizeof(tr69_info_tree_t));
    TSL_VASSERT_RV(pg_ghost_tree_malloc != NULL, NULL);

#ifdef TR69_SUPPORT_HIDE_OBJ_FROM_ACS
    p_root = tr69_ghost_func_add_child_node(root_node, NULL, NULL, &mem_index, TSL_B_FALSE);
#else
    p_root = tr69_ghost_func_add_child_node(root_node, NULL, NULL, &mem_index);
#endif

    xmlFreeDoc(doc);
    //xmlCleanupParser();

	return p_root;
}


tr69_info_tree_t *tr69_func_parse_xml(tsl_char_t *p_file_name)
{
    xmlNode          *root_node = NULL;
    xmlDoc           *doc = NULL;
	tr69_info_tree_t *p_root = NULL;
    tsl_int_t count = 0;

    doc = xmlReadFile(p_file_name, NULL, 0);
    TSL_VASSERT_RV(doc != NULL, NULL);
    root_node = xmlDocGetRootElement(doc);

    tr69_func_count_child_node(root_node, &count);
    ctllog_debug("cfg.xml all nodes count=%d\n", count);
#ifdef TR69_SUPPORT_HIDE_OBJ_FROM_ACS
    p_root = tr69_func_add_child_node(root_node, NULL, NULL, 1, TSL_B_FALSE);
#else
	p_root = tr69_func_add_child_node(root_node, NULL, NULL, 1);
#endif

    xmlFreeDoc(doc);
    //xmlCleanupParser();

	return p_root;
}

tsl_rv_t tr69_func_tree_to_xml(tsl_char_t *p_file_name, tr69_info_tree_t *p_root)
{
        xmlDoc  *doc = NULL;
        xmlNode *root_node = NULL;

        doc = xmlNewDoc((xmlChar *)"1.0");
        root_node = tr69_func_inline_tree_to_xml(p_root, root_node);
        xmlDocSetRootElement(doc, root_node);

        xmlSaveFormatFile(p_file_name, doc, 1);

        xmlFreeDoc(doc);
        //xmlCleanupParser();

        return TSL_RV_SUC;
}

xmlNode *tr69_func_inline_tree_to_xml(tr69_info_tree_t *p_root, xmlNode *p_parent_node)
{
        tsl_list_head_t     *p_list = NULL;
        tr69_info_tree_t    *p_tree = NULL;
        tsl_char_t          tmp[TR69_OID_FULL_NAME_SIZE] = "\0";
        xmlNode             *p_node = NULL;
        xmlChar             *p_str = NULL;

        if (p_root->node_type == TR69_NODE_TYPE_OBJECT ){
                p_node = xmlNewNode(NULL, (xmlChar *)"Object");
                xmlNewProp(p_node, (xmlChar *)"name", (xmlChar *)p_root->info.info_obj.name);

                if (sizeof(tsl_char_t *) == 8){
                        //64 Bit Machine
                        sprintf(tmp, "%d", p_root->info.info_obj.data_size/2);
                }else {
                        sprintf(tmp, "%d", p_root->info.info_obj.data_size);
                }
                xmlNewProp(p_node, (xmlChar *)"data_size", (xmlChar *)tmp);
                if (!strcmp(p_root->info.info_obj.name, "Device")){
                        xmlNewProp(p_node, (xmlChar *)"framework_xml_version", (xmlChar *)cfg_xml_version);
                }
#ifdef TR69_SUPPORT_HIDE_OBJ_FROM_ACS
			if (p_root->info.info_obj.attr == TR69_NODE_LEAF_ATTR_HIDDEN){
                    xmlNewProp(p_node, (xmlChar *)"hideObjectFromAcs", (xmlChar *)"true");
            }
#endif

        }else if (p_root->node_type == TR69_NODE_TYPE_INSTANCE){
#ifdef TR69_SUPPORT_INSTANCE_NOSAVE
            if (p_root->info.info_inst.nosave == TR69_NODE_LEAF_ACL2_NOSAVE){

            } else
#endif
            {
                p_node = xmlNewNode(NULL, (xmlChar *)"Instance");

                sprintf(tmp, "%d", p_root->info.info_inst.size);
                xmlNewProp(p_node, (xmlChar *)"size", (xmlChar *)tmp);

                switch (p_root->info.info_inst.acl){
                    case TR69_NODE_LEAF_ACL_READONLY:
                        xmlNewProp(p_node, (xmlChar *)"acl", (xmlChar *)"ReadOnly");
                        break;
                    case TR69_NODE_LEAF_ACL_WRITEONLY:
                        xmlNewProp(p_node, (xmlChar *)"acl", (xmlChar *)"WriteOnly");
                        break;
                    case TR69_NODE_LEAF_ACL_READWRITE:
                        xmlNewProp(p_node, (xmlChar *)"acl", (xmlChar *)"ReadWrite");
                        break;
                }
#ifdef TR69_SUPPORT_HIDE_OBJ_FROM_ACS
			if (p_root->info.info_inst.attr == TR69_NODE_LEAF_ATTR_HIDDEN){
                    xmlNewProp(p_node, (xmlChar *)"hideObjectFromAcs", (xmlChar *)"true");
            }
#endif

                if (p_root->info.info_inst.nosave == TR69_NODE_LEAF_ACL2_NOSAVE){
                    xmlNewProp(p_node, (xmlChar *)"nosave", (xmlChar *)"1");
                }else {
                    xmlNewProp(p_node, (xmlChar *)"nosave", (xmlChar *)"0");
                }

                if (sizeof(tsl_char_t *) == 8){
                    //64 Bit Machine
                    sprintf(tmp, "%d", p_root->info.info_inst.data_size/2);
                }else {
                    sprintf(tmp, "%d", p_root->info.info_inst.data_size);
                }
                xmlNewProp(p_node, (xmlChar *)"data_size", (xmlChar *)tmp);
            }
	}else if (p_root->node_type == TR69_NODE_TYPE_LEAF){
	    p_node = xmlNewNode(NULL, (xmlChar *)"Parameter");
	    xmlNewProp(p_node, (xmlChar *)"name", (xmlChar *)p_root->info.info_leaf.name);

	    switch (p_root->info.info_leaf.acl){
	    case TR69_NODE_LEAF_ACL_READONLY:
		xmlNewProp(p_node, (xmlChar *)"acl", (xmlChar *)"ReadOnly");
		break;
	    case TR69_NODE_LEAF_ACL_WRITEONLY:
		xmlNewProp(p_node, (xmlChar *)"acl", (xmlChar *)"WriteOnly");
		break;
	    case TR69_NODE_LEAF_ACL_READWRITE:
		xmlNewProp(p_node, (xmlChar *)"acl", (xmlChar *)"ReadWrite");
		break;
	    }

		if (p_root->info.info_leaf.nosave == TR69_NODE_LEAF_ACL2_NOSAVE){
			xmlNewProp(p_node, (xmlChar *)"nosave", (xmlChar *)"1");
		}else {
			xmlNewProp(p_node, (xmlChar *)"nosave", (xmlChar *)"0");
		}

	    switch (p_root->info.info_leaf.type){
	    case TR69_NODE_LEAF_TYPE_STRING:
                    xmlNewProp(p_node, (xmlChar *)"type", (xmlChar *)"string");

                    if (p_root->info.info_leaf.max != 0){
                            sprintf(tmp, "%d", p_root->info.info_leaf.max);
                            xmlNewProp(p_node, (xmlChar *)"maxLength", (xmlChar *)tmp);
                    }

                    break;
	    case TR69_NODE_LEAF_TYPE_UINT:
                    xmlNewProp(p_node, (xmlChar *)"type", (xmlChar *)"unsignedInt");

                    if (p_root->info.info_leaf.min != 0){
                            sprintf(tmp, "%d", p_root->info.info_leaf.min);
                            xmlNewProp(p_node, (xmlChar *)"minValue", (xmlChar *)tmp);
                    }

                    if (p_root->info.info_leaf.max != 0){
                            sprintf(tmp, "%d", p_root->info.info_leaf.max);
                            xmlNewProp(p_node, (xmlChar *)"maxValue", (xmlChar *)tmp);
                    }

		break;
	    case TR69_NODE_LEAF_TYPE_INT:
                    xmlNewProp(p_node, (xmlChar *)"type", (xmlChar *)"Int");

                    if (p_root->info.info_leaf.min != 0){
                            sprintf(tmp, "%d", p_root->info.info_leaf.min);
                            xmlNewProp(p_node, (xmlChar *)"minValue", (xmlChar *)tmp);
                    }

                    if (p_root->info.info_leaf.max != 0){
                            sprintf(tmp, "%d", p_root->info.info_leaf.max);
                            xmlNewProp(p_node, (xmlChar *)"maxValue", (xmlChar *)tmp);
                    }

                    break;
	    case TR69_NODE_LEAF_TYPE_BOOL:
		xmlNewProp(p_node, (xmlChar *)"type", (xmlChar *)"boolean");
		break;
	    case TR69_NODE_LEAF_TYPE_DATE:
		xmlNewProp(p_node, (xmlChar *)"type", (xmlChar *)"dateTime");
		break;
	    }

		if (p_root->info.info_leaf.nosave == TR69_NODE_LEAF_ACL2_NOSAVE){
			if (strlen(p_root->info.info_leaf.def_value)){
                    p_str = xmlEncodeSpecialChars(NULL, (const xmlChar *)p_root->info.info_leaf.def_value);
                    xmlNodeSetContent(p_node, (xmlChar *)p_str);
                    xmlFree(p_str);
            }
		}else {
	            if (p_root->p_parent->node_type == TR69_NODE_TYPE_OBJECT){
	                    if (p_root->info.info_leaf.type == TR69_NODE_LEAF_TYPE_STRING||
	                        p_root->info.info_leaf.type == TR69_NODE_LEAF_TYPE_DATE){
	                            p_str = xmlEncodeSpecialChars(NULL, (const xmlChar *)(*((char **)((char *)(p_root->p_parent->info.info_obj.p_data)+ p_root->info.info_leaf.data_offset))));
	                            xmlNodeSetContent(p_node, (xmlChar *)p_str);
                                xmlFree(p_str);
	                    }else {
	                            sprintf(tmp, "%d", *((int *)((char *)(p_root->p_parent->info.info_obj.p_data)+ p_root->info.info_leaf.data_offset)));
                                p_str = xmlEncodeSpecialChars(NULL, (const xmlChar *)tmp);
	                            xmlNodeSetContent(p_node, (xmlChar *)p_str);
                                xmlFree(p_str);
	                    }


	            }else if (p_root->p_parent->node_type == TR69_NODE_TYPE_INSTANCE){
	                    if (p_root->info.info_leaf.type == TR69_NODE_LEAF_TYPE_STRING||
	                        p_root->info.info_leaf.type == TR69_NODE_LEAF_TYPE_DATE){
	                            p_str = xmlEncodeSpecialChars(NULL, (const xmlChar *)(*((char **)((char *)(p_root->p_parent->info.info_inst.p_data)+ p_root->info.info_leaf.data_offset))));
	                            xmlNodeSetContent(p_node, (xmlChar *)p_str);
                                xmlFree(p_str);
	                            }else {
	                            sprintf(tmp, "%d", *((int *)((char *)(p_root->p_parent->info.info_inst.p_data)+ p_root->info.info_leaf.data_offset)));
                                p_str = xmlEncodeSpecialChars(NULL, (const xmlChar *)tmp);
	                            xmlNodeSetContent(p_node, (xmlChar *)p_str);
                                xmlFree(p_str);
	                    }
	            }
			}

            if (p_root->info.info_leaf.notification == TR69_NODE_LEAF_NOTIFICATION){
                    xmlNewProp(p_node, (xmlChar *)"notification", (xmlChar *)"1");
            }else if (p_root->info.info_leaf.notification == TR69_NODE_LEAF_ACTIVENOTIFICATION){
                    xmlNewProp(p_node, (xmlChar *)"notification", (xmlChar *)"2");
            }else {
                    xmlNewProp(p_node, (xmlChar *)"notification", (xmlChar *)"0");
            }

			if (p_root->info.info_leaf.attr == TR69_NODE_LEAF_ATTR_HIDDEN){
				xmlNewProp(p_node, (xmlChar *)"hideParameterFromAcs", (xmlChar *)"true");
			}
        }

#ifdef TR69_SUPPORT_INSTANCE_NOSAVE
        if( NULL == p_node ) {
            // Code somes here if Instance nosave
            return p_parent_node;
        }
#endif
        if (p_parent_node != NULL){
	    xmlAddChild(p_parent_node, p_node);
        }

        for(p_list = p_root->list_head.prev; p_list != &p_root->list_head;){

                p_tree = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
                tr69_func_inline_tree_to_xml(p_tree, p_node);
                p_list = p_list->prev;
        }
        return p_node;
}




tsl_rv_t tr69_inline_save_data(tr69_info_tree_t *p_tree, tsl_void_t *p_data)
{
        tsl_list_head_t   *p_list = NULL;
        tr69_info_tree_t  *p_child = NULL;
        tsl_char_t        **pp_src;
        tsl_char_t        **pp_dst;
        tsl_int_t         *p_isrc;
        tsl_int_t         *p_idst;

        if (p_tree->node_type == TR69_NODE_TYPE_OBJECT ){
                if (p_tree->info.info_obj.data_size){
                        for(p_list = p_tree->list_head.prev; p_list != &p_tree->list_head;){
                                p_child = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
                                p_list = p_list->prev;
                                if (p_child->node_type == TR69_NODE_TYPE_LEAF){
                                        if (p_child->info.info_leaf.type == TR69_NODE_LEAF_TYPE_STRING||
                                            p_child->info.info_leaf.type == TR69_NODE_LEAF_TYPE_DATE){
                                                pp_dst = pp_src = NULL;

                                                pp_dst = ((char **)((char *)(p_tree->info.info_obj.p_data)+ p_child->info.info_leaf.data_offset));
                                                pp_src = ((char **)((char *)(p_data)+ p_child->info.info_leaf.data_offset));

                                                if (p_child->info.info_leaf.notification){
                                                        if (*pp_dst && pp_src && *pp_src){
                                                                if (strcmp(*pp_dst, *pp_src)){
                                                                        tr69_inline_add_notification(p_child->full_name, p_child->info.info_leaf.notification);
                                                                }
                                                        }else if (*pp_dst == NULL && pp_src && *pp_src) {
                                                                tr69_inline_add_notification(p_child->full_name, p_child->info.info_leaf.notification);
                                                        }else if (*pp_dst && pp_src && *pp_src == NULL){
                                                                tr69_inline_add_notification(p_child->full_name, p_child->info.info_leaf.notification);
                                                        }
                                                }

                                                if (*pp_dst && pp_src && *pp_src){
                                                        if (strcmp(*pp_dst, *pp_src)){
                                                                free(*pp_dst);
                                                                *pp_dst = NULL;

                                                                *pp_dst = strdup(*pp_src);

                                                                if (p_child->info.info_leaf.nosave != TR69_NODE_LEAF_ACL2_NOSAVE){
                                                                    tr69_is_data_changed = 1;
                                                                }
                                                        }
                                                }else {
                                                        if (*pp_dst){
                                                                free(*pp_dst);
                                                                *pp_dst = NULL;

                                                                if (p_child->info.info_leaf.nosave != TR69_NODE_LEAF_ACL2_NOSAVE){
                                                                    tr69_is_data_changed = 1;
                                                                }
                                                        }

                                                        if (pp_src && *pp_src ){
                                                                *pp_dst = strdup(*pp_src);

                                                                if (p_child->info.info_leaf.nosave != TR69_NODE_LEAF_ACL2_NOSAVE){
                                                                    tr69_is_data_changed = 1;
                                                                }
                                                        }
                                                }
                                        }else {
                                                p_idst = p_isrc = NULL;

                                                p_idst = ((int *)((char *)(p_tree->info.info_obj.p_data)+ p_child->info.info_leaf.data_offset));
                                                p_isrc = ((int *)((char *)(p_data)+ p_child->info.info_leaf.data_offset));

                                                if (p_child->info.info_leaf.notification){
                                                        if (*p_idst != *p_isrc){
                                                                tr69_inline_add_notification(p_child->full_name, p_child->info.info_leaf.notification);
                                                        }
                                                }
                                                if ((*p_idst != *p_isrc) && (p_child->info.info_leaf.nosave != TR69_NODE_LEAF_ACL2_NOSAVE)){
                                                    tr69_is_data_changed = 1;
                                                }
                                                *p_idst = *p_isrc;
                                        }
                                }
                        }
                }
        }else if (p_tree->node_type == TR69_NODE_TYPE_INSTANCE){
                if (p_tree->info.info_inst.data_size){
                        for(p_list = p_tree->list_head.prev; p_list != &p_tree->list_head;){
                                p_child = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
                                p_list = p_list->prev;
                                if (p_child->node_type == TR69_NODE_TYPE_LEAF){
                                        if (p_child->info.info_leaf.type == TR69_NODE_LEAF_TYPE_STRING||
                                            p_child->info.info_leaf.type == TR69_NODE_LEAF_TYPE_DATE){
                                                pp_dst = pp_src = NULL;

                                                pp_dst = ((char **)((char *)(p_tree->info.info_inst.p_data)+ p_child->info.info_leaf.data_offset));
                                                pp_src = ((char **)((char *)(p_data)+ p_child->info.info_leaf.data_offset));

                                                if (p_child->info.info_leaf.notification){
                                                        if (*pp_dst && pp_src && *pp_src){
                                                                if (strcmp(*pp_dst, *pp_src)){
                                                                        tr69_inline_add_notification(p_child->full_name, p_child->info.info_leaf.notification);
                                                                }
                                                        }else if (*pp_dst == NULL && pp_src && *pp_src) {
                                                                tr69_inline_add_notification(p_child->full_name, p_child->info.info_leaf.notification);
                                                        }else if (*pp_dst && pp_src && *pp_src == NULL){
                                                                tr69_inline_add_notification(p_child->full_name, p_child->info.info_leaf.notification);
                                                        }
                                                }

                                                if (*pp_dst && pp_src && *pp_src){
                                                        if (strcmp(*pp_dst, *pp_src)){
                                                                free(*pp_dst);
                                                                *pp_dst = NULL;

                                                                *pp_dst = strdup(*pp_src);

                                                                if (p_child->info.info_leaf.nosave != TR69_NODE_LEAF_ACL2_NOSAVE){
                                                                    tr69_is_data_changed = 1;
                                                                }
                                                        }
                                                }else {
                                                        if (*pp_dst){
                                                                free(*pp_dst);
                                                                *pp_dst = NULL;

                                                                if (p_child->info.info_leaf.nosave != TR69_NODE_LEAF_ACL2_NOSAVE){
                                                                    tr69_is_data_changed = 1;
                                                                }
                                                        }
                                                        if (pp_src && *pp_src ){
                                                                *pp_dst = strdup(*pp_src);

                                                                if (p_child->info.info_leaf.nosave != TR69_NODE_LEAF_ACL2_NOSAVE){
                                                                    tr69_is_data_changed = 1;
                                                                }
                                                        }
                                                }

                                        }else {
                                                p_idst = p_isrc = NULL;

                                                p_idst = ((int *)((char *)(p_tree->info.info_inst.p_data)+ p_child->info.info_leaf.data_offset));
                                                p_isrc = ((int *)((char *)(p_data)+ p_child->info.info_leaf.data_offset));

                                                if (p_child->info.info_leaf.notification){
                                                        if (*p_idst != *p_isrc){
                                                                tr69_inline_add_notification(p_child->full_name, p_child->info.info_leaf.notification);
                                                        }
                                                }
                                                if ((*p_idst != *p_isrc) && (p_child->info.info_leaf.nosave != TR69_NODE_LEAF_ACL2_NOSAVE)){
                                                    tr69_is_data_changed = 1;
                                                }
                                                *p_idst = *p_isrc;
                                        }
                                }
                        }
                }
        }
        return TSL_RV_SUC;
}

tsl_rv_t tr69_inline_free_data(tr69_info_tree_t *p_tree, tsl_void_t **pp_data)
{
        tsl_list_head_t   *p_list = NULL;
        tr69_info_tree_t  *p_child = NULL;
        tsl_char_t        **pp_dst;

        if (p_tree->node_type == TR69_NODE_TYPE_OBJECT ){
                if (p_tree->info.info_obj.data_size){
                        for(p_list = p_tree->list_head.prev; p_list != &p_tree->list_head;){
                                p_child = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
                                p_list = p_list->prev;
                                if (p_child->node_type == TR69_NODE_TYPE_LEAF){
                                        if (p_child->info.info_leaf.type == TR69_NODE_LEAF_TYPE_STRING||
                                            p_child->info.info_leaf.type == TR69_NODE_LEAF_TYPE_DATE){
                                                pp_dst = ((char **)((char *)(*pp_data)+ p_child->info.info_leaf.data_offset));

                                                if (*pp_dst){
                                                        free(*pp_dst);
                                                }
                                                *pp_dst = NULL;

                                        }
                                }
                        }
                }
        }else if (p_tree->node_type == TR69_NODE_TYPE_INSTANCE){
                if (p_tree->info.info_inst.data_size){
                        for(p_list = p_tree->list_head.prev; p_list != &p_tree->list_head;){
                                p_child = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
                                p_list = p_list->prev;
                                if (p_child->node_type == TR69_NODE_TYPE_LEAF){
                                        if (p_child->info.info_leaf.type == TR69_NODE_LEAF_TYPE_STRING||
                                            p_child->info.info_leaf.type == TR69_NODE_LEAF_TYPE_DATE){
                                                pp_dst = ((char **)((char *)(*pp_data)+ p_child->info.info_leaf.data_offset));
                                                if (*pp_dst){
                                                        free(*pp_dst);
                                                }
                                                *pp_dst = NULL;
                                        }
                                }
                        }
                }
        }

        if (*pp_data){
                free(*pp_data);
                *pp_data = NULL;
        }

        return TSL_RV_SUC;
}


tsl_void_t *tr69_inline_dump_data(tr69_info_tree_t *p_tree)
{
        tsl_list_head_t   *p_list = NULL;
        tr69_info_tree_t  *p_child = NULL;
        tsl_void_t        *p_data = NULL;
        tsl_char_t        **pp_src;
        tsl_char_t        **pp_dst;
        tsl_int_t         *p_isrc;
        tsl_int_t         *p_idst;


        if (p_tree->node_type == TR69_NODE_TYPE_OBJECT ){
                if (p_tree->info.info_obj.data_size){
                        p_data = calloc(1, p_tree->info.info_obj.data_size);

                        for(p_list = p_tree->list_head.prev; p_list != &p_tree->list_head;){
                                p_child = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
                                p_list = p_list->prev;
                                if (p_child->node_type == TR69_NODE_TYPE_LEAF){
                                        if (p_child->info.info_leaf.type == TR69_NODE_LEAF_TYPE_STRING||
                                            p_child->info.info_leaf.type == TR69_NODE_LEAF_TYPE_DATE){
                                                pp_dst = pp_src = NULL;

                                                pp_src = ((char **)((char *)(p_tree->info.info_obj.p_data)+ p_child->info.info_leaf.data_offset));
                                                pp_dst = ((char **)((char *)(p_data)+ p_child->info.info_leaf.data_offset));

                                                if (*pp_dst ){
                                                        free(*pp_dst);
                                                        *pp_dst = NULL;
                                                }

                                                if (pp_src && *pp_src ){
                                                        *pp_dst = strdup(*pp_src);
                                                }
                                                printf("dump string value :%s %p\n", *pp_dst, *pp_dst);
                                        }else {
                                                p_idst = p_isrc = NULL;

                                                p_isrc = ((int *)((char *)(p_tree->info.info_obj.p_data)+ p_child->info.info_leaf.data_offset));
                                                p_idst = ((int *)((char *)(p_data)+ p_child->info.info_leaf.data_offset));

                                                *p_idst = *p_isrc;
                                                printf("dump int value :%d\n", *p_idst);
                                        }
                                }
                        }
                }
        }else if (p_tree->node_type == TR69_NODE_TYPE_INSTANCE){
                if (p_tree->info.info_inst.data_size){
                        p_data = calloc(1, p_tree->info.info_inst.data_size);

                        for(p_list = p_tree->list_head.prev; p_list != &p_tree->list_head;){
                                p_child = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
                                p_list = p_list->prev;
                                if (p_child->node_type == TR69_NODE_TYPE_LEAF){
                                        if (p_child->info.info_leaf.type == TR69_NODE_LEAF_TYPE_STRING||
                                            p_child->info.info_leaf.type == TR69_NODE_LEAF_TYPE_DATE){
                                                pp_dst = pp_src = NULL;

                                                pp_src = ((char **)((char *)(p_tree->info.info_inst.p_data)+ p_child->info.info_leaf.data_offset));
                                                pp_dst = ((char **)((char *)(p_data)+ p_child->info.info_leaf.data_offset));

                                                if (*pp_dst ){
                                                        free(*pp_dst);
                                                        *pp_dst = NULL;
                                                }

                                                if (pp_src && *pp_src ){
                                                        *pp_dst = strdup(*pp_src);
                                                }
                                                printf("string value :%s %p\n", *pp_dst, *pp_dst);
                                        }else {
                                                p_idst = p_isrc = NULL;

                                                p_isrc = ((int *)((char *)(p_tree->info.info_inst.p_data)+ p_child->info.info_leaf.data_offset));
                                                p_idst = ((int *)((char *)(p_data)+ p_child->info.info_leaf.data_offset));

                                                *p_idst = *p_isrc;
                                                printf("int value :%d\n", *p_idst);
                                        }
                                }
                        }
                }
        }
        return p_data;
}

tsl_rv_t tr69_inline_clone_obj(tr69_info_tree_t *p_root, tr69_info_tree_t *p_clone)
{
        tsl_list_head_t     *p_list = NULL;
        tr69_info_tree_t    *p_tree = NULL;
        tr69_info_tree_t    *p_new_tree = NULL;
        tr69_info_tree_t    *p_bak_parent = NULL;

        p_bak_parent = p_clone->p_parent;
        memcpy((tsl_char_t *)p_clone, (tsl_char_t *)p_root, sizeof(tr69_info_tree_t));
        p_clone->p_parent = p_bak_parent;

        if (p_root->node_type == TR69_NODE_TYPE_OBJECT ){
                printf("%d !!OBJECT %s %s\n", p_clone->info.info_obj.data_size, p_root->info.info_obj.name, p_root->full_name);
                if (p_clone->info.info_obj.data_size){
                        p_clone->info.info_obj.p_data = calloc(1, p_clone->info.info_obj.data_size);
                }
        }else if (p_root->node_type == TR69_NODE_TYPE_INSTANCE){
                printf("%d !!INSTANCE %d %s\n", p_clone->info.info_inst.data_size, p_root->info.info_inst.size, p_root->full_name);
                if (p_clone->info.info_inst.data_size){
                        p_clone->info.info_inst.p_data = calloc(1, p_clone->info.info_inst.data_size);
                }
        }else if (p_root->node_type == TR69_NODE_TYPE_LEAF){
                if (p_clone->p_parent->node_type == TR69_NODE_TYPE_OBJECT){
                        if (p_clone->info.info_leaf.type == TR69_NODE_LEAF_TYPE_STRING||
                            p_clone->info.info_leaf.type == TR69_NODE_LEAF_TYPE_DATE){
                                if (strlen(p_clone->info.info_leaf.def_value)){
                                        (*((char **)((char *)(p_clone->p_parent->info.info_obj.p_data)+ p_clone->info.info_leaf.data_offset))) =
                                                strdup(p_clone->info.info_leaf.def_value);
                                }
                        }else if (p_clone->info.info_leaf.type == TR69_NODE_LEAF_TYPE_BOOL){
                        	if ((strcasecmp(p_clone->info.info_leaf.def_value, "true") == 0) || (strcmp(p_clone->info.info_leaf.def_value, "1") == 0)){
								*((int *)((char *)(p_clone->p_parent->info.info_obj.p_data)+ p_clone->info.info_leaf.data_offset)) = 1;
							}else if ((strcasecmp(p_clone->info.info_leaf.def_value, "false") == 0) || (strcmp(p_clone->info.info_leaf.def_value, "0") == 0)){
								*((int *)((char *)(p_clone->p_parent->info.info_obj.p_data)+ p_clone->info.info_leaf.data_offset)) = 0;
							}else {
								*((int *)((char *)(p_clone->p_parent->info.info_obj.p_data)+ p_clone->info.info_leaf.data_offset)) = 0;
							}
                        }else {
                                *((int *)((char *)(p_clone->p_parent->info.info_obj.p_data)+ p_clone->info.info_leaf.data_offset)) =
                                        atoi(p_clone->info.info_leaf.def_value);
                        }
                }else if (p_clone->p_parent->node_type == TR69_NODE_TYPE_INSTANCE){
                        if (p_clone->info.info_leaf.type == TR69_NODE_LEAF_TYPE_STRING||
                            p_clone->info.info_leaf.type == TR69_NODE_LEAF_TYPE_DATE){
                                if (strlen(p_clone->info.info_leaf.def_value)){
                                        (*((char **)((char *)(p_clone->p_parent->info.info_inst.p_data)+ p_clone->info.info_leaf.data_offset))) =
                                                strdup(p_clone->info.info_leaf.def_value);
                                }
                        }else if (p_clone->info.info_leaf.type == TR69_NODE_LEAF_TYPE_BOOL){
                        	if ((strcasecmp(p_clone->info.info_leaf.def_value, "true") == 0) || (strcmp(p_clone->info.info_leaf.def_value, "1") == 0)){
								*((int *)((char *)(p_clone->p_parent->info.info_inst.p_data)+ p_clone->info.info_leaf.data_offset)) = 1;
							}else if ((strcasecmp(p_clone->info.info_leaf.def_value, "false") == 0) || (strcmp(p_clone->info.info_leaf.def_value, "0") == 0)){
								*((int *)((char *)(p_clone->p_parent->info.info_inst.p_data)+ p_clone->info.info_leaf.data_offset)) = 0;
							}else {
								*((int *)((char *)(p_clone->p_parent->info.info_inst.p_data)+ p_clone->info.info_leaf.data_offset)) = 0;
							}
                        }else {
                                *((int *)((char *)(p_clone->p_parent->info.info_inst.p_data)+ p_clone->info.info_leaf.data_offset)) =
                                        atoi(p_clone->info.info_leaf.def_value);
                        }
                }
                printf("!!PARAMETER %s %s\n", p_root->info.info_leaf.name, p_root->full_name);
        }

        TSL_INIT_LIST_HEAD(&p_clone->list_head);
        TSL_INIT_LIST_HEAD(&p_clone->list_node);

        for(p_list = p_root->list_head.prev; p_list != &p_root->list_head;){

                p_tree = tsl_list_entry(p_list, tr69_info_tree_t, list_node);

                if (p_tree->node_type == TR69_NODE_TYPE_INSTANCE &&
#ifndef SUPPORT_PROTYPE0
                    p_tree->info.info_inst.size != 1){
#else
                    p_tree->info.info_inst.size == 0){
#endif
                        p_list = p_list->prev;
                        continue;
                }

                p_new_tree = (tr69_info_tree_t *)calloc(1, sizeof(tr69_info_tree_t));
                p_new_tree->p_parent = p_clone;

                tr69_inline_clone_obj(p_tree, p_new_tree);

                tsl_list_add(&p_new_tree->list_node, &p_clone->list_head);
                p_list = p_list->prev;
        }
        return TSL_RV_SUC;
}

tsl_rv_t tr69_inline_del_tree(tr69_info_tree_t *p_root)
{
        tsl_list_head_t  *p_list = NULL;
        tr69_info_tree_t *p_tree = NULL;
        //tsl_void_t       *p_cur_data = NULL;

        for(p_list = p_root->list_head.next; p_list != &p_root->list_head;){
                p_tree = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
                p_list = p_list->next;
                tr69_inline_del_tree(p_tree);
        }

        if (p_root->node_type == TR69_NODE_TYPE_OBJECT ){
#if 0
                if (p_root->info.info_obj.data_size != 0 && p_root->info.info_obj.set_func != NULL){
                        p_cur_data = tr69_inline_dump_data(p_root);
                        p_root->info.info_obj.set_func(p_root->full_name, p_cur_data, NULL);
                        tr69_inline_free_data(p_root, &p_cur_data);
                }
#endif
                tr69_inline_free_data(p_root, &p_root->info.info_obj.p_data);
        }else if (p_root->node_type == TR69_NODE_TYPE_INSTANCE){
#if 0
                if (p_root->p_parent->info.info_obj.set_func != NULL){
                        p_cur_data = tr69_inline_dump_data(p_root);
                        p_root->p_parent->info.info_obj.set_func(p_root->full_name, p_cur_data, NULL);
                        tr69_inline_free_data(p_root, &p_cur_data);
                }
#endif
                tr69_inline_free_data(p_root, &p_root->info.info_inst.p_data);
        }else if (p_root->node_type == TR69_NODE_TYPE_LEAF){
        }

        tsl_list_del(&p_root->list_node);
        free(p_root);
        p_root = NULL;

        return TSL_RV_SUC;
}

tsl_rv_t tr69_inline_adjust_para(tr69_info_tree_t *p_root, tsl_char_t *p_name)
{
        tr69_info_tree_t *p_inst = NULL;
        tsl_list_head_t  *p_list = NULL;

        if (p_root->node_type == TR69_NODE_TYPE_OBJECT ){
                sprintf(p_root->full_name, "%s.%s", p_name, p_root->info.info_obj.name);
        }else if (p_root->node_type == TR69_NODE_TYPE_INSTANCE){
                sprintf(p_root->full_name, "%s.%d", p_name, p_root->info.info_inst.size);
        }else if (p_root->node_type == TR69_NODE_TYPE_LEAF){
                sprintf(p_root->full_name, "%s.%s", p_name, p_root->info.info_leaf.name);
        }

        for(p_list = p_root->list_head.prev; p_list != &p_root->list_head;){
                p_inst = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
                tr69_inline_adjust_para(p_inst, p_root->full_name);
                p_list = p_list->prev;
        }
        return TSL_RV_SUC;
}


tsl_rv_t tr69_inline_adjust_instance(tr69_info_tree_t *p_root, tr69_info_tree_t *p_new_inst, tsl_char_t *p_name)
{
        tr69_info_tree_t *p_inst = NULL;
        tsl_list_head_t  *p_list = NULL;
        tsl_int_t        inst = 1;

        for(p_list = p_root->list_head.prev; p_list != &p_root->list_head;){
                p_inst = tsl_list_entry(p_list, tr69_info_tree_t, list_node);

                if (p_inst->info.info_inst.size != inst){
                        tsl_list_add(&p_new_inst->list_node, &p_inst->list_node);
                        p_new_inst->info.info_inst.size = inst;
                        tr69_inline_adjust_para(p_new_inst, p_name);
                        return TSL_RV_SUC;
                }
                inst++;

                p_list = p_list->prev;
        }

        tsl_list_add(&p_new_inst->list_node, &p_root->list_head);
        p_new_inst->info.info_inst.size = inst;
        tr69_inline_adjust_para(p_new_inst, p_name);

        return TSL_RV_SUC;
}

tsl_rv_t tr69_inline_get_node(tr69_info_tree_t *p_root, tsl_char_t *p_name, tr69_info_tree_t **pp_find_node)
{
        tsl_list_head_t  *p_list = NULL;
        tr69_info_tree_t *p_tree = NULL;
		tsl_int_t root_name_len = 0;
		tsl_int_t name_len = 0;

		TSL_VASSERT_RV(*pp_find_node == NULL, TSL_RV_SUC);

		root_name_len = strlen(p_root->full_name);
		name_len = strlen(p_name);
		TSL_VASSERT_RV(root_name_len <= name_len, TSL_RV_SUC);
		TSL_VASSERT_RV(strncmp(p_root->full_name, p_name, root_name_len) == 0, TSL_RV_SUC);

        if (p_root->node_type == TR69_NODE_TYPE_OBJECT ){
                if (strcmp(p_root->full_name, p_name) == 0){
                        printf("OBJECT %s %s %s\n", p_root->info.info_obj.name, p_root->full_name, p_name);
                        *pp_find_node = p_root;
                        return TSL_RV_SUC;
                }
        }else if (p_root->node_type == TR69_NODE_TYPE_INSTANCE){
                if (strcmp(p_root->full_name, p_name) == 0){
                        *pp_find_node = p_root;
                        printf("INSTANCE %d %s\n", p_root->info.info_inst.size, p_root->full_name);
                        return TSL_RV_SUC;
                }
        }else if (p_root->node_type == TR69_NODE_TYPE_LEAF){
                if (strcmp(p_root->full_name, p_name) == 0){
                        *pp_find_node = p_root;
                        printf("PARAMETER %s %s\n", p_root->info.info_leaf.name, p_root->full_name);
                        return TSL_RV_SUC;
                }
        }

        for(p_list = p_root->list_head.prev; p_list != &p_root->list_head;){
                p_tree = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
                tr69_inline_get_node(p_tree, p_name, pp_find_node);
                p_list = p_list->prev;
				TSL_VASSERT_RV(*pp_find_node == NULL, TSL_RV_SUC);
        }
        return TSL_RV_SUC;
}

tsl_rv_t tr69_inline_fresh_obj_data(tr69_info_tree_t *p_root, tr69_info_tree_t *p_tree)
{
        tsl_void_t *p_find_data = NULL;
        tsl_rv_t   rv;

        TSL_VASSERT(p_tree != NULL);
        TSL_VASSERT(p_tree->node_type == TR69_NODE_TYPE_OBJECT ||
                    p_tree->node_type == TR69_NODE_TYPE_INSTANCE);

        p_find_data = tr69_inline_dump_data(p_tree);

        TSL_VASSERT(p_find_data != NULL);

        if (p_tree->node_type == TR69_NODE_TYPE_OBJECT){
                if (p_tree->info.info_obj.get_func != NULL){
                        rv = p_tree->info.info_obj.get_func(p_tree->full_name, p_find_data);

                        if ((rv == TR69_RT_SUCCESS_VALUE_CHANGED) || (rv == TSL_RV_SUC)){
                                tr69_inline_save_data(p_tree, p_find_data);
                        }
                }
        }else if (p_tree->node_type == TR69_NODE_TYPE_INSTANCE){
                if (p_tree->p_parent->info.info_obj.get_func != NULL){
                        rv = p_tree->p_parent->info.info_obj.get_func(p_tree->full_name, p_find_data);

                        if ((rv == TR69_RT_SUCCESS_VALUE_CHANGED) || (rv == TSL_RV_SUC)){
                                tr69_inline_save_data(p_tree, p_find_data);
                        }
                }
        }
        tr69_inline_free_data(p_tree, &p_find_data);

        return TSL_RV_SUC;
}

tsl_rv_t tr69_inline_cleanup_tree(tr69_info_tree_t *p_root, tsl_int_t free_data)
{
        tsl_list_head_t  *p_list = NULL;
        tr69_info_tree_t *p_tree = NULL;

        TSL_VASSERT(p_root != NULL);

        for(p_list = p_root->list_head.prev; p_list != &p_root->list_head;){
	    p_tree = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
	    tr69_inline_cleanup_tree(p_tree, free_data);
            p_list = p_list->prev;

            if (p_tree->node_type == TR69_NODE_TYPE_OBJECT && free_data){
                    if (p_tree->info.info_obj.data_size){
                            tr69_inline_free_data(p_tree, &(p_tree->info.info_obj.p_data));
                    }
            }else if (p_tree->node_type == TR69_NODE_TYPE_INSTANCE && free_data){
                    if (p_tree->info.info_inst.data_size){
                            tr69_inline_free_data(p_tree, &(p_tree->info.info_inst.p_data));
                    }
            }
            tsl_list_del(&p_tree->list_node);
            free(p_tree);
            p_tree = NULL;
        }
        return TSL_RV_SUC;
}

tsl_rv_t tr69_inline_get_next_node(tr69_info_tree_t *p_root, tsl_char_t *p_name, tsl_int_t *p_find, tr69_node_t **pp_next_node)
{
    tsl_list_head_t  *p_list = NULL;
    tr69_info_tree_t *p_tree = NULL;
	tsl_int_t root_name_len = 0;
	tsl_int_t name_len = 0;

	TSL_VASSERT(p_root != NULL);

	TSL_VASSERT_RV(*p_find != 2, TSL_RV_SUC);
	if ((p_name != NULL) && (*p_find == 0)){
		root_name_len = strlen(p_root->full_name);
		name_len = strlen(p_name);
		TSL_VASSERT_RV(root_name_len <= name_len, TSL_RV_SUC);
		TSL_VASSERT_RV(strncmp(p_root->full_name, p_name, root_name_len) == 0, TSL_RV_SUC);
	}

    if ((p_name == NULL || *p_find == 1)){
            if (*pp_next_node == NULL){
                    *pp_next_node = (tr69_node_t *)calloc(1, sizeof(tr69_node_t));
            }

            sprintf((*pp_next_node)->full_name, "%s", p_root->full_name);
            (*pp_next_node)->node_type = p_root->node_type;

            *p_find = 2;
            return TSL_RV_SUC;
    }

    if (p_root->node_type == TR69_NODE_TYPE_OBJECT ){
            if ((strcmp(p_root->full_name, p_name) == 0) && (p_root->node_type == (*pp_next_node)->node_type)){
                    *p_find = 1;
            }
    }else if (p_root->node_type == TR69_NODE_TYPE_INSTANCE){
    		if ((strcmp(p_root->full_name, p_name) == 0) && (p_root->node_type == (*pp_next_node)->node_type)){
                    *p_find = 1;
            }
    }else if (p_root->node_type == TR69_NODE_TYPE_LEAF){
			if ((strcmp(p_root->full_name, p_name) == 0) && (p_root->node_type == (*pp_next_node)->node_type)){
                    *p_find = 1;
            }
    }

    for(p_list = p_root->list_head.prev; p_list != &p_root->list_head;){
            p_tree = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
            tr69_inline_get_next_node(p_tree, p_name, p_find, pp_next_node);
            p_list = p_list->prev;
			TSL_VASSERT_RV(*p_find != 2, TSL_RV_SUC);
    }
    return TSL_RV_SUC;
}

tsl_rv_t tr69_inline_get_next_node2(tr69_info_tree_t *p_root, tsl_char_t *p_name, tsl_int_t *p_find, tr69_node_t **pp_next_node)
{
	tsl_list_head_t *p_list = NULL;
	tr69_info_tree_t *p_tree = NULL;
	tsl_int_t root_name_len = 0;
	tsl_int_t name_len = 0;

	TSL_VASSERT(p_root != NULL);

	TSL_VASSERT_RV(*p_find != 2, TSL_RV_SUC);
	if ((p_name != NULL) && (*p_find == 0)){
		root_name_len = strlen(p_root->full_name);
		name_len = strlen(p_name);
		TSL_VASSERT_RV(root_name_len <= name_len, TSL_RV_SUC);
		TSL_VASSERT_RV(strncmp(p_root->full_name, p_name, root_name_len) == 0, TSL_RV_SUC);
	}

#ifdef TR69_SUPPORT_HIDE_OBJ_FROM_ACS
	if ((p_name == NULL || *p_find == 1) && p_root->info.info_leaf.hide != TSL_B_TRUE){
#else
	if ((p_name == NULL || *p_find == 1) && p_root->info.info_leaf.attr != TR69_NODE_LEAF_ATTR_HIDDEN){
#endif
		if (*pp_next_node == NULL) {
			*pp_next_node = (tr69_node_t *)calloc(1, sizeof(tr69_node_t));
		}

		sprintf((*pp_next_node)->full_name, "%s", p_root->full_name);
		(*pp_next_node)->node_type = p_root->node_type;

		*p_find = 2;
		return TSL_RV_SUC;
	}

	if (p_name)
	{
		if (p_root->node_type == TR69_NODE_TYPE_OBJECT) {
			if ((strcmp(p_root->full_name, p_name) == 0) && (p_root->node_type == (*pp_next_node)->node_type)){
				*p_find = 1;
			}
		}
		if (p_root->node_type == TR69_NODE_TYPE_INSTANCE) {
			if ((strcmp(p_root->full_name, p_name) == 0) && (p_root->node_type == (*pp_next_node)->node_type)){
				*p_find = 1;
			}
		}
		if (p_root->node_type == TR69_NODE_TYPE_LEAF) {
			if ((strcmp(p_root->full_name, p_name) == 0) && (p_root->node_type == (*pp_next_node)->node_type)){
				*p_find = 1;
			}
		}
	}

	for (p_list = p_root->list_head.prev; p_list != &p_root->list_head;) {
		p_tree = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
		tr69_inline_get_next_node2(p_tree, p_name, p_find, pp_next_node);
		TSL_VASSERT_RV(*p_find != 2, TSL_RV_SUC);
		p_list = p_list->prev;
	}

	return TSL_RV_SUC;
}

tsl_rv_t tr69_inline_get_next_child_node(tr69_info_tree_t *p_root, tsl_char_t *p_name, tsl_int_t *p_find, tr69_node_t **pp_next_node)
{
	tsl_list_head_t *p_list = NULL;
	tr69_info_tree_t *p_tree = NULL;

	TSL_VASSERT(p_root != NULL);

	for (p_list = p_root->list_head.prev; p_list != &p_root->list_head;){
		p_tree = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
		TSL_VASSERT(p_tree != NULL);

		//if ((p_name == NULL || *p_find == 1) && p_tree->info.info_leaf.attr != TR69_NODE_LEAF_ATTR_HIDDEN){
		if (p_name == NULL || *p_find == 1) {
			if (*pp_next_node == NULL){
				*pp_next_node = (tr69_node_t *)calloc(1, sizeof(tr69_node_t));
			}

			sprintf((*pp_next_node)->full_name, "%s", p_tree->full_name);
                (*pp_next_node)->node_type = p_tree->node_type;

            *p_find = 2;
			return TSL_RV_SUC;
		}

		if (p_tree->node_type == TR69_NODE_TYPE_OBJECT){
			if (strcmp(p_tree->full_name, p_name) == 0){
				*p_find = 1;
			}
		}
		if (p_tree->node_type == TR69_NODE_TYPE_INSTANCE){
			if (strcmp(p_tree->full_name, p_name) == 0){
				*p_find = 1;
			}
		}
		if (p_tree->node_type == TR69_NODE_TYPE_LEAF){
			if (strcmp(p_tree->full_name, p_name) == 0){
				*p_find = 1;
			}
		}

		p_list = p_list->prev;
	}

	return TSL_RV_SUC;
}

tsl_rv_t tr69_inline_get_next_obj_node(tr69_info_tree_t *p_root, tsl_char_t *p_name, tsl_int_t *p_find, tr69_node_t **pp_next_node)
{
        tsl_list_head_t  *p_list = NULL;
        tr69_info_tree_t *p_tree = NULL;

	TSL_VASSERT(p_root != NULL);

        if ((p_name == NULL || *p_find == 1) &&
            p_root->node_type != TR69_NODE_TYPE_LEAF){
                if (*pp_next_node == NULL){
                        *pp_next_node = (tr69_node_t *)calloc(1, sizeof(tr69_node_t));
                }

                sprintf((*pp_next_node)->full_name, "%s", p_root->full_name);
                (*pp_next_node)->node_type = p_root->node_type;

                *p_find = 2;

                return TSL_RV_SUC;
        }

        if (p_root->node_type == TR69_NODE_TYPE_OBJECT ){
                if (strcmp(p_root->full_name, p_name) == 0){
                        *p_find = 1;
                }
        }else if (p_root->node_type == TR69_NODE_TYPE_INSTANCE){
                if (strcmp(p_root->full_name, p_name) == 0){
                        *p_find = 1;
                }
        }

        for(p_list = p_root->list_head.prev; p_list != &p_root->list_head;){
                p_tree = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
                tr69_inline_get_next_obj_node(p_tree, p_name, p_find, pp_next_node);
                p_list = p_list->prev;
        }
        return TSL_RV_SUC;
}

tsl_rv_t tr69_inline_get_next_inst_node(tr69_info_tree_t *p_root, tsl_char_t *p_name, tsl_int_t *p_find, tr69_node_t **pp_next_node)
{
	tsl_list_head_t *p_list = NULL;
	tr69_info_tree_t *p_tree = NULL;

	TSL_VASSERT(p_root != NULL);

	for (p_list = p_root->list_head.prev; p_list != &p_root->list_head;){
		p_tree = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
		TSL_VASSERT(p_tree != NULL);

		//if ((p_name == NULL || *p_find == 1) && p_tree->info.info_leaf.attr != TR69_NODE_LEAF_ATTR_HIDDEN){
		if (p_name == NULL || *p_find == 1) {
			if (p_tree->node_type == TR69_NODE_TYPE_INSTANCE){
				if (*pp_next_node == NULL){
					*pp_next_node = (tr69_node_t *)calloc(1, sizeof(tr69_node_t));
				}

				sprintf((*pp_next_node)->full_name, "%s", p_tree->full_name);
				(*pp_next_node)->node_type = p_tree->node_type;

				*p_find = 2;
				return TSL_RV_SUC;
			}
		}

		if (p_tree->node_type == TR69_NODE_TYPE_INSTANCE){
			if (strcmp(p_tree->full_name, p_name) == 0){
				*p_find = 1;
			}
		}

		p_list = p_list->prev;
	}

	return TSL_RV_SUC;
}

static tsl_int_t strcmp_same_depth(tsl_char_t *p_s1, tsl_char_t *p_s2)
{
	tsl_char_t oid_stack_s1[32][128];
	tsl_char_t oid_stack_s2[32][128];
	tsl_char_t *p_match = NULL;
	tsl_char_t *p_tmp = NULL;
	tsl_int_t depth_s1 = 0;
	tsl_int_t depth_s2 = 0;
	int i = 0;
	int ret = 0;

	TSL_VASSERT(p_s1 != NULL);
	TSL_VASSERT(p_s2 != NULL);

	memset(oid_stack_s1, 0x0, sizeof(oid_stack_s1));
	memset(oid_stack_s2, 0x0, sizeof(oid_stack_s2));

	p_tmp = p_s1;
	while ((p_match = strchr(p_tmp, '.')) != NULL){
		memcpy(oid_stack_s1[depth_s1++], p_tmp, p_match - p_tmp);
		p_tmp = p_match + 1;
	}
	memcpy(oid_stack_s1[depth_s1++], p_tmp, p_s1 + strlen(p_s1) - p_tmp);

	p_tmp = p_s2;
	while ((p_match = strchr(p_tmp, '.')) != NULL){
		memcpy(oid_stack_s2[depth_s2++], p_tmp, p_match - p_tmp);
		p_tmp = p_match + 1;
	}
	memcpy(oid_stack_s2[depth_s2++], p_tmp, p_s2 + strlen(p_s2) - p_tmp);

	if (depth_s1 != depth_s2)
		return -1;

	i = depth_s1 - 1;
	while (i > 0){
		if (strcasecmp(oid_stack_s1[i], "%d") == 0){
			i--;
			continue;
		}
		if (strcmp(oid_stack_s1[i], oid_stack_s2[i]))
		{
			ret = -1;
			break;
		}
		i--;
	}

	return ret;
}

tsl_rv_t tr69_inline_get_next_samedepth_node(tr69_info_tree_t *p_root, tsl_char_t *p_name, tsl_int_t *p_find, tr69_node_t **pp_next_node, tsl_char_t *p_match_oid)
{
	tsl_list_head_t *p_list = NULL;
	tr69_info_tree_t *p_tree = NULL;

	TSL_VASSERT_RV(*p_find != 2, TSL_RV_SUC);

	if (p_name == NULL || *p_find == 1){
        if (strcmp_same_depth(p_match_oid, p_root->full_name) == 0){
            if (*pp_next_node == NULL){
			    *pp_next_node = (tr69_node_t *)calloc(1, sizeof(tr69_node_t));
		    }

            sprintf((*pp_next_node)->full_name, "%s", p_root->full_name);
			(*pp_next_node)->node_type = p_root->node_type;

            *p_find = 2;

			return TSL_RV_SUC;
        }
	}

    if (p_name){
    	if (p_root->node_type == TR69_NODE_TYPE_OBJECT){
    		if ((strcmp(p_root->full_name, p_name) == 0) && (p_root->node_type == (*pp_next_node)->node_type)){
    			*p_find = 1;
    		}
    	}else if (p_root->node_type == TR69_NODE_TYPE_INSTANCE){
    		if ((strcmp(p_root->full_name, p_name) == 0) && (p_root->node_type == (*pp_next_node)->node_type)){
    			*p_find = 1;
    		}
    	}else if (p_root->node_type == TR69_NODE_TYPE_LEAF){
    		if ((strcmp(p_root->full_name, p_name) == 0) && (p_root->node_type == (*pp_next_node)->node_type)){
    			*p_find = 1;
    		}
    	}
    }

	for (p_list = p_root->list_head.prev; p_list != &p_root->list_head; p_list = p_list->prev){
		p_tree = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
        tr69_inline_get_next_samedepth_node(p_tree, p_name, p_find, pp_next_node, p_match_oid);
		TSL_VASSERT_RV(*p_find != 2, TSL_RV_SUC);
	}
	return TSL_RV_SUC;
}

tsl_rv_t tr69_inline_clear_notification()
{
        tsl_list_head_t     *p_list = NULL;
        tr69_notification_t *p_notf = NULL;

        for(p_list = global_notification_list_head.prev; p_list != &global_notification_list_head;){
                p_notf = tsl_list_entry(p_list, tr69_notification_t, list_node);
                p_list = p_list->prev;

                TSL_FREE(p_notf);
        }

        TSL_INIT_LIST_HEAD(&global_notification_list_head);

        return TSL_RV_SUC;
}

tsl_rv_t tr69_inline_add_notification(tsl_char_t *p_oid_name, tsl_int_t active)
{
        tr69_notification_t *p_notf = NULL;
        tsl_list_head_t     *p_list = NULL;
        tsl_char_t *p_dev_notiflimit = NULL;
        tsl_int_t type = 0;

        TSL_VASSERT(p_oid_name != NULL);

        for(p_list = global_notification_list_head.prev; p_list != &global_notification_list_head;){
                p_notf = tsl_list_entry(p_list, tr69_notification_t, list_node);
                p_list = p_list->prev;

                if (!strcmp(p_oid_name, p_notf->full_name)){
                        ctllog_debug("already exist %s\n", p_oid_name);
                        //already in notification, duplicated
                        if (active == TR69_NODE_LEAF_UNNOTIFICATION){
                            tsl_list_del(p_list);
                            TSL_FREE(p_notf);
                        }
                        return TSL_RV_SUC;
                }
        }

        if (active == TR69_NODE_LEAF_UNNOTIFICATION)
            return TSL_RV_SUC;

        /*special dispose for some cases*/
        if (!strcmp(p_oid_name, global_delaynotifications[NOTIF_DEVNUM_INDEX].full_name)){
		if (active == TR69_NODE_LEAF_NOTIFICATION){
                global_delaynotifications[NOTIF_DEVNUM_INDEX].is_add = 0;
		}else{
                if (global_delaynotifications[NOTIF_DEVNUM_INDEX].is_add == 2){
                    global_delaynotifications[NOTIF_DEVNUM_INDEX].is_add = 0;
                    global_delaynotifications[NOTIF_DEVNUM_INDEX].last_time = TSL_CURRENT_TIME();
                    ctllog_debug("sending, last_time=%ld\n", global_delaynotifications[NOTIF_DEVNUM_INDEX].last_time);
                }else if (global_delaynotifications[NOTIF_DEVNUM_INDEX].is_add == 1){
                    return TSL_RV_SUC;
                }else if (global_delaynotifications[NOTIF_DEVNUM_INDEX].is_add == 0){
                    TSL_FVASSERT(tr69_get_unfresh_leaf_data("InternetGatewayDevice.ManagementServer.ManageableDeviceNotificationLimit", (tsl_void_t *)&p_dev_notiflimit, (tsl_void_t *)&type) == TSL_RV_SUC);
                    if ((tsl_int_t)p_dev_notiflimit > 0){
                        global_delaynotifications[NOTIF_DEVNUM_INDEX].is_add = 1;
                        global_delaynotifications[NOTIF_DEVNUM_INDEX].active = active;
                        return TSL_RV_SUC;
                    }
                }
            }
        }

        TSL_MALLOC(p_notf, tr69_notification_t);

        snprintf(p_notf->full_name, sizeof(p_notf->full_name), "%s", p_oid_name);

        tsl_list_add(&p_notf->list_node, &global_notification_list_head);

        ctllog_debug("%s %d %s active:%d\n", __FUNCTION__, __LINE__, p_oid_name, active);

        //sent a msg to TR69 process
        if (active == TR69_NODE_LEAF_ACTIVENOTIFICATION){
                tsl_msg_client_send(TSL_MSG_SOCK_APP_TR69, tsl_msg_event_tr69_active_notification);
        }else if (active == TR69_NODE_LEAF_NOTIFICATION){
                tsl_msg_client_send(TSL_MSG_SOCK_APP_TR69, tsl_msg_event_tr69_passive_notification);
        }

        return TSL_RV_SUC;
}

tsl_rv_t tr69_inline_get_notification(tr69_notification_t **pp_notf_tb, tsl_int_t *p_tb_count)
{
        tsl_list_head_t     *p_list    = NULL;
        tr69_notification_t *p_notf    = NULL;
        tr69_notification_t *p_notf_tb = NULL;
        tsl_int_t           count      = 0;
        tsl_int_t           i          = 0;

        TSL_VASSERT(pp_notf_tb != NULL);
        TSL_VASSERT(p_tb_count != NULL);

        for(p_list = global_notification_list_head.prev; p_list != &global_notification_list_head;){
                count ++;
                p_list = p_list->prev;
        }

        *p_tb_count = count;
        *pp_notf_tb = NULL;

        if (count){
                TSL_SMALLOC(p_notf_tb, tr69_notification_t, count * sizeof(tr69_notification_t));
                for(p_list = global_notification_list_head.prev; p_list != &global_notification_list_head;){
                        p_notf = tsl_list_entry(p_list, tr69_notification_t, list_node);
                        p_list = p_list->prev;
                        sprintf(p_notf_tb[i++].full_name, "%s", p_notf->full_name);
                }
                *pp_notf_tb = p_notf_tb;

                tr69_inline_clear_notification();
        }

        return TSL_RV_SUC;
}


tsl_rv_t tr69_inline_parse_oid(tsl_char_t *p_name, tsl_char_t *p_new_name)
{
        tsl_char_t oid_stack[32][128] = {{"\0"}};
        tsl_char_t *ptr = NULL;
        tsl_char_t *match = NULL;
        tsl_char_t tmp[512] = "\0";
        tsl_int_t i = 0;
        tsl_int_t j = 0;

        ptr = p_name;
        while((match = strchr(ptr, '.')) != NULL){
                memcpy(oid_stack[j++], ptr, match - ptr);
                ptr = match + 1;
        }
        memcpy(oid_stack[j++], ptr, p_name + strlen(p_name) - ptr);

        for (i = 0; i < j ; i++){
                if (atoi(oid_stack[i]) == 0){
                        strcat(tmp, oid_stack[i]);
                }else {
#ifndef SUPPORT_PROTYPE0
                        strcat(tmp, "1");
#else
                        strcat(tmp, "0");
#endif
                }
                if ( i != j -1 ){
                        strcat(tmp, ".");
                }
        }

        sprintf(p_new_name, "%s", tmp);

        printf("%s %d %s %s\n", __FUNCTION__, __LINE__, p_name, tmp);

        return TSL_RV_SUC;
}


tsl_rv_t tr69_inline_parse_oid2(tsl_char_t *p_name, tsl_char_t *p_new_name)
{
        tsl_char_t oid_stack[32][128] = {{"\0"}};
        tsl_char_t *ptr = NULL;
        tsl_char_t *match = NULL;
        tsl_char_t tmp[512] = "\0";
        tsl_int_t i = 0;
        tsl_int_t j = 0;

        ptr = p_name;
        while((match = strchr(ptr, '.')) != NULL){
                memcpy(oid_stack[j++], ptr, match - ptr);
                ptr = match + 1;
        }
        memcpy(oid_stack[j++], ptr, p_name + strlen(p_name) - ptr);

        for (i = 0; i < j ; i++){
#ifndef SUPPORT_PROTYPE0
                if (atoi(oid_stack[i]) == 0){
                        strcat(tmp, oid_stack[i]);
                }else {
                        strcat(tmp, "i");
                }
#else
				if (atoi(oid_stack[i]) || (isdigit(oid_stack[i][0]))){
						strcat(tmp, "i");
                }else {
                		strcat(tmp, oid_stack[i]);
                }
#endif
                if ( i != j -1 ){
                        strcat(tmp, ".");
                }
        }

        sprintf(p_new_name, "%s", tmp);

        printf("%s %d %s %s\n", __FUNCTION__, __LINE__, p_name, tmp);

        return TSL_RV_SUC;
}

tsl_rv_t tr69_inline_add_delaynotification_timeout(tsl_int_t def_wait_timeout)
{
    tsl_char_t *p_dev_notiflimit = NULL;
    tsl_int_t type = 0;
    time_t cur_time;
    tsl_uint_t wait_timeout = 1<<31;

    cur_time = TSL_CURRENT_TIME();
    if (global_delaynotifications[NOTIF_DEVNUM_INDEX].is_add == 1){
        TSL_VASSERT_RV(tr69_get_unfresh_leaf_data("InternetGatewayDevice.ManagementServer.ManageableDeviceNotificationLimit", (tsl_void_t *)&p_dev_notiflimit, (tsl_void_t *)&type) == TSL_RV_SUC, wait_timeout);

        if (global_delaynotifications[NOTIF_DEVNUM_INDEX].last_time + (tsl_int_t)p_dev_notiflimit <=  cur_time){
            global_delaynotifications[NOTIF_DEVNUM_INDEX].is_add = 2;
            ctllog_debug("last_time=%ld (%d)\n", global_delaynotifications[NOTIF_DEVNUM_INDEX].last_time, (tsl_int_t)p_dev_notiflimit);
            ctllog_debug("cur_time=%d\n", (int)cur_time);
            tr69_inline_add_notification(global_delaynotifications[NOTIF_DEVNUM_INDEX].full_name, global_delaynotifications[NOTIF_DEVNUM_INDEX].active);
        }else{
            wait_timeout = global_delaynotifications[NOTIF_DEVNUM_INDEX].last_time + (tsl_int_t)p_dev_notiflimit - cur_time;
        }
    }

    return wait_timeout;
}

tsl_rv_t tr69_inline_add_instance_timeout(tsl_char_t *p_name, tsl_int_t timeout)
{
        tr69_timeout_instance_t *p_timeout_inst = NULL;
        tsl_list_head_t *p_list = NULL;

        TSL_VASSERT(p_name != NULL);
        TSL_VASSERT(timeout > 0);

        for(p_list = global_timeout_instance_list_head.prev; p_list != &global_timeout_instance_list_head;){
                p_timeout_inst = tsl_list_entry(p_list, tr69_timeout_instance_t, list_node);
                p_list = p_list->prev;

                if (!strcmp(p_name, p_timeout_inst->full_name)){
                        ctllog_debug("already exist %s\n", p_name);
                        //already in, duplicated
                        return TSL_RV_SUC;
                }
        }

        TSL_MALLOC(p_timeout_inst, tr69_timeout_instance_t);

        snprintf(p_timeout_inst->full_name, sizeof(p_timeout_inst->full_name), "%s", p_name);
        p_timeout_inst->timeout = TSL_CURRENT_TIME() + timeout;

        tsl_list_add(&p_timeout_inst->list_node, &global_timeout_instance_list_head);

        ctllog_debug("%s %d %s\n", __FUNCTION__, __LINE__, p_name);

        return TSL_RV_SUC;
}

tsl_rv_t tr69_inline_del_instance_timeout(tsl_int_t def_wait_timeout)
{
        tr69_timeout_instance_t *p_timeout_inst = NULL;
        tsl_list_head_t *p_list = NULL;
        time_t cur_time;
        tsl_uint_t wait_timeout = def_wait_timeout;

        cur_time = TSL_CURRENT_TIME();

        printf("%s %d \n", __FUNCTION__, __LINE__);

        for(p_list = global_timeout_instance_list_head.prev; p_list != &global_timeout_instance_list_head;){
                p_timeout_inst = tsl_list_entry(p_list, tr69_timeout_instance_t, list_node);
                p_list = p_list->prev;

                if (p_timeout_inst->timeout <= cur_time){
                        ctllog_debug("%s %d %u %u \n", __FUNCTION__, __LINE__, (tsl_uint_t)cur_time, (tsl_uint_t)p_timeout_inst->timeout);

                        tr69_del2_instance(p_timeout_inst->full_name);
                        tsl_list_del(&p_timeout_inst->list_node);

                        TSL_FREE(p_timeout_inst);
                }else {
                        if ((p_timeout_inst->timeout - cur_time) < wait_timeout){
                                wait_timeout = p_timeout_inst->timeout - cur_time;
                        }
                }
        }

        return wait_timeout;
}

tsl_rv_t tr69_inline_clear_instance_timeout()
{
        tr69_timeout_instance_t *p_timeout_inst = NULL;
        tsl_list_head_t *p_list = NULL;

        for(p_list = global_timeout_instance_list_head.prev; p_list != &global_timeout_instance_list_head;){
                p_timeout_inst = tsl_list_entry(p_list, tr69_timeout_instance_t, list_node);
                p_list = p_list->prev;

                TSL_FREE(p_timeout_inst);
        }

        TSL_INIT_LIST_HEAD(&global_timeout_instance_list_head);

        return TSL_RV_SUC;
}

tsl_rv_t tr69_inline_timeout(tsl_int_t def_wait_timeout, tsl_int_t *p_timeout_reason)
{
    tsl_uint_t wait_timeout = def_wait_timeout;
    tsl_uint_t wait_timeout2 = def_wait_timeout;

    wait_timeout = tr69_inline_del_instance_timeout(def_wait_timeout);
    wait_timeout2 = tr69_inline_add_delaynotification_timeout(def_wait_timeout);
    if (wait_timeout2 < wait_timeout){
        *p_timeout_reason = TIMEOUT_REASON_DELAYNOTIF;
        return wait_timeout2;
    }else{
        *p_timeout_reason = TIMEOUT_REASON_DELINST;
        return wait_timeout;
    }
    return wait_timeout;
}

tsl_rv_t tr69_inline_check_move_condition(tsl_char_t *p_name_dst, tsl_char_t *p_name_src)
{
    tsl_char_t *match = NULL;
    tsl_char_t oid_stack_dst[32][128] = {{"\0"}};
    tsl_char_t oid_stack_src[32][128] = {{"\0"}};
    tsl_char_t *ptr = NULL;
    tsl_int_t i = 0;
    tsl_int_t j = 0;

    ptr = p_name_dst;
    while((match = strchr(ptr, '.')) != NULL){
            memcpy(oid_stack_dst[i++], ptr, match - ptr);
            ptr = match + 1;
    }
    memcpy(oid_stack_dst[i++], ptr, p_name_dst + strlen(p_name_dst) - ptr);

    ptr = p_name_src;
    while((match = strchr(ptr, '.')) != NULL){
            memcpy(oid_stack_src[j++], ptr, match - ptr);
            ptr = match + 1;
    }
    memcpy(oid_stack_src[j++], ptr, p_name_src + strlen(p_name_src) - ptr);

    if (i != j -1){
        return TSL_RV_ERR_PARM;
    }

    for (i=0; i<j-1; i++){
        if (!isdigit(oid_stack_dst[i][0])){
            if (strcmp(oid_stack_dst[i], oid_stack_src[i]) != 0){
                return TSL_RV_ERR_PARM;
            }
        }
    }

    return TSL_RV_SUC;
}

tsl_rv_t tr69_inline_move_instance(tr69_info_tree_t *p_tree_dst, tr69_info_tree_t *p_tree_src, tsl_int_t *p_inst_numb)
{
    tsl_int_t i = 0;
    tsl_char_t tmp[MAX_NAME_SIZE] = {0};

    tr69_uninit_tree(p_tree_src);
    tsl_list_del(&p_tree_src->list_node);
    sprintf(tmp, "%s", p_tree_src->full_name);
    for (i = strlen(tmp)-1; i > 0; i--){
        if (tmp[i] == '.'){
                tmp[i] = '\0';
                break;
        }
    }
    tr69_intf_mod_instance(tmp, -1);

    p_tree_src->p_parent = p_tree_dst;
    tr69_inline_adjust_instance(p_tree_dst, p_tree_src, p_tree_dst->full_name);
    tr69_intf_mod_instance(p_tree_dst->full_name, 1);
    *p_inst_numb = p_tree_src->info.info_inst.size;
    tr69_init_tree(p_tree_src);

    return TSL_RV_SUC;
}

/*Internal Interface Function*/

tsl_rv_t tr69_intf_mod_instance(tsl_char_t *p_name, tsl_int_t rel)
{
        tsl_int_t  i = 0;
        tsl_int_t  j = 0;
        tsl_char_t oid_stack[32][128] = {{"\0"}};
        tsl_char_t *ptr = NULL;
        tsl_char_t *match = NULL;
        tsl_char_t tmp[512] = "\0";
        tsl_char_t numb_oid[512] = "\0";
        tsl_int_t  numb = 0;
        tsl_int_t  type = 0;

        ctllog_debug("%s %d %s\n", __FUNCTION__, __LINE__, p_name);

        ptr = p_name;
        while((match = strchr(ptr, '.')) != NULL){
                memcpy(oid_stack[j++], ptr, match - ptr);
                ptr = match + 1;
        }
        memcpy(oid_stack[j++], ptr, p_name + strlen(p_name) - ptr);

        for (i = 0; i < j ; i++){
                if (atoi(oid_stack[i]) == 0){
                        strcat(tmp, oid_stack[i]);
                }else {
                        strcat(tmp, "i");
                }
                strcat(tmp, ".");

                if (i < j - 1){
                        strcat(numb_oid, oid_stack[i]);
                        strcat(numb_oid, ".");
                }
        }

        strcat(tmp, "i");
        for (i = 0; i < tr69_inst_numb_tb_count; i ++){
                if (!strcmp(tmp, tr69_inst_numb_tb[i].entries_oid)){
                        memset(tmp, 0, sizeof(tmp));
                        sprintf(tmp, "%s%s", numb_oid, tr69_inst_numb_tb[i].numb_of_entries_oid);

                        TSL_VASSERT(tr69_get_unfresh_leaf_data(tmp, (void **)&numb, &type) == TSL_RV_SUC);

                        numb = numb + rel;

                        ctllog_debug("begin to set_leaf_data %s\n", tmp);
                        TSL_VASSERT(tr69_set_unfresh_leaf_data(tmp, (void *)&numb, type) == TSL_RV_SUC);
                        ctllog_debug("end to set_leaf_data %s\n", tmp);

                        break;
                }
        }

        return TSL_RV_SUC;
}


tsl_rv_t tr69_intf_add_instance(tr69_info_tree_t *p_root, tsl_char_t *p_name, tsl_int_t *p_inst_numb)
{
        tr69_info_tree_t    *p_tree = NULL;
        tr69_info_tree_t    *p_inst = NULL;
        tr69_info_tree_t    *p_new_inst = NULL;
        tsl_char_t          tmp[TR69_OID_FULL_NAME_SIZE] = "\0";
        tsl_char_t          new_name[TR69_OID_FULL_NAME_SIZE] = "\0";


        TSL_VASSERT(p_root != NULL);
        TSL_VASSERT(p_name != NULL);
        TSL_VASSERT(p_inst_numb != NULL);

        tr69_inline_get_node(p_root, p_name, &p_tree);
        TSL_VASSERT(p_tree != NULL);

        tr69_inline_parse_oid(p_name, new_name);
#ifndef SUPPORT_PROTYPE0
        sprintf(tmp, "%s.1", new_name);
#else
        sprintf(tmp, "%s.0", new_name);
#endif

        tr69_inline_get_node(pg_ghost_tree, tmp, &p_inst);
        TSL_VASSERT(p_inst != NULL);

        p_new_inst = (tr69_info_tree_t *)calloc(1, sizeof(tr69_info_tree_t));

        tr69_inline_clone_obj(p_inst, p_new_inst);

        p_new_inst->p_parent = p_tree;

        tr69_inline_adjust_instance(p_tree, p_new_inst, p_name);

        tr69_intf_mod_instance(p_name, 1);

        ctllog_debug("add instance %d\n", p_new_inst->info.info_inst.size);
        *p_inst_numb = p_new_inst->info.info_inst.size;

        tr69_init_tree(p_new_inst);

        return TSL_RV_SUC;
}


tsl_rv_t tr69_intf2_del_instance(tr69_info_tree_t *p_root, tsl_char_t *p_name)
{
        tr69_info_tree_t    *p_tree = NULL;
        tr69_info_tree_t    *p_parent = NULL;
        char tmp[TR69_OID_FULL_NAME_SIZE] = "\0";
        tsl_int_t           i = 0;

        TSL_VASSERT(p_root != NULL);
        TSL_VASSERT(p_name != NULL);

        tr69_inline_get_node(p_root, p_name, &p_tree);
        TSL_VASSERT(p_tree != NULL);
        TSL_VASSERT(p_tree->node_type == TR69_NODE_TYPE_INSTANCE);

        p_parent = p_tree->p_parent;
        TSL_VASSERT(p_parent != NULL);
        TSL_VASSERT(p_parent->node_type == TR69_NODE_TYPE_OBJECT ||
                    p_parent->node_type == TR69_NODE_TYPE_INSTANCE);

        //tsl_list_del(&p_tree->list_node);
        tr69_uninit_tree(p_tree);
        tr69_inline_del_tree(p_tree);

        sprintf(tmp, "%s", p_name);

        for (i = strlen(p_name); i > 0; i--){
                if (tmp[i] == '.'){
                        tmp[i] = '\0';
                        break;
                }
        }
        tr69_intf_mod_instance(tmp, -1);

        return TSL_RV_SUC;
}


tsl_rv_t tr69_intf_del_instance(tr69_info_tree_t *p_root, tsl_char_t *p_name, tsl_int_t inst)
{
        tr69_info_tree_t    *p_tree = NULL;
        tr69_info_tree_t    *p_parent = NULL;
        char tmp[TR69_OID_FULL_NAME_SIZE] = "\0";

        TSL_VASSERT(p_root != NULL);
        TSL_VASSERT(p_name != NULL);
        TSL_VASSERT(inst != 0);

        sprintf(tmp,"%s.%d", p_name, inst);

        tr69_inline_get_node(p_root, tmp, &p_tree);
        TSL_VASSERT(p_tree != NULL);
        TSL_VASSERT(p_tree->node_type == TR69_NODE_TYPE_INSTANCE);

        p_parent = p_tree->p_parent;
        TSL_VASSERT(p_parent != NULL);
        TSL_VASSERT(p_parent->node_type == TR69_NODE_TYPE_OBJECT ||
                    p_parent->node_type == TR69_NODE_TYPE_INSTANCE);

        //tsl_list_del(&p_tree->list_node);
        tr69_uninit_tree(p_tree);
        tr69_inline_del_tree(p_tree);

        tr69_intf_mod_instance(p_name, -1);

        return TSL_RV_SUC;
}

tsl_rv_t tr69_intf_restore_default_node(tr69_info_tree_t *p_root, tsl_char_t *p_name)
{
        tr69_info_tree_t    *p_tree = NULL;
        tr69_info_tree_t    *p_inst = NULL;
        tr69_info_tree_t    *p_new_inst = NULL;
        tr69_info_tree_t    *p_parent = NULL;
        tsl_list_head_t       *p_list_node_prev = NULL;

        TSL_VASSERT(p_root != NULL);
        TSL_VASSERT(p_name != NULL);

        tr69_inline_get_node(p_root, p_name, &p_tree);

        TSL_VASSERT(p_tree != NULL);
        p_parent = p_tree->p_parent;

        TSL_VASSERT(p_parent != NULL);
        TSL_VASSERT(p_parent->node_type == TR69_NODE_TYPE_OBJECT);

        p_list_node_prev = p_tree->list_node.prev;

        tr69_uninit_tree(p_tree);
        tr69_inline_del_tree(p_tree);
        p_tree = NULL;

        tr69_inline_get_node(pg_origin_tree, p_name, &p_inst);

        TSL_VASSERT(p_inst != NULL);
        tr69_inline_get_node(p_root, p_inst->p_parent->full_name, &p_parent);
        p_new_inst = (tr69_info_tree_t *)calloc(1, sizeof(tr69_info_tree_t));

        tr69_inline_clone_obj(p_inst, p_new_inst);

        p_new_inst->p_parent = p_parent;

        tsl_list_add(&p_new_inst->list_node, p_list_node_prev);

        tr69_init_tree(p_new_inst);

        return TSL_RV_SUC;
}



tsl_rv_t tr69_intf_get_unfresh_obj_data(tr69_info_tree_t *p_root, tsl_char_t *p_name, tsl_void_t **pp_find_data)
{
        tr69_info_tree_t    *p_tree = NULL;

        TSL_VASSERT(p_root != NULL);
        TSL_VASSERT(p_name != NULL);
        TSL_VASSERT(pp_find_data != NULL);

        TSL_VASSERT(tr69_inline_get_node(p_root, p_name, &p_tree) == TSL_RV_SUC);
        TSL_VASSERT(p_tree != NULL);
        TSL_VASSERT(p_tree->node_type == TR69_NODE_TYPE_OBJECT ||
                    p_tree->node_type == TR69_NODE_TYPE_INSTANCE);

        *pp_find_data = tr69_inline_dump_data(p_tree);

        return TSL_RV_SUC;
}

tsl_rv_t tr69_intf_get_unfresh_obj_data2(tr69_info_tree_t *p_tree, tsl_char_t *p_name, tsl_void_t **pp_find_data)
{
	TSL_VASSERT(p_tree != NULL);
	TSL_VASSERT(p_name != NULL);
	TSL_VASSERT(pp_find_data != NULL);

	TSL_VASSERT(p_tree->p_parent->node_type == TR69_NODE_TYPE_OBJECT ||
				p_tree->p_parent->node_type == TR69_NODE_TYPE_INSTANCE);

	*pp_find_data = tr69_inline_dump_data(p_tree);

	return TSL_RV_SUC;
}

tsl_rv_t tr69_intf_fresh_obj_data(tr69_info_tree_t *p_root, tsl_char_t *p_name)
{
        tr69_info_tree_t    *p_tree = NULL;

        TSL_VASSERT(p_root != NULL);
        TSL_VASSERT(p_name != NULL);

        TSL_VASSERT(tr69_inline_get_node(p_root, p_name, &p_tree) == TSL_RV_SUC);
        TSL_VASSERT(p_tree != NULL);
        TSL_VASSERT(p_tree->node_type == TR69_NODE_TYPE_OBJECT ||
                    p_tree->node_type == TR69_NODE_TYPE_INSTANCE);

        tr69_inline_fresh_obj_data(p_root, p_tree);

        return TSL_RV_SUC;
}

tsl_rv_t tr69_intf_get_obj_data_size(tr69_info_tree_t *p_root, tsl_char_t *p_name, tsl_int_t *data_size)
{
        tr69_info_tree_t    *p_tree = NULL;

        TSL_VASSERT(p_root != NULL);
        TSL_VASSERT(p_name != NULL);
        TSL_VASSERT(data_size != NULL);


        TSL_VASSERT(tr69_inline_get_node(p_root, p_name, &p_tree) == TSL_RV_SUC);
        TSL_VASSERT(p_tree != NULL);
        TSL_VASSERT(p_tree->node_type == TR69_NODE_TYPE_OBJECT ||
                    p_tree->node_type == TR69_NODE_TYPE_INSTANCE);

        if (p_tree->node_type == TR69_NODE_TYPE_OBJECT){
                *data_size = p_tree->info.info_obj.data_size;
        }else if (p_tree->node_type == TR69_NODE_TYPE_INSTANCE ){
                *data_size = p_tree->info.info_inst.data_size;
        }

        return TSL_RV_SUC;
}

tsl_rv_t tr69_intf_get_obj_data(tr69_info_tree_t *p_root, tsl_char_t *p_name, tsl_void_t **pp_find_data)
{
        tr69_info_tree_t    *p_tree = NULL;
        tsl_rv_t rv;

        TSL_VASSERT(p_root != NULL);
        TSL_VASSERT(p_name != NULL);
        TSL_VASSERT(pp_find_data != NULL);

        TSL_VASSERT(tr69_inline_get_node(p_root, p_name, &p_tree) == TSL_RV_SUC);
        TSL_VASSERT(p_tree != NULL);
        TSL_VASSERT(p_tree->node_type == TR69_NODE_TYPE_OBJECT ||
                    p_tree->node_type == TR69_NODE_TYPE_INSTANCE);

        *pp_find_data = tr69_inline_dump_data(p_tree);

        if(*pp_find_data == NULL)
        {
            return TSL_RV_SUC;
        }
        if (p_tree->node_type == TR69_NODE_TYPE_OBJECT){
                if (p_tree->info.info_obj.get_func != NULL){
                        rv = p_tree->info.info_obj.get_func(p_name, *pp_find_data);

                        if ((rv == TR69_RT_SUCCESS_VALUE_CHANGED) || (rv == TSL_RV_SUC)){
                                tr69_inline_save_data(p_tree, *pp_find_data);
                        }
                }
        }else if (p_tree->node_type == TR69_NODE_TYPE_INSTANCE){
                if (p_tree->p_parent->info.info_obj.get_func != NULL){
                        rv = p_tree->p_parent->info.info_obj.get_func(p_name, *pp_find_data);

                        if ((rv == TR69_RT_SUCCESS_VALUE_CHANGED) || (rv == TSL_RV_SUC)){
                                tr69_inline_save_data(p_tree, *pp_find_data);
                        }
                }
        }

        return TSL_RV_SUC;
}

tsl_rv_t tr69_intf_get_obj_data2(tr69_info_tree_t *p_tree, tsl_char_t *p_name, tsl_void_t **pp_find_data)
{
	tsl_rv_t rv;

	TSL_VASSERT(p_tree != NULL);
    TSL_VASSERT(p_name != NULL);
    TSL_VASSERT(pp_find_data != NULL);

	TSL_VASSERT(p_tree->node_type == TR69_NODE_TYPE_OBJECT ||
				p_tree->node_type == TR69_NODE_TYPE_INSTANCE);

	*pp_find_data = tr69_inline_dump_data(p_tree);
	if(*pp_find_data == NULL)
    {
        return TSL_RV_SUC;
    }

	if (p_tree->node_type == TR69_NODE_TYPE_OBJECT){
            if (p_tree->info.info_obj.get_func != NULL){
                    rv = p_tree->info.info_obj.get_func(p_name, *pp_find_data);

                    if ((rv == TR69_RT_SUCCESS_VALUE_CHANGED) || (rv == TSL_RV_SUC)){
                            tr69_inline_save_data(p_tree, *pp_find_data);
                    }
            }
    }else if (p_tree->node_type == TR69_NODE_TYPE_INSTANCE){
            if (p_tree->p_parent->info.info_obj.get_func != NULL){
                    rv = p_tree->p_parent->info.info_obj.get_func(p_name, *pp_find_data);

                    if ((rv == TR69_RT_SUCCESS_VALUE_CHANGED) || (rv == TSL_RV_SUC)){
                            tr69_inline_save_data(p_tree, *pp_find_data);
                    }
            }
    }

    return TSL_RV_SUC;
}

tsl_rv_t tr69_intf_get_commit_obj_data(tr69_info_tree_t *p_root, tsl_char_t *p_name, tsl_void_t **pp_find_data, tsl_int_t alloc)
{
        tr69_info_tree_t    *p_tree = NULL;

        TSL_VASSERT(p_root != NULL);
        TSL_VASSERT(p_name != NULL);
        TSL_VASSERT(pp_find_data != NULL);

        TSL_VASSERT(tr69_inline_get_node(p_root, p_name, &p_tree) == TSL_RV_SUC);
        TSL_VASSERT(p_tree != NULL);
        TSL_VASSERT(p_tree->node_type == TR69_NODE_TYPE_OBJECT ||
                    p_tree->node_type == TR69_NODE_TYPE_INSTANCE);

        if (p_tree->node_type == TR69_NODE_TYPE_OBJECT){
                if (alloc && p_tree->info.info_obj.p_commit_data == NULL){
                        p_tree->info.info_obj.p_commit_data = tr69_inline_dump_data(p_tree);
                }

                *pp_find_data = p_tree->info.info_obj.p_commit_data;
        }else if (p_tree->node_type == TR69_NODE_TYPE_INSTANCE){
                if (alloc && p_tree->info.info_inst.p_commit_data == NULL){
                        p_tree->info.info_inst.p_commit_data = tr69_inline_dump_data(p_tree);
                }

                *pp_find_data = p_tree->info.info_inst.p_commit_data;
        }

        return TSL_RV_SUC;
}


tsl_rv_t tr69_intf_set_commit_obj_data(tr69_info_tree_t *p_root, tsl_char_t *p_name, tsl_void_t *p_find_data)
{
        tr69_info_tree_t    *p_tree = NULL;

        TSL_VASSERT(p_root != NULL);
        TSL_VASSERT(p_name != NULL);

        TSL_VASSERT(tr69_inline_get_node(p_root, p_name, &p_tree) == TSL_RV_SUC);
        TSL_VASSERT(p_tree != NULL);
        TSL_VASSERT(p_tree->node_type == TR69_NODE_TYPE_OBJECT ||
                    p_tree->node_type == TR69_NODE_TYPE_INSTANCE);

        if (p_tree->node_type == TR69_NODE_TYPE_OBJECT){
                p_tree->info.info_obj.p_commit_data = p_find_data;
        }else if (p_tree->node_type == TR69_NODE_TYPE_INSTANCE){
                p_tree->info.info_inst.p_commit_data = p_find_data;
        }

        return TSL_RV_SUC;
}



tsl_rv_t tr69_intf_save_obj_data(tr69_info_tree_t *p_root, tsl_char_t *p_name, tsl_void_t **pp_data)
{
        tr69_info_tree_t    *p_tree = NULL;
        tsl_void_t *p_cur_data = NULL;
        tsl_rv_t rv = TSL_RV_SUC;

        TSL_VASSERT(p_root != NULL);
        TSL_VASSERT(p_name != NULL);
        TSL_VASSERT(pp_data != NULL);

        TSL_VASSERT(tr69_inline_get_node(p_root, p_name, &p_tree) == TSL_RV_SUC);
        TSL_VASSERT(p_tree != NULL);
        TSL_VASSERT(p_tree->node_type == TR69_NODE_TYPE_OBJECT ||
                    p_tree->node_type == TR69_NODE_TYPE_INSTANCE);


        p_cur_data = tr69_inline_dump_data(p_tree);
        TSL_VASSERT(p_cur_data != NULL);

        if (p_tree->node_type == TR69_NODE_TYPE_OBJECT){
                tr69_inline_save_data(p_tree, *pp_data);

                if (p_tree->info.info_obj.set_func != NULL){
                        rv = p_tree->info.info_obj.set_func(p_name, p_cur_data, *pp_data);
                }

                if ((rv == TR69_RT_SUCCESS_VALUE_CHANGED) || (rv == TSL_RV_SUC)){
                        tr69_inline_save_data(p_tree, *pp_data);
                }else {
                        tr69_inline_save_data(p_tree, p_cur_data);
                }
        }else if (p_tree->node_type == TR69_NODE_TYPE_INSTANCE){
                tr69_inline_save_data(p_tree, *pp_data);

                if (p_tree->p_parent->info.info_obj.set_func != NULL){
                        rv = p_tree->p_parent->info.info_obj.set_func(p_name, p_cur_data, *pp_data);
                }

                if ((rv == TR69_RT_SUCCESS_VALUE_CHANGED) || (rv == TSL_RV_SUC)){
                        tr69_inline_save_data(p_tree, *pp_data);
                }else {
                        tr69_inline_save_data(p_tree, p_cur_data);
                }
        }

        tr69_inline_free_data(p_tree, &p_cur_data);

        if (rv == TR69_RT_SUCCESS_VALUE_CHANGED){
            rv = TSL_RV_SUC;
        }

        return rv;
}


tsl_rv_t tr69_intf_save_commit_obj_data(tr69_info_tree_t *p_root, tsl_char_t *p_name)
{
        tr69_info_tree_t    *p_tree = NULL;
        tsl_void_t *p_cur_data = NULL;
        tsl_void_t *p_new_data = NULL;
        tsl_rv_t rv = TSL_RV_SUC;

        TSL_VASSERT(p_root != NULL);
        TSL_VASSERT(p_name != NULL);

        TSL_VASSERT(tr69_inline_get_node(p_root, p_name, &p_tree) == TSL_RV_SUC);
        TSL_VASSERT(p_tree != NULL);
        TSL_VASSERT(p_tree->node_type == TR69_NODE_TYPE_OBJECT ||
                    p_tree->node_type == TR69_NODE_TYPE_INSTANCE);

        tr69_intf_get_commit_obj_data(p_root, p_tree->full_name, &p_new_data, 0);
        TSL_VASSERT(p_new_data != NULL);

        p_cur_data = tr69_inline_dump_data(p_tree);
        TSL_VASSERT(p_cur_data != NULL);


        if (p_tree->node_type == TR69_NODE_TYPE_OBJECT){
                tr69_inline_save_data(p_tree, p_new_data);

                if (p_tree->info.info_obj.set_func != NULL){
                        rv = p_tree->info.info_obj.set_func(p_name, p_cur_data, p_new_data);
                }

                if ((rv == TR69_RT_SUCCESS_VALUE_CHANGED) || (rv == TSL_RV_SUC)){
                        tr69_inline_save_data(p_tree, p_new_data);
                }else {
                        tr69_inline_save_data(p_tree, p_cur_data);
                }

        }else if (p_tree->node_type == TR69_NODE_TYPE_INSTANCE){
                tr69_inline_save_data(p_tree, p_new_data);

                if (p_tree->p_parent->info.info_obj.set_func != NULL){
                        rv = p_tree->p_parent->info.info_obj.set_func(p_name, p_cur_data, p_new_data);
                }

                if ((rv == TR69_RT_SUCCESS_VALUE_CHANGED) || (rv == TSL_RV_SUC)){
                        tr69_inline_save_data(p_tree, p_new_data);
                }else {
                        tr69_inline_save_data(p_tree, p_cur_data);
                }
        }

        tr69_inline_free_data(p_tree, &p_cur_data);
        tr69_inline_free_data(p_tree, &p_new_data);

        TSL_VASSERT_RV(tr69_intf_set_commit_obj_data(p_root, p_tree->full_name, NULL) == TSL_RV_SUC, TSL_RV_FAIL);

        if (rv == TR69_RT_SUCCESS_VALUE_CHANGED){
            rv = TSL_RV_SUC;
        }

        return rv;
}


tsl_rv_t tr69_intf_save_unfresh_obj_data(tr69_info_tree_t *p_root, tsl_char_t *p_name, tsl_void_t **pp_data)
{
        tr69_info_tree_t    *p_tree = NULL;
        tsl_void_t *p_cur_data = NULL;

        TSL_VASSERT(p_root != NULL);
        TSL_VASSERT(p_name != NULL);
        TSL_VASSERT(pp_data != NULL);

        TSL_VASSERT(tr69_inline_get_node(p_root, p_name, &p_tree) == TSL_RV_SUC);
        TSL_VASSERT(p_tree != NULL);
        TSL_VASSERT(p_tree->node_type == TR69_NODE_TYPE_OBJECT ||
                    p_tree->node_type == TR69_NODE_TYPE_INSTANCE);


        p_cur_data = tr69_inline_dump_data(p_tree);
        TSL_VASSERT(p_cur_data != NULL);

        tr69_inline_save_data(p_tree, *pp_data);

        tr69_inline_free_data(p_tree, &p_cur_data);

        return TSL_RV_SUC;
}


tsl_rv_t tr69_intf_free_obj_data(tr69_info_tree_t *p_root, tsl_char_t *p_name, tsl_void_t **pp_data)
{
        tr69_info_tree_t *p_tree = NULL;

        TSL_VASSERT(p_root != NULL);
        TSL_VASSERT(p_name != NULL);
        TSL_VASSERT(pp_data != NULL);

        TSL_VASSERT(tr69_inline_get_node(p_root, p_name, &p_tree) == TSL_RV_SUC);
        TSL_VASSERT(p_tree != NULL);
        TSL_VASSERT(p_tree->node_type == TR69_NODE_TYPE_OBJECT ||
                    p_tree->node_type == TR69_NODE_TYPE_INSTANCE);

        tr69_inline_free_data(p_tree, pp_data);

        return TSL_RV_SUC;
}

tsl_rv_t tr69_intf_get_unfresh_leaf_data(tr69_info_tree_t *p_root, tsl_char_t *p_name, tsl_void_t **pp_find_data, tsl_int_t *type)
{
        tr69_info_tree_t *p_tree = NULL;
        tsl_void_t       *p_data = NULL;
        tsl_char_t       **pp_src = NULL;
        tsl_int_t        *p_isrc = NULL;

        TSL_VASSERT(p_root != NULL);
        TSL_VASSERT(p_name != NULL);
        TSL_VASSERT(pp_find_data != NULL);
        TSL_VASSERT(type != NULL);

        TSL_VASSERT(tr69_inline_get_node(p_root, p_name, &p_tree) == TSL_RV_SUC);
        TSL_VASSERT(p_tree != NULL);

        if(p_tree->node_type == TR69_NODE_TYPE_OBJECT ||
           p_tree->node_type == TR69_NODE_TYPE_INSTANCE){
                tr69_inline_fresh_obj_data(p_root, p_tree);
                return TSL_RV_ERR;
        }

        TSL_VASSERT(p_tree->node_type == TR69_NODE_TYPE_LEAF);


        TSL_VASSERT(p_tree->p_parent != NULL);
        TSL_VASSERT(p_tree->p_parent->node_type == TR69_NODE_TYPE_OBJECT ||
                    p_tree->p_parent->node_type == TR69_NODE_TYPE_INSTANCE);

        p_data = tr69_inline_dump_data(p_tree->p_parent);

        //tr69_intf_get_unfresh_obj_data(p_root, p_tree->p_parent->full_name, &p_data);
        //tr69_intf_get_unfresh_obj_data2(p_tree, p_tree->p_parent->full_name, &p_data);
        TSL_VASSERT(p_data != NULL);

        if (p_tree->info.info_leaf.type == TR69_NODE_LEAF_TYPE_STRING||
            p_tree->info.info_leaf.type == TR69_NODE_LEAF_TYPE_DATE){
                pp_src = ((char **)((char *)(p_data)+ p_tree->info.info_leaf.data_offset));
                *type = p_tree->info.info_leaf.type;
                if (pp_src && *pp_src ){
                        *pp_find_data = strdup(*pp_src);
                }else {
                        *pp_find_data = strdup("");//NULL;
                }
        }else {
                p_isrc = ((int *)((char *)(p_data)+ p_tree->info.info_leaf.data_offset));
                *(int *)pp_find_data = *p_isrc;
                *type = p_tree->info.info_leaf.type;
        }
        //tr69_intf_free_obj_data(p_root, p_tree->p_parent->full_name, &p_data);
        tr69_inline_free_data(p_tree->p_parent, &p_data);

        return TSL_RV_SUC;
}


tsl_rv_t tr69_intf_get_leaf_data(tr69_info_tree_t *p_root, tsl_char_t *p_name, tsl_void_t **pp_find_data, tsl_int_t *type)
{
        tr69_info_tree_t *p_tree = NULL;
        tsl_void_t       *p_data = NULL;
        tsl_char_t       **pp_src = NULL;
        tsl_int_t        *p_isrc = NULL;


        TSL_VASSERT(p_root != NULL);
        TSL_VASSERT(p_name != NULL);
        TSL_VASSERT(pp_find_data != NULL);
        TSL_VASSERT(type != NULL);

        TSL_VASSERT(tr69_inline_get_node(p_root, p_name, &p_tree) == TSL_RV_SUC);
        TSL_VASSERT(p_tree != NULL);
        TSL_VASSERT(p_tree->node_type == TR69_NODE_TYPE_LEAF);
        //TSL_VASSERT_RV(p_tree->info.info_leaf.acl != TR69_NODE_LEAF_ACL_WRITEONLY, TR69_ERROR_BAD_ACL);

        //tr69_intf_get_obj_data(p_root, p_tree->p_parent->full_name, &p_data);
        tr69_intf_get_obj_data2(p_tree->p_parent, p_tree->p_parent->full_name, &p_data);
        TSL_VASSERT(p_data != NULL);

        if (p_tree->info.info_leaf.type == TR69_NODE_LEAF_TYPE_STRING||
            p_tree->info.info_leaf.type == TR69_NODE_LEAF_TYPE_DATE){
                pp_src = ((char **)((char *)(p_data)+ p_tree->info.info_leaf.data_offset));
                *type = p_tree->info.info_leaf.type;
                if (pp_src && *pp_src ){
                        *pp_find_data = strdup(*pp_src);
                }else {
                        *pp_find_data = strdup("");//NULL;
                }
        }else {
                p_isrc = ((int *)((char *)(p_data)+ p_tree->info.info_leaf.data_offset));
                *(int *)pp_find_data = *p_isrc;
                *type = p_tree->info.info_leaf.type;
        }
        //tr69_intf_free_obj_data(p_root, p_tree->p_parent->full_name, &p_data);
        tr69_inline_free_data(p_tree->p_parent, &p_data);

        return TSL_RV_SUC;
}

tsl_rv_t tr69_intf_commit_set_leaf_data(tr69_info_tree_t *p_root, tsl_char_t *p_name, tsl_void_t *p_value, tsl_int_t type, tsl_int_t acl)
{
        tr69_info_tree_t *p_tree = NULL;
        tsl_char_t       **pp_dst = NULL;
        tsl_int_t        *p_idst = NULL;
        tsl_void_t       *p_data = NULL;
        tsl_rv_t              rv = TSL_RV_SUC;

        TSL_VASSERT(p_root != NULL);
        TSL_VASSERT(p_name != NULL);
        TSL_VASSERT(p_value != NULL);
        TSL_VASSERT(type != 0);

        TSL_VASSERT(tr69_inline_get_node(p_root, p_name, &p_tree) == TSL_RV_SUC);
        TSL_VASSERT(p_tree != NULL);
        TSL_VASSERT(p_tree->node_type == TR69_NODE_TYPE_LEAF);

        if (acl == 1){
                TSL_VASSERT_RV(p_tree->info.info_leaf.acl != TR69_NODE_LEAF_ACL_READONLY, TR69_ERROR_BAD_ACL);
        }

        //to avoid call get_func(), used get_unfresh_obj_data() to instead of get_obj_data()
        tr69_intf_get_commit_obj_data(p_root, p_tree->p_parent->full_name, &p_data, 1);

        TSL_VASSERT(p_data != NULL);

        if (p_tree->info.info_leaf.type == TR69_NODE_LEAF_TYPE_STRING||
            p_tree->info.info_leaf.type == TR69_NODE_LEAF_TYPE_DATE){
                pp_dst = ((char **)((char *)(p_data)+ p_tree->info.info_leaf.data_offset));

                if (*pp_dst){
                        free(*pp_dst);
                        *pp_dst = NULL;
                }

                *pp_dst = strdup((char *)p_value);

        }else if (p_tree->info.info_leaf.type == TR69_NODE_LEAF_TYPE_BOOL){
        	p_idst = ((int *)((char *)(p_data)+ p_tree->info.info_leaf.data_offset));

        	if (type == TR69_NODE_LEAF_TYPE_STRING){
				if ((strcasecmp(p_value, "true") == 0) || (strcmp(p_value, "1") == 0)){
					*p_idst = 1;
				}else if ((strcasecmp(p_value, "false") == 0) || (strcmp(p_value, "0") == 0)){
					*p_idst = 0;
				}else {
					rv = TSL_RV_FAIL;
				}
        	}else if (type == TR69_NODE_LEAF_TYPE_INT || TR69_NODE_LEAF_TYPE_UINT){
        		if (((*(int *)p_value) == 0) || ((*(int *)p_value) == 1)){
	        		*p_idst = *(int *)p_value;
        		}else {
					rv = TSL_RV_FAIL;
        		}
        	}
        }else {
                p_idst = ((int *)((char *)(p_data)+ p_tree->info.info_leaf.data_offset));

                if (type == TR69_NODE_LEAF_TYPE_STRING){
                        *p_idst = atoi(p_value);
                }else if (type == TR69_NODE_LEAF_TYPE_INT || TR69_NODE_LEAF_TYPE_UINT){
                        *p_idst = *(int *)p_value;
                }else {
					rv = TSL_RV_FAIL;
                }
        }

        return rv;
}



tsl_rv_t tr69_intf_set_leaf_data(tr69_info_tree_t *p_root, tsl_char_t *p_name, tsl_void_t *p_value, tsl_int_t type, tsl_int_t acl)
{
        tr69_info_tree_t *p_tree = NULL;
        tsl_char_t       **pp_dst = NULL;
        tsl_int_t        *p_idst = NULL;
        tsl_void_t       *p_data = NULL;
		tsl_rv_t 			  rv = TSL_RV_SUC;


        TSL_VASSERT(p_root != NULL);
        TSL_VASSERT(p_name != NULL);
        TSL_VASSERT(p_value != NULL);
        TSL_VASSERT(type != 0);

        TSL_VASSERT(tr69_inline_get_node(p_root, p_name, &p_tree) == TSL_RV_SUC);
        TSL_VASSERT(p_tree != NULL);
        TSL_VASSERT(p_tree->node_type == TR69_NODE_TYPE_LEAF);

        if (acl == 1){
                TSL_VASSERT_RV(p_tree->info.info_leaf.acl != TR69_NODE_LEAF_ACL_READONLY, TR69_ERROR_BAD_ACL);
        }

        //to avoid call get_func(), used get_unfresh_obj_data() to instead of get_obj_data()
        tr69_intf_get_unfresh_obj_data(p_root, p_tree->p_parent->full_name, &p_data);
        TSL_VASSERT(p_data != NULL);

        if (p_tree->info.info_leaf.type == TR69_NODE_LEAF_TYPE_STRING||
            p_tree->info.info_leaf.type == TR69_NODE_LEAF_TYPE_DATE){
                pp_dst = ((char **)((char *)(p_data)+ p_tree->info.info_leaf.data_offset));

                if (*pp_dst){
                        free(*pp_dst);
                        *pp_dst = NULL;
                }

                *pp_dst = strdup((char *)p_value);

        }else if (p_tree->info.info_leaf.type == TR69_NODE_LEAF_TYPE_BOOL){
        	p_idst = ((int *)((char *)(p_data)+ p_tree->info.info_leaf.data_offset));

        	if (type == TR69_NODE_LEAF_TYPE_STRING){
				if ((strcasecmp(p_value, "true") == 0) || (strcmp(p_value, "1") == 0)){
					*p_idst = 1;
				}else if ((strcasecmp(p_value, "false") == 0) || (strcmp(p_value, "0") == 0)){
					*p_idst = 0;
				}else {
					rv = TSL_RV_FAIL;
				}
        	}else if (type == TR69_NODE_LEAF_TYPE_INT || TR69_NODE_LEAF_TYPE_UINT){
        		if (((*(int *)p_value) == 0) || ((*(int *)p_value) == 1)){
	        		*p_idst = *(int *)p_value;
        		}else {
					rv = TSL_RV_FAIL;
        		}
        	}
        }else {
                p_idst = ((int *)((char *)(p_data)+ p_tree->info.info_leaf.data_offset));

                if (type == TR69_NODE_LEAF_TYPE_STRING){
                        *p_idst = atoi(p_value);
                }else if (type == TR69_NODE_LEAF_TYPE_INT || TR69_NODE_LEAF_TYPE_UINT){
                        *p_idst = *(int *)p_value;
                }else {
					rv = TSL_RV_FAIL;
                }
        }

        if (rv == TSL_RV_SUC){
            if (acl == 2){
                    //TSL_VASSERT_RV(tr69_intf_save_unfresh_obj_data(p_root, p_tree->p_parent->full_name, &p_data) == TSL_RV_SUC, TSL_RV_FAIL);
                    rv = tr69_intf_save_unfresh_obj_data(p_root, p_tree->p_parent->full_name, &p_data);
            }else {
                    //TSL_VASSERT_RV(tr69_intf_save_obj_data(p_root, p_tree->p_parent->full_name, &p_data) == TSL_RV_SUC, TSL_RV_FAIL);
                    rv = tr69_intf_save_obj_data(p_root, p_tree->p_parent->full_name, &p_data);
            }
        }

        TSL_VASSERT_RV(tr69_intf_free_obj_data(p_root, p_tree->p_parent->full_name, &p_data) == TSL_RV_SUC, TSL_RV_FAIL);

        return rv;
}


tsl_rv_t tr69_intf_get_node(tr69_info_tree_t *p_root, tsl_char_t *p_name, tr69_info_tree_t **pp_find_node)
{
        tr69_info_tree_t *p_tree = NULL;

        TSL_VASSERT(p_root != NULL);
        TSL_VASSERT(p_name != NULL);
        TSL_VASSERT(pp_find_node != NULL);


        tr69_inline_get_node(p_root, p_name, &p_tree);
        TSL_VASSERT(p_tree != NULL);

        *pp_find_node = (tr69_info_tree_t *)calloc(1, sizeof(tr69_info_tree_t));
        memcpy(*pp_find_node, p_tree, sizeof(tr69_info_tree_t));

        TSL_INIT_LIST_HEAD(&(*pp_find_node)->list_head);
        TSL_INIT_LIST_HEAD(&(*pp_find_node)->list_node);
        (*pp_find_node)->p_parent = NULL;

        if (p_tree->node_type == TR69_NODE_TYPE_OBJECT){
                (*pp_find_node)->info.info_obj.p_data = NULL;
        }else if (p_tree->node_type == TR69_NODE_TYPE_INSTANCE){
                (*pp_find_node)->info.info_inst.p_data = NULL;
        }

        return TSL_RV_SUC;
}

tsl_rv_t tr69_intf_move_instance(tr69_info_tree_t *p_root, tsl_char_t *p_name_dst, tsl_char_t *p_name_src, tsl_int_t *p_inst_numb)
{
    tr69_info_tree_t *p_tree_dst = NULL;
    tr69_info_tree_t *p_tree_src = NULL;

    TSL_VASSERT(p_name_dst != NULL);
    TSL_VASSERT(p_name_src != NULL);
    TSL_VASSERT(p_inst_numb != NULL);

    TSL_VASSERT(tr69_inline_check_move_condition(p_name_dst, p_name_src) == TSL_RV_SUC);

    tr69_inline_get_node(p_root, p_name_dst, &p_tree_dst);
    TSL_VASSERT(p_tree_dst != NULL);

    tr69_inline_get_node(p_root, p_name_src, &p_tree_src);
    TSL_VASSERT(p_tree_src != NULL);
    TSL_VASSERT(p_tree_src->node_type == TR69_NODE_TYPE_INSTANCE);

    TSL_FVASSERT(tr69_inline_move_instance(p_tree_dst, p_tree_src, p_inst_numb) == TSL_RV_SUC);

    return TSL_RV_SUC;
}


tsl_rv_t tr69_inline_register_tree(tr69_info_tree_t *p_root)
{
        tsl_list_head_t  *p_list = NULL;
        tr69_info_tree_t *p_tree = NULL;
        tsl_char_t       new_oid[TR69_OID_FULL_NAME_SIZE] = "\0";
        tsl_int_t        i = 0;

        if (p_root->node_type == TR69_NODE_TYPE_OBJECT){
                memset(new_oid, 0, sizeof(new_oid));
                tr69_inline_parse_oid2(p_root->full_name, new_oid);

                for (i = 0; i < tr69_regfunc_tb_count; i ++ ){
                        if (!strcmp(tr69_regfunc_tb[i].oid_name, new_oid)){
                                p_root->info.info_obj.get_func = tr69_regfunc_tb[i].get_func;
                                p_root->info.info_obj.set_func = tr69_regfunc_tb[i].set_func;
                        }
                }
        }

        for(p_list = p_root->list_head.prev; p_list != &p_root->list_head;){
	    p_tree = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
            tr69_inline_register_tree(p_tree);
	    p_list = p_list->prev;
        }
        return TSL_RV_SUC;
}


tsl_rv_t tr69_intf_cleanup_tree(tr69_info_tree_t *p_root, tsl_int_t free_data)
{
        TSL_VASSERT(p_root != NULL);

        tr69_inline_cleanup_tree(p_root, free_data);

        if (p_root->node_type == TR69_NODE_TYPE_OBJECT && free_data){
                if (p_root->info.info_obj.data_size){
                        tr69_inline_free_data(p_root, &(p_root->info.info_obj.p_data));
                }
        }else if (p_root->node_type == TR69_NODE_TYPE_INSTANCE && free_data){
                if (p_root->info.info_inst.data_size ){
                        tr69_inline_free_data(p_root, &(p_root->info.info_inst.p_data));
                }
        }
        free(p_root);
        p_root = NULL;

        return TSL_RV_SUC;
}

/*User Interface Function*/
tsl_rv_t tr69_init(tsl_char_t *p_protype_xml, tsl_char_t *p_cfg_xml, tsl_char_t *p_ori_cfg_xml)
{
        pg_origin_tree = tr69_func_parse_xml(p_ori_cfg_xml);
        pg_root_tree = tr69_func_parse_xml(p_cfg_xml);
        pg_ghost_tree = tr69_ghost_func_parse_xml(p_protype_xml);

        TSL_VASSERT(pg_origin_tree != NULL);
        TSL_VASSERT(pg_root_tree != NULL);
        TSL_VASSERT(pg_ghost_tree != NULL);

#if 0 //TODO TR69_VERSION
        TSL_VASSERT_RV(strcmp(protype_xml_version, cfg_xml_version) == 0, -2);
        TSL_VASSERT_RV(strcmp(protype_xml_version, st_version) == 0, -2);
#endif

        tr69_inline_register_tree(pg_origin_tree);
        tr69_inline_register_tree(pg_root_tree);
        tr69_inline_register_tree(pg_ghost_tree);

        tr69_init_tree(pg_root_tree);
        return TSL_RV_SUC;
}

tsl_rv_t tr69_register(tsl_void_t *p_inst_tb,
                       tsl_int_t inst_tb_count,
                       tsl_void_t *p_reg_tb,
                       tsl_int_t reg_tb_count)
{
        //tr69_inst_numb_tb = (struct tr69_inst_numb_of_entries_s *)p_inst_tb;
        //tr69_regfunc_tb = (struct tr69_register_func *)p_reg_tb;
        tr69_inst_numb_tb_count = inst_tb_count;
        tr69_regfunc_tb_count = reg_tb_count;

        return TSL_RV_SUC;
}

tsl_void_t tr69_cleanup()
{
        if (pg_origin_tree)
        {
            tr69_intf_cleanup_tree(pg_origin_tree, 1);
            pg_origin_tree = NULL;
        }


        if (pg_root_tree){
                tr69_intf_cleanup_tree(pg_root_tree, 1);
                pg_root_tree = NULL;
        }

        if (pg_ghost_tree_malloc){
                //tr69_intf_cleanup_tree(pg_ghost_tree, 0);
                free(pg_ghost_tree_malloc);
                pg_ghost_tree_malloc = NULL;
                pg_ghost_tree = NULL;
        }

        xmlCleanupParser();
}


tsl_rv_t tr69_save_xml(tsl_char_t *p_file_name)
{
        if (tr69_is_data_changed){
            tr69_is_data_changed = 0;
            return tr69_func_tree_to_xml(p_file_name, pg_root_tree);
        }
        return TSL_RV_FAIL;
}

tsl_rv_t tr69_get_obj_data_size(tsl_char_t *p_name, tsl_int_t *data_size)
{
        return tr69_intf_get_obj_data_size(pg_root_tree, p_name, data_size);
}

tsl_rv_t tr69_get_obj_data(tsl_char_t *p_name, tsl_void_t **pp_data)
{
        return tr69_intf_get_obj_data(pg_root_tree, p_name, pp_data);
}

tsl_rv_t tr69_get_unfresh_obj_data(tsl_char_t *p_name, tsl_void_t **pp_data)
{
        return tr69_intf_get_unfresh_obj_data(pg_root_tree, p_name, pp_data);
}
tsl_rv_t tr69_free_obj_data(tsl_char_t *p_name, tsl_void_t **pp_data)
{
        return tr69_intf_free_obj_data(pg_root_tree, p_name, pp_data);
}

tsl_rv_t tr69_set_unfresh_obj_data(tsl_char_t *p_name, tsl_void_t **pp_data)
{
        return tr69_intf_save_unfresh_obj_data(pg_root_tree, p_name, pp_data);
}

tsl_rv_t tr69_set_obj_data(tsl_char_t *p_name, tsl_void_t **pp_data)
{
        return tr69_intf_save_obj_data(pg_root_tree, p_name, pp_data);
}

tsl_rv_t tr69_get_leaf_data(tsl_char_t *p_name, tsl_void_t **pp_data, tsl_int_t *type)
{
        return tr69_intf_get_leaf_data(pg_root_tree, p_name, pp_data, type);
}

tsl_rv_t tr69_get_unfresh_leaf_data(tsl_char_t *p_name, tsl_void_t **pp_data, tsl_int_t *type)
{
        return tr69_intf_get_unfresh_leaf_data(pg_root_tree, p_name, pp_data, type);
}

tsl_rv_t tr69_set_leaf_data(tsl_char_t *p_name, tsl_void_t *p_value, tsl_int_t type)
{
        return tr69_intf_set_leaf_data(pg_root_tree, p_name, p_value, type, 0);
}

tsl_rv_t tr69_commit_set_leaf_data(tsl_char_t *p_name, tsl_void_t *p_value, tsl_int_t type)
{
        return tr69_intf_commit_set_leaf_data(pg_root_tree, p_name, p_value, type, 0);
}

tsl_rv_t tr69_acl_commit_set_leaf_data(tsl_char_t *p_name, tsl_void_t *p_value, tsl_int_t type)
{
        return tr69_intf_commit_set_leaf_data(pg_root_tree, p_name, p_value, type, 1);
}

tsl_rv_t tr69_commit_set_leaf_data_end(tsl_char_t *p_name)
{
        return tr69_intf_save_commit_obj_data(pg_root_tree, p_name);
}

tsl_rv_t tr69_set_unfresh_leaf_data(tsl_char_t *p_name, tsl_void_t *p_value, tsl_int_t type)
{
        return tr69_intf_set_leaf_data(pg_root_tree, p_name, p_value, type, 2);
}

tsl_rv_t tr69_acl_set_leaf_data(tsl_char_t *p_name, tsl_void_t *p_value, tsl_int_t type)
{
        return tr69_intf_set_leaf_data(pg_root_tree, p_name, p_value, type, 1);
}

tsl_rv_t tr69_set_node(tsl_char_t *p_name, tr69_node_t *p_node)
{
        tr69_info_tree_t *p_tree = NULL;

        TSL_VASSERT(p_name != NULL);
        TSL_VASSERT(p_node != NULL);


        TSL_VASSERT(tr69_inline_get_node(pg_root_tree, p_name, &p_tree) == TSL_RV_SUC);
        TSL_VASSERT(p_tree != NULL);
        TSL_VASSERT(p_tree->node_type == TR69_NODE_TYPE_LEAF);

        p_tree->info.info_leaf.notification = p_node->notification;
        p_tree->info.info_leaf.acl = p_node->acl;

        if (p_tree->info.info_leaf.notification == TR69_NODE_LEAF_UNNOTIFICATION){
            tr69_inline_add_notification(p_tree->full_name, TR69_NODE_LEAF_UNNOTIFICATION);
        }
        if (!strcmp(p_name, global_delaynotifications[NOTIF_DEVNUM_INDEX].full_name)){
            if (p_node->notification == TR69_NODE_LEAF_NOTIFICATION){
                global_delaynotifications[NOTIF_DEVNUM_INDEX].is_add = 0;
            }
        }

        return TSL_RV_SUC;
}

tsl_rv_t tr69_set_notification(tsl_char_t *p_name, tsl_int_t notification)
{
        tr69_node_t node;

        node.notification = notification;
        return tr69_set_node(p_name, &node);
}

tsl_rv_t tr69_get_node(tsl_char_t *p_name, tr69_node_t **pp_node)
{
        tr69_info_tree_t *p_tree = NULL;


        TSL_VASSERT(tr69_intf_get_node(pg_root_tree, p_name, &p_tree) == TSL_RV_SUC);
        TSL_VASSERT(p_tree != NULL);
        //Coverity CID 17253: Resource leak
        TSL_VASSERT_ACT((p_tree->node_type == TR69_NODE_TYPE_LEAF), free(p_tree);p_tree=NULL);

        *pp_node = (tr69_node_t *)calloc(1, sizeof(tr69_node_t));

        sprintf((*pp_node)->full_name, "%s", p_tree->full_name);
        (*pp_node)->node_type = p_tree->node_type;
        (*pp_node)->notification = p_tree->info.info_leaf.notification;
        (*pp_node)->acl = p_tree->info.info_leaf.acl;

        free(p_tree);
        p_tree = NULL;

        return TSL_RV_SUC;
}

tsl_rv_t tr69_get_next_node(tsl_char_t *p_root_name, tr69_node_t **pp_next_node)
{
        tsl_int_t found = 0;
        tr69_info_tree_t *p_root = NULL;


        TSL_VASSERT(p_root_name != NULL);
        TSL_VASSERT(pp_next_node != NULL);

        TSL_VASSERT(tr69_inline_get_node(pg_root_tree, p_root_name, &p_root) == TSL_RV_SUC);
        TSL_VASSERT(p_root != NULL);

        if (*pp_next_node){
                tr69_inline_get_next_node(p_root, (*pp_next_node)->full_name, &found, pp_next_node);
        }else {
                tr69_inline_get_next_node(p_root, NULL, &found, pp_next_node);
        }

        if (found == 2){
                return TSL_RV_SUC;
        }else {
                if (*pp_next_node){
                        free(*pp_next_node);
                        *pp_next_node = NULL;
                }
                return TSL_RV_ERR;
        }

        return TSL_RV_SUC;
}

tsl_rv_t tr69_get_next_node2(tsl_char_t *p_root_name, tr69_node_t **pp_next_node)
{
	tsl_int_t found = 0;
	tr69_info_tree_t *p_root = NULL;

	TSL_VASSERT(p_root_name != NULL);
	TSL_VASSERT(pp_next_node != NULL);

	TSL_VASSERT(tr69_inline_get_node(pg_root_tree, p_root_name, &p_root) == TSL_RV_SUC);
	TSL_VASSERT(p_root != NULL);

	if (*pp_next_node) {
		tr69_inline_get_next_node2(p_root, (*pp_next_node)->full_name, &found, pp_next_node);
	}else {
		tr69_inline_get_next_node2(p_root, NULL, &found, pp_next_node);
	}

	if (found == 2) {
		return TSL_RV_SUC;
	}else {
		if (*pp_next_node) {
			free(*pp_next_node);
			*pp_next_node = NULL;
		}
		return TSL_RV_ERR;
	}

	return TSL_RV_SUC;
}

tsl_rv_t tr69_get_next_child_node(tsl_char_t *p_root_name, tr69_node_t **pp_next_node)
{
	tsl_int_t found = 0;
	tr69_info_tree_t *p_root = NULL;

	TSL_VASSERT(p_root_name != NULL);
	TSL_VASSERT(pp_next_node != NULL);

	TSL_VASSERT(tr69_inline_get_node(pg_root_tree, p_root_name, &p_root) == TSL_RV_SUC);
	TSL_VASSERT(p_root != NULL);

	if (*pp_next_node){
		tr69_inline_get_next_child_node(p_root, (*pp_next_node)->full_name, &found, pp_next_node);
	}else {
		tr69_inline_get_next_child_node(p_root, NULL, &found, pp_next_node);
	}

	if (found == 2){
		return TSL_RV_SUC;
	}else {
		if (*pp_next_node){
			free(*pp_next_node);
			*pp_next_node = NULL;
		}
		return TSL_RV_ERR;
	}

	return TSL_RV_SUC;
}

tsl_rv_t tr69_get_next_obj_node(tsl_char_t *p_root_name, tr69_node_t **pp_next_node)
{
        tsl_int_t found = 0;
        tr69_info_tree_t *p_root = NULL;


        TSL_VASSERT(p_root_name != NULL);
        TSL_VASSERT(pp_next_node != NULL);

        TSL_VASSERT(tr69_inline_get_node(pg_root_tree, p_root_name, &p_root) == TSL_RV_SUC);
        TSL_VASSERT(p_root != NULL);

        if (*pp_next_node){
                tr69_inline_get_next_obj_node(p_root, (*pp_next_node)->full_name, &found, pp_next_node);
        }else {
                tr69_inline_get_next_obj_node(p_root, NULL, &found, pp_next_node);
        }

        if (found == 2){
                return TSL_RV_SUC;
        }else {
                if (*pp_next_node){
                        free(*pp_next_node);
                        *pp_next_node = NULL;
                }
                return TSL_RV_ERR;
        }

        return TSL_RV_SUC;
}

tsl_rv_t tr69_get_next_inst_node(tsl_char_t *p_root_name, tr69_node_t **pp_next_node)
{
    tsl_int_t found = 0;
    tr69_info_tree_t *p_root = NULL;


    TSL_VASSERT(p_root_name != NULL);
    TSL_VASSERT(pp_next_node != NULL);

    TSL_VASSERT(tr69_inline_get_node(pg_root_tree, p_root_name, &p_root) == TSL_RV_SUC);
    TSL_VASSERT(p_root != NULL);

    if (*pp_next_node){
            tr69_inline_get_next_inst_node(p_root, (*pp_next_node)->full_name, &found, pp_next_node);
    }else {
            tr69_inline_get_next_inst_node(p_root, NULL, &found, pp_next_node);
    }

    if (found == 2){
            return TSL_RV_SUC;
    }else {
            if (*pp_next_node){
                    free(*pp_next_node);
                    *pp_next_node = NULL;
            }
            return TSL_RV_ERR;
    }

    return TSL_RV_SUC;
}

tsl_rv_t tr69_get_next_samedepth_node(tsl_char_t *p_root_name, tr69_node_t **pp_next_node)
{
	tsl_int_t found = 0;
	tr69_info_tree_t *p_root = NULL;
	tsl_char_t root_name[TR69_OID_FULL_NAME_SIZE] = "\0";
    tsl_char_t *p_root_pos = NULL;

	TSL_VASSERT(p_root_name != NULL);
	TSL_VASSERT(pp_next_node != NULL);
//Coverity comment 18314
    if ((p_root_pos = strstr(p_root_name, "%d")) != NULL){
        snprintf(root_name,p_root_pos - p_root_name,"%s", p_root_name);
	}else {
		snprintf(root_name,sizeof(root_name),"%s", p_root_name);
    }

	TSL_FVASSERT(tr69_inline_get_node(pg_root_tree, root_name, &p_root) == TSL_RV_SUC);
	TSL_VASSERT(p_root != NULL);

	if (*pp_next_node){
		tr69_inline_get_next_samedepth_node(p_root, (*pp_next_node)->full_name, &found, pp_next_node, p_root_name);
	}else {
		tr69_inline_get_next_samedepth_node(p_root, NULL, &found, pp_next_node, p_root_name);
	}

	if (found == 2){
		return TSL_RV_SUC;
	}else {
		if (*pp_next_node){
			free(*pp_next_node);
			*pp_next_node = NULL;
		}
		return TSL_RV_ERR;
	}

	return TSL_RV_SUC;
}

tsl_rv_t tr69_clear_notification()
{
        return tr69_inline_clear_notification();
}
tsl_rv_t tr69_add_instance(tsl_char_t *p_name, tsl_int_t *p_inst_numb)
{
        return tr69_intf_add_instance(pg_root_tree, p_name, p_inst_numb);
}

tsl_rv_t tr69_del_instance(tsl_char_t *p_name, tsl_int_t inst)
{
        return tr69_intf_del_instance(pg_root_tree, p_name, inst);
}

tsl_rv_t tr69_restore_default_node(tsl_char_t *p_name)
{
        return  tr69_intf_restore_default_node(pg_root_tree, p_name);
}

tsl_rv_t tr69_del2_instance(tsl_char_t *p_name)
{
        return tr69_intf2_del_instance(pg_root_tree, p_name);
}

tsl_rv_t tr69_get_notification(tr69_notification_t **pp_notf_tb, tsl_int_t *p_tb_count)
{
        return tr69_inline_get_notification(pp_notf_tb, p_tb_count);
}

tsl_rv_t tr69_add_instance_timeout(tsl_char_t *p_name, tsl_int_t timeout)
{
        return tr69_inline_add_instance_timeout(p_name, timeout);
}

tsl_rv_t tr69_move_instance(tsl_char_t *p_name_dstobj, tsl_char_t *p_name_srcinst, tsl_int_t *p_inst_numb)
{
    return tr69_intf_move_instance(pg_root_tree, p_name_dstobj, p_name_srcinst, p_inst_numb);
}

