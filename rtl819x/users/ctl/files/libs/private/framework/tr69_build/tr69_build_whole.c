#include "tr69_tree_build_whole.h"
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <ctype.h>
#include "ctl_log.h"

int CTL_LOG_LEVEL = 1;
#define SPECIAL_PARSE_INST            1
#define STD_CLEARUP_DEFAULT_INST      1
#define STD_CLEARUP_NUMBEROFENTRIES   1

tr69_info_tree_t *pg_root_tree  = NULL;
#ifdef GEN_PROTYPE
tr69_info_tree_t *pg_root_tree2  = NULL;
tr69_info_tree_t *pg_root_tree3  = NULL;
#endif
tsl_char_t pg_obj_name[8192][128] = {"\0"};
tsl_char_t pg_oid_name[8192][256] = {"\0"};
tsl_char_t pg_profile_name[8192][128] = {"\0"};
tsl_int_t g_profile_index = 0;
tsl_int_t g_obj_name_index = 0;

tsl_int_t  g_inst_index = 0;
tsl_char_t pg_inst_numb_name[512][256] = {"\0"};
tsl_char_t pg_inst_oid_name[512][256] = {"\0"};
tsl_char_t pg_inst_name[512][256] = {"\0"};

tsl_int_t rand_enable = 0;
tsl_char_t g_version[128] = "\0";
tsl_int_t g_debug = 0;

#ifdef SPECIAL_PARSE_INST
typedef struct _inst_table{
        tsl_char_t *p_inst_numb_name;
		tsl_char_t *p_oid_stack;
}inst_table_t;
tsl_int_t act_strcmp_inst(tsl_char_t *p_inst_numb_name, tsl_char_t *p_inst_oid, tsl_char_t *p_oid_stack, tsl_char_t *p_obj_oid)
{
	tsl_char_t *p1 = p_inst_numb_name;
	tsl_char_t *p2 = p_oid_stack;
	tsl_char_t *p3 = p_inst_oid;
	tsl_char_t *p4 = p_obj_oid;
	tsl_char_t inst_oid[256] = {0};
	tsl_char_t obj_oid[256] = {0};
	tsl_char_t *match = NULL;
	tsl_int_t i = 0;
	tsl_int_t retval = -1;
	inst_table_t table[] = {
		{"LANWLANConfigurationNumberOfEntries", "WLANConfiguration"},
		{"ForwardNumberOfEntries",              "Forwarding"},
		{"ForwardNumberOfEntries",              "IPv6Forwarding"},
		{"AdvancedFilterListNumberOfEntries",   "AdvancedFiltersList"},
		//{"IPv6InterfaceNumberOfEntries", ""},	//to be deleted
		//{"WANMoCAInterfaceNumberOfEntries", ""},	//to be deleted
		//{"NumberOfEntries", ""}			//to be deleted
		//{"WANMoCAInterfaceNumberOfEntries", ""}	//to be deleted
		{"", ""},
	};

	TSL_VASSERT_RV(p1 != NULL, -1);
	TSL_VASSERT_RV(p2 != NULL, -1);
	TSL_VASSERT_RV(p3 != NULL, -1);
	TSL_VASSERT_RV(p4 != NULL, -1);
	while (*p1 == ' ')
		p1++;
	while (*p2 == ' ')
		p2++;
	while (*p3 == ' ')
		p2++;
	while (*p4 == ' ')
		p2++;
	TSL_VASSERT_RV(p1 != NULL, -1);
	TSL_VASSERT_RV(p2 != NULL, -1);
	TSL_VASSERT_RV(p3 != NULL, -1);
	TSL_VASSERT_RV(p4 != NULL, -1);

	/* Special dispose */
	i = 0;
	while (strcasecmp(table[i].p_inst_numb_name, "") != 0)
	{
		if (!strcasecmp(table[i].p_inst_numb_name, p_inst_numb_name))
		{
			if (!strcasecmp(table[i].p_oid_stack, p_oid_stack))
			{
				retval = 0;
				goto CHECK_OID;
			}
		}
		i++;
	}
	/* Normal dispose */
	retval = strncasecmp(p1, p2, strlen(p2));
	if (0 == retval)
		retval = strncasecmp(p1+strlen(p2), "NumberOfEntries", strlen("NumberOfEntries"));

CHECK_OID:
	if (retval == 0)
	{
		i = 0;
		while((match = strchr(p3, '.')) != NULL){
                memcpy(&inst_oid[i], p3, match - p3 + 1);
				i += match - p3 + 1;
                p3 = match + 1;
        }
		i = 0;
		while((match = strchr(p4, '.')) != NULL){
                memcpy(&obj_oid[i], p4, match - p4 + 1);
				i += match - p4 + 1;
                p4 = match + 1;
        }
		//l_printf("inst_oid=%s, obj_oid=%s\n", inst_oid, obj_oid);
		if (strstr(obj_oid, inst_oid) != NULL)
			retval = 0;
		else
			retval = -1;
	}
	return retval;
}

typedef struct _instnumb_table{
        tsl_char_t *p_instnumb;
		tsl_char_t *p_tr98_instnumb;
}instnumb_table_t;
tsl_int_t act_get_tr98_numberofentries(tsl_char_t *p_numberofentries, tsl_char_t *p_tr98_numberofentries)
{
    tsl_int_t i = 0;
    tsl_int_t j = 0;
    tsl_int_t max_stack = 0;
    tsl_int_t tr98_max_stack = 0;
    tsl_char_t *p_str = p_numberofentries;
    tsl_char_t *p_match = NULL;
    tsl_char_t fmt_oid[256] = {0};
    tsl_char_t oid_stack[32][128] = {0};
    tsl_char_t tr98_oid_stack[32][128] = {0};
    instnumb_table_t table[] = {
        //TR98
        {"InternetGatewayDevice.LANDevice.%d.LANEthernetInterfaceConfigNumberOfEntries",            "InternetGatewayDevice.LANDevice.%d.LANEthernetInterfaceNumberOfEntries"},
        {"InternetGatewayDevice.LANDevice.%d.LANUSBInterfaceConfigNumberOfEntries",                 "InternetGatewayDevice.LANDevice.%d.LANUSBInterfaceNumberOfEntries"},
        {"InternetGatewayDevice.LANDevice.%d.WLANConfigurationNumberOfEntries",                     "InternetGatewayDevice.LANDevice.%d.LANWLANConfigurationNumberOfEntries"},
        {"InternetGatewayDevice.WANDevice.%d.WANConnectionDeviceNumberOfEntries",                   "InternetGatewayDevice.WANDevice.%d.WANConnectionNumberOfEntries"},
        //Not TR98
        //{"InternetGatewayDevice.LANDevice.%d.LANMoCAInterfaceConfigNumberOfEntries",                "InternetGatewayDevice.LANDevice.%d.LANMoCAInterfaceNumberOfEntries"},  //Not TR98
        {"InternetGatewayDevice.LANDevice.%d.WLANConfiguration.%d.AssociatedDeviceNumberOfEntries", ""},    //Not found in TR98
        {"InternetGatewayDevice.LANDevice.%d.WLANConfiguration.%d.WEPKeyNumberOfEntries",           ""},    //Not found in TR98
        {"InternetGatewayDevice.LANDevice.%d.WLANConfiguration.%d.PreSharedKeyNumberOfEntries",     ""},    //Not found in TR98
        {"InternetGatewayDevice.LANDevice.%d.WLANConfiguration.%d.APWMMParameterNumberOfEntries",   ""},    //Not found in TR98
        {"InternetGatewayDevice.LANDevice.%d.LANMoCAConfiguration.%d.AssociatedDeviceNumberOfEntries",  ""},//Not TR98
        {"InternetGatewayDevice.ACTIPv6_LANDevice.%d.LANEthernetInterfaceConfigNumberOfEntries",     "InternetGatewayDevice.ACTIPv6_LANDevice.%d.LANEthernetInterfaceNumberOfEntries"},//Not TR98
        {"InternetGatewayDevice.ACTIPv6_LANDevice.%d.LANEthernetInterfaceConfig.%d.StatsNumberOfEntries", ""},//Not TR98
        {"InternetGatewayDevice.ACTIPv6_WANDevice.%d.WANConnectionDeviceNumberOfEntries",           "InternetGatewayDevice.ACTIPv6_WANDevice.%d.WANConnectionNumberOfEntries"},
        {"InternetGatewayDevice.Device.WiFi.AccessPoint.%d.RegistrarNumberOfEntries", ""},
        {"InternetGatewayDevice.Device.DNS.Relay.ForwardingNumberOfEntries", ""},
        {"", ""},
    };

    TSL_VASSERT_RV(p_numberofentries != NULL, -1);
    TSL_VASSERT_RV(p_tr98_numberofentries != NULL, -1);

    memset(oid_stack, 0x0, sizeof(oid_stack));
    memset(tr98_oid_stack, 0x0, sizeof(oid_stack));
    i = 0;
	while((p_match = strchr(p_str, '.')) != NULL){
            memcpy(&oid_stack[i++], p_str, p_match - p_str);
            p_str = p_match + 1;
    }
    memcpy(oid_stack[i], p_str, p_numberofentries + strlen(p_numberofentries) - p_str);
    max_stack = i;

    for (i=0; i<=max_stack; i++){
        if (isdigit(oid_stack[i][0])){
            strcat(fmt_oid, "%d");
        }else{
            strcat(fmt_oid, oid_stack[i]);
        }
        if (i < max_stack){
            strcat(fmt_oid, ".");
        }
    }
    //ctllog_debug("fmt_oid=%s\n", fmt_oid);

    i = 0;
    while (strcasecmp(table[i].p_instnumb, "") != 0){
		if (strcasecmp(table[i].p_instnumb, fmt_oid) == 0){

            j = 0;
            p_str = table[i].p_tr98_instnumb;
            while((p_match = strchr(p_str, '.')) != NULL){
                memcpy(&tr98_oid_stack[j++], p_str, p_match - p_str);
                p_str = p_match + 1;
            }
            memcpy(tr98_oid_stack[j], p_str, table[i].p_tr98_instnumb + strlen(table[i].p_tr98_instnumb) - p_str);
            tr98_max_stack = j;

            for (j=0; j<=tr98_max_stack; j++){
                if (strcasecmp(tr98_oid_stack[j], "%d") == 0){
                    strcat(p_tr98_numberofentries, oid_stack[j]);
                }else{
                    strcat(p_tr98_numberofentries, tr98_oid_stack[j]);
                }
                if (j < tr98_max_stack){
                    strcat(p_tr98_numberofentries, ".");
                }
            }
            //ctllog_debug("p_tr98_numberofentries=%s\n", p_tr98_numberofentries);
            if (strlen(p_tr98_numberofentries) == 0){
                return 1;
            }
            return 0;
        }
        i++;
    }

    return -1;
}
#endif

void convert_to_leafname_from_name(char *profile, char *oid)
{
        sprintf(oid, "%s", profile);

        if (isupper(*oid) && *oid != 'X' && islower(*(oid+1))){
                *oid = tolower(*oid);
        }

        return ;
}

void convert_to_object_from_profile(char *profile, char *oid)
{
        char *ptr = NULL;
        char *ptr_end = NULL;
        int first = 1;

        ptr = profile + strlen("./auto/tr69_func_");
        ptr_end = strstr(profile, ".c");

        snprintf(oid, ptr_end - ptr + 1, "%s", ptr);

        return ;
}

void convert_to_oid_from_profile(char *profile, char *oid)
{
        char *ptr = NULL;
        char *ptr_end = NULL;
        int first = 1;

        ptr = profile + strlen("./auto/tr69_func");
        ptr_end = strstr(profile, "Object.c");

        if(ptr_end == NULL){
                return;
        }

        sprintf(oid, "TR69_OID_");
        oid += strlen(oid);

        while (ptr++ != '\0' && ptr != ptr_end){
                if (isupper(*ptr)){
                        if (isupper(*(ptr+1)) && isupper(*(ptr+2))){
                                *oid++ = *ptr;
                        }else {
                                if (first == 0){
                                        *oid++ = '_';
                                }
                                first = 0;
                                *oid++ = *ptr;
                        }
                }else if (islower(*ptr)){
                        *oid++ = toupper(*ptr);
                }else {
                        *oid++ = *ptr;
                }
        }

        *oid++ = '\0';

        return ;
}


