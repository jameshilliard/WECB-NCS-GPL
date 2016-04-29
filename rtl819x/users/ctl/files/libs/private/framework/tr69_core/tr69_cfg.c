#include "ctl.h"
#include "tsl_common.h"
#include "tr69_tree.h"
#include "tsl_msg.h"
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <ctype.h>
#include "ctl_log.h"
#include "tsl_strconv.h"

typedef struct tr69_inst_list{
        tsl_char_t full_name[TR69_OID_FULL_NAME_SIZE];
        tsl_list_head_t list_node;
}tr69_inst_list_t;

tr69_inst_list_t *p_def_inst_list = NULL;
tr69_inst_list_t *p_pre_inst_list = NULL;

extern struct tr69_inst_numb_of_entries_s tr69_inst_numb_tb[];
extern struct tr69_register_func tr69_regfunc_tb[];
extern tsl_char_t st_version[];
tsl_int_t tr69_inst_numb_tb_count = 0;
tsl_int_t tr69_regfunc_tb_count = 0;

tr69_info_tree_t *pg_root_prev_tree   = NULL;
tr69_info_tree_t *pg_root_tree   = NULL;
tr69_info_tree_t *pg_ghost_tree  = NULL;
#ifdef AEI_PARAM_FORCEUPDATE
tr69_info_tree_t *pg_prev_ver_tree  = NULL;
#endif
tsl_list_head_t global_notification_list_head = {&global_notification_list_head, &global_notification_list_head};

tsl_list_head_t global_timeout_instance_list_head = {&global_timeout_instance_list_head, &global_timeout_instance_list_head};

tsl_void_t *tr69_inline_dump_data(tr69_info_tree_t *p_tree);
tsl_rv_t tr69_inline_free_data(tr69_info_tree_t *p_tree, tsl_void_t **pp_data);
tsl_rv_t tr69_inline_get_node(tr69_info_tree_t *p_root, tsl_char_t *p_name, tr69_info_tree_t **pp_find_node);
tsl_rv_t tr69_inline_add_notification(tsl_char_t *p_oid_name, tsl_int_t active);
tsl_rv_t tr69_get_leaf_data(tsl_char_t *p_name, tsl_void_t **pp_data, tsl_int_t *type);
tsl_rv_t tr69_get_unfresh_leaf_data(tsl_char_t *p_name, tsl_void_t **pp_data, tsl_int_t *type);
tsl_rv_t tr69_set_leaf_data(tsl_char_t *p_name, tsl_void_t *p_value, tsl_int_t type);
tsl_rv_t tr69_del2_instance(tsl_char_t *p_name);
tsl_rv_t tr69_set_unfresh_leaf_data(tsl_char_t *p_name, tsl_void_t *p_value, tsl_int_t type);
xmlNode *tr69_func_inline_tree_to_xml(tr69_info_tree_t *p_root, xmlNode *p_parent_node);
tsl_rv_t tr69_intf_add_instance2(tr69_info_tree_t *p_root, tsl_char_t *p_name);
tsl_rv_t tr69_intf2_del_instance(tr69_info_tree_t *p_root, tsl_char_t *p_name);

tsl_rv_t create_delta_inst_list();

#define XML_VER_NUM			128
#define MAX_TOKEN_COUNT		10
#define MAX_TOKEN_LENGTH		64

tsl_char_t protype_xml_version[XML_VER_NUM] = "\0";
tsl_char_t cfg_xml_version[XML_VER_NUM] = "\0";
tsl_char_t pre_cfg_xml_version[XML_VER_NUM] = "\0";

tsl_bool_t Is_new_inst_by_update(tsl_char_t* full_name)
{
	tsl_bool_t found = TSL_B_FALSE;
	tsl_list_head_t *p_def_list  =NULL;
	tr69_inst_list_t *p_def_inst = NULL;	
	if( p_def_inst_list )
	{
		for(p_def_list = p_def_inst_list->list_node.prev; p_def_list != &p_def_inst_list->list_node;){
			p_def_inst = tsl_list_entry(p_def_list, tr69_inst_list_t, list_node);
			p_def_list = p_def_list->prev;
			ctllog_debug("%s: ===def_inst[%s][%d] full_name[%s][%d]\n",__func__,p_def_inst->full_name,strlen(p_def_inst->full_name),full_name,strlen(full_name));
			if( strcmp(p_def_inst->full_name,full_name) == 0 )	
			{
				found = TSL_B_TRUE;
				break;
			}
		}	
	}
	return found;
}

tsl_rv_t Clear_Inst_list(tr69_inst_list_t**p_root)
{
	if( *p_root )
	{
		tsl_list_head_t *p_list  =NULL;
		tr69_inst_list_t *p_inst = NULL;

		for(p_list =(*p_root)->list_node.prev; p_list != &(*p_root)->list_node;){
			p_inst = tsl_list_entry(p_list, tr69_inst_list_t, list_node);
			p_list = p_list->prev;
			tsl_list_del(&p_inst->list_node);
			free(p_inst);
			p_inst =  NULL;
		}	

		tsl_list_del(&(*p_root)->list_node);
		free(*p_root);
		*p_root = NULL;
	}
	return TSL_RV_SUC;
}

tsl_rv_t Init_Inst_list( tsl_char_t* filepath,tr69_inst_list_t**p_root)
{
	FILE * pfile = NULL;
	tsl_rv_t	ret = TSL_RV_ERR;
	tsl_char_t buf[TR69_OID_FULL_NAME_SIZE] = {0};
	tr69_inst_list_t *p_inst = NULL;

	if (*p_root == NULL)
	{
		*p_root = (tr69_inst_list_t *)calloc(1, sizeof(tr69_inst_list_t));
	}
	TSL_INIT_LIST_HEAD(&(*p_root)->list_node);
	
	pfile = fopen( filepath, "r");
	if( NULL == pfile ) {
		ctllog_debug("fail to open file %s", filepath );
		return ret;
	}

	while(fgets(buf, TR69_OID_FULL_NAME_SIZE, pfile))
	{
		p_inst = (tr69_inst_list_t *)calloc(1, sizeof(tr69_inst_list_t));
		buf[strlen(buf)-1] = '\0';
		snprintf(p_inst->full_name,TR69_OID_FULL_NAME_SIZE-1,"%s",buf);
		TSL_INIT_LIST_HEAD(&p_inst->list_node);

		tsl_list_add(&p_inst->list_node, &(*p_root)->list_node);
	}

	ctllog_debug("%s:load instoid.lst [%s] \n",__func__,filepath);

	fclose(pfile);
	pfile  = NULL;
	
	return TSL_RV_SUC;
}

tsl_rv_t create_delta_inst_list()
{
	//check pre_inst list
	tsl_int_t ret = access(PRE_INST_LIST_FILE, R_OK);

    if( 0 != ret) {
	    tsl_char_t cmd[128]={"0"};

        ctllog_warn("%s:there is no instoid.lst file in rt_conf\n",__func__);
        // Create an empty/dummy instoid.lst for old version WECB
        snprintf( cmd, sizeof(cmd), "echo > %s", PRE_INST_LIST_FILE );
        system( cmd );
        system("sync");
        ret = access(PRE_INST_LIST_FILE, R_OK);
    }
	if( 0 == ret )
	{
		//load def instance list
		ret = Init_Inst_list(DEF_INST_LIST_FILE,&p_def_inst_list);
		if( ret == TSL_RV_ERR )
		{
			ctllog_debug("%s:fail to load [%s]!!!\n",__func__,DEF_INST_LIST_FILE);
			return TSL_RV_ERR;
		}

		//load pre instance list
		ret = Init_Inst_list(PRE_INST_LIST_FILE,&p_pre_inst_list);
		if( ret == TSL_RV_ERR )
		{
			ctllog_debug("%s:fail to load [%s]!!!\n",__func__,PRE_INST_LIST_FILE);
			return TSL_RV_ERR;
		}

		ctllog_debug("%s:load def instance list and pre instance list\n",__func__);

		//create delta instance list
		tsl_list_head_t *p_def_list  =NULL,*p_pre_list = NULL;
		tr69_inst_list_t *p_def_inst = NULL,*p_pre_inst = NULL;
			
		for(p_pre_list = p_pre_inst_list->list_node.prev; p_pre_list != &p_pre_inst_list->list_node;){
			
			p_pre_inst = tsl_list_entry(p_pre_list, tr69_inst_list_t, list_node);

			for(p_def_list = p_def_inst_list->list_node.prev; p_def_list != &p_def_inst_list->list_node;){
				p_def_inst = tsl_list_entry(p_def_list, tr69_inst_list_t, list_node);
				p_def_list = p_def_list->prev;
				if( strcmp(p_def_inst->full_name,p_pre_inst->full_name) == 0 )	
				{
					//ctllog_debug("%s:del[%s]\n",__func__,p_def_inst->full_name);
					tsl_list_del(&p_def_inst->list_node);
					free(p_def_inst);
					p_def_inst =  NULL;
					break;
				}
			}
			p_pre_list = p_pre_list->prev;
		}
		
	}
	else
	{
		ctllog_warn("%s:there is no instoid.lst file in rt_conf\n",__func__);
	}

	return TSL_RV_SUC;
}

//Version number format : 
//company-productname-main version-sub number-internal number
//FTH-BHRK2-10-10-01A
int split(char** token_2D_array, char* message,const char* seperators)
{
	int ret = 0;
	int count = 0;
	char *token;
	char dupmessage[XML_VER_NUM]={0};
	char *ptr = NULL;

	if(token_2D_array == 0 || message == 0 || seperators == 0)
		return 0;

	strncpy(dupmessage, message, sizeof(dupmessage)-1);

	token = strtok_r( dupmessage, seperators, &ptr );
	while( token != 0 )
	{
		if(count == MAX_TOKEN_COUNT )
		{/* While there are more tokens in message*/
		}		
		else
		{
			strncpy(token_2D_array[count], token,MAX_TOKEN_LENGTH-1 );
			count ++;
		}
		ret ++;
		/* Get next token: */
		token = strtok_r( NULL, seperators, &ptr );
	}

	return ret;    
}

/*
Check current version number of cfg.xml and default version number
return
TSL_RV_FAIL					:invalid cfg.xml, must use default_cfg.xml
TSL_RV_SUC					:not the same files, should be update cfg.xml based default_cfg.xml
TSL_RV_FAIL_FUNC|TSL_RV_FAIL_NONE	:same files, do nothing
*/
tsl_rv_t checkver(tsl_bool_t cached)
{
	tsl_rv_t ret = TSL_RV_FAIL;

	if( strcmp(cfg_xml_version,pre_cfg_xml_version) == 0 )		
	{
		//Internal version, need merge cfg file every time
		if( strcmp(CFG_VER_FOR_RD,cfg_xml_version) == 0 )
		{
			ret = TSL_RV_SUC;
			ctllog_debug("%s: Internal version, need merge cfg.xml", __func__);
		}
		//same version, do nothing
		else
		{	
			//should use cached file
			if( cached )
			{
				ret = TSL_RV_FAIL_NONE;
			}
			//use last 
			else
			{
				ret = TSL_RV_FAIL_FUNC;
				ctllog_debug("%s: default ver[%s] current ver[%s],they are same cfg.xml", 
					__func__,cfg_xml_version,pre_cfg_xml_version);
			}
		}
	}
	else
	{
		//split 
		char tokens_default[MAX_TOKEN_COUNT][MAX_TOKEN_LENGTH],tokens[MAX_TOKEN_COUNT][MAX_TOKEN_LENGTH];
		char *token_ptr_default[MAX_TOKEN_COUNT],*token_ptr[MAX_TOKEN_COUNT];
		int token_num_default,token_num;
			
		memset(&tokens_default[0][0], 0 , MAX_TOKEN_COUNT * MAX_TOKEN_LENGTH);
		int k;

                /*If pre ver is NULL, also do the merge operation*/
                if (pre_cfg_xml_version == NULL ||strlen(pre_cfg_xml_version) == 0)
                {
                    ctllog_error("pre_cfg_xml_version == NULL\n");
                    return TSL_RV_SUC;
                }

		for( k = 0; k < MAX_TOKEN_COUNT; k ++)
		{
			token_ptr_default[k] = &tokens_default[k][0];
		}

		token_num_default = split(token_ptr_default, cfg_xml_version, "-");
/*
		if( token_num_default != 5)
		{
			ctllog_debug("%s:default ver[%s] invalid!!!", __func__,cfg_xml_version);
			return ret;
		}
*/
		memset(&tokens[0][0], 0 , MAX_TOKEN_COUNT * MAX_TOKEN_LENGTH);
		int i;
		for( i = 0; i < MAX_TOKEN_COUNT; i ++)
		{
			token_ptr[i] = &tokens[i][0];
		}

		token_num = split(token_ptr,pre_cfg_xml_version, "-");
/*
		if( token_num != 5)
		{
			ctllog_debug("%s:curent ver[%s] invalid!!! ", __func__,pre_cfg_xml_version);
			return ret;
		}
*/
		//check company and product number first
		if(  strcmp(tokens_default[0],tokens[0]) == 0  &&  strcmp(tokens_default[1],tokens[1]) == 0 )	
		{
			ret = TSL_RV_SUC;
			ctllog_error("%s: default ver[%s] current ver[%s]. they are not same. update it!!!", 
				__func__,cfg_xml_version,pre_cfg_xml_version);
		}
	}

	return ret;
}