tr69_info_tree_t *tr69_func_add_child_node(xmlNode *p_node, tr69_info_tree_t *p_tree_head, tsl_char_t *p_name)
{
        tr69_info_tree_t *p_tree  = NULL;
        tsl_char_t        type    = 0;
        xmlChar          *p_char  = NULL;
        xmlNode          *p_child = NULL;
		tsl_int_t        acl      = 0;
		tsl_int_t        nosave      = 0;
#ifdef AEI_PARAM_FORCEUPDATE
        tsl_int_t        forceupdate = 0;
#endif

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
		memset(p_tree, 0x0, sizeof(tr69_info_tree_t));

        switch(type){
        case TR69_NODE_TYPE_OBJECT:

	    p_tree->node_type = type;
	    p_char = xmlGetProp(p_node, (xmlChar *)"name");
	    sprintf(p_tree->info.info_obj.name, "%s", p_char);
	    printf("OBJ:%s %s\n", p_node->name, p_char);
	    sprintf(p_tree->info.info_obj.short_name, "%s", p_char);
	    xmlFree(p_char);

	    if(p_name == NULL){
		sprintf(p_tree->full_name, "%s", p_tree->info.info_obj.name);
	    }else {
		sprintf(p_tree->full_name, "%s.%s", p_name, p_tree->info.info_obj.name);
	    }

            p_char = xmlGetProp(p_node, (xmlChar *)"profile");
            if (p_char){
                    sprintf(p_tree->info.info_obj.profile, "./auto/%s", p_char);
                    printf("profile:%s\n", p_char);
                    xmlFree(p_char);
            }else {
                    sprintf(p_tree->info.info_obj.profile, "./auto/%s", "tr69_func_global.c");
            }

            if (!strcmp(p_tree->info.info_obj.name, "InternetGatewayDevice")){
                    p_char = xmlGetProp(p_node, (xmlChar *)"framework_xml_version");
                    if (p_char){
                            sprintf(g_version, "%s", p_char);
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

	    break;
        case TR69_NODE_TYPE_LEAF:
	    p_tree->node_type = type;
#ifdef GEN_PROTYPE
		p_char = xmlGetProp(p_node, (xmlChar *)"specSource");
		if (p_char)
		{
			sprintf(p_tree->info.info_leaf.spec_source, "%s", p_char);
			xmlFree(p_char);
			p_tree->info.info_leaf.has_spec_source = 1;
		}
		else
		{
			p_tree->info.info_leaf.has_spec_source = 0;
		}

		p_char = xmlGetProp(p_node, (xmlChar *)"profile");
		if (p_char)
		{
			sprintf(p_tree->info.info_leaf.profile, "%s", p_char);
			xmlFree(p_char);
			p_tree->info.info_leaf.has_profile = 1;
		}
		else
		{
			p_tree->info.info_leaf.has_profile = 0;
		}

		p_char = xmlGetProp(p_node, (xmlChar *)"minValue");
        if (p_char){
                p_tree->info.info_leaf.has_range = 1;
                xmlFree(p_char);
        }
		else
		{
			p_tree->info.info_leaf.has_range = 0;
		}

		p_char = xmlGetProp(p_node, (xmlChar *)"nosave");
		if (p_char != NULL)
		{
		    xmlFree(p_char);
			p_tree->info.info_leaf.has_nosave = 1;
		}
		else
		{
			p_tree->info.info_leaf.has_nosave = 0;
		}

		p_char = xmlGetProp(p_node, (xmlChar *)"hideParameterFromAcs");
        if (p_char)
		{
			xmlFree(p_char);
			p_tree->info.info_leaf.has_hideParameterFromAcs = 1;
        }
		else
		{
			p_tree->info.info_leaf.has_hideParameterFromAcs = 0;
		}

		p_char = xmlGetProp(p_node, (xmlChar *)"maxLength");
        if (p_char)
		{
			xmlFree(p_char);
			p_tree->info.info_leaf.has_maxLength = 1;
        }
		else
		{
			p_tree->info.info_leaf.has_maxLength = 0;
		}

		p_char = xmlGetProp(p_node, (xmlChar *)"type");
		sprintf(p_tree->info.info_leaf.typestr, "%s", p_char);
#endif
	    p_char = xmlGetProp(p_node, (xmlChar *)"type");

	    if (strstr((tsl_char_t *)p_char, "string")){
		p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_STRING;
	    }else if (strstr((tsl_char_t *)p_char, "unsignedInt")){
		p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_UINT;
	    }else if (strstr((tsl_char_t *)p_char, "int")){
		p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_INT;
	    }else if (!strcmp((tsl_char_t *)p_char, "boolean")){
		p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_BOOL;
	    }else if (!strcmp((tsl_char_t *)p_char, "dateTime")){
		p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_DATE;
	    }else if (strstr((tsl_char_t *)p_char, "base64")){
                p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_STRING;
            }else if (strstr((tsl_char_t *)p_char, "hexBinary")){
                p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_STRING;
            }else if (strstr((tsl_char_t *)p_char, "unsignedLong")){
                p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_UINT;
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

#ifdef AEI_PARAM_FORCEUPDATE
        p_tree->info.info_leaf.forceupdate = 0;
        p_char = xmlGetProp(p_node, (xmlChar *)"forceupdate");
        if (p_char != NULL){
            forceupdate = atoi((char *)p_char);
            xmlFree(p_char);
            if (forceupdate){
                p_tree->info.info_leaf.forceupdate = forceupdate;
            }
        }
#endif

	    p_char = xmlGetProp(p_node, (xmlChar *)"name");
	    sprintf(p_tree->info.info_leaf.name, "%s", p_char);
	    printf("LEAF:%s %s\n", p_node->name, p_char);
	    xmlFree(p_char);

	    if (p_name == NULL){
		sprintf(p_tree->full_name, "%s", p_tree->info.info_leaf.name);
	    }else {
		sprintf(p_tree->full_name, "%s.%s", p_name, p_tree->info.info_leaf.name);
	    }

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

            p_char = xmlGetProp(p_node, (xmlChar *)"default");
            if (strcmp((tsl_char_t *)p_char, "(null)")){
                    if (!strcmp((tsl_char_t *)p_char, "TRUE")||
			!strcmp((tsl_char_t *)p_char, "true")){
                            sprintf(p_tree->info.info_leaf.def_value, "1");
                    }else if (!strcmp((tsl_char_t *)p_char, "FALSE")||
			      !strcmp((tsl_char_t *)p_char, "false")){
                            sprintf(p_tree->info.info_leaf.def_value, "0");
                    }else {
                            sprintf(p_tree->info.info_leaf.def_value, "%s", p_char);
                    }
            }
            xmlFree(p_char);

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
                    p_tree->info.info_leaf.min = atoi(p_char);
                    xmlFree(p_char);
            }

            p_char = xmlGetProp(p_node, (xmlChar *)"maxValue");
            if (p_char){
                    p_tree->info.info_leaf.max = atoi(p_char);
                    xmlFree(p_char);
            }

            p_char = xmlGetProp(p_node, (xmlChar *)"maxLength");
            if (p_char){
                    p_tree->info.info_leaf.max = atoi(p_char);
                    xmlFree(p_char);
            }


            break;
        }

        TSL_INIT_LIST_HEAD(&p_tree->list_head);
        TSL_INIT_LIST_HEAD(&p_tree->list_node);

        if (p_tree_head == NULL && pg_root_tree == NULL){
	    pg_root_tree = p_tree;
        }else {
	    TSL_VASSERT_RV(p_tree_head != NULL, NULL);
	    tsl_list_add(&p_tree->list_node, &p_tree_head->list_head);
            p_tree->p_parent = p_tree_head;
        }

        if (p_node->type == XML_ELEMENT_NODE){
	    p_child = p_node->children;
	    while(p_child != NULL){
		if (p_child->type == XML_ELEMENT_NODE){
		    tr69_func_add_child_node(p_child, p_tree, p_tree->full_name);
		}
		p_child = p_child->next;
	    }
        }

        return p_tree;
}


tsl_rv_t tr69_travel_tree(tr69_info_tree_t *p_root)
{
        tsl_list_head_t *p_list = NULL;
        tr69_info_tree_t    *p_tree = NULL;

        if (p_root->node_type == TR69_NODE_TYPE_OBJECT ){
	    printf("OBJECT %s %s\n", p_root->info.info_obj.name, p_root->full_name);

        }else if (p_root->node_type == TR69_NODE_TYPE_INSTANCE){
	    printf("INSTANCE %d %s\n", p_root->info.info_inst.size, p_root->full_name);

	}else if (p_root->node_type == TR69_NODE_TYPE_LEAF){
	    printf("PARAMETER %s %s\n", p_root->info.info_leaf.name, p_root->full_name);

        }

        for(p_list = p_root->list_head.prev; p_list != &p_root->list_head;){
	    p_tree = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
	    tr69_travel_tree(p_tree);
	    p_list = p_list->prev;
        }
        return TSL_RV_SUC;
}

FILE *g_hfile_fd;

#define act_printf(format, arg...)  \
do{\
    if (g_hfile_fd == NULL){\
        g_hfile_fd = fopen("./auto/tr69_func.h", "wa+");\
    }\
    fprintf(g_hfile_fd, format, ##arg);\
    fflush(g_hfile_fd);\
}while(0)

FILE *g_cfile_fd;
#define act_cprintf(format, arg...)  \
do{\
    if (g_cfile_fd == NULL){\
        g_cfile_fd = fopen("./auto/tr69_func.c", "wa+");\
    }\
    fprintf(g_cfile_fd, format, ##arg);\
    fflush(g_cfile_fd);\
}while(0)

#define act_fcprintf(file, format, arg...)       \
do{\
    FILE *fp = NULL;                        \
    fp = fopen(file, "a+");\
    fprintf(fp, format, ##arg);\
    fflush(fp);\
    fclose(fp);\
}while(0)


FILE *g_st_fd;
#define act_stprintf(format, arg...)  \
do{\
    if (g_st_fd == NULL){\
        g_st_fd = fopen("./auto/tr69_st.h", "wa+");\
    }\
    fprintf(g_st_fd, format, ##arg);\
    fflush(g_st_fd);\
}while(0)

FILE *g_inst_fd;
#define act_instprintf(format, arg...)  \
do{\
    if (g_inst_fd == NULL){\
        g_inst_fd = fopen("./auto/tr69_inst.h", "wa+");\
    }\
    fprintf(g_inst_fd, format, ##arg);\
    fflush(g_inst_fd);\
}while(0)


#define tr69_inline_dump_comments(printf, p_func_name, p_desc, p_arg1, p_arg2, p_arg3) \
do\
{\
        printf("\n\n");                                            \
        printf("/**************************************************************************\n");\
        printf(" *	[FUNCTION NAME]:\n");\
        printf(" *	        %s\n", p_func_name);\
        printf(" *\n");\
        printf(" *	[DESCRIPTION]:\n");\
        printf(" *	        %s\n", p_desc);\
        printf(" *\n");\
        printf(" *	[PARAMETER]:\n");\
        if (p_arg1 ){\
                printf(" *	        %s\n", p_arg1);\
        }\
        if (p_arg2 ){\
                printf(" *	        %s\n", p_arg2);\
        }\
        if (p_arg3 ){\
                printf(" *	        %s\n", p_arg3);\
        }\
        printf(" *\n");\
        printf(" *	[RETURN]\n");\
        printf(" *              >= 0          SUCCESS\n");\
        printf(" *              <  0          ERROR\n");\
        printf(" **************************************************************************/\n");\
 }while(0)

void dump_get_comment(char *func_name, char *desc)
{
        char func[128] = "\0";
        char t2[128] = "\0";

        sprintf(func, "tf69_func_get_%s_value", func_name);
        sprintf(t2, "_%s *p_cur_data", func_name);

        tr69_inline_dump_comments(act_cprintf, func, desc, "tsl_char_t *p_oid_name", t2, NULL);
        tr69_inline_dump_comments(act_printf, func, desc, "tsl_char_t *p_oid_name", t2, NULL);
}

void dump_set_comment(char *func_name, char *desc)
{
        char func[128] = "\0";
        char t2[128] = "\0";
        char t3[128] = "\0";

        sprintf(func, "tf69_func_set_%s_value", func_name);
        sprintf(t2, "_%s *p_cur_data", func_name);
        sprintf(t3, "_%s *p_new_data", func_name);

        tr69_inline_dump_comments(act_cprintf, func, desc, "tsl_char_t *p_oid_name", t2, t3);
        tr69_inline_dump_comments(act_printf, func, desc, "tsl_char_t *p_oid_name", t2, t3);
}


#define tr69_inline_dump_comments2(printf, file, p_func_name, p_desc, p_arg1, p_arg2, p_arg3) \
do\
{\
        printf(file, "\n\n");                                            \
        printf(file, "/**************************************************************************\n"); \
        printf(file, " *	[FUNCTION NAME]:\n");                           \
        printf(file, " *	        %s\n", p_func_name);                    \
        printf(file, " *\n");                                            \
        printf(file, " *	[DESCRIPTION]:\n");                             \
        printf(file, " *	        %s\n", p_desc);                         \
        printf(file, " *\n");                                            \
        printf(file, " *	[PARAMETER]:\n");                                \
        if (p_arg1 ){\
                printf(file, " *	        %s\n", p_arg1); \
        }\
        if (p_arg2 ){\
                printf(file, " *	        %s\n", p_arg2); \
        }\
        if (p_arg3 ){\
                printf(file, " *	        %s\n", p_arg3); \
        }\
        printf(file, " *\n");                    \
        printf(file, " *	[RETURN]\n");                           \
        printf(file, " *              >= 0          SUCCESS\n");         \
        printf(file, " *              <  0          ERROR\n");           \
        printf(file, " **************************************************************************/\n"); \
 }while(0)

void dump_get_comment2(char *file, char *func_name, char *desc)
{
        char func[128] = "\0";
        char t2[128] = "\0";

        sprintf(func, "tf69_func_get_%s_value", func_name);
        sprintf(t2, "st_%s_t *p_cur_data", func_name);

        tr69_inline_dump_comments2(act_fcprintf, file, func, desc, "tsl_char_t *p_oid_name", t2, NULL);
}

void dump_set_comment2(char *file, char *func_name, char *desc)
{
        char func[128] = "\0";
        char t2[128] = "\0";
        char t3[128] = "\0";

        sprintf(func, "tf69_func_set_%s_value", func_name);
        sprintf(t2, "st_%s_t *p_cur_data", func_name);
        sprintf(t3, "st_%s_t *p_new_data", func_name);

        tr69_inline_dump_comments2(act_fcprintf, file, func, desc, "tsl_char_t *p_oid_name", t2, t3);
}


tsl_int_t tr69_common_func_parse_oid(tsl_char_t *p_name, tsl_char_t oid_stack[32][128], tsl_int_t *max_stack)
{
        tsl_char_t *ptr = NULL;
        tsl_char_t *match = NULL;
        tsl_int_t j = 0;

        ptr = p_name;
        while((match = strchr(ptr, '.')) != NULL){
                memcpy(oid_stack[j++], ptr, match - ptr);
                ptr = match + 1;
        }

        memcpy(oid_stack[j], ptr, p_name + strlen(p_name) - ptr);
        *max_stack = j;

        return 0;
}

tsl_void_t tr69_convert_oid_to_long_name(tsl_char_t *p_name, tsl_char_t *p_new_name)
{
        tsl_char_t oid_stack[32][128];
        tsl_int_t max_stack;
        tsl_int_t i = 0;
        tsl_char_t oid[256] = "\0";

        memset(oid_stack, 0, sizeof(oid_stack));
        max_stack = 0;

        tr69_common_func_parse_oid(p_name, oid_stack, &max_stack);

        for (i = 0; i <= max_stack; i++){
#ifndef SUPPORT_PROTYPE0
                if (atoi(oid_stack[i])){
                        strcat(oid, "i");
                }else {
                        strcat(oid, oid_stack[i]);
                }
#else
				if (atoi(oid_stack[i]) || isdigit(oid_stack[i][0])){
                        strcat(oid, "i");
                }else {
                        strcat(oid, oid_stack[i]);
                }
#endif
                if (i != max_stack){
                        strcat(oid, ".");
                }
        }

        //l_printf("new_oid %s\n", oid);
        sprintf(p_new_name, "%s", oid);
}

tsl_void_t tr69_convert_oid_to_sprintf_name(tsl_char_t *p_name, tsl_char_t *p_new_name)
{
        tsl_char_t oid_stack[32][128];
        tsl_int_t max_stack;
        tsl_int_t i = 0;
        tsl_char_t oid[256] = "\0";

        memset(oid_stack, 0, sizeof(oid_stack));
        max_stack = 0;

        tr69_common_func_parse_oid(p_name, oid_stack, &max_stack);

        for (i = 0; i <= max_stack; i++){
#ifndef SUPPORT_PROTYPE0
                if (atoi(oid_stack[i])){
                        strcat(oid, "%d");
                }else {
                        strcat(oid, oid_stack[i]);
                }
#else
                if (atoi(oid_stack[i]) || isdigit(oid_stack[i][0])){
                        strcat(oid, "%d");
                }else {
                        strcat(oid, oid_stack[i]);
                }
#endif
                if (i != max_stack){
                        strcat(oid, ".");
                }
        }

        ctllog_debug("new_oid %s\n", oid);
        sprintf(p_new_name, "%s", oid);
}


tsl_rv_t tr69_tree_to_struct(tr69_info_tree_t *p_root, tsl_char_t *prev_name, tsl_char_t *prev_file, tsl_char_t *prev_desc)
{
        tsl_list_head_t     *p_list = NULL;
        tr69_info_tree_t    *p_tree = NULL;
        tsl_int_t i, k;
        tsl_char_t oid_stack[32][128];
        tsl_int_t max_stack;
        char tmp[256];
        char oid[256];
        char long_oid[256];
        tsl_char_t leaf[256] = "\0";
        tsl_int_t skip = 0;

        tr69_convert_oid_to_long_name(p_root->full_name, long_oid);

        if (
#ifdef SUPPORT_PROTYPE0
            strstr(p_root->full_name, ".1.")||
#endif
            strstr(p_root->full_name, ".2.")||
            strstr(p_root->full_name, ".3.")||
            strstr(p_root->full_name, ".4.")||
            strstr(p_root->full_name, ".5.")||
            strstr(p_root->full_name, ".6.")||
            strstr(p_root->full_name, ".7.")||
            strstr(p_root->full_name, ".8.")||
            strstr(p_root->full_name, ".9.")
            ){
                skip = 1;
        }

        if (p_root->node_type == TR69_NODE_TYPE_OBJECT ){

                if (skip == 0){

                convert_to_object_from_profile(p_root->info.info_obj.profile, p_root->info.info_obj.short_name);

                i = 0;
                k = 0;
                for(i = 0; i < g_obj_name_index; i++){
                        if (!strcmp(p_root->info.info_obj.short_name, pg_obj_name[i])){
                                k = g_obj_name_index++;
                                if (p_root->p_parent->node_type == TR69_NODE_TYPE_INSTANCE &&
                                    p_root->p_parent->p_parent->node_type == TR69_NODE_TYPE_OBJECT){
                                        sprintf(pg_obj_name[k], "%s_%s",
                                                p_root->p_parent->p_parent->info.info_obj.short_name,
                                                p_root->info.info_obj.short_name);
                                }else if (p_root->p_parent->node_type == TR69_NODE_TYPE_OBJECT){
                                        sprintf(pg_obj_name[k], "%s_%s",
                                                p_root->p_parent->info.info_obj.short_name,
                                                p_root->info.info_obj.short_name);

                                }
                                //sprintf(pg_obj_name[k], "%d_%s", k, p_root->info.info_obj.short_name);
                                memset(p_root->info.info_obj.short_name, 0, sizeof(p_root->info.info_obj.short_name));

                                //convert_to_object_from_profile(p_root->info.info_obj.profile, p_root->info.info_obj.short_name);

                                sprintf(p_root->info.info_obj.short_name, "%s", pg_obj_name[k]);

                                //sprintf(p_root->info.info_obj.short_name, "%s", long_oid);
                                break;
                        }
                }
                if (k == 0){
                        k = g_obj_name_index++;
                        sprintf(pg_obj_name[k], "%s", p_root->info.info_obj.short_name);
                }
                sprintf(pg_oid_name[k], "%s", p_root->full_name);

                if (strlen(prev_name) != 0){

                        act_printf("}%s;\n", prev_name);
                        act_printf("typedef %s _%s;\n", prev_name, prev_name);

                        dump_get_comment(prev_name, prev_desc);

                        act_cprintf("\ntsl_int_t tr69_func_get_%s_value(tsl_char_t *p_oid_name, _%s *p_cur_data)\n{\n \ttsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;\n\n\tprintf(\"FUNCTION LINE %s \\n\", __FUNCTION__, __LINE__);\n\n\treturn rv;\n}\n",
                                    prev_name,
                                    prev_name,
                                    "%s %d");

			act_printf("\ntsl_int_t tr69_func_get_%s_value(tsl_char_t *p_oid_name, _%s *p_cur_data);",
				   prev_name,
				   prev_name);



                        dump_set_comment(prev_name, prev_desc);
                        act_cprintf("\ntsl_int_t tr69_func_set_%s_value(tsl_char_t *p_oid_name, _%s *p_cur_data, _%s *p_new_data)\n{\n \ttsl_int_t rv = TR69_RT_SUCCESS_VALUE_CHANGED;\n\n\tprintf(\"FUNCTION LINE %s \\n\", __FUNCTION__, __LINE__);\n\n\treturn rv;\n}\n\n",
                                    prev_name,
                                    prev_name,
                                    prev_name,
                                    "%s %d");

                        act_printf("\ntsl_int_t tr69_func_set_%s_value(tsl_char_t *p_oid_name, _%s *p_cur_data, _%s *p_new_data);",
				   prev_name,
                                   prev_name,
				   prev_name);

                        memset(prev_name, 0, sizeof(prev_name));
                        sprintf(prev_name, "%s", p_root->info.info_obj.short_name);
                        sprintf(prev_file, "%s", p_root->info.info_obj.profile);
                        sprintf(prev_desc, "%s", long_oid);
                }else {

                        act_printf("/*TR69_FUNC.h*/\n\n");
                        act_printf("#ifndef TR69_FUNC_H\n");
                        act_printf("#define TR69_FUNC_H\n\n");

                        act_printf("#include \"tsl_common.h\"\n\n");

                        act_printf("#define TR69_RT_SUCCESS_VALUE_UNCHANGED 0x101\n");
                        act_printf("#define TR69_RT_SUCCESS_VALUE_CHANGED 0x102\n");

                        act_cprintf("/*TR69_FUNC.c*/\n\n");

                        act_cprintf("#include \"tr69_func.h\"\n");

                        sprintf(prev_name, "%s", p_root->info.info_obj.short_name);
                        sprintf(prev_file, "%s", p_root->info.info_obj.profile);
                        sprintf(prev_desc, "%s", long_oid);
                }
                act_printf("\n\n\n/*%s*/", p_root->full_name);

                act_cprintf("\n/*%s*/\n", p_root->full_name);

                act_printf("\ntypedef struct st_%s_obj {\n", p_root->info.info_obj.short_name);


                }
        }else if (p_root->node_type == TR69_NODE_TYPE_INSTANCE){

                if (skip == 0){

                memset(oid, 0, sizeof(oid));
                memset(oid_stack, 0, sizeof(oid_stack));
                max_stack = 0;

                tr69_common_func_parse_oid(p_root->full_name, oid_stack, &max_stack);

                for (i = 0; i <= max_stack; i++){
                        //if(atoi(oid_stack[i])){
                        if (atoi(oid_stack[i]) || isdigit(oid_stack[i][0])){
                                strcat(oid, "i");
                        }else {
                                strcat(oid, oid_stack[i]);
                        }

                        if (i != max_stack ){
                                strcat(oid, ".");
                        }
                }

                for (i = g_inst_index - 1; i >= 0; i--){
#ifdef SPECIAL_PARSE_INST
						if (!act_strcmp_inst(pg_inst_numb_name[i], pg_inst_oid_name[i], oid_stack[max_stack-1], p_root->full_name)){
#else
                        if (strstr(pg_inst_numb_name[i], oid_stack[max_stack-1])){
#endif
                                sprintf(pg_inst_name[i], oid);
                                break;
                        }

                        if (strstr(oid_stack[max_stack-1], "Device") || strstr(oid_stack[max_stack-1], "Config")){
                                memset(tmp, '\0', sizeof(tmp));
                                snprintf(tmp, strlen(oid_stack[max_stack-1]) - strlen("Device") + 1, "%s",oid_stack[max_stack-1]);

#ifdef SPECIAL_PARSE_INST
								if (!act_strcmp_inst(pg_inst_numb_name[i], pg_inst_oid_name[i], tmp, p_root->full_name)){
#else
                                if (strstr(pg_inst_numb_name[i], tmp)){
#endif
                                        sprintf(pg_inst_name[i], oid);
                                        break;
                                }
                        }

                }

                }
        }else if (p_root->node_type == TR69_NODE_TYPE_LEAF){
            if (p_root->p_parent->node_type == TR69_NODE_TYPE_OBJECT){
                    p_root->p_parent->info.info_obj.data_size += sizeof(char *);
            }else if (p_root->p_parent->node_type == TR69_NODE_TYPE_INSTANCE){
                    p_root->p_parent->info.info_inst.data_size += sizeof(char *);
            }
	    if ((p_root->p_parent->node_type ==  TR69_NODE_TYPE_OBJECT
#ifndef SUPPORT_PROTYPE0
                 || p_root->p_parent->info.info_inst.size == 1) &&
#else
                 || p_root->p_parent->info.info_inst.size == 0) &&
#endif
                skip == 0)
                {


                switch(p_root->info.info_leaf.type){
		case TR69_NODE_LEAF_TYPE_STRING:
		    act_printf("\t tsl_char_t *");
		    break;
		case TR69_NODE_LEAF_TYPE_UINT:
		    act_printf("\t tsl_uint_t ");
		    break;
		case TR69_NODE_LEAF_TYPE_INT:
		    act_printf("\t tsl_int_t ");
		    break;
		case TR69_NODE_LEAF_TYPE_BOOL:
		    act_printf("\t tsl_int_t ");
		    break;
		case TR69_NODE_LEAF_TYPE_DATE:
		    act_printf("\t tsl_char_t *");
		    break;
		}
                //act_printf("%s;\n", p_root->info.info_leaf.name);

                convert_to_leafname_from_name(p_root->info.info_leaf.name, leaf);
                act_printf("%s;\n", leaf);

                if(strstr(p_root->info.info_leaf.name, "NumberOfEntries")){
                        sprintf(pg_inst_numb_name[g_inst_index], p_root->info.info_leaf.name);
						sprintf(pg_inst_oid_name[g_inst_index], p_root->full_name);
						g_inst_index++;
                }

                }
        }

        for(p_list = p_root->list_head.prev; p_list != &p_root->list_head;){
                p_tree = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
                tr69_tree_to_struct(p_tree, prev_name, prev_file, prev_desc);
                p_list = p_list->prev;
        }
        return TSL_RV_SUC;
}


tsl_void_t tr69_tree_to_head(tr69_info_tree_t *p_root)
{
    tsl_char_t prev_name[256] = "\0";
    tsl_char_t prev_desc[256] = "\0";
    tsl_char_t prev_file[256] = "\0";
    char long_oid[256];
    tsl_int_t i;



    tr69_tree_to_struct(p_root, prev_name, prev_file, prev_desc);

    if (strlen(prev_name) != 0){
	act_printf("}_%s;\n", prev_name);

        dump_get_comment(prev_name, prev_desc);
        act_cprintf("\ntsl_int_t tr69_func_get_%s_value(tsl_char_t *p_oid_name, _%s *p_cur_data)\n{\n \ttsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;\n\n\tprintf(\"FUNCTION LINE %s \\n\", __FUNCTION__, __LINE__);\n\n\treturn rv;\n}\n",
                    prev_name,
                    prev_name,
                    "%s %d");

        act_printf("\ntsl_int_t tr69_func_get_%s_value(tsl_char_t *p_oid_name, _%s *p_cur_data);",
                   prev_name,
                   prev_name);

        dump_set_comment(prev_name, prev_desc);
        act_cprintf("\nint tr69_func_set_%s_value(char *p_oid_name, _%s *p_cur_data, _%s *p_new_data)\n{\n \tint rv = TR69_RT_SUCCESS_VALUE_CHANGED;\n\n\tprintf(\"FUNCTION LINE %s \\n\", __FUNCTION__, __LINE__);\n\n\treturn rv;\n}\n\n",
                    prev_name,
                    prev_name,
                    prev_name,
                    "%s %d");

        act_printf("\ntsl_int_t tr69_func_set_%s_value(tsl_char_t *p_oid_name, _%s *p_cur_data, _%s *p_new_data);",
                   prev_name,
                   prev_name,
                   prev_name);

    }

    act_instprintf("#ifndef TR69_INST_H\n");
    act_instprintf("#define TR69_INST_H\n\n");

    act_instprintf("struct tr69_inst_numb_of_entries_s{\n\tchar numb_of_entries_oid[256];\n\tchar entries_oid[256];\n};\n\n");

    act_instprintf("struct tr69_inst_numb_of_entries_s tr69_inst_numb_tb[]={\n");
    for(i = 0; i < g_inst_index; i++){
            act_instprintf("\t{\n\t\t\"%s\",\n", pg_inst_numb_name[i]);
            act_instprintf("\t\t\"%s\"\n\t},\n\n", pg_inst_name[i]);
    }
    act_instprintf("};\n\n#endif\n");



    act_stprintf("#ifndef TR69_ST_H\n");
    act_stprintf("#define TR69_ST_H\n");
    act_stprintf("\n\n");
    act_stprintf("tsl_char_t st_version[128] = \"%s\";\n", g_version);
    act_stprintf("\ntypedef tsl_int_t (*tr69_func_get_obj_t)(tsl_char_t *p_oid_name, tsl_void_t *p_cur_data);");
    act_stprintf("\ntypedef tsl_int_t (*tr69_func_set_obj_t)(tsl_char_t *p_oid_name, tsl_void_t *p_cur_data, tsl_void_t *p_new_data);");

    act_stprintf("\nstruct tr69_register_func{\n\t tsl_char_t oid_name[256];\n\ttr69_func_get_obj_t get_func;\n\ttr69_func_set_obj_t set_func;\n};\n\n");
    act_stprintf("\nstruct tr69_register_func tr69_regfunc_tb[]={");



    for(i = 0; i < g_obj_name_index; i++){
            memset(long_oid, 0, sizeof(long_oid));
            tr69_convert_oid_to_long_name(pg_oid_name[i], long_oid);
            act_stprintf("\n\t{\n\t\"%s\", \n\t(tr69_func_get_obj_t )tr69_func_get_%s_value,", long_oid, pg_obj_name[i]);
            act_stprintf("\n\t(tr69_func_set_obj_t )tr69_func_set_%s_value\n\t},\n", pg_obj_name[i]);

    }

    act_stprintf("\n};\n\n");
    act_stprintf("\n\n#endif\n\n");

    act_printf("\n\n#endif\n\n");

}





tsl_rv_t tr69_func_parse_xml()
{
        xmlNode *root_node = NULL;
        xmlDoc *doc = NULL;
	tr69_info_tree_t *p_root;
        doc = xmlReadFile("protype.xml", NULL, 0);

        TSL_VASSERT(doc != NULL);
        root_node = xmlDocGetRootElement(doc);

	p_root = tr69_func_add_child_node(root_node, NULL, NULL);

        xmlFreeDoc(doc);
        xmlCleanupParser();

	return TSL_RV_SUC;
}



xmlNode *tr69_func_inline_tree_to_xml(tr69_info_tree_t *p_root, xmlNode *p_parent_node)
{
        tsl_list_head_t *p_list = NULL;
        tr69_info_tree_t    *p_tree = NULL;
        xmlNode         *p_node = NULL;
        char tmp[256] = "\0";

        if (p_root->node_type == TR69_NODE_TYPE_OBJECT ){
	    printf("OBJECT %s %s\n", p_root->info.info_obj.name, p_root->full_name);
	    p_node = xmlNewNode(NULL, (xmlChar *)"Object");
	    xmlNewProp(p_node, (xmlChar *)"name", (xmlChar *)p_root->info.info_obj.name);

            sprintf(tmp, "%d", p_root->info.info_obj.data_size);

            xmlNewProp(p_node, (xmlChar *)"data_size", (xmlChar *)tmp);

            if (!strcmp(p_root->info.info_obj.name, "InternetGatewayDevice")){
                    xmlNewProp(p_node, (xmlChar *)"framework_xml_version", (xmlChar *)g_version);
            }
        }else if (p_root->node_type == TR69_NODE_TYPE_INSTANCE){
	    printf("INSTANCE %d %s\n", p_root->info.info_inst.size, p_root->full_name);
#ifdef SUPPORT_PROTYPE0
		if (p_root->info.info_inst.size == 0){
			return NULL;
		}
#endif
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

		if (p_root->info.info_inst.nosave == TR69_NODE_LEAF_ACL2_NOSAVE){
			xmlNewProp(p_node, (xmlChar *)"nosave", (xmlChar *)"1");
		}else {
			xmlNewProp(p_node, (xmlChar *)"nosave", (xmlChar *)"0");
		}

            sprintf(tmp, "%d", p_root->info.info_inst.data_size);

            xmlNewProp(p_node, (xmlChar *)"data_size", (xmlChar *)tmp);
	}else if (p_root->node_type == TR69_NODE_TYPE_LEAF){
                printf("PARAMETER %s %s\n", p_root->info.info_leaf.name, p_root->full_name);

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

#ifdef AEI_PARAM_FORCEUPDATE
        if (1==p_root->info.info_leaf.forceupdate){
            xmlNewProp(p_node, (xmlChar *)"forceupdate", (xmlChar *)"1");
        } else if (2==p_root->info.info_leaf.forceupdate) {
            xmlNewProp(p_node, (xmlChar *)"forceupdate", (xmlChar *)"2");
        }
#endif

		memset(tmp, 0x0, sizeof(tmp));
	    switch (p_root->info.info_leaf.type){
	    case TR69_NODE_LEAF_TYPE_STRING:
                    xmlNewProp(p_node, (xmlChar *)"type", (xmlChar *)"string");
                    if (rand_enable){
                            sprintf(tmp, "string%d", rand()%1000000);
                    }
                    xmlNodeSetContent(p_node, (xmlChar *)tmp);

                    if (p_root->info.info_leaf.max != 0){
                            sprintf(tmp, "%d", p_root->info.info_leaf.max);
                            xmlNewProp(p_node, (xmlChar *)"maxLength", (xmlChar *)tmp);
                    }

                    break;
	    case TR69_NODE_LEAF_TYPE_UINT:
                    xmlNewProp(p_node, (xmlChar *)"type", (xmlChar *)"unsignedInt");
                    if (rand_enable){
                            sprintf(tmp, "%d", rand()%1000000);
                    }
                    xmlNodeSetContent(p_node, (xmlChar *)tmp);

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
                    if (rand_enable){
                            sprintf(tmp, "%d", rand()%1000000);
                    }
                    xmlNodeSetContent(p_node, (xmlChar *)tmp);

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
                    if (rand_enable){
                            sprintf(tmp, "%d", rand()%1);
                    }
                    xmlNodeSetContent(p_node, (xmlChar *)tmp);
                    break;
	    case TR69_NODE_LEAF_TYPE_DATE:
                    xmlNewProp(p_node, (xmlChar *)"type", (xmlChar *)"dateTime");
                    time_t ti;
                    ti = time(NULL);
                    if (rand_enable){
                            xmlNodeSetContent(p_node, (xmlChar *)ctime(&ti));
                    }
                    break;
	    }

            if (strlen(p_root->info.info_leaf.def_value)){
                    xmlNodeSetContent(p_node, (xmlChar *)p_root->info.info_leaf.def_value);
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

tsl_rv_t tr69_tree_to_object(tr69_info_tree_t *p_root)
{
        tsl_list_head_t     *p_list = NULL;
        tr69_info_tree_t    *p_tree = NULL;
        tsl_int_t i,k;
        char long_oid[256];
        char sprintf_oid[256] = "\0";
        char define_oid[256] = "\0";
        tsl_int_t skip = 0;

        tr69_convert_oid_to_long_name(p_root->full_name, long_oid);

        if (
#ifdef SUPPORT_PROTYPE0
            strstr(p_root->full_name, ".1.")||
#endif
            strstr(p_root->full_name, ".2.")||
            strstr(p_root->full_name, ".3.")||
            strstr(p_root->full_name, ".4.")||
            strstr(p_root->full_name, ".5.")||
            strstr(p_root->full_name, ".6.")||
            strstr(p_root->full_name, ".7.")||
            strstr(p_root->full_name, ".8.")||
            strstr(p_root->full_name, ".9.")
            ){
                return;
        }


        if (p_root->node_type == TR69_NODE_TYPE_OBJECT ){
                i = 0;
                k = 0;
                for(i = 0; i < g_profile_index; i++){
                        if (!strcmp(p_root->info.info_obj.profile, pg_profile_name[i])){
                                k = g_profile_index++;
                                sprintf(pg_profile_name[k], "%s", p_root->info.info_obj.profile);
                                break;
                        }
                }
                if (k == 0){
                        k = g_profile_index++;
                        sprintf(pg_profile_name[k], "%s", p_root->info.info_obj.profile);

                        act_fcprintf(p_root->info.info_obj.profile, "/*%s*/\n\n", p_root->info.info_obj.profile);
                        act_fcprintf(p_root->info.info_obj.profile, "#include \"tsl_common.h\"\n");
                        act_fcprintf(p_root->info.info_obj.profile, "#include \"tr69_func.h\"\n");
                        act_fcprintf(p_root->info.info_obj.profile, "#include \"tr69_func_common.h\"\n");
                }


                //act_fcprintf(p_root->info.info_obj.profile, "\n/*%s*/\n", p_root->full_name);
                dump_get_comment2(p_root->info.info_obj.profile, p_root->info.info_obj.short_name, long_oid);
                act_fcprintf(p_root->info.info_obj.profile, "\ntsl_int_t tr69_func_get_%s_value(tsl_char_t *p_oid_name, _%s *p_cur_data)\n{\n \ttsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;\n\n\tctllog_debug(\"FUNCTION LINE %s \\n\", p_oid_name, __FUNCTION__, __LINE__);\n\n\treturn rv;\n}\n",
                             p_root->info.info_obj.short_name,
                             p_root->info.info_obj.short_name,
                             "%s %s %d");

                dump_set_comment2(p_root->info.info_obj.profile, p_root->info.info_obj.short_name, long_oid);
                act_fcprintf(p_root->info.info_obj.profile, "\ntsl_int_t tr69_func_set_%s_value(tsl_char_t *p_oid_name, _%s *p_cur_data, _%s *p_new_data)\n{\n \ttsl_int_t rv = TR69_RT_SUCCESS_VALUE_CHANGED;\n\n\tctllog_debug(\"FUNCTION LINE %s \\n\", p_oid_name, __FUNCTION__, __LINE__);\n\n\treturn rv;\n}\n\n",
                             p_root->info.info_obj.short_name,
                             p_root->info.info_obj.short_name,
                             p_root->info.info_obj.short_name,
                             "%s %s %d");

                tr69_convert_oid_to_sprintf_name(p_root->full_name, sprintf_oid);
                convert_to_oid_from_profile(p_root->info.info_obj.profile, define_oid);

                act_fcprintf("./auto/OID.h", "#define %s \"%s\" \n\n", define_oid, sprintf_oid);
        }

        for(p_list = p_root->list_head.prev; p_list != &p_root->list_head;){
                p_tree = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
                tr69_tree_to_object(p_tree);
                p_list = p_list->prev;
        }
        return TSL_RV_SUC;
}


tsl_rv_t tr69_func_tree_to_xml(tsl_char_t *p_file_name, tr69_info_tree_t *p_root)
{
    xmlDoc *doc = NULL;
    xmlNode *root_node = NULL;

    doc = xmlNewDoc((xmlChar *)"1.0");
    root_node = tr69_func_inline_tree_to_xml(p_root, root_node);

    xmlDocSetRootElement(doc, root_node);

    xmlSaveFormatFile(p_file_name, doc, 1);

    xmlFreeDoc(doc);
    xmlCleanupParser();

    return TSL_RV_SUC;
}


tsl_rv_t tr69_inline_cleanup_tree(tr69_info_tree_t *p_root)
{
        tsl_list_head_t *p_list = NULL;
        tr69_info_tree_t    *p_tree = NULL;

        for(p_list = p_root->list_head.prev; p_list != &p_root->list_head;){
	    p_tree = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
	    tr69_inline_cleanup_tree(p_tree);
            p_list = p_list->prev;
            free(p_tree);
        }
        return TSL_RV_SUC;
}

tsl_rv_t tr69_intf_cleanup_tree(tr69_info_tree_t *p_root)
{
        tr69_inline_cleanup_tree(p_root);
        free(p_root);
        return TSL_RV_SUC;
}

#if 0
tsl_int_t main(tsl_int_t argc, tsl_char_t **argv)
{
        tsl_int_t i = 0;
        tsl_char_t file[128] = "\0";

        if (argc >= 2){
                rand_enable = 1;
        }

        system("rm -rf ./auto.old; mv auto auto.old; mkdir ./auto");

        srand(time(NULL));

        tr69_func_parse_xml();

        tr69_tree_to_head(pg_root_tree);

        act_fcprintf("./auto/OID.h", "#ifndef OID_H\n#define OID_H\n\n");

        act_fcprintf("./auto/OID.h",
                     "\nvoid oid_to_fullname(char *MDM_OID, char *OID, ...)\n{\n\tva_list args;\n\tva_start(args, OID);\n\tvsprintf(MDM_OID, OID, args);\n\tva_end(args);\n}\n");


        tr69_tree_to_object(pg_root_tree);
        act_fcprintf("./auto/OID.h", "#endif\n");

        tr69_func_tree_to_xml("cfg.xml", pg_root_tree);
        act_fcprintf("./auto/Auto_Build_Files", "OBJS = \\\n");
        for(i = 0; i < g_profile_index; i++){
                memset(file, 0, sizeof(file));
                sprintf(file, "%s", pg_profile_name[i]);
                file[strlen(file)-1] = 'o';
                act_fcprintf("./auto/Auto_Build_Files", "%s \\\n", file+strlen("./auto/"));
        }

        tr69_intf_cleanup_tree(pg_root_tree);

        system("diff -Nur auto.old auto > auto_patch");

        return 0;
}

#endif

#define TR69_OID_COMM_NAME_SIZE                 128
#define TR69_OID_FULL_NAME_SIZE                 256
#define TR69_DEF_VALUE_SIZE                     128

#if 0
tsl_bool_t tr69_check_oid_is_default_inst(tsl_char_t *p_name)
{
    tsl_int_t i = 0;
	tsl_char_t oid_stack[32][128];
	tsl_int_t max_stack;

	memset(oid_stack, 0, sizeof(oid_stack));
    max_stack = 0;

	tr69_common_func_parse_oid(p_name, oid_stack, &max_stack);
	for (i = 0; i <= max_stack; i++){
        if (isdigit(oid_stack[i][0]))
        {
            if (atoi(oid_stack[i]) == 0)
            {
			    return tsl_b_true;
		    }
            else
            {
                return tsl_b_false;
            }
        }
	}

	return tsl_b_false;
}
#else
/*return A.0.B.0.C.0, A.0.B.1.C.1*/
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

#endif

tsl_bool_t tr69_check_oid_is_user_inst(tsl_char_t *p_name)
{
#if 0
	tsl_int_t i = 0;
	tsl_char_t oid_stack[32][128];
	tsl_int_t max_stack;

	memset(oid_stack, 0, sizeof(oid_stack));
    max_stack = 0;

	tr69_common_func_parse_oid(p_name, oid_stack, &max_stack);
	for (i = 0; i <= max_stack; i++){
		if (atoi(oid_stack[i])){
			return tsl_b_true;
		}
	}

	return tsl_b_false;
#else
    tsl_int_t i = 0;
	tsl_char_t oid_stack[32][128];
	tsl_int_t max_stack;
    tsl_int_t has_default_inst = 0;
    tsl_int_t has_user_inst = 0;

	memset(oid_stack, 0, sizeof(oid_stack));
    max_stack = 0;

	tr69_common_func_parse_oid(p_name, oid_stack, &max_stack);
	for (i = 0; i <= max_stack; i++){
		if (atoi(oid_stack[i])){
			has_user_inst = 1;
		}
        else if (isdigit(oid_stack[i][0])){
            has_default_inst = 1;
        }
	}
    if (has_default_inst == 1){
        return tsl_b_false;
    }
    return tsl_b_true;
#endif
}

tr69_info_tree_t *tr69_func_add_child_node2(xmlNode *p_node, tr69_info_tree_t *p_tree_head, tsl_char_t *p_name, tr69_info_tree_t **pp_root_tree)
{
        tr69_info_tree_t *p_tree  = NULL;
        tsl_char_t        type    = 0;
        xmlChar          *p_char  = NULL;
        xmlNode          *p_child = NULL;
		tsl_int_t        acl      = 0;
		tsl_int_t        nosave      = 0;
#ifdef AEI_PARAM_FORCEUPDATE
        tsl_int_t        forceupdate = 0;
#endif

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
		memset(p_tree, 0x0, sizeof(tr69_info_tree_t));

        switch(type){
        case TR69_NODE_TYPE_OBJECT:

	    p_tree->node_type = type;
	    p_char = xmlGetProp(p_node, (xmlChar *)"name");
	    sprintf(p_tree->info.info_obj.name, "%s", p_char);
	    printf("OBJ:%s %s\n", p_node->name, p_char);
	    sprintf(p_tree->info.info_obj.short_name, "%s", p_char);
	    xmlFree(p_char);

	    if(p_name == NULL){
		sprintf(p_tree->full_name, "%s", p_tree->info.info_obj.name);
	    }else {
		sprintf(p_tree->full_name, "%s.%s", p_name, p_tree->info.info_obj.name);
	    }

            p_char = xmlGetProp(p_node, (xmlChar *)"profile");
            if (p_char){
                    sprintf(p_tree->info.info_obj.profile, "%s", p_char);
                    printf("profile:%s\n", p_char);
                    xmlFree(p_char);
            }else {
                    sprintf(p_tree->info.info_obj.profile, "%s", "tr69_func_global.c");
            }

            if (!strcmp(p_tree->info.info_obj.name, "InternetGatewayDevice")){
                    p_char = xmlGetProp(p_node, (xmlChar *)"framework_xml_version");
                    if (p_char){
                            sprintf(g_version, "%s", p_char);
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

	    break;
        case TR69_NODE_TYPE_LEAF:
	    p_tree->node_type = type;
#ifdef GEN_PROTYPE
		p_char = xmlGetProp(p_node, (xmlChar *)"specSource");
		if (p_char)
		{
			sprintf(p_tree->info.info_leaf.spec_source, "%s", p_char);
			xmlFree(p_char);
			p_tree->info.info_leaf.has_spec_source = 1;
		}
		else
		{
			p_tree->info.info_leaf.has_spec_source = 0;
		}

		p_char = xmlGetProp(p_node, (xmlChar *)"profile");
		if (p_char)
		{
			sprintf(p_tree->info.info_leaf.profile, "%s", p_char);
			xmlFree(p_char);
			p_tree->info.info_leaf.has_profile = 1;
		}
		else
		{
			p_tree->info.info_leaf.has_profile = 0;
		}

		p_char = xmlGetProp(p_node, (xmlChar *)"minValue");
        if (p_char){
                p_tree->info.info_leaf.has_range = 1;
                xmlFree(p_char);
        }
		else
		{
			p_tree->info.info_leaf.has_range = 0;
		}

		p_char = xmlGetProp(p_node, (xmlChar *)"nosave");
		if (p_char != NULL)
		{
		    xmlFree(p_char);
			p_tree->info.info_leaf.has_nosave = 1;
		}
		else
		{
			p_tree->info.info_leaf.has_nosave = 0;
		}

		p_char = xmlGetProp(p_node, (xmlChar *)"hideParameterFromAcs");
        if (p_char)
		{
			xmlFree(p_char);
			p_tree->info.info_leaf.has_hideParameterFromAcs = 1;
        }
		else
		{
			p_tree->info.info_leaf.has_hideParameterFromAcs = 0;
		}

		p_char = xmlGetProp(p_node, (xmlChar *)"maxLength");
        if (p_char)
		{
			xmlFree(p_char);
			p_tree->info.info_leaf.has_maxLength = 1;
        }
		else
		{
			p_tree->info.info_leaf.has_maxLength = 0;
		}

		p_char = xmlGetProp(p_node, (xmlChar *)"type");
		sprintf(p_tree->info.info_leaf.typestr, "%s", p_char);
#endif
	    p_char = xmlGetProp(p_node, (xmlChar *)"type");

	    if (strstr((tsl_char_t *)p_char, "string")){
		p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_STRING;
	    }else if (strstr((tsl_char_t *)p_char, "unsignedInt")){
		p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_UINT;
	    }else if (strstr((tsl_char_t *)p_char, "int")){
		p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_INT;
	    }else if (!strcmp((tsl_char_t *)p_char, "boolean")){
		p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_BOOL;
	    }else if (!strcmp((tsl_char_t *)p_char, "dateTime")){
		p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_DATE;
	    }else if (strstr((tsl_char_t *)p_char, "base64")){
                p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_STRING;
            }else if (strstr((tsl_char_t *)p_char, "hexBinary")){
                p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_STRING;
            }else if (strstr((tsl_char_t *)p_char, "unsignedLong")){
                p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_UINT;
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

#ifdef AEI_PARAM_FORCEUPDATE
        p_tree->info.info_leaf.forceupdate = 0;
        p_char = xmlGetProp(p_node, (xmlChar *)"forceupdate");
        if (p_char != NULL){
            forceupdate = atoi((char *)p_char);
            xmlFree(p_char);
            if (forceupdate){
                p_tree->info.info_leaf.forceupdate = forceupdate;
            }
        }
#endif

	    p_char = xmlGetProp(p_node, (xmlChar *)"name");
	    sprintf(p_tree->info.info_leaf.name, "%s", p_char);
	    printf("LEAF:%s %s\n", p_node->name, p_char);
	    xmlFree(p_char);

	    if (p_name == NULL){
		sprintf(p_tree->full_name, "%s", p_tree->info.info_leaf.name);
	    }else {
		sprintf(p_tree->full_name, "%s.%s", p_name, p_tree->info.info_leaf.name);
	    }
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

            p_char = xmlGetProp(p_node, (xmlChar *)"default");
#if 0
            if (strcmp((tsl_char_t *)p_char, "(null)")){
                    if (!strcmp((tsl_char_t *)p_char, "TRUE")||
			!strcmp((tsl_char_t *)p_char, "true")){
                            sprintf(p_tree->info.info_leaf.def_value, "1");
                    }else if (!strcmp((tsl_char_t *)p_char, "FALSE")||
			      !strcmp((tsl_char_t *)p_char, "false")){
                            sprintf(p_tree->info.info_leaf.def_value, "0");
                    }else {
                            sprintf(p_tree->info.info_leaf.def_value, "%s", p_char);
                    }
            }
#endif
			//sprintf(p_tree->info.info_leaf.def_value, "%s", p_char);
#ifdef STD_CLEARUP_NUMBEROFENTRIES
			if (strstr(p_tree->info.info_leaf.name, "NumberOfEntries") != NULL){
                sprintf(p_tree->info.info_leaf.def_value, "0");
            }else{
    			snprintf(p_tree->info.info_leaf.def_value, sizeof(p_tree->info.info_leaf.def_value), "%s", p_char);
            }
#else
            snprintf(p_tree->info.info_leaf.def_value, sizeof(p_tree->info.info_leaf.def_value), "%s", p_char);
#endif
            xmlFree(p_char);

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
                    p_tree->info.info_leaf.min = atoi(p_char);
                    xmlFree(p_char);
            }

            p_char = xmlGetProp(p_node, (xmlChar *)"maxValue");
            if (p_char){
                    p_tree->info.info_leaf.max = atoll(p_char);
                    xmlFree(p_char);
            }

            p_char = xmlGetProp(p_node, (xmlChar *)"maxLength");
            if (p_char){
                    p_tree->info.info_leaf.max = (tsl_uint_t)atoi(p_char);
                    xmlFree(p_char);
            }


            break;
        }

        TSL_INIT_LIST_HEAD(&p_tree->list_head);
        TSL_INIT_LIST_HEAD(&p_tree->list_node);

        if (p_tree_head == NULL && (*pp_root_tree) == NULL){
	    *pp_root_tree = p_tree;
        }else {
		if ((p_tree->node_type != TR69_NODE_TYPE_OBJECT) &&
			(p_tree->node_type != TR69_NODE_TYPE_INSTANCE) &&
			(p_tree->node_type != TR69_NODE_TYPE_LEAF))
		{
			ctllog_debug("add child: node type is not object/instance/leaf (%02X)!!!!\n", p_tree->node_type);
		}
	    TSL_VASSERT_RV(p_tree_head != NULL, NULL);
	    tsl_list_add(&p_tree->list_node, &p_tree_head->list_head);
            p_tree->p_parent = p_tree_head;
        }

        if (p_node->type == XML_ELEMENT_NODE){
	    p_child = p_node->children;
	    while(p_child != NULL){
		if (p_child->type == XML_ELEMENT_NODE){
		    tr69_func_add_child_node2(p_child, p_tree, p_tree->full_name, pp_root_tree);
		}
		p_child = p_child->next;
	    }
        }

        return p_tree;
}

tr69_info_tree_t *tr69_func_filter_child_node(xmlNode *p_node, tr69_info_tree_t *p_tree_head, tsl_char_t *p_name, tr69_info_tree_t **pp_root_tree, tsl_int_t is_user_inst)
{
		tr69_info_tree_t *p_tree  = NULL;
		tsl_char_t		  type	  = 0;
		xmlChar 		 *p_char  = NULL;
		xmlNode 		 *p_child = NULL;
		tsl_int_t		 acl	  = 0;
		tsl_int_t		 nosave 	 = 0;
#ifdef AEI_PARAM_FORCEUPDATE
        tsl_int_t        forceupdate = 0;
#endif

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
		memset(p_tree, 0x0, sizeof(tr69_info_tree_t));

		switch(type){
		case TR69_NODE_TYPE_OBJECT:

		p_tree->node_type = type;
		p_char = xmlGetProp(p_node, (xmlChar *)"name");
		sprintf(p_tree->info.info_obj.name, "%s", p_char);
		printf("OBJ:%s %s\n", p_node->name, p_char);
		sprintf(p_tree->info.info_obj.short_name, "%s", p_char);
		xmlFree(p_char);

		if(p_name == NULL){
		sprintf(p_tree->full_name, "%s", p_tree->info.info_obj.name);
		}else {
		sprintf(p_tree->full_name, "%s.%s", p_name, p_tree->info.info_obj.name);
		}

			p_char = xmlGetProp(p_node, (xmlChar *)"profile");
			if (p_char){
					sprintf(p_tree->info.info_obj.profile, "%s", p_char);
					printf("profile:%s\n", p_char);
					xmlFree(p_char);
			}else {
					sprintf(p_tree->info.info_obj.profile, "%s", "tr69_func_global.c");
			}

			if (!strcmp(p_tree->info.info_obj.name, "InternetGatewayDevice")){
					p_char = xmlGetProp(p_node, (xmlChar *)"framework_xml_version");
					if (p_char){
							sprintf(g_version, "%s", p_char);
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
		if (is_user_inst == 0)
		{
#ifdef STD_CLEARUP_DEFAULT_INST
            //only get A.0.B.0.
			if (p_tree->info.info_inst.size != 0)
			{
				free(p_tree);
				p_tree = NULL;
				return NULL;
			}
#else
            //get A.0.B.1, A.0.B.2
            tsl_char_t oid[MAX_NAME_SIZE] = {0};
            sprintf(oid, "%s.%d", p_name, p_tree->info.info_inst.size);
            if (tr69_check_oid_is_default_inst(oid) == tsl_b_false)
            {
                free(p_tree);
				p_tree = NULL;
				return NULL;
            }
#endif
		}
		else
		{
			if (p_tree->info.info_inst.size == 0)
			{
				free(p_tree);
				p_tree = NULL;
				return NULL;
			}
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

		break;
		case TR69_NODE_TYPE_LEAF:
		p_tree->node_type = type;
#ifdef GEN_PROTYPE
		p_char = xmlGetProp(p_node, (xmlChar *)"specSource");
		if (p_char)
		{
			sprintf(p_tree->info.info_leaf.spec_source, "%s", p_char);
			xmlFree(p_char);
			p_tree->info.info_leaf.has_spec_source = 1;
		}
		else
		{
			p_tree->info.info_leaf.has_spec_source = 0;
		}

		p_char = xmlGetProp(p_node, (xmlChar *)"profile");
		if (p_char)
		{
			sprintf(p_tree->info.info_leaf.profile, "%s", p_char);
			xmlFree(p_char);
			p_tree->info.info_leaf.has_profile = 1;
		}
		else
		{
			p_tree->info.info_leaf.has_profile = 0;
		}

		p_char = xmlGetProp(p_node, (xmlChar *)"minValue");
		if (p_char){
				p_tree->info.info_leaf.has_range = 1;
				xmlFree(p_char);
		}
		else
		{
			p_tree->info.info_leaf.has_range = 0;
		}

		p_char = xmlGetProp(p_node, (xmlChar *)"nosave");
		if (p_char != NULL)
		{
			xmlFree(p_char);
			p_tree->info.info_leaf.has_nosave = 1;
		}
		else
		{
			p_tree->info.info_leaf.has_nosave = 0;
		}

		p_char = xmlGetProp(p_node, (xmlChar *)"hideParameterFromAcs");
		if (p_char)
		{
			xmlFree(p_char);
			p_tree->info.info_leaf.has_hideParameterFromAcs = 1;
		}
		else
		{
			p_tree->info.info_leaf.has_hideParameterFromAcs = 0;
		}

		p_char = xmlGetProp(p_node, (xmlChar *)"maxLength");
		if (p_char)
		{
			xmlFree(p_char);
			p_tree->info.info_leaf.has_maxLength = 1;
		}
		else
		{
			p_tree->info.info_leaf.has_maxLength = 0;
		}

		p_char = xmlGetProp(p_node, (xmlChar *)"type");
		sprintf(p_tree->info.info_leaf.typestr, "%s", p_char);
#endif
		p_char = xmlGetProp(p_node, (xmlChar *)"type");

		if (strstr((tsl_char_t *)p_char, "string")){
		p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_STRING;
		}else if (strstr((tsl_char_t *)p_char, "unsignedInt")){
		p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_UINT;
		}else if (strstr((tsl_char_t *)p_char, "int")){
		p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_INT;
		}else if (!strcmp((tsl_char_t *)p_char, "boolean")){
		p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_BOOL;
		}else if (!strcmp((tsl_char_t *)p_char, "dateTime")){
		p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_DATE;
		}else if (strstr((tsl_char_t *)p_char, "base64")){
				p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_STRING;
			}else if (strstr((tsl_char_t *)p_char, "hexBinary")){
				p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_STRING;
			}else if (strstr((tsl_char_t *)p_char, "unsignedLong")){
				p_tree->info.info_leaf.type = TR69_NODE_LEAF_TYPE_UINT;
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

#ifdef AEI_PARAM_FORCEUPDATE
        p_tree->info.info_leaf.forceupdate = 0;
        p_char = xmlGetProp(p_node, (xmlChar *)"forceupdate");
        if (p_char != NULL){
            forceupdate = atoi((char *)p_char);
            xmlFree(p_char);
            if (forceupdate){
                p_tree->info.info_leaf.forceupdate = forceupdate;
            }
        }
#endif

		p_char = xmlGetProp(p_node, (xmlChar *)"name");
		sprintf(p_tree->info.info_leaf.name, "%s", p_char);
		printf("LEAF:%s %s\n", p_node->name, p_char);
		xmlFree(p_char);

		if (p_name == NULL){
		sprintf(p_tree->full_name, "%s", p_tree->info.info_leaf.name);
		}else {
		sprintf(p_tree->full_name, "%s.%s", p_name, p_tree->info.info_leaf.name);
		}
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

			p_char = xmlGetProp(p_node, (xmlChar *)"default");
#if 0
			if (strcmp((tsl_char_t *)p_char, "(null)")){
					if (!strcmp((tsl_char_t *)p_char, "TRUE")||
			!strcmp((tsl_char_t *)p_char, "true")){
							sprintf(p_tree->info.info_leaf.def_value, "1");
					}else if (!strcmp((tsl_char_t *)p_char, "FALSE")||
				  !strcmp((tsl_char_t *)p_char, "false")){
							sprintf(p_tree->info.info_leaf.def_value, "0");
					}else {
							sprintf(p_tree->info.info_leaf.def_value, "%s", p_char);
					}
			}
#endif
			//sprintf(p_tree->info.info_leaf.def_value, "%s", p_char);
#ifdef STD_CLEARUP_NUMBEROFENTRIES
			if (strstr(p_tree->info.info_leaf.name, "NumberOfEntries") != NULL){
                //if (tr69_check_oid_is_user_inst(p_tree->full_name) == tsl_b_true){
                    sprintf(p_tree->info.info_leaf.def_value, "0");
                //}else{
                //    snprintf(p_tree->info.info_leaf.def_value, sizeof(p_tree->info.info_leaf.def_value), "%s", p_char);
                //}
                ctllog_debug("%s=%s\n", p_tree->full_name, p_tree->info.info_leaf.def_value);
            }else{
			    snprintf(p_tree->info.info_leaf.def_value, sizeof(p_tree->info.info_leaf.def_value), "%s", p_char);
            }
#else
            snprintf(p_tree->info.info_leaf.def_value, sizeof(p_tree->info.info_leaf.def_value), "%s", p_char);
#endif
			xmlFree(p_char);

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
					p_tree->info.info_leaf.min = atoi(p_char);
					xmlFree(p_char);
			}

			p_char = xmlGetProp(p_node, (xmlChar *)"maxValue");
			if (p_char){
					p_tree->info.info_leaf.max = atoll(p_char);
					xmlFree(p_char);
			}

			p_char = xmlGetProp(p_node, (xmlChar *)"maxLength");
			if (p_char){
					p_tree->info.info_leaf.max = (tsl_uint_t)atoi(p_char);
					xmlFree(p_char);
			}


			break;
		}

		TSL_INIT_LIST_HEAD(&p_tree->list_head);
		TSL_INIT_LIST_HEAD(&p_tree->list_node);

		if (p_tree_head == NULL && (*pp_root_tree) == NULL){
		*pp_root_tree = p_tree;
		}else {
		if ((p_tree->node_type != TR69_NODE_TYPE_OBJECT) &&
			(p_tree->node_type != TR69_NODE_TYPE_INSTANCE) &&
			(p_tree->node_type != TR69_NODE_TYPE_LEAF))
		{
			ctllog_debug("add child: node type is not object/instance/leaf (%02X)!!!!\n", p_tree->node_type);
		}
		TSL_VASSERT_RV(p_tree_head != NULL, NULL);
		tsl_list_add(&p_tree->list_node, &p_tree_head->list_head);
			p_tree->p_parent = p_tree_head;
		}

		if (p_node->type == XML_ELEMENT_NODE){
		p_child = p_node->children;
		while(p_child != NULL){
		if (p_child->type == XML_ELEMENT_NODE){
			tr69_func_filter_child_node(p_child, p_tree, p_tree->full_name, pp_root_tree, is_user_inst);
		}
		p_child = p_child->next;
		}
		}

		return p_tree;
}

#define DPRINTF(fmt, args...) do{if (g_debug) fprintf(stdout, fmt, ##args);}while(0);
tsl_rv_t tr69_inline_get_node(tr69_info_tree_t *p_root, tsl_char_t *p_name, tr69_info_tree_t **pp_find_node)
{
        tsl_list_head_t  *p_list = NULL;
        tr69_info_tree_t *p_tree = NULL;
		tsl_int_t root_name_len = 0;
		tsl_int_t name_len = 0;
DPRINTF("Frank: 1\n");

		TSL_VASSERT_RV(*pp_find_node == NULL, TSL_RV_SUC);
DPRINTF("Frank: 2\n");

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

	if ((p_name == NULL) || (*p_find == 1)) {
		if (*pp_next_node == NULL) {
			*pp_next_node = (tr69_node_t *)calloc(1, sizeof(tr69_node_t));
		}

		sprintf((*pp_next_node)->full_name, "%s", p_root->full_name);
		(*pp_next_node)->node_type = p_root->node_type;

		*p_find = 2;
		return TSL_RV_SUC;
	}

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

	for (p_list = p_root->list_head.prev; p_list != &p_root->list_head;) {
		p_tree = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
		tr69_inline_get_next_node2(p_tree, p_name, p_find, pp_next_node);
		TSL_VASSERT_RV(*p_find != 2, TSL_RV_SUC);
		p_list = p_list->prev;
	}

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
                tr69_inline_get_next_node2(p_root, (*pp_next_node)->full_name, &found, pp_next_node);
        }else {
                tr69_inline_get_next_node2(p_root, NULL, &found, pp_next_node);
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

xmlNode *tr69_func_inline_tree_to_protype(tr69_info_tree_t *p_root, xmlNode *p_parent_node)
{
        tsl_list_head_t *p_list = NULL;
        tr69_info_tree_t    *p_tree = NULL;
        xmlNode         *p_node = NULL;
        char tmp[256] = "\0";

        if (p_root->node_type == TR69_NODE_TYPE_OBJECT)
		{
		    printf("OBJECT %s %s\n", p_root->info.info_obj.name, p_root->full_name);
		    p_node = xmlNewNode(NULL, (xmlChar *)"Object");
		    xmlNewProp(p_node, (xmlChar *)"name", (xmlChar *)p_root->info.info_obj.name);
            xmlNewProp(p_node, (xmlChar *)"profile", (xmlChar *)p_root->info.info_obj.profile);
        }
		else if (p_root->node_type == TR69_NODE_TYPE_INSTANCE)
		{
	    	printf("INSTANCE %d %s\n", p_root->info.info_inst.size, p_root->full_name);
		    p_node = xmlNewNode(NULL, (xmlChar *)"Instance");

		    sprintf(tmp, "%d", p_root->info.info_inst.size);
		    xmlNewProp(p_node, (xmlChar *)"size", (xmlChar *)tmp);

		    switch (p_root->info.info_inst.acl)
			{
				case TR69_NODE_LEAF_ACL_WRITEONLY:
					xmlNewProp(p_node, (xmlChar *)"writable", (xmlChar *)"1");
					xmlNewProp(p_node, (xmlChar *)"readable", (xmlChar *)"0");
					break;
			    case TR69_NODE_LEAF_ACL_READONLY:
					xmlNewProp(p_node, (xmlChar *)"writable", (xmlChar *)"0");
					xmlNewProp(p_node, (xmlChar *)"readable", (xmlChar *)"1");
					break;
			    case TR69_NODE_LEAF_ACL_READWRITE:
					xmlNewProp(p_node, (xmlChar *)"writable", (xmlChar *)"1");
					xmlNewProp(p_node, (xmlChar *)"readable", (xmlChar *)"1");
					break;
				default:
					xmlNewProp(p_node, (xmlChar *)"writable", (xmlChar *)"0");
					xmlNewProp(p_node, (xmlChar *)"readable", (xmlChar *)"0");
					break;
		    }
		}
		else if (p_root->node_type == TR69_NODE_TYPE_LEAF)
		{
            printf("PARAMETER %s %s\n", p_root->info.info_leaf.name, p_root->full_name);

            p_node = xmlNewNode(NULL, (xmlChar *)"Parameter");

			xmlNewProp(p_node, (xmlChar *)"type", (xmlChar *)p_root->info.info_leaf.typestr);

            /*
	            if (strlen(p_root->info.info_leaf.def_value))
				{
	            	xmlNodeSetContent(p_node, (xmlChar *)p_root->info.info_leaf.def_value);
	            }
	            */
			if (p_root->info.info_leaf.has_spec_source)
			{//del
				xmlNewProp(p_node, (xmlChar *)"specSource", (xmlChar *)p_root->info.info_leaf.spec_source);
			}
			if (p_root->info.info_leaf.has_profile)
			{//del
				xmlNewProp(p_node, (xmlChar *)"profile", (xmlChar *)p_root->info.info_leaf.profile);
			}

			switch (p_root->info.info_leaf.acl)
			{
				case TR69_NODE_LEAF_ACL_WRITEONLY:
	                    xmlNewProp(p_node, (xmlChar *)"writable", (xmlChar *)"1");
						xmlNewProp(p_node, (xmlChar *)"readable", (xmlChar *)"0");
		                break;
			    case TR69_NODE_LEAF_ACL_READONLY:
	                    xmlNewProp(p_node, (xmlChar *)"writable", (xmlChar *)"0");
						xmlNewProp(p_node, (xmlChar *)"readable", (xmlChar *)"1");
	                    break;
			    case TR69_NODE_LEAF_ACL_READWRITE:
		                xmlNewProp(p_node, (xmlChar *)"writable", (xmlChar *)"1");
						xmlNewProp(p_node, (xmlChar *)"readable", (xmlChar *)"1");
	                    break;
				default:
		                xmlNewProp(p_node, (xmlChar *)"writable", (xmlChar *)"0");
						xmlNewProp(p_node, (xmlChar *)"readable", (xmlChar *)"0");
	                    break;
		    }
			if (p_root->info.info_leaf.has_nosave)
			{
				if (p_root->info.info_leaf.nosave == TR69_NODE_LEAF_ACL2_NOSAVE)
				{
					xmlNewProp(p_node, (xmlChar *)"nosave", (xmlChar *)"1");
				}
				else
				{
					xmlNewProp(p_node, (xmlChar *)"nosave", (xmlChar *)"0");
				}
			}

			xmlNewProp(p_node, (xmlChar *)"default", (xmlChar *)p_root->info.info_leaf.def_value);
			if (p_root->info.info_leaf.has_maxLength)
			{
				sprintf(tmp, "%d", p_root->info.info_leaf.max);
				xmlNewProp(p_node, (xmlChar *)"maxLength", (xmlChar *)tmp);
			}

			if (p_root->info.info_leaf.has_range)
			{
				if (p_root->info.info_leaf.type == TR69_NODE_LEAF_TYPE_UINT)
				{
					sprintf(tmp, "%u", p_root->info.info_leaf.min);
					xmlNewProp(p_node, (xmlChar *)"minValue", (xmlChar *)tmp);
					sprintf(tmp, "%u", p_root->info.info_leaf.max);
					xmlNewProp(p_node, (xmlChar *)"maxValue", (xmlChar *)tmp);
				}
				else
				{
					sprintf(tmp, "%d", p_root->info.info_leaf.min);
					xmlNewProp(p_node, (xmlChar *)"minValue", (xmlChar *)tmp);
					sprintf(tmp, "%d", p_root->info.info_leaf.max);
					xmlNewProp(p_node, (xmlChar *)"maxValue", (xmlChar *)tmp);
				}
			}

			if (p_root->info.info_leaf.notification == TR69_NODE_LEAF_NOTIFICATION)
			{
            	xmlNewProp(p_node, (xmlChar *)"notification", (xmlChar *)"1");
            }
			else if (p_root->info.info_leaf.notification == TR69_NODE_LEAF_ACTIVENOTIFICATION)
			{
				xmlNewProp(p_node, (xmlChar *)"notification", (xmlChar *)"2");
            }
			else
			{
            	xmlNewProp(p_node, (xmlChar *)"notification", (xmlChar *)"0");
            }

			if (p_root->info.info_leaf.has_hideParameterFromAcs)
			{
				if (p_root->info.info_leaf.attr == TR69_NODE_LEAF_ATTR_HIDDEN)
				{
	            	xmlNewProp(p_node, (xmlChar *)"hideParameterFromAcs", (xmlChar *)"true");
	            }
			}

            xmlNewProp(p_node, (xmlChar *)"name", (xmlChar *)p_root->info.info_leaf.name);
        }

        if (p_parent_node != NULL)
		{
	    	xmlAddChild(p_parent_node, p_node);
        }

        for(p_list = p_root->list_head.prev; p_list != &p_root->list_head;)
		{
		    p_tree = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
		    tr69_func_inline_tree_to_protype(p_tree, p_node);
		    p_list = p_list->prev;
        }
        return p_node;
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
                        strcat(tmp, "1");
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
                	if ( i != j -1 ){
                        strcat(tmp, "0");
                	}else {
                		if (atoi(oid_stack[i]) != 1)
						{
							//ctllog_debug("atoi(oid_stack[i])=%d\n", atoi(oid_stack[i]));
							return TSL_RV_ERR;
						}
                		strcat(tmp, "1");
                	}
                }
                if ( i != j -1 ){
                        strcat(tmp, ".");
                }
        }

        sprintf(p_new_name, "%s", tmp);

        printf("%s %d %s %s\n", __FUNCTION__, __LINE__, p_name, tmp);

        return TSL_RV_SUC;
}

//A.0.B.0.C.0
tsl_rv_t tr69_inline_parse_oid3(tsl_char_t *p_name, tsl_char_t *p_new_name)
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

//A.1.B.1.C.0
//A.1.B.2.C.0
tsl_rv_t tr69_inline_parse_oid4(tsl_char_t *p_name, tsl_char_t *p_new_name)
{
        tsl_char_t oid_stack[32][128] = {{"\0"}};
        tsl_char_t *ptr = NULL;
        tsl_char_t *match = NULL;
        tsl_char_t tmp[512] = "\0";
        tsl_int_t i = 0;
        tsl_int_t j = 0;
		tsl_int_t inst_index = -1;

        ptr = p_name;
        while((match = strchr(ptr, '.')) != NULL){
                memcpy(oid_stack[j++], ptr, match - ptr);
                ptr = match + 1;
        }
        memcpy(oid_stack[j++], ptr, p_name + strlen(p_name) - ptr);

        for (i = 0; i < j; i++){
				if (i == j-1) {
					if ((atoi(oid_stack[i])) || isdigit(oid_stack[i][0])){
						//strcat(tmp, "0");
						inst_index = atoi(oid_stack[i]);
					}else {
						strcat(tmp, oid_stack[i]);
					}
				}else {
					strcat(tmp, oid_stack[i]);
				}
                if ( i < j - 2 ){
                 	strcat(tmp, ".");
                }
        }

        sprintf(p_new_name, "%s", tmp);

        printf("%s %d %s %s\n", __FUNCTION__, __LINE__, p_name, tmp);

        return inst_index;
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
#if 0
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
#endif
        TSL_INIT_LIST_HEAD(&p_clone->list_head);
        TSL_INIT_LIST_HEAD(&p_clone->list_node);

        for(p_list = p_root->list_head.prev; p_list != &p_root->list_head;){

                p_tree = tsl_list_entry(p_list, tr69_info_tree_t, list_node);

#if 0
                if (p_tree->node_type == TR69_NODE_TYPE_INSTANCE &&
#ifndef SUPPORT_PROTYPE0
                    p_tree->info.info_inst.size != 1){
#else
                    p_tree->info.info_inst.size == 0){
#endif
                        p_list = p_list->prev;
                        continue;
                }
#else
				if (p_tree->node_type == TR69_NODE_TYPE_INSTANCE) {
					p_list = p_list->prev;
					continue;
				}
#endif
                p_new_tree = (tr69_info_tree_t *)calloc(1, sizeof(tr69_info_tree_t));
                p_new_tree->p_parent = p_clone;

                tr69_inline_clone_obj(p_tree, p_new_tree);

                tsl_list_add(&p_new_tree->list_node, &p_clone->list_head);
                p_list = p_list->prev;
        }
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
				if (p_inst->info.info_inst.size == 0){
					p_list = p_list->prev;
					continue;
				}
                if (p_inst->info.info_inst.size != inst){
                        tsl_list_add(&p_new_inst->list_node, &p_inst->list_node);
                        p_new_inst->info.info_inst.size = inst;
                        tr69_inline_adjust_para(p_new_inst, p_name);
						//ctllog_debug("        1 inst size= %d\n", p_new_inst->info.info_inst.size);
                        return TSL_RV_SUC;
                }
                inst++;

                p_list = p_list->prev;
        }

        tsl_list_add(&p_new_inst->list_node, &p_root->list_head);
        p_new_inst->info.info_inst.size = inst;
        tr69_inline_adjust_para(p_new_inst, p_name);
		//ctllog_debug("        2 inst size= %d\n", p_new_inst->info.info_inst.size);

        return TSL_RV_SUC;
}

tsl_rv_t tr69_inline_merge_data(tr69_info_tree_t *p_root, tr69_info_tree_t *p_ghost, tsl_char_t *p_name)
{
    tsl_list_head_t *p_list = NULL;
    tr69_info_tree_t *p_tree = NULL;
    tr69_info_tree_t *p_find_node = NULL;
    tsl_char_t oid[TR69_OID_FULL_NAME_SIZE] = {0};
    tsl_int_t i = 0;
    tsl_int_t num = 0;
    tsl_char_t *ptr = NULL;
    tsl_char_t *ptr2 = NULL;

    TSL_VASSERT(p_root != NULL);
    TSL_VASSERT(p_ghost != NULL);
    TSL_VASSERT(p_name != NULL);

    if (strstr(p_root->full_name, p_name) != NULL)
    {
        tr69_inline_get_node(p_ghost, p_root->full_name, &p_find_node);
    }
    else
    {
        ptr = p_name;
        while ((ptr2 = strchr(ptr, '.')) != NULL)
        {
            ptr = ptr2+1;
            num++;
        }
        num++;

        ptr = p_root->full_name;
        while ((ptr2 = strchr(ptr, '.')) != NULL)
        {
            ptr = ptr2+1;
            i++;
            if (i == num)
                break;
        }
        strcpy(oid, p_name);
        if (ptr2)
        {
            strcat(oid, ptr2);
        }
        //ctllog_debug("    oid=%s\n", oid);
        tr69_inline_get_node(p_ghost, oid, &p_find_node);
    }


    TSL_VASSERT(p_find_node != NULL);
    if (p_find_node == NULL)
    {
        ctllog_debug("  Couldn't find this node!!!(%s)\n", p_ghost->full_name);
        return TSL_RV_SUC;
    }

    if (p_root->node_type == TR69_NODE_TYPE_OBJECT)
    {

    }
    else if (p_root->node_type == TR69_NODE_TYPE_INSTANCE)
    {

    }
    else if (p_root->node_type == TR69_NODE_TYPE_LEAF)
    {
        memcpy(p_root->info.info_leaf.def_value, p_find_node->info.info_leaf.def_value, sizeof(p_find_node->info.info_leaf.def_value));
    }

    for (p_list=p_root->list_head.prev; p_list!=&p_root->list_head; p_list=p_list->prev)
    {
        p_tree = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
        tr69_inline_merge_data(p_tree, p_ghost, p_name);
    }

    return TSL_RV_SUC;
}

tsl_rv_t tr69_inline_replace_numtofmt(tsl_char_t *p_name, tsl_char_t *p_new_name)
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

        for (i = 0; i < j ; i++)
		{
                if ((atoi(oid_stack[i]) != 0) || (isdigit(oid_stack[i][0])))
				{
					strcat(tmp, "%d");
                }
				else
                {
					strcat(tmp, oid_stack[i]);
                }
                if ( i != j -1 ){
                        strcat(tmp, ".");
                }
        }

        sprintf(p_new_name, "%s", tmp);

        printf("%s %d %s %s\n", __FUNCTION__, __LINE__, p_name, tmp);

        return TSL_RV_SUC;
}

tsl_rv_t tr69_func_inline_tree_to_txt(FILE *fp, tr69_info_tree_t *p_root, tsl_int_t inst_index)
{
	tsl_list_head_t *p_list = NULL;
	tr69_info_tree_t	*p_tree = NULL;
	tsl_char_t *p_tmp = NULL;
	tsl_char_t *p_tmp2 = NULL;
	tsl_char_t new_name[256] = {0};

	if (p_root->node_type == TR69_NODE_TYPE_OBJECT)
	{
		//tr69_inline_replace_numtofmt(p_root->full_name, new_name);
		//fprintf(fp, "%s	(Obj)\n", new_name);
        fprintf(fp, "%s	(Obj)\n", p_root->full_name);
	}
	else if (p_root->node_type == TR69_NODE_TYPE_INSTANCE)
	{
		p_tmp = p_root->full_name;
		while ((p_tmp2 = strchr(p_tmp, '.')) != NULL)
		{
			p_tmp = p_tmp2 + 1;
		}
		if (atoi(p_tmp) != inst_index)
		{
		//	return TSL_RV_SUC;
		}
		//tr69_inline_replace_numtofmt(p_root->full_name, new_name);
		//fprintf(fp, "%s	(Inst)\n", new_name);
		fprintf(fp, "%s	(Inst)\n", p_root->full_name);
	}
	else if (p_root->node_type == TR69_NODE_TYPE_LEAF)
	{
		//tr69_inline_replace_numtofmt(p_root->full_name, new_name);
		//fprintf(fp, "%s	(Leaf)\n", new_name);
		fprintf(fp, "%s	(Leaf)\n", p_root->full_name);
	}

	for(p_list = p_root->list_head.prev; p_list != &p_root->list_head; p_list = p_list->prev)
	{
		p_tree = tsl_list_entry(p_list, tr69_info_tree_t, list_node);
		tr69_func_inline_tree_to_txt(fp, p_tree, inst_index);
	}
	return TSL_RV_SUC;
}

tsl_rv_t tr69_intf_mod_instance2(tr69_info_tree_t *p_root,  tsl_int_t rel)
{
        tsl_int_t  ret = -1;
        tsl_int_t  i = 0;
        tsl_int_t  j = 0;
        tsl_char_t oid_stack[32][128] = {{"\0"}};
        tsl_char_t *ptr = NULL;
        tsl_char_t *match = NULL;
        tsl_char_t parent_oid[256] = "\0";
        tsl_char_t numberofentries_oid[256] = "\0";
        tsl_char_t tmp_numberofentries_oid[256] = "\0";
        tsl_int_t  numb = 0;
		tr69_info_tree_t *p_tree = NULL;

        TSL_VASSERT(p_root != NULL);
        TSL_VASSERT(p_root->p_parent);
        TSL_VASSERT(p_root->p_parent->p_parent);

        ptr = p_root->full_name;
        if (tr69_check_oid_is_user_inst(ptr) == tsl_b_false){
            ctllog_debug("Not user instance!(%s)\n", p_root->full_name);
            return -1;
        }

        while((match = strchr(ptr, '.')) != NULL){
                memcpy(oid_stack[j++], ptr, match - ptr);
                ptr = match + 1;
        }
        memcpy(oid_stack[j++], ptr, p_root->full_name + strlen(p_root->full_name) - ptr);

        for (i=0; i<j-2; i++){
            strcat(numberofentries_oid, oid_stack[i]);
            strcat(numberofentries_oid, ".");
        }
        strcat(numberofentries_oid, oid_stack[i]);
        strcat(numberofentries_oid, "NumberOfEntries");
#ifdef SPECIAL_PARSE_INST
        ret = act_get_tr98_numberofentries(numberofentries_oid, tmp_numberofentries_oid);
        if (ret == 1){
            ctllog_debug("WARNING: maybe not exist in TR98(%s)!\n", numberofentries_oid);
            return TSL_RV_SUC;
        }else if (ret == 0){
            strncpy(numberofentries_oid, tmp_numberofentries_oid, sizeof(numberofentries_oid));
        }
#endif

        tr69_inline_get_node(p_root->p_parent->p_parent, numberofentries_oid, &p_tree);
        if (p_tree == NULL){
            //ctllog_debug("%s\n", p_root->full_name);
            ctllog_debug("Error: couldn't find NumberOfEntries(%s)!!!\n", numberofentries_oid);
            return -1;
        }
        TSL_VASSERT(p_tree->node_type == TR69_NODE_TYPE_LEAF);
        numb = atoi(p_tree->info.info_leaf.def_value) + rel;
        //ctllog_debug("p_tree->full_name=%s\n", p_tree->full_name);
        //ctllog_debug("p_root->full_name=%s\n", p_root->full_name);
        //ctllog_debug("numb=%d(%s, %d)\n", numb, p_tree->info.info_leaf.def_value, rel);
        //ctllog_debug("\n");

#ifdef STD_CLEARUP_NUMBEROFENTRIES
        snprintf(p_tree->info.info_leaf.def_value, sizeof(p_tree->info.info_leaf.def_value), "%d", numb);
#endif

        return TSL_RV_SUC;
}

tsl_rv_t tr69_func_add_instance0(tr69_info_tree_t *p_root, tsl_char_t *p_name, tsl_int_t *p_inst_numb)
{
	tr69_info_tree_t    *p_tree = NULL;
	tr69_info_tree_t    *p_parent = NULL;
	tr69_info_tree_t    *p_inst = NULL;
	tr69_info_tree_t    *p_new_inst = NULL;
	tsl_char_t          tmp[TR69_OID_FULL_NAME_SIZE] = "\0";
	tsl_char_t          new_name[TR69_OID_FULL_NAME_SIZE] = "\0";
	tsl_char_t          new_name_ghost[TR69_OID_FULL_NAME_SIZE] = "\0";

	TSL_VASSERT(p_root != NULL);
	TSL_VASSERT(p_name != NULL);
	TSL_VASSERT(p_inst_numb != NULL);

//ctllog_debug("p_name=%s\n", p_name);
	if (tr69_inline_parse_oid2(p_name, new_name) != TSL_RV_SUC)
	{
		ctllog_debug("\n");
		return 0;
	}
//ctllog_debug("  1p_name=%s\n", new_name);
	tr69_inline_get_node(p_root, new_name, &p_tree);
	if (p_tree == NULL)
	{
		tr69_inline_parse_oid(p_name, new_name);
//ctllog_debug("  2p_name=%s\n", new_name);
		tr69_inline_get_node(p_root, new_name, &p_tree);
	}
	TSL_VASSERT(p_tree != NULL);
	p_parent = p_tree->p_parent;
	TSL_VASSERT(p_parent != NULL);

//ctllog_debug("new_name=%s\n", new_name);

	tr69_inline_parse_oid(p_name, new_name_ghost);
	sprintf(tmp, "%s", new_name_ghost);

	tr69_inline_get_node(pg_root_tree, tmp, &p_inst);
	TSL_VASSERT(p_inst != NULL);

	p_new_inst = (tr69_info_tree_t *)calloc(1, sizeof(tr69_info_tree_t));
	TSL_VASSERT(p_new_inst != NULL);

	tr69_inline_clone_obj(p_inst, p_new_inst);

	p_new_inst->p_parent = p_parent;

//ctllog_debug("p_parent->full_name = %s\n", p_parent->full_name);

	tr69_inline_adjust_instance(p_parent, p_new_inst, p_parent->full_name);

	*p_inst_numb = p_new_inst->info.info_inst.size;

	return 0;
}

tsl_rv_t tr69_func_inline_tree_to_wholeprotype(tr69_info_tree_t *p_root, tsl_char_t *p_name)
{
	tr69_info_tree_t	*p_tree = NULL;
	tr69_info_tree_t	*p_parent = NULL;
	tr69_info_tree_t	*p_inst = NULL;
	tr69_info_tree_t	*p_new_inst = NULL;
	tsl_char_t			tmp[TR69_OID_FULL_NAME_SIZE] = "\0";
	tsl_char_t			new_name[TR69_OID_FULL_NAME_SIZE] = "\0";
	tsl_int_t 			inst_index = 0;

	TSL_VASSERT(p_root != NULL);
	TSL_VASSERT(p_name != NULL);

	inst_index = tr69_inline_parse_oid4(p_name, new_name);
	tr69_inline_get_node(p_root, new_name, &p_parent);
	TSL_VASSERT(p_parent != NULL);

	tr69_inline_parse_oid3(p_name, new_name);
	sprintf(tmp, "%s", new_name);

	tr69_inline_get_node(pg_root_tree, tmp, &p_inst);
	TSL_VASSERT(p_inst != NULL);

	p_new_inst = (tr69_info_tree_t *)calloc(1, sizeof(tr69_info_tree_t));
	TSL_VASSERT(p_new_inst != NULL);

	tr69_inline_clone_obj(p_inst, p_new_inst);

	p_new_inst->p_parent = p_parent;

	tr69_inline_adjust_instance(p_parent, p_new_inst, p_parent->full_name);
	if (inst_index != p_new_inst->info.info_inst.size)
	{
		ctllog_debug("Warning: inst size is wrong(%d, %d, %s)!!!!!!\n", inst_index, p_new_inst->info.info_inst.size, p_new_inst->full_name);
	}

    /*merge data */
    p_inst = NULL;
    tr69_inline_get_node(pg_root_tree, p_name, &p_inst);
    TSL_VASSERT(p_inst != NULL);
    sprintf(tmp, "%s", p_name);
    tr69_inline_merge_data(p_new_inst, p_inst->p_parent, tmp);

//#ifdef STD_CLEARUP_NUMBEROFENTRIES
	tr69_intf_mod_instance2(p_new_inst, 1);
//#endif
//	ctllog_debug("(%d, %s)\n\n", p_new_inst->info.info_inst.size, p_new_inst->full_name);

	return 0;
}


tsl_rv_t tr69_func_add_protype0(void)
{
	tr69_node_t *p_next_node = NULL;
	tsl_int_t inst_numb = 0;
	do
	{
    	tr69_get_next_node("InternetGatewayDevice", &p_next_node);
		if (p_next_node == NULL)
		{
			ctllog_debug("why p_next_node is null!\n");
			continue;
		}
		if (p_next_node->node_type == TR69_NODE_TYPE_INSTANCE)
		{
			//ctllog_debug("  %s\n", p_next_node->full_name);
			tr69_func_add_instance0(pg_root_tree2, p_next_node->full_name, &inst_numb);
		}
	}while (p_next_node);
	return 0;
}

tsl_rv_t tr69_func_tree_to_protype(tsl_char_t *p_file_name, tr69_info_tree_t *p_root)
{
	xmlDoc *doc = NULL;
	xmlNode *root_node = NULL;

	doc = xmlNewDoc((xmlChar *)"1.0");
	root_node = tr69_func_inline_tree_to_protype(p_root, root_node);

	xmlDocSetRootElement(doc, root_node);

	xmlSaveFormatFile(p_file_name, doc, 1);

	xmlFreeDoc(doc);
	xmlCleanupParser();

	return TSL_RV_SUC;
}


tsl_rv_t tr69_func_tree_to_txt(tsl_char_t *p_file_name, tr69_info_tree_t *p_root, tsl_int_t inst_index)
{
	FILE *fp = fopen(p_file_name, "wb+");
	if (fp == NULL)
	{
		ctllog_debug("create file failed!\n");
		return -1;
	}
	tr69_func_inline_tree_to_txt(fp, p_root, inst_index);
	fclose(fp);

	return TSL_RV_SUC;
}

tsl_rv_t tr69_func_tree_to_wholeprotype(tr69_info_tree_t *p_root)
{
	tr69_node_t *p_next_node = NULL;
	tsl_int_t inst_numb = 0;
	do
	{
    	tr69_get_next_node("InternetGatewayDevice", &p_next_node);
		if (p_next_node == NULL)
		{
			ctllog_debug("why p_next_node is null!\n");
			continue;
		}
		if (p_next_node->node_type == TR69_NODE_TYPE_INSTANCE)
		{
			if (tr69_check_oid_is_user_inst(p_next_node->full_name) == tsl_b_true)
				tr69_func_inline_tree_to_wholeprotype(p_root, p_next_node->full_name);
		}
	}while (p_next_node);

	return 0;
}

tsl_rv_t tr69_func_STD_CLEARUP_NUMBEROFENTRIES(tr69_info_tree_t *p_root)
{
    TSL_VASSERT(p_root != NULL);



    return 0;
}

tsl_rv_t tr69_func_parse_xml2(tr69_info_tree_t **pp_root_tree)
{
	xmlNode *root_node = NULL;
	xmlDoc *doc = NULL;
	tr69_info_tree_t *p_root;
	doc = xmlReadFile("protype.xml", NULL, 0);

	TSL_VASSERT(doc != NULL);
	root_node = xmlDocGetRootElement(doc);

	p_root = tr69_func_add_child_node2(root_node, NULL, NULL, pp_root_tree);

	xmlFreeDoc(doc);
	xmlCleanupParser();

	return TSL_RV_SUC;
}

tsl_rv_t tr69_func_filter_xml2(tr69_info_tree_t **pp_root_tree, tsl_int_t is_user_inst)
{
	xmlNode *root_node = NULL;
	xmlDoc *doc = NULL;
	tr69_info_tree_t *p_root;
	doc = xmlReadFile("protype.xml", NULL, 0);

	TSL_VASSERT(doc != NULL);
	root_node = xmlDocGetRootElement(doc);

	p_root = tr69_func_filter_child_node(root_node, NULL, NULL, pp_root_tree, is_user_inst);

	xmlFreeDoc(doc);
	xmlCleanupParser();

	return TSL_RV_SUC;
}


#define oid_to_fullname(MDM_OID, OID, args...) do {\
	vsprintf(MDM_OID, OID, ##args);\
	}while (0)


tsl_int_t main(tsl_int_t argc, tsl_char_t **argv)
{
	tr69_func_parse_xml2(&pg_root_tree);
	tr69_func_filter_xml2(&pg_root_tree2, 0);
	tr69_func_filter_xml2(&pg_root_tree3, 0);
    //remove not needed NumberOfEntries
    //...

	tr69_tree_to_head(pg_root_tree);
	//tr69_func_tree_to_xml("cfg.xml", pg_root_tree);
#if 0
	tr69_func_add_protype0();
	tr69_func_tree_to_protype("protype_save.xml", pg_root_tree2);
	tr69_func_tree_to_protype("protype_origin.xml", pg_root_tree);

	tr69_func_tree_to_txt("protype0.txt", pg_root_tree, 0);
	tr69_func_tree_to_txt("protype1.txt", pg_root_tree, 1);

#endif
	tr69_func_tree_to_wholeprotype(pg_root_tree3);

    //tr69_func_tree_to_protype("protype0.xml", pg_root_tree2);
    //tr69_func_STD_CLEARUP_NUMBEROFENTRIES(pg_root_tree3);
	tr69_func_tree_to_protype("protype_whole.xml", pg_root_tree3);

	tr69_intf_cleanup_tree(pg_root_tree);
	tr69_intf_cleanup_tree(pg_root_tree2);
	tr69_intf_cleanup_tree(pg_root_tree3);


	//system("diff -Nur auto.old auto > auto_patch");

	return 0;
}