tr69_info_tree_t *tr69_ghost_func_add_child_node(xmlNode *p_node, tr69_info_tree_t *p_tree_head, tsl_char_t *p_name)
{
        tr69_info_tree_t *p_tree  = NULL;
        tsl_char_t        type    = 0;
        xmlChar          *p_char  = NULL;
        xmlNode          *p_child = NULL;
		tsl_int_t        acl      = 0;
		tsl_int_t        nosave      = 0;
        static tsl_int_t sg_offset = 0;
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

            sg_offset = 0;

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

            p_char = xmlGetProp(p_node, (xmlChar *)"hideParameterFromAcs");
            if (p_char){
                    if (!strcmp((tsl_char_t *)p_char, "true")){
                            p_tree->info.info_leaf.attr = TR69_NODE_LEAF_ATTR_HIDDEN;
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
	    p_child = p_node->children;
	    while(p_child != NULL){
		if (p_child->type == XML_ELEMENT_NODE){
		    tr69_ghost_func_add_child_node(p_child, p_tree, p_tree->full_name);
		}
		p_child = p_child->next;
	    }
        }
        
        return p_tree;
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

#ifdef AEI_PARAM_FORCEUPDATE
                p_tree->info.info_leaf.forceupdate = 0;
                p_char = xmlGetProp(p_node, (xmlChar *)"forceupdate");
                if (p_char != NULL){
                    tsl_printf("found forceupdate, node name[%s]\n", p_node->name);
                    p_tree->info.info_leaf.forceupdate = atoi((char *)p_char);
                    xmlFree(p_char);
                }
#endif

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
                // ctllog_debug("%s %d %d\n", __FUNCTION__, __LINE__,recursion_count1++);
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

#ifdef AEI_PARAM_FORCEUPDATE
tsl_rv_t tr69_intf_get_unfresh_leaf_data(tr69_info_tree_t *p_root, tsl_char_t *p_name, tsl_void_t **pp_find_data, tsl_int_t *type);

static tsl_bool_t is_acs_keyparam( tsl_char_t * node_name )
{
    tsl_bool_t keyparam = TSL_B_FALSE;

    do {
        ctllog_error( "node_name = %s\n", node_name );
#ifdef AEI_WECB
        if( ! strcmp(node_name, "Device.ManagementServer.URL" ) ) {
            keyparam = TSL_B_TRUE;
        }
#endif
    } while(0) ;
    return keyparam;
}


static int need_load_client_value(tr69_info_tree_t * p_prev_ver_tree, tr69_info_tree_t *p_root)
{
    tsl_char_t *p_prev_def_val = NULL;
    tsl_char_t *p_cur_def_val = NULL;
    int type1, type2;
    int ret = 1, ret1 = 1, ret2 = 1;
    tr69_info_tree_t *p_tree = NULL;
    int forceupdate = 0;
    tsl_char_t *node_name = NULL;
    static tsl_bool_t b_acs_reset = TSL_B_FALSE;

    if (!p_prev_ver_tree || !p_root) {
    	goto CLEANUP;
    }

    node_name = p_root->full_name;

    tr69_inline_get_node(pg_root_tree, node_name, &p_tree);
    if (p_tree) {
        if (p_tree->node_type == TR69_NODE_TYPE_LEAF) {
            forceupdate = p_tree->info.info_leaf.forceupdate;
            if (forceupdate) {
            	ctllog_error("found forceupdate %d, param name: %s\n", forceupdate, p_root->full_name);
            }
        }
    }

    if ((ret1 = tr69_intf_get_unfresh_leaf_data(p_prev_ver_tree, node_name, (tsl_void_t**)&p_prev_def_val, &type1)) != 0) {
        ret = 0;
        goto CLEANUP;
    }

    if ((ret2 = tr69_intf_get_unfresh_leaf_data(pg_root_tree, node_name, (tsl_void_t**)&p_cur_def_val, &type2)) != 0) {
        ret = 0;
        goto CLEANUP;
    }

    if (type1 != type2) {
        ret = 0;
        goto CLEANUP;
    }


    if (type1 == TR69_NODE_LEAF_TYPE_STRING || type1 == TR69_NODE_LEAF_TYPE_DATE) {
        if (p_prev_def_val && p_cur_def_val) { 
            if ((b_acs_reset && (2==forceupdate)) || strcmp(p_prev_def_val, p_cur_def_val)) {
                if (forceupdate) {
                    ret = 0;
                    goto CLEANUP;
                }
                else {
                    if (p_root->p_parent->node_type == TR69_NODE_TYPE_OBJECT){
                        if (tsl_strcmp(p_prev_def_val, (*((char **)((char *)(p_root->p_parent->info.info_obj.p_data)+
                                                p_root->info.info_leaf.data_offset))))) {
                            ret = 1;
                            goto CLEANUP;
                        }
                        else {
                            ret = 0;
                            goto CLEANUP;
                        }
                    }
                    else { /* must be an instance, leaf's parent can't be a leaf */
                        if (tsl_strcmp(p_prev_def_val, (*((char **)((char *)(p_root->p_parent->info.info_inst.p_data)+
                                                p_root->info.info_leaf.data_offset))))) {
                            ret = 1;
                            goto CLEANUP;
                        }
                        else {
                            ret = 0;
                            goto CLEANUP;
                        }
                    }
                }
            }
            else {
                ret = 1;
                goto CLEANUP;
            }
        }
        else {
            if (p_prev_def_val == p_cur_def_val) {
                ret = 1;
                goto CLEANUP;
            }
            else {
                if (forceupdate) {
                    ret = 0;
                    goto CLEANUP;
                }
                else {
                    if (p_prev_def_val) {
                        if (p_root->p_parent->node_type == TR69_NODE_TYPE_OBJECT){
                            if (tsl_strcmp(p_prev_def_val, (*((char **)((char *)(p_root->p_parent->info.info_obj.p_data)+
                                                p_root->info.info_leaf.data_offset))))) {
                                ret = 1;
                                goto CLEANUP;
                            }
                            else {
                                ret = 0;
                                goto CLEANUP;
                            }
                        }
                        else { /* must be an instance */
                            if (tsl_strcmp(p_prev_def_val, (*((char **)((char *)(p_root->p_parent->info.info_inst.p_data)+
                                                p_root->info.info_leaf.data_offset))))) {
                                ret = 1;
                                goto CLEANUP;
                            }
                            else {
                                ret = 0;
                                goto CLEANUP;
                            }
                        }
                    }
                    else {
                        if (p_root->p_parent->node_type == TR69_NODE_TYPE_OBJECT){
                            if (tsl_strlen((*((char **)((char *)(p_root->p_parent->info.info_obj.p_data)+
                                                p_root->info.info_leaf.data_offset))))) {
                                ret = 1;
                                goto CLEANUP;                        
                            }
                            else {
                                ret = 0;
                                goto CLEANUP;

                            }
                        }
                        else { /* must be an instance */
                            if (tsl_strlen((*((char **)((char *)(p_root->p_parent->info.info_inst.p_data)+
                                                p_root->info.info_leaf.data_offset))))) {
                                ret = 1;
                                goto CLEANUP;                        
                            }
                            else {
                                ret = 0;
                                goto CLEANUP;

                            }
                        }
                    }
                }

            }
        }
    }
    else {
        if ( ((!b_acs_reset) || (2!=forceupdate)) && (p_prev_def_val == p_cur_def_val)) {
            ret = 1;
            goto CLEANUP;
        }
        else {
            if (forceupdate) {
                ret = 0;
                goto CLEANUP;
            }
            else {
                if (p_root->p_parent->node_type == TR69_NODE_TYPE_OBJECT){
                    if (((int*)p_prev_def_val) == ((int *)((char *)(p_root->p_parent->info.info_obj.p_data)+
                                    p_root->info.info_leaf.data_offset))) {
                        ret = 1;
                        goto CLEANUP;
                    }
                    else {
                        ret = 0;
                        goto CLEANUP;
                    }
                }
                else { /* must be an instance */
                    if (((int*)p_prev_def_val) == ((int *)((char *)(p_root->p_parent->info.info_inst.p_data)+
                                    p_root->info.info_leaf.data_offset))) {
                        ret = 1;
                        goto CLEANUP;
                    }
                    else {
                        ret = 0;
                        goto CLEANUP;
                    }
                }
            }
        }
    }

CLEANUP:

    if (ret1 == 0 && (type1 == TR69_NODE_LEAF_TYPE_STRING || type1 == TR69_NODE_LEAF_TYPE_DATE))
        free(p_prev_def_val);
    if (ret2 == 0 && (type2 == TR69_NODE_LEAF_TYPE_STRING || type2 == TR69_NODE_LEAF_TYPE_DATE))
        free(p_cur_def_val);

    // Support ACS server info change smoothly
    // When ACS URL force updated, PeriodicInformInterval should also reset to default value
    if( (0 == ret)  && (!b_acs_reset) ) {
        b_acs_reset = is_acs_keyparam( node_name );
        if( b_acs_reset ) {
            ctllog_error("acs_reset\n" );
        }
    }

    return ret;
}
#endif

tsl_rv_t tr69_travel_set_tree(tr69_info_tree_t *p_root)
{
        tsl_list_head_t     *p_list = NULL;
        tr69_info_tree_t    *p_tree = NULL;
        tsl_char_t          tmp_name[TR69_OID_FULL_NAME_SIZE] = "\0";

	 //ctllog_debug("%s %d %d\n", __FUNCTION__, __LINE__,recursion_count++);

        if (p_root->node_type == TR69_NODE_TYPE_OBJECT ){
	    //ctllog_debug("OBJECT %s %s\n", p_root->info.info_obj.name, p_root->full_name);
	    
        }else if (p_root->node_type == TR69_NODE_TYPE_INSTANCE){
	   // ctllog_debug("INSTANCE %d %s\n", p_root->info.info_inst.size, p_root->full_name);

		tr69_info_tree_t *p_tree1 = NULL;
		 tr69_inline_get_node(pg_root_tree, p_root->full_name, &p_tree1);
		 if (p_tree1 == NULL){
                    memset(tmp_name, 0, sizeof(tmp_name));
                    snprintf(tmp_name, sizeof(tmp_name), p_root->p_parent->full_name);
                    tr69_intf_add_instance2(pg_root_tree, tmp_name);
		 }

	}else if (p_root->node_type == TR69_NODE_TYPE_LEAF){
               // ctllog_debug("PARAMETER %s %s\n", p_root->info.info_leaf.name, p_root->full_name);

                if (strstr(p_root->full_name, "NumberOfEntries") == NULL){
#ifdef AEI_PARAM_FORCEUPDATE                   
                    if (need_load_client_value(pg_prev_ver_tree, p_root)) {
#endif
                        if (p_root->p_parent->node_type == TR69_NODE_TYPE_OBJECT){
                                                
                                if (p_root->info.info_leaf.type == TR69_NODE_LEAF_TYPE_STRING||
                                    p_root->info.info_leaf.type == TR69_NODE_LEAF_TYPE_DATE){
                                        
                                        tr69_set_unfresh_leaf_data(p_root->full_name, 
                                                                   (*((char **)((char *)(p_root->p_parent->info.info_obj.p_data)+ 
                                                                                p_root->info.info_leaf.data_offset))),
                                                                   TR69_NODE_LEAF_TYPE_STRING);                      
                                }else {
                                        tr69_set_unfresh_leaf_data(p_root->full_name, 
                                                                   ((int *)((char *)(p_root->p_parent->info.info_obj.p_data)+ 
                                                                            p_root->info.info_leaf.data_offset)),
                                                                   TR69_NODE_LEAF_TYPE_INT);
                                }
                                
                        }else if (p_root->p_parent->node_type == TR69_NODE_TYPE_INSTANCE){
                                
                                if (p_root->info.info_leaf.type == TR69_NODE_LEAF_TYPE_STRING||
                                    p_root->info.info_leaf.type == TR69_NODE_LEAF_TYPE_DATE){
                                        tr69_set_unfresh_leaf_data(p_root->full_name,
                                                                   *((char **)((char *)(p_root->p_parent->info.info_inst.p_data)+ 
                                                                               p_root->info.info_leaf.data_offset)),
                                                                   TR69_NODE_LEAF_TYPE_STRING);
                                }else {
                                        tr69_set_unfresh_leaf_data(p_root->full_name, 
                                                                   ((int *)((char *)(p_root->p_parent->info.info_inst.p_data)+ 
                                                                            p_root->info.info_leaf.data_offset)),
                                                                   TR69_NODE_LEAF_TYPE_INT);
                                }
                        }
#ifdef AEI_PARAM_FORCEUPDATE
                    }
#endif
                }
        }

        for(p_list = p_root->list_head.prev; p_list != &p_root->list_head;){
	    p_tree = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
	    tr69_travel_set_tree(p_tree);
	    p_list = p_list->prev;
        }
        return TSL_RV_SUC;
}

//Remove redundant instance 
tsl_rv_t tr69_travel_filter_tree(tr69_info_tree_t *p_root)
{
        tsl_list_head_t     *p_list = NULL;
        tr69_info_tree_t    *p_tree = NULL;

        if (p_root->node_type == TR69_NODE_TYPE_OBJECT ){
	    //ctllog_debug("OBJECT %s %s\n", p_root->info.info_obj.name, p_root->full_name);
	    
        }else if (p_root->node_type == TR69_NODE_TYPE_INSTANCE){
	   // ctllog_debug("INSTANCE %d %s\n", p_root->info.info_inst.size, p_root->full_name);

		tr69_info_tree_t *p_tree1 = NULL;
		 tr69_inline_get_node(pg_root_prev_tree, p_root->full_name, &p_tree1);
		 //current instance is not in user's cfg.xml. check it
		 if (p_tree1 == NULL){
	 		ctllog_debug("%s: instance[%d] fullname[%s]\n", __func__,p_root->info.info_inst.size, p_root->full_name);

			//current instance is not supplemented by update
			//it was deleted by user. delete that instance
			if( Is_new_inst_by_update(p_root->full_name) == TSL_B_FALSE )
				tr69_intf2_del_instance(pg_root_tree, p_root->full_name);
			
		       return TSL_RV_SUC;			
		 }
	}

        for(p_list = p_root->list_head.prev; p_list != &p_root->list_head;){
	    	p_tree = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
		p_list = p_list->prev;
		tr69_travel_filter_tree(p_tree);	    
        }
        return TSL_RV_SUC;
}


tr69_info_tree_t *tr69_ghost_func_parse_xml(tsl_char_t *p_file_name)
{
        xmlNode          *root_node = NULL;
        xmlDoc           *doc = NULL;
	tr69_info_tree_t *p_root = NULL;
	
        doc = xmlReadFile(p_file_name, NULL, 0);
        TSL_VASSERT_RV(doc != NULL, NULL);
        root_node = xmlDocGetRootElement(doc);
	
	p_root = tr69_ghost_func_add_child_node(root_node, NULL, NULL);
        
        xmlFreeDoc(doc);
        //xmlCleanupParser();
        
	return p_root;
}


tr69_info_tree_t *tr69_func_parse_xml(tsl_char_t *p_file_name)
{
        xmlNode          *root_node = NULL;
        xmlDoc           *doc = NULL;
	tr69_info_tree_t *p_root = NULL;
	
        doc = xmlReadFile(p_file_name, NULL, 0);
        TSL_VASSERT_RV(doc != NULL, NULL);
        root_node = xmlDocGetRootElement(doc);
	
#ifdef TR69_SUPPORT_HIDE_OBJ_FROM_ACS
	p_root = tr69_func_add_child_node(root_node, NULL, NULL, 1,TSL_B_FALSE);
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
                    xmlNodeSetContent(p_node, (xmlChar *)p_root->info.info_leaf.def_value);
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

                                                if (*pp_dst && pp_src && *pp_src){
                                                        if (strcmp(*pp_dst, *pp_src)){
                                                                free(*pp_dst);
                                                                *pp_dst = NULL;
                                                                
                                                                *pp_dst = strdup(*pp_src);
                                                        }
                                                }else {
                                                        if (*pp_dst){
                                                                free(*pp_dst);
                                                                *pp_dst = NULL;
                                                        }
                                                        
                                                        if (pp_src && *pp_src ){
                                                                *pp_dst = strdup(*pp_src);
                                                        }
                                                }
                                        }else {
                                                p_idst = p_isrc = NULL;
                                                
                                                p_idst = ((int *)((char *)(p_tree->info.info_obj.p_data)+ p_child->info.info_leaf.data_offset));
                                                p_isrc = ((int *)((char *)(p_data)+ p_child->info.info_leaf.data_offset));

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

                                                
                                                if (*pp_dst && pp_src && *pp_src){
                                                        if (strcmp(*pp_dst, *pp_src)){
                                                                free(*pp_dst);
                                                                *pp_dst = NULL;
                                                                
                                                                *pp_dst = strdup(*pp_src);
                                                        }
                                                }else {
                                                        if (*pp_dst){
                                                                free(*pp_dst);
                                                                *pp_dst = NULL;
                                                        }
                                                        if (pp_src && *pp_src ){
                                                                *pp_dst = strdup(*pp_src);
                                                        }
                                                }
                                                
                                        }else {
                                                p_idst = p_isrc = NULL;
                                                
                                                p_idst = ((int *)((char *)(p_tree->info.info_inst.p_data)+ p_child->info.info_leaf.data_offset));
                                                p_isrc = ((int *)((char *)(p_data)+ p_child->info.info_leaf.data_offset));

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
	  			    //ctllog_debug("****** %d %s\n",p_child->node_type,p_child->full_name);								
                                if (p_child->node_type == TR69_NODE_TYPE_LEAF){
						//if(strcmp(p_tree->full_name,"InternetGatewayDevice.Device") == 0)
						//	ctllog_debug("@@@@@@  %x\n",p_child->info.info_leaf.type);
                                        if (p_child->info.info_leaf.type == TR69_NODE_LEAF_TYPE_STRING||
                                            p_child->info.info_leaf.type == TR69_NODE_LEAF_TYPE_DATE){
                                                pp_dst = ((char **)((char *)(*pp_data)+ p_child->info.info_leaf.data_offset));


                                                if (*pp_dst){
                                                        free(*pp_dst);
                                                }
                                                *pp_dst = NULL;
                                        }
								
                                }
				    //ctllog_debug("&&&&&& %d %s\n",p_child->node_type,p_child->full_name);						
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

	//if(strcmp(p_tree->full_name,"InternetGatewayDevice.Device") == 0)
	//	ctllog_debug("####### %d %s\n",p_tree->node_type,p_tree->full_name);
        
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
                    p_tree->info.info_inst.size == 0){
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
        tsl_void_t       *p_cur_data = NULL;

        if (p_root->node_type == TR69_NODE_TYPE_OBJECT ){
                if (p_root->info.info_obj.data_size != 0 && p_root->info.info_obj.set_func != NULL){
                        p_cur_data = tr69_inline_dump_data(p_root);
                        p_root->info.info_obj.set_func(p_root->full_name, p_cur_data, NULL);
                        tr69_inline_free_data(p_root, &p_cur_data);
                }
                tr69_inline_free_data(p_root, &p_root->info.info_obj.p_data);
        }else if (p_root->node_type == TR69_NODE_TYPE_INSTANCE){
                if (p_root->p_parent->info.info_obj.set_func != NULL){
                        p_cur_data = tr69_inline_dump_data(p_root);
                        p_root->p_parent->info.info_obj.set_func(p_root->full_name, p_cur_data, NULL);
                        tr69_inline_free_data(p_root, &p_cur_data);
                }
                tr69_inline_free_data(p_root, &p_root->info.info_inst.p_data);
        }else if (p_root->node_type == TR69_NODE_TYPE_LEAF){
        }
        
        for(p_list = p_root->list_head.next; p_list != &p_root->list_head;){
                p_tree = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
                p_list = p_list->next;
                tr69_inline_del_tree(p_tree);
        }
        
        free(p_root);
        
        return TSL_RV_SUC;
}

tsl_rv_t tr69_inline_del_tree_filter(tr69_info_tree_t *p_root)
{
        tsl_list_head_t  *p_list = NULL;
        tr69_info_tree_t *p_tree = NULL;

        if (p_root->node_type == TR69_NODE_TYPE_OBJECT ){
                tr69_inline_free_data(p_root, &p_root->info.info_obj.p_data);
        }else if (p_root->node_type == TR69_NODE_TYPE_INSTANCE){
                tr69_inline_free_data(p_root, &p_root->info.info_inst.p_data);
        }else if (p_root->node_type == TR69_NODE_TYPE_LEAF){
        }
        
        for(p_list = p_root->list_head.next; p_list != &p_root->list_head;){
                p_tree = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
                p_list = p_list->next;
                tr69_inline_del_tree_filter(p_tree);
        }
        
        free(p_root);
        
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

	   if( p_tree )
	   	{
	            free(p_tree);
			p_tree = NULL;
	   	}
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
                        strcat(tmp, "0");
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
                if (atoi(oid_stack[i]) == 0){
                        strcat(tmp, oid_stack[i]);
                }else {
                        strcat(tmp, "i");
                }
                if ( i != j -1 ){
                        strcat(tmp, ".");
                }
        }
        
        sprintf(p_new_name, "%s", tmp);
        
        printf("%s %d %s %s\n", __FUNCTION__, __LINE__, p_name, tmp);

        return TSL_RV_SUC;
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

tsl_rv_t tr69_intf_add_instance2(tr69_info_tree_t *p_root, tsl_char_t *p_name)
{
        tr69_info_tree_t    *p_tree = NULL;
        tr69_info_tree_t    *p_inst = NULL;
        tr69_info_tree_t    *p_new_inst = NULL;
        tsl_char_t          tmp[TR69_OID_FULL_NAME_SIZE] = "\0";
        tsl_char_t          new_name[TR69_OID_FULL_NAME_SIZE] = "\0";
        

        TSL_VASSERT(p_root != NULL);
        TSL_VASSERT(p_name != NULL);
        
        tr69_inline_get_node(p_root, p_name, &p_tree);
        TSL_VASSERT(p_tree != NULL);
        
        tr69_inline_parse_oid(p_name, new_name); 
        sprintf(tmp, "%s.0", new_name); 
        
        tr69_inline_get_node(pg_ghost_tree, tmp, &p_inst);
        TSL_VASSERT(p_inst != NULL);
        
        p_new_inst = (tr69_info_tree_t *)calloc(1, sizeof(tr69_info_tree_t));
        
        tr69_inline_clone_obj(p_inst, p_new_inst);
        
        p_new_inst->p_parent = p_tree;
        
        tr69_inline_adjust_instance(p_tree, p_new_inst, p_name);
        tr69_intf_mod_instance(p_name, 1);
        
//        ctllog_debug("add instance %d\n", p_new_inst->info.info_inst.size);
        
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
        sprintf(tmp, "%s.0", new_name); 
        
        tr69_inline_get_node(pg_ghost_tree, tmp, &p_inst);
        TSL_VASSERT(p_inst != NULL);
        
        p_new_inst = (tr69_info_tree_t *)calloc(1, sizeof(tr69_info_tree_t));
        
        tr69_inline_clone_obj(p_inst, p_new_inst);
        
        p_new_inst->p_parent = p_tree;
        
        tr69_inline_adjust_instance(p_tree, p_new_inst, p_name);
        
        tr69_intf_mod_instance(p_name, 1);
        
        ctllog_debug("add instance %d\n", p_new_inst->info.info_inst.size);
        *p_inst_numb = p_new_inst->info.info_inst.size;
        
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

	 ctllog_debug("tr69_intf2_del_instance [%s] [1]\n",p_name);
        
        tr69_inline_get_node(p_root, p_name, &p_tree);
        TSL_VASSERT(p_tree != NULL);
        TSL_VASSERT(p_tree->node_type == TR69_NODE_TYPE_INSTANCE);

	 ctllog_debug("tr69_intf2_del_instance [%s] [2]\n",p_name);

        p_parent = p_tree->p_parent;
        TSL_VASSERT(p_parent != NULL);
        TSL_VASSERT(p_parent->node_type == TR69_NODE_TYPE_OBJECT ||
                    p_parent->node_type == TR69_NODE_TYPE_INSTANCE);
        
        tsl_list_del(&p_tree->list_node);
        tr69_inline_del_tree_filter(p_tree);
        
        sprintf(tmp, "%s", p_name);
		
        for (i = strlen(p_name); i > 0; i--){
                if (tmp[i] == '.'){
                        tmp[i] = '\0';
                        break;
                }
        }
        ctllog_debug("tr69_intf2_del_instance [%s] [%s]\n",p_name,tmp);

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
        
        tsl_list_del(&p_tree->list_node);
        tr69_inline_del_tree(p_tree);
        
        tr69_intf_mod_instance(p_name, -1);
        
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



tsl_rv_t tr69_intf_save_obj_data(tr69_info_tree_t *p_root, tsl_char_t *p_name, tsl_void_t **pp_data)
{
        tr69_info_tree_t    *p_tree = NULL;
        tsl_void_t *p_cur_data = NULL;
        tsl_rv_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;

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
        
        return TSL_RV_SUC;
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
                //tr69_inline_fresh_obj_data(p_root, p_tree);
                return TSL_RV_ERR;
        }
        
        TSL_VASSERT(p_tree->node_type == TR69_NODE_TYPE_LEAF);
        TSL_VASSERT(p_tree->p_parent != NULL);
        TSL_VASSERT(p_tree->p_parent->node_type == TR69_NODE_TYPE_OBJECT ||
                    p_tree->p_parent->node_type == TR69_NODE_TYPE_INSTANCE);
        
        p_data = tr69_inline_dump_data(p_tree->p_parent);

        TSL_VASSERT(p_data != NULL);
        
        if (p_tree->info.info_leaf.type == TR69_NODE_LEAF_TYPE_STRING||
            p_tree->info.info_leaf.type == TR69_NODE_LEAF_TYPE_DATE){
                pp_src = ((char **)((char *)(p_data)+ p_tree->info.info_leaf.data_offset));
                *type = p_tree->info.info_leaf.type;
                if (pp_src && *pp_src ){
                        *pp_find_data = strdup(*pp_src);
                }else {
                        *pp_find_data = NULL;
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
        
        tr69_intf_get_obj_data(p_root, p_tree->p_parent->full_name, &p_data);
        TSL_VASSERT(p_data != NULL);
        
        if (p_tree->info.info_leaf.type == TR69_NODE_LEAF_TYPE_STRING||
            p_tree->info.info_leaf.type == TR69_NODE_LEAF_TYPE_DATE){
                pp_src = ((char **)((char *)(p_data)+ p_tree->info.info_leaf.data_offset));
                *type = p_tree->info.info_leaf.type;
                if (pp_src && *pp_src ){
                        *pp_find_data = strdup(*pp_src);
                }else {
                        *pp_find_data = NULL;
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

tsl_rv_t tr69_intf_set_leaf_data(tr69_info_tree_t *p_root, tsl_char_t *p_name, tsl_void_t *p_value, tsl_int_t type, tsl_int_t acl)
{
        tr69_info_tree_t *p_tree = NULL;
        tsl_char_t       **pp_dst = NULL;
        tsl_int_t        *p_idst = NULL;
        tsl_void_t       *p_data = NULL;
        
        
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
                }
                
                *pp_dst = strdup((char *)p_value);
                
        }else if (p_tree->info.info_leaf.type == TR69_NODE_LEAF_TYPE_BOOL){
        	p_idst = ((int *)((char *)(p_data)+ p_tree->info.info_leaf.data_offset));
			
        	if (type == TR69_NODE_LEAF_TYPE_STRING){
				if ((strcasecmp(p_value, "true") == 0) || (strcmp(p_value, "1") == 0))
					*p_idst = 1;
				else if ((strcasecmp(p_value, "false") == 0) || (strcmp(p_value, "0") == 0))
					*p_idst = 0;
        	}else if (type == TR69_NODE_LEAF_TYPE_INT || TR69_NODE_LEAF_TYPE_UINT){
        		if (((*(int *)p_value) == 0) || ((*(int *)p_value) == 1))
	        		*p_idst = *(int *)p_value;
        	}
        }else {
                p_idst = ((int *)((char *)(p_data)+ p_tree->info.info_leaf.data_offset));
                
                if (type == TR69_NODE_LEAF_TYPE_STRING){
                        *p_idst = atoi(p_value);
                }else if (type == TR69_NODE_LEAF_TYPE_INT || TR69_NODE_LEAF_TYPE_UINT){
                        *p_idst = *(int *)p_value;
                }
        }

        if (acl == 2){
                tr69_intf_save_unfresh_obj_data(p_root, p_tree->p_parent->full_name, &p_data);
        }else {
                tr69_intf_save_obj_data(p_root, p_tree->p_parent->full_name, &p_data);
        }

        tr69_intf_free_obj_data(p_root, p_tree->p_parent->full_name, &p_data);
        
        return TSL_RV_SUC;
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
                                break;
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
        
        return TSL_RV_SUC;
}

/*User Interface Function*/
tsl_rv_t tr69_cfg_init(tsl_char_t *p_protype_xml, tsl_char_t *p_cfg_xml, tsl_char_t *p_prev_cfg_xml, tsl_char_t *p_cache_cfg_xml)
{
	tsl_rv_t ret = TSL_RV_FAIL_FUNC;	//no need update
	tsl_bool_t cached = TSL_B_FALSE;

#ifdef AEI_PARAM_FORCEUPDATE
    system(ZIP_COMMAND " -d -o " PREV_VERSION_TMP_XML_FILE " " PREV_VERSION_CFG_XML_FILE);
    pg_prev_ver_tree = tr69_func_parse_xml(PREV_VERSION_TMP_XML_FILE);
    if (pg_prev_ver_tree) {
        system("rm -f " PREV_VERSION_TMP_XML_FILE);
    }
#endif

	pg_root_prev_tree = tr69_func_parse_xml(p_prev_cfg_xml);
	if (NULL == pg_root_prev_tree)
	{
        	ctllog_warn("%s: fail to load last cfg file %s\n",__func__,p_prev_cfg_xml);

		if (NULL == p_cache_cfg_xml) {
			return TSL_RV_ERR;
        }

		pg_root_prev_tree = tr69_func_parse_xml(p_cache_cfg_xml);
		cached = TSL_B_TRUE;
	}

	TSL_VASSERT(pg_root_prev_tree != NULL);   

	snprintf(pre_cfg_xml_version,XML_VER_NUM-1,"%s",cfg_xml_version);
	memset(cfg_xml_version, 0, sizeof(cfg_xml_version));

	pg_ghost_tree = tr69_ghost_func_parse_xml(p_protype_xml);

	pg_root_tree = tr69_func_parse_xml(p_cfg_xml);

	TSL_VASSERT(pg_ghost_tree != NULL);
	TSL_VASSERT(pg_root_tree != NULL);

	ret  = checkver(cached);

	return ret;
}

tsl_rv_t tr69_register(tsl_void_t *p_inst_tb, 
                       tsl_int_t inst_tb_count,
                       tsl_void_t *p_reg_tb,
                       tsl_int_t reg_tb_count)
{
	tr69_inst_numb_tb_count = inst_tb_count;
	tr69_regfunc_tb_count = reg_tb_count;

	tr69_travel_set_tree(pg_root_prev_tree);

	//prepare delta instance list
	ctllog_debug("%s:create_delta_inst_list\n",__func__);

	create_delta_inst_list();
	
	//remove redunant instance
	tr69_travel_filter_tree(pg_root_tree);

	//clean instance list
	Clear_Inst_list(&p_def_inst_list);
	Clear_Inst_list(&p_pre_inst_list);

	//save current instance list file
	tsl_char_t cmd_line[128]={"0"};			
   	snprintf(cmd_line,sizeof(cmd_line)-1,"cp %s %s",DEF_INST_LIST_FILE,PRE_INST_LIST_FILE);
	system( cmd_line);
    
#ifdef AEI_PARAM_FORCEUPDATE
    //save current /etc/cfg.xml as previous version cfg file for upgrade using
    system(ZIP_COMMAND " -o " PREV_VERSION_CFG_XML_FILE " " CFG_XML_FILE);
#endif

	system("sync");

	ctllog_debug("tr69_travel_filter_tree \n");

	return TSL_RV_SUC;
}

tsl_void_t tr69_cleanup()
{
	//need trace Segmentation fault issue later
/*
        if (pg_root_tree){
                tr69_intf_cleanup_tree(pg_root_tree, 1);
        }

	ctllog_debug("free pg_root_tree\n");


        if (pg_root_prev_tree){
                tr69_intf_cleanup_tree(pg_root_prev_tree, 1);
        }

	ctllog_debug("free pg_root_prev_tree\n");
*/

        if (pg_ghost_tree){
                tr69_intf_cleanup_tree(pg_ghost_tree, 0);
        }

	ctllog_debug("free pg_ghost_tree\n");
	
        xmlCleanupParser();

#ifdef AEI_PARAM_FORCEUPDATE
    //if no backup upgrade file, create one
    if (access(PREV_VERSION_CFG_XML_FILE, R_OK))
        system(ZIP_COMMAND " -o " PREV_VERSION_CFG_XML_FILE " " CFG_XML_FILE);
#endif

}


tsl_rv_t tr69_save_xml(tsl_char_t *p_file_name)
{
        return tr69_func_tree_to_xml(p_file_name, pg_root_tree);
}

tsl_rv_t tr69_set_unfresh_leaf_data(tsl_char_t *p_name, tsl_void_t *p_value, tsl_int_t type)
{
        return tr69_intf_set_leaf_data(pg_root_tree, p_name, p_value, type, 2);
}

tsl_rv_t tr69_get_unfresh_leaf_data(tsl_char_t *p_name, tsl_void_t **pp_data, tsl_int_t *type)
{
        return tr69_intf_get_unfresh_leaf_data(pg_root_tree, p_name, pp_data, type);
}
