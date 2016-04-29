#include "tr69_func.h"
#include <net/if.h>
#include <sys/ioctl.h>
#include "libtr69_func.h"
#include "tr69_cms_object.h"
#include "tsl_strconv.h"
#include "ctl_log.h"
#include "tr69_func_common.h"

#define MAX_FULL_NAME_LEN 256


tsl_char_t __g_cmd[BUFLEN_1024] = {0};     // global var for macro DO_SYSTEM()
tsl_char_t __g_strPara[BUFLEN_1024] = {0}; // global var for macro ACCESS_LEAF()

tsl_int_t tr69_common_func_parse_oid(tsl_char_t *p_name, tsl_char_t oid_stack[32][128], tsl_int_t *max_stack)
{
    tsl_char_t *ptr = NULL;
    tsl_char_t *match = NULL;
    tsl_int_t j = 0;

    memset(oid_stack, 0, 32*128);
    ptr = p_name;
    while((match = strchr(ptr, '.')) != NULL){
        memcpy(oid_stack[j++], ptr, match - ptr);
        ptr = match + 1;
    }        
    memcpy(oid_stack[j], ptr, p_name + strlen(p_name) - ptr);
    *max_stack = j;

    return 0;
}

tsl_int_t tr69_common_func_get_ip2(tsl_char_t *ipaddr, tsl_char_t *netmask, tsl_char_t *ethname)
{
    tsl_int_t fd, intrface;
    struct ifreq buf[16];
    struct ifconf ifc;

    //Coverity CID 16095: Side effect in assertion (ASSERT_SIDE_EFFECT)
    TSL_VASSERT((fd = socket(AF_INET, SOCK_DGRAM, 0)) != -1);

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = (caddr_t)buf;

    //TSL_VASSERT_ACT((ioctl(fd, SIOCGIFCONF, (char *)&ifc) >= 0), close(fd));

    intrface = ifc.ifc_len/sizeof(struct ifreq);

    while(intrface-- > 0){
        if (strstr(buf[intrface].ifr_name, ethname)){
            TSL_VASSERT_ACT((ioctl(fd, SIOCGIFADDR, (char *)&buf[intrface]) >= 0), close(fd));
            sprintf(ipaddr, "%s", inet_ntoa(((struct sockaddr_in *)(&buf[intrface].ifr_addr))->sin_addr));

            TSL_VASSERT_ACT((ioctl(fd, SIOCGIFNETMASK, (char *)&buf[intrface]) >= 0), close(fd));
            sprintf(netmask, "%s", inet_ntoa(((struct sockaddr_in *)(&buf[intrface].ifr_addr))->sin_addr));

            close(fd);
            return TSL_RV_SUC;
        }
    }

    close(fd);
    return TSL_RV_SUC;

}
tsl_int_t tr69_common_func_get_link_local_address(tsl_char_t *lladdr, tsl_char_t *ifname)
{
    tsl_int_t rv = TSL_RV_ERR;
    char cmd[100] = {0};

    FILE *fp = NULL;
    sprintf(cmd, "ip -6 address show %s",ifname);
    if ((fp = popen(cmd, "r")) != NULL)
    {
        char line[1024] = {0};
        while (fgets(line, sizeof(line), fp))
        {
            char* p = NULL;
            p = strstr(line, "scope link");
            if ( p != NULL)
            {
                p = strstr(line, "/");
                strcpy(p, "\0");
                p = strstr(line, "inet6");
                if(p != NULL)
                {
                    sprintf(lladdr, "%s", p+6);
                    rv = TSL_RV_SUC;
                    break;
                }
            }

        }
        pclose(fp);
    }

    return rv;
}

tsl_int_t tr69_common_func_get_ip(tsl_char_t *ipaddr, tsl_char_t *netmask, tsl_char_t *ethname)
{
    tsl_int_t fd;
    struct ifreq lan;

    //Coverity CID 16094: Side effect in assertion (ASSERT_SIDE_EFFECT)
    TSL_VASSERT((fd = socket(AF_INET, SOCK_DGRAM, 0)) != -1);

    snprintf(lan.ifr_name,sizeof(lan.ifr_name),"%s", ethname);
    //Coverity CID 17255: Resource leak 
    TSL_VASSERT_ACT((ioctl(fd, SIOCGIFADDR, (char *)&lan) >= 0), close(fd));

    sprintf(ipaddr, "%s", inet_ntoa(((struct sockaddr_in *)(&lan.ifr_addr))->sin_addr));

    TSL_VASSERT_ACT((ioctl(fd, SIOCGIFNETMASK, (char *)&lan) >= 0), close(fd));
    sprintf(netmask, "%s", inet_ntoa(((struct sockaddr_in *)(&lan.ifr_netmask))->sin_addr));

    close(fd);
    return TSL_RV_SUC;
}

tsl_int_t tr69_common_func_get_lan_state(tsl_char_t *ethname, tsl_int_t *state)
{
    tsl_int_t fd;
    struct ifreq intf;

    //Coverity CID 17257: Resource leak
    //Coverity CID 16096: Side effect in assertion (ASSERT_SIDE_EFFECT)
    TSL_VASSERT((fd = socket(AF_INET, SOCK_DGRAM, 0)) != -1);

    snprintf(intf.ifr_name,sizeof(intf.ifr_name),"%s", ethname);

    if (strchr(ethname, ':') != NULL ){
        if (ioctl(fd, SIOCGIFADDR, (char *)&intf) < 0){
            close(fd);
            *state = 0;
            return TSL_RV_SUC;
        }
    }

    if (ioctl(fd, SIOCGIFNETMASK, &intf) == -1){
        *state = 0;
    }else {
        *state = (intf.ifr_flags & IFF_UP)?1:0;
    } 
    close(fd);

    return TSL_RV_SUC;
}

static void free_node(tr69_node_t **node)
{
    if(!node || !*node)
        return;
    free(*node);
    *node = NULL;
}

tsl_rv_t oid_to_iidstack(char *oid,tr69_oid_stack_id *p_iidStack)
{
    tsl_char_t oid_stack[32][128] = {{"\0"}};
    tsl_int_t max_stack;
    tsl_char_t *match = NULL;
    tsl_int_t j = 0;
    tsl_int_t i = 0;
    char *ptr = oid;
    while((match = strchr(ptr, '.')) != NULL){
        memcpy(oid_stack[j++], ptr, match - ptr);
        ptr = match + 1;
    }        
    memcpy(oid_stack[j++], ptr, oid + strlen(oid) - ptr);
    max_stack = j;

    j = 0;
    for (i = 0; i < max_stack; i++){
        if (atoi(oid_stack[i])){
            p_iidStack->instance[p_iidStack->currentDepth++]=atoi(oid_stack[i]);
        }
    }

    return TSL_RV_SUC;
}

static int oid_get_depth(tr69_oid oid)
{
    int d_num = 0;
    char *ptr = NULL, *match = NULL;
    /* count the number of ".%d." */
    ptr = oid;
    while((match = strstr(ptr, ".%d.")) != NULL)
    {
        d_num++;
        ptr = match + strlen(".%d.");
    }
    return d_num;
}

static char* oid_transformer1(tr69_oid oid, tr69_oid_stack_id *iidStack)
{
    static char tmp_oid[MAX_FULL_NAME_LEN] = {0};
    int tmp_depth = iidStack->currentDepth;
    char *ptr = NULL, *match = NULL;

    snprintf(tmp_oid, MAX_FULL_NAME_LEN,"%s",oid);

    ptr = tmp_oid;
    while((match = strstr(ptr, ".%d.")) != NULL)
    {
        tmp_depth--;
        if(tmp_depth < 0)
        {
            match[1] = '0';
            match[2] = '0';
        }
        ptr = match + strlen(".%d.");
    }

    return tmp_oid;
}

static char* oid_transformer2(tr69_oid oid, tr69_oid_stack_id *iidStack)
{
    static char tmp_oid[MAX_FULL_NAME_LEN] = {0};
    char *ptr = NULL, *match = NULL;

    snprintf(tmp_oid, MAX_FULL_NAME_LEN,"%s",oid);

    ptr = tmp_oid;
    while((match = strstr(ptr, ".00.")) != NULL)
    {
        match[1] = '%';
        match[2] = 'd';
        ptr = match + strlen(".%d.");
    }

    return tmp_oid;
}

static char *oid_to_fullname_ext(char *MDM_OID, char *OID, ...)
{
    va_list args;
    va_start(args, OID);
    vsnprintf(MDM_OID, MAX_FULL_NAME_LEN, OID, args);
    va_end(args);

    return MDM_OID;
}

/*static */char *oid_to_fullname_ByIid(char *full_name, char *obj_oid,
        tr69_oid_stack_id *iidStack)
{
    if(iidStack->currentDepth == 6)
    {
        return oid_to_fullname_ext(full_name,
                obj_oid, 
                iidStack->instance[0] , 
                iidStack->instance[1], 
                iidStack->instance[2], 
                iidStack->instance[3], 
                iidStack->instance[4], 
                iidStack->instance[5]); 
    }
    else if(iidStack->currentDepth == 5)
    {
        return oid_to_fullname_ext(full_name,
                obj_oid, 
                iidStack->instance[0] , 
                iidStack->instance[1], 
                iidStack->instance[2], 
                iidStack->instance[3], 
                iidStack->instance[4],iidStack->instance[5]); 
    }
    else if(iidStack->currentDepth == 4)
    {
        return oid_to_fullname_ext(full_name,
                obj_oid, 
                iidStack->instance[0] , 
                iidStack->instance[1], 
                iidStack->instance[2], 
                iidStack->instance[3],iidStack->instance[4]); 
    }
    else if(iidStack->currentDepth == 3)
    {
        return oid_to_fullname_ext(full_name,
                obj_oid, 
                iidStack->instance[0] , 
                iidStack->instance[1], 
                iidStack->instance[2],iidStack->instance[3]); 
    }
    else if(iidStack->currentDepth == 2)
    {
        return oid_to_fullname_ext(full_name,
                obj_oid, 
                iidStack->instance[0] , 
                iidStack->instance[1],iidStack->instance[2]); 
    }
    else if(iidStack->currentDepth == 1)
    {
        return oid_to_fullname_ext(full_name,
                obj_oid, 
                iidStack->instance[0],iidStack->instance[1]); 
    }
    else if(iidStack->currentDepth == 0)
    {
        return oid_to_fullname_ext(full_name,
                obj_oid); 
    }
    else
    {
        ctllog_error("%s: invalid iidStack = %s\n",
                __FUNCTION__, tr69_dump_stackid(iidStack));
        return NULL;
    }
}

static char *oid_to_fullname(tr69_oid obj_oid, tr69_oid_stack_id *iidStack, char *param)
{
    static char full_name[MAX_FULL_NAME_LEN] = {0};
    char tmp_obj_oid[MAX_FULL_NAME_LEN];
    int obj_data_size = 0;
    tsl_rv_t ret;
    int d_num = oid_get_depth(obj_oid);

    snprintf(tmp_obj_oid, MAX_FULL_NAME_LEN, "%s", obj_oid);

    if(iidStack->currentDepth < d_num)
    {
        ctllog_error("%s: failed to get full_name for oid = %s, iidStack = %s, iidStack is not enough!\n",
                __FUNCTION__, tmp_obj_oid, tr69_dump_stackid(iidStack));
        return NULL;
    }

    oid_to_fullname_ByIid(full_name, tmp_obj_oid, iidStack);

    if((ret = tr69_get_obj_data_size(full_name, &obj_data_size)) != TSL_RV_SUC)
    {
        ctllog_error("%s: tr69_get_obj_data_size for %s error, oid = %s, iidStack = %s, ret = %d!\n",
                __FUNCTION__, full_name, tmp_obj_oid, tr69_dump_stackid(iidStack), ret);
        return NULL;
    }

    /* if the obj_data_size == 0, it means it is the parent 
     * of a multiple instance object, so append the %d at the end of oid*/
    if(obj_data_size == 0)
    {
        tsl_strncat(tmp_obj_oid, ".%d", MAX_FULL_NAME_LEN);
        d_num++;

        if(iidStack->currentDepth < d_num)
        {
            ctllog_error("%s: failed to get full_name for oid = %s, iidStack = %s, iidStack is not enough!!\n",
                    __FUNCTION__, tmp_obj_oid, tr69_dump_stackid(iidStack));
            return NULL;
        }

        oid_to_fullname_ByIid(full_name, tmp_obj_oid, iidStack);

        /* check the multiple instance existed or not */
        if((ret = tr69_get_obj_data_size(full_name, &obj_data_size)) != TSL_RV_SUC)
        {
            ctllog_error("%s: tr69_get_obj_data_size for %s error, oid = %s, iidStack = %s, ret = %d!!\n",
                    __FUNCTION__, full_name, tmp_obj_oid, tr69_dump_stackid(iidStack), ret);
            return NULL;
        }

        /* impossible */
        if(obj_data_size == 0)
        {
            ctllog_error("%s: tr69_get_obj_data_size for %s error, oid = %s, iidStack = %s, size is zero\n",
                    __FUNCTION__, full_name, tmp_obj_oid, tr69_dump_stackid(iidStack));
            return NULL;
        }
    }

    /* the object or multiple instance must be existed if get here,
     * append the parameter name if it is specified */
    if(param && *param != '\0')
    {
        tsl_strncat(full_name, ".", MAX_FULL_NAME_LEN);
        tsl_strncat(full_name, param, MAX_FULL_NAME_LEN);
    }

    if(!strstr(full_name, "InternetGatewayDevice")&&!strstr(full_name, "Device"))
    {
        ctllog_error("%s: oid = %s, iidStack = %s, full_name = %s!\n",
                __FUNCTION__, obj_oid, tr69_dump_stackid(iidStack),
                full_name);
        return NULL;
    }

    return full_name;
}

static char *oid_get_comm_part(tr69_oid parent_oid, tr69_oid oid, char *comm_oid,int size)
{
	char *match = NULL, *ptr = NULL;
	char part_name[MAX_FULL_NAME_LEN] = {0};
	int len = 0;
	memset(comm_oid, 0x0, size);

    /* find the common root name */
    ptr = parent_oid;
    while((match = strchr(ptr, '.')) != NULL)
    {
        memset(part_name, 0x0, MAX_FULL_NAME_LEN);
        strncpy(part_name, ptr, match - ptr);
        if(!strstr(oid, part_name))
        {
        	len = ptr - parent_oid;
		if( len > size )
		{
			len = size-1;
		}
            strncpy(comm_oid, parent_oid, len);
            break;
        }
        ptr = match + 1;
    }

    /* check the last part */
    if(comm_oid[0] == '\0')
    {
        memset(part_name, 0x0, MAX_FULL_NAME_LEN);
        strncpy(part_name, ptr, parent_oid + tsl_strlen(parent_oid) - ptr);
        if(!strstr(oid, part_name))
        {
              len = ptr - parent_oid;
		if( len > size )
		{
			len = size-1;
		}
            strncpy(comm_oid, parent_oid, len);
        }
    }

    /* assume parent_oid and oid are matched */
    if(comm_oid[0] == '\0')
    {
        ptr = parent_oid + tsl_strlen(parent_oid);
	len = ptr - parent_oid;
	if( len > size )
	{
		len = size-1;
	}
	 strncpy(comm_oid, parent_oid, len);
    }

    /* get rid of the last dot */
    if(comm_oid[tsl_strlen(comm_oid)-1] == '.')
        comm_oid[tsl_strlen(comm_oid)-1] = '\0';

    return ptr;
}

tsl_rv_t fullname_compare_oid(char *full_name,int node_type, char *oid, tr69_oid_stack_id *p_iidStack)
{
    tsl_char_t oid_stack[32][128];
    tsl_int_t max_stack;
    tsl_char_t tmp_oid[512] = "\0";
    tsl_char_t *ptr = NULL;
    tsl_char_t *match = NULL;
    tsl_int_t j = 0;
    tsl_int_t i = 0;

    tr69_oid_stack_id tmpStack = EMPTY_INSTANCE_ID_STACK;
    tmpStack.currentDepth = p_iidStack->currentDepth;
    tmpStack.instance[0] = p_iidStack->instance[0];
    tmpStack.instance[1] = p_iidStack->instance[1];
    tmpStack.instance[2] = p_iidStack->instance[2];
    tmpStack.instance[3] = p_iidStack->instance[3];
    tmpStack.instance[4] = p_iidStack->instance[4];
    tmpStack.instance[5] = p_iidStack->instance[5];

    //ctllog_debug("before set tmp_oid=%s oid=%s tmpstack=%s iidstack=%s\n",tmp_oid,oid,tr69_dump_stackid(&tmpStack),tr69_dump_stackid(p_iidStack));
    memset(oid_stack, 0, 32*128);
    ptr = full_name;
    while((match = strchr(ptr, '.')) != NULL){
        memcpy(oid_stack[j++], ptr, match - ptr);
        ptr = match + 1;
    }        
    memcpy(oid_stack[j++], ptr, full_name + strlen(full_name) - ptr);
    max_stack = j;

    j = 0;
    for (i = 0; i < max_stack; i++){
        if (atoi(oid_stack[i])){
            if(i + 1 < max_stack)
            {
                tsl_strncat(tmp_oid, "%d",sizeof(tmp_oid));
            }
            p_iidStack->instance[j++] = atoi(oid_stack[i]);
        }else {
            tsl_strncat(tmp_oid, oid_stack[i],sizeof(tmp_oid));
        }
        if(i + 2 < max_stack)
        {
            tsl_strncat(tmp_oid, ".",sizeof(tmp_oid));
        }
        else if(i +1 < max_stack && node_type == TR69_NODE_TYPE_OBJECT)
        {
            tsl_strncat(tmp_oid,".",sizeof(tmp_oid));
        }

    }

    p_iidStack->currentDepth = j;
    tsl_32_t ret = tr69_compare_stackid(&tmpStack,p_iidStack);

    if(tsl_strcmp(oid, tmp_oid) || ret >=0)
    {
        memcpy(p_iidStack,&tmpStack,sizeof(tr69_oid_stack_id));
        return 1;
    }

    return 0;
}

tsl_rv_t tr69_oid_to_fullpath(tr69_path_desc *pathDesc, char **fullpath)
{
    char tmp_oid[256] = "\0";
    char *tmp_path = malloc(256*sizeof(char));

    memset(tmp_path,0,256);
    snprintf(tmp_oid,sizeof(tmp_oid),"%s.%s",pathDesc->oid,"%d");

    oid_to_fullname_ByIid(tmp_path, tmp_oid, &(pathDesc->iidStack));

    *fullpath = tmp_path;
    return TSL_RV_SUC;
}

//Coverity comments 20206
tsl_rv_t tr69_fullpath_to_oid(const char *fullpath, tr69_path_desc *pathDesc)
{
    memset(pathDesc->oid,'\0',100);
    INIT_INSTANCE_ID_STACK(&(pathDesc->iidStack));
    tsl_char_t oid_stack[32][128];
    tsl_int_t max_stack;
    const tsl_char_t *ptr = NULL;
    tsl_char_t *match = NULL;
    tsl_int_t j = 0;
    tsl_int_t i = 0;

    memset(oid_stack, 0, 32*128);
    ptr = fullpath;
    while((match = strchr(ptr, '.')) != NULL){
        memcpy(oid_stack[j++], ptr, match - ptr);
        ptr = match + 1;
    }        
    memcpy(oid_stack[j++], ptr, fullpath + strlen(fullpath) - ptr);
    max_stack = j;

    j = 0;
    for (i = 0; i < max_stack; i++){
        if (atoi(oid_stack[i])){
            if(i + 1 < max_stack)
            {
                tsl_strncat(pathDesc->oid, "%d",sizeof(pathDesc->oid));
            }
            pathDesc->iidStack.instance[j++] = atoi(oid_stack[i]);
        }else {
            tsl_strncat(pathDesc->oid, oid_stack[i],sizeof(pathDesc->oid));
        }
        if(i + 2 < max_stack)
        {
            tsl_strncat(pathDesc->oid, ".",sizeof(pathDesc->oid));
        }
    }

    if (tsl_strcmp(&(pathDesc->oid[strlen(pathDesc->oid) - 3]), ".%d") == 0)
    {
        pathDesc->oid[strlen(pathDesc->oid)-3] = '\0';
    }
    pathDesc->iidStack.currentDepth = j;
    return TSL_RV_SUC;
}

tsl_32_t tr69_compare_stackid(const tr69_oid_stack_id *iidStack1, const tr69_oid_stack_id *iidStack2)
{
    if(iidStack1->currentDepth == iidStack2->currentDepth)
    {
        if(iidStack1->currentDepth > 0)
        {
            return (iidStack1->instance[iidStack1->currentDepth-1] - iidStack2->instance[iidStack2->currentDepth-1]);
        }
        else
        {
            return 0;
        }

    }
    else
    {
        return (iidStack1->currentDepth - iidStack2->currentDepth);
    }
}
char *tr69_dump_stackid(const tr69_oid_stack_id *iidStack)
{

    static char rtnValue[256] = "\0";

    snprintf(rtnValue,sizeof(rtnValue),"{%hu,{%lu,%lu,%lu,%lu,%lu,%lu}}",iidStack->currentDepth,
            iidStack->instance[0], 
            iidStack->instance[1], 
            iidStack->instance[2], 
            iidStack->instance[3], 
            iidStack->instance[4], 
            iidStack->instance[5]); 
    return rtnValue;
}

tsl_rv_t tr69_get_obj_by_oid(tr69_oid oid, tr69_oid_stack_id *iidStack, int getFlags, void **obj)
{
    char OID_fullname[256] = "\0";

    if(iidStack->currentDepth > 0)
    {
        char tmp_fullnamebef[256] = "\0";
        oid_to_fullname_ByIid(tmp_fullnamebef, oid, iidStack);
        char tmp_fullnameafter[256] = "\0";
        char tmp_oid[256] = "\0";

        snprintf(tmp_oid,sizeof(tmp_oid),"%s.%s",oid,"%d");
        
        oid_to_fullname_ByIid(tmp_fullnameafter, tmp_oid, iidStack); 

        if((tmp_fullnameafter[strlen(tmp_fullnameafter) -1]) == '0' && (tmp_fullnameafter[strlen(tmp_fullnameafter) -2] == '.'))
        {
            tsl_strncpy(OID_fullname,tmp_fullnamebef,256);
        }
        else
        {
            tsl_strncpy(OID_fullname,tmp_fullnameafter,256);
        }
    }
    else
    {
        oid_to_fullname_ByIid(OID_fullname, oid, iidStack); 
    }

    if(getFlags == 0)
    {
        return tr69_get_obj_data(OID_fullname, obj);
    }
    else
    {
        return tr69_get_unfresh_obj_data(OID_fullname, obj);

    }
}

tsl_rv_t tr69_set_unfresh_obj_by_oid(tr69_oid oid, void *obj, tr69_oid_stack_id *iidStack)
{
    char OID_fullname[256] = "\0";

    if(iidStack->currentDepth > 0)
    {
        char tmp_fullnamebef[256] = "\0";
        oid_to_fullname_ByIid(tmp_fullnamebef, oid, iidStack); 
        char tmp_fullnameafter[256] = "\0";
        char tmp_oid[256] = "\0";

        snprintf(tmp_oid,sizeof(tmp_oid),"%s.%s",oid,"%d");

        oid_to_fullname_ByIid(tmp_fullnameafter, tmp_oid, iidStack); 

        if((tmp_fullnameafter[strlen(tmp_fullnameafter) -1]) == '0' && (tmp_fullnameafter[strlen(tmp_fullnameafter) -2] == '.'))
        {
            tsl_strncpy(OID_fullname,tmp_fullnamebef,256);
        }
        else
        {
            tsl_strncpy(OID_fullname,tmp_fullnameafter,256);
        }
    }
    else
    {
        oid_to_fullname_ByIid(OID_fullname, oid, iidStack); 
    }

    return tr69_set_unfresh_obj_data(OID_fullname, &obj);
}

//CmsRet tr69_set_obj_by_oid(void *obj, tr69_oid_stack_id iidStack)
tsl_rv_t tr69_set_obj_by_oid(tr69_oid oid, void *obj, tr69_oid_stack_id *iidStack)
{
    char OID_fullname[256] = "\0";

    if(iidStack->currentDepth > 0)
    {
        char tmp_fullnamebef[256] = "\0";
        oid_to_fullname_ByIid(tmp_fullnamebef, oid, iidStack); 
        char tmp_fullnameafter[256] = "\0";
        char tmp_oid[256] = "\0";

        snprintf(tmp_oid,sizeof(tmp_oid),"%s.%s",oid,"%d");

        oid_to_fullname_ByIid(tmp_fullnameafter, tmp_oid, iidStack); 

        if((tmp_fullnameafter[strlen(tmp_fullnameafter) -1]) == '0' && (tmp_fullnameafter[strlen(tmp_fullnameafter) -2] == '.'))
        {
            tsl_strncpy(OID_fullname,tmp_fullnamebef,256);
        }
        else
        {
            tsl_strncpy(OID_fullname,tmp_fullnameafter,256);
        }
    }
    else
    {
        oid_to_fullname_ByIid(OID_fullname, oid, iidStack); 
    }

    return tr69_set_obj_data(OID_fullname, &obj);
}

//CmsRet cmsObj_free(void **obj)
tsl_rv_t tr69_free_obj_by_oid(tr69_oid oid, tr69_oid_stack_id *iidStack, void **obj)
{
    char OID_fullname[256] = "\0";

    if(iidStack->currentDepth > 0)
    {
        char tmp_fullnamebef[256] = "\0";
        oid_to_fullname_ByIid(tmp_fullnamebef, oid, iidStack); 
        char tmp_fullnameafter[256] = "\0";
        char tmp_oid[256] = "\0";

        snprintf(tmp_oid,sizeof(tmp_oid),"%s.%s",oid,"%d");

        oid_to_fullname_ByIid(tmp_fullnameafter, tmp_oid, iidStack); 

        if((tmp_fullnameafter[strlen(tmp_fullnameafter) -1]) == '0' && (tmp_fullnameafter[strlen(tmp_fullnameafter) -2] == '.'))
        {
            tsl_strncpy(OID_fullname,tmp_fullnamebef,256);
        }
        else
        {
            tsl_strncpy(OID_fullname,tmp_fullnameafter,256);
        }
    }
    else
    {
        oid_to_fullname_ByIid(OID_fullname, oid, iidStack); 
    }

    return tr69_free_obj_data(OID_fullname, obj);
}

static char *fullname_to_oid(char* full_name, tr69_oid_stack_id *p_iidStack)
{
    static tsl_char_t oid[MAX_FULL_NAME_LEN];
    tsl_char_t oid_stack[32][128];
    tsl_int_t max_stack;
    tsl_char_t *ptr = NULL;
    tsl_char_t *match = NULL;
    tsl_int_t j = 0;
    tsl_int_t i = 0;

    memset(oid, 0x0, MAX_FULL_NAME_LEN);
    memset(oid_stack, 0, 32*128);

    ptr = full_name;
    while((match = strchr(ptr, '.')) != NULL){
        memcpy(oid_stack[j++], ptr, match - ptr);
        ptr = match + 1;
    }        
    memcpy(oid_stack[j++], ptr, full_name + strlen(full_name) - ptr);
    max_stack = j;

    j = 0;
    for (i = 0; i < max_stack; i++){
        if (atoi(oid_stack[i])){
            tsl_strncat(oid, "%d", MAX_FULL_NAME_LEN);
            p_iidStack->instance[j++] = atoi(oid_stack[i]);/*for instance object last id*/
        }else {
            tsl_strncat(oid, oid_stack[i], MAX_FULL_NAME_LEN);
        }

        tsl_strncat(oid, ".", MAX_FULL_NAME_LEN);
    }
    p_iidStack->currentDepth = j;

    if(!tsl_strcmp(oid + tsl_strlen(oid) - tsl_strlen(".%d."), ".%d."))
    {
        char *p = oid + strlen(oid) - strlen(".%d.");
        *p = '\0';/* no obj/ins oid ends with ".%d." */
    }

    if(oid[tsl_strlen(oid) - 1] == '.')
        oid[tsl_strlen(oid) - 1] = '\0';/* no obj/ins oid ends with "." */

    return oid;
}

tsl_rv_t tr69_get_obj_by_oid_flag(char* full_name, tsl_u32_t getFlags, void **obj)
{
    tsl_rv_t rv;

    if(getFlags == 0)
        rv = tr69_get_obj_data(full_name, obj);
    else
        rv = tr69_get_unfresh_obj_data(full_name, obj);

    if(!(rv == TSL_RV_SUC && *obj != NULL))
    {
        if(rv != TSL_RV_SUC)
            ctllog_error("%s: get full_name = %s, error(%d)\n",
                    __FUNCTION__, full_name, rv);
        rv = CMSRET_NO_MORE_INSTANCES;
    }

    return rv;
}

tsl_rv_t tr69_get_next_inst_by_fullpath(char *tmp_name, tr69_node_t **p_next_ins_node)
{
    tsl_rv_t rv = TSL_RV_SUC;
    char tail_name[MAX_FULL_NAME_LEN] = {0};
    char *match = NULL;

    for(; rv == TSL_RV_SUC && (match = strstr(tmp_name, ".%d"));)
    {
        /* get the tail */
        snprintf(tail_name,MAX_FULL_NAME_LEN,"%s", match + tsl_strlen(".%d") );

        /* truck the tail */
        *match = '\0';

        if((rv = tr69_get_next_inst_node(tmp_name, p_next_ins_node)) != TSL_RV_SUC
                || *p_next_ins_node == NULL)
        {
            if(rv == TSL_RV_ERR && p_next_ins_node == NULL)
            {
                rv = CMSRET_NO_MORE_INSTANCES;
                goto Exit;
            }
            else
            {
                ctllog_error("%s: tr69_get_next_inst_node for %s error, ret = %d!\n",
                        __FUNCTION__, tmp_name, rv);
                rv = TSL_RV_FAIL;
                goto Exit;
            }
        }

        if((*p_next_ins_node)->node_type != TR69_NODE_TYPE_INSTANCE)
        {
            ctllog_error("%s: p_next_ins_node %s is not a instance\n",
                    __FUNCTION__, (*p_next_ins_node)->full_name);
            rv = TSL_RV_FAIL;
            goto Exit;
        }

        /* get it */
        snprintf(tmp_name,MAX_FULL_NAME_LEN,"%s", (*p_next_ins_node)->full_name);
        /* append the tail */
        tsl_strncat(tmp_name, tail_name, MAX_FULL_NAME_LEN);
        /* Free the node to get the node of next level */
        free_node(p_next_ins_node);
    }

Exit:
    return rv;
}

tsl_rv_t tr69_get_next_inst_by_oid(tr69_oid obj_oid, tr69_oid_stack_id *iidStack,
        char *full_name)
{
    tsl_rv_t rv = TSL_RV_FAIL;
    char tmp_name[MAX_FULL_NAME_LEN] = {0};
    int obj_data_size = 0;
    tr69_node_t *p_next_ins_node = NULL;
    tr69_oid_stack_id tmp_iidStack = EMPTY_INSTANCE_ID_STACK;
    char *tmp_oid = NULL;

    if(!full_name)
    {
        ctllog_error("%s: full_name is NULL!\n", __FUNCTION__);
        goto Error;
    }

    if(iidStack->currentDepth == 0)
    {
        /* First Time */
        memset(iidStack, 0x0, sizeof(tr69_oid_stack_id));
        snprintf(tmp_name,MAX_FULL_NAME_LEN,"%s",obj_oid);
        if((rv = tr69_get_next_inst_by_fullpath(tmp_name, &p_next_ins_node))
                != TSL_RV_SUC)
            goto Error;
    }
    else
    {
        /* TODO */
        ctllog_error("%s: iidStack is not empty!\n", __FUNCTION__);
        goto Error;
    }

    /* check obj/ins existed */
    if((rv = tr69_get_obj_data_size(tmp_name, &obj_data_size)) != TSL_RV_SUC)
    {
        ctllog_error("%s: tr69_get_obj_data_size for %s error, ret = %d!\n",
                __FUNCTION__, tmp_name, rv);
        goto Error;
    }

    /* if the obj_data_size == 0, it means it is the parent 
     * of a multiple instance object*/
    if(obj_data_size == 0)
    {
        free_node(&p_next_ins_node);

        if((rv = tr69_get_next_inst_node(tmp_name, &p_next_ins_node)) != TSL_RV_SUC
                || p_next_ins_node == NULL)
        {
//            ctllog_error("%s: tr69_get_next_inst_node for %s error, ret = %d!\n",
//                    __FUNCTION__, tmp_name, rv);
            goto Error;
        }

        snprintf(tmp_name,MAX_FULL_NAME_LEN,"%s", p_next_ins_node->full_name);
    }

    /* get iidStack and oid */
    if((tmp_oid = fullname_to_oid(tmp_name, &tmp_iidStack)) == NULL)
    {
        rv = TSL_RV_FAIL;
        ctllog_error("%s: fullname_to_oid for %s error\n", __FUNCTION__, tmp_name);
        goto Error;
    }

    snprintf(full_name,MAX_FULL_NAME_LEN,"%s", tmp_name);
    memcpy(iidStack, &tmp_iidStack, sizeof(tr69_oid_stack_id));

    free_node(&p_next_ins_node);
    return rv;

Error:
    free_node(&p_next_ins_node);
    *full_name = '\0';
    return rv;
}

static tsl_rv_t tr69_get_next_inst_by_oid_flag(tr69_oid obj_oid,
        tr69_oid_stack_id *iidStack, tsl_u32_t getFlags, void **obj)
{
    tsl_rv_t rv;
    char full_name[MAX_FULL_NAME_LEN] = {0};

    if((rv = tr69_get_next_inst_by_oid(obj_oid, iidStack, full_name)) != TSL_RV_SUC
            || *full_name == '\0')
    {
//        ctllog_error("%s: Can't get net obj oid = %s, iidStack = %s\n",
//                __FUNCTION__, obj_oid, tr69_dump_stackid(iidStack));
        return rv;
    }

    rv = tr69_get_obj_by_oid_flag(full_name, getFlags, obj);

    return rv;
}

#define ITER_SAME_DEPTH 1
#define ITER_NEXT_INST 2
#define ITER_NEXT_INSTOBJ 3
static tsl_rv_t tr69_get_iter_by_oid(char *root_name, tr69_node_t *p_next_node, tr69_oid obj_oid, tr69_oid_stack_id *iidStack, tsl_u32_t getFlags, int cmp_flag, void **obj, int iter_type)
{
    tsl_rv_t rv = CMSRET_NO_MORE_INSTANCES;
    char *tmp_oid, tmp_obj_oid[MAX_FULL_NAME_LEN] = {0};
    tr69_oid_stack_id tmp_iidStack = EMPTY_INSTANCE_ID_STACK;

    do {
        /* if p_next_node is valided, then continue to search,
         * if p_next_node is NULL, then search from scratch */
        if(iter_type == ITER_SAME_DEPTH)
        {
            snprintf(tmp_obj_oid,MAX_FULL_NAME_LEN,"%s", obj_oid);
            if(p_next_node->node_type == TR69_NODE_TYPE_INSTANCE)
//Coverity comment 19471				
                tsl_strncat(tmp_obj_oid, ".%d", MAX_FULL_NAME_LEN);

            rv = tr69_get_next_samedepth_node(tmp_obj_oid, &p_next_node);

            if(rv != TSL_RV_SUC && rv != TSL_RV_ERR_PARM)
                rv = CMSRET_NO_MORE_INSTANCES;
            else if(rv != TSL_RV_SUC)
                ctllog_error("%s: obj_oid = %s, p_next_node = %s, error = %d\n",
                        __FUNCTION__, tmp_obj_oid, p_next_node->full_name, rv);
        }
        else if(iter_type == ITER_NEXT_INST)
        {
            if(root_name == NULL)
                return TSL_RV_FAIL;
            rv = tr69_get_next_inst_node(root_name, &p_next_node);
        }
        else if(iter_type == ITER_NEXT_INSTOBJ)
        {
            rv = tr69_get_next_inst_by_oid_flag(obj_oid, iidStack, getFlags, obj);
            return rv;
        }

        if(rv != TSL_RV_SUC)
            break;

        if(!p_next_node)
        {
            rv = CMSRET_NO_MORE_INSTANCES;
            break;
        }

        memset(&tmp_iidStack, 0x0, sizeof(tr69_oid_stack_id));
        tmp_oid = fullname_to_oid(p_next_node->full_name, &tmp_iidStack);

        if((rv = tr69_get_obj_by_oid_flag(p_next_node->full_name, getFlags, obj))
                == TSL_RV_SUC)
        {
            memcpy(iidStack, &tmp_iidStack, sizeof(tr69_oid_stack_id));
            break;
        }
    }while(p_next_node != NULL);

    if(p_next_node)
        free(p_next_node);

    return rv;
}

tsl_rv_t tr69_check_obj_by_oid(tr69_oid oid, tr69_oid_stack_id *iidStack,
        char *full_name, tr69_node_t **p_next_node)
{
    tsl_rv_t rv;
    char *ptr = oid_to_fullname(oid, iidStack, "");

    if(ptr == NULL)
    {
        ctllog_error("%s: get oid(%s), iidStack(%s), get full_name error!\n",
                __FUNCTION__, oid, tr69_dump_stackid(iidStack));
        return TSL_RV_FAIL;
    }

    snprintf(full_name,MAX_FULL_NAME_LEN,"%s", ptr);

    if(full_name[tsl_strlen(full_name) - 1] == '.')
        full_name[tsl_strlen(full_name) - 1] = '\0';

    if((rv = tr69_get_next_node(full_name, p_next_node)) != TSL_RV_SUC
            || *p_next_node == NULL)
    {
        ctllog_error("%s: get full_name(%s), oid(%s), iidStack(%s), error(%d)!\n",
                __FUNCTION__, full_name?:"NULL", oid, tr69_dump_stackid(iidStack), rv);
        return TSL_RV_FAIL;
    }
    else
    {
        return rv;
    }
}

tsl_rv_t tr69_valid_param_by_oid(tr69_oid oid, tr69_oid_stack_id *iidStack)
{
    if(oid == NULL || *oid == '\0' )
    {
        ctllog_error("%s: invalid oid!\n", __FUNCTION__);
        return CMSRET_INVALID_PARAM_NAME;   
    }

    if(iidStack == NULL)
    {
        ctllog_error("%s: invalid iidStack!\n", __FUNCTION__);
        return CMSRET_INVALID_PARAM_VALUE;   
    }

    return TSL_RV_SUC;
}

tsl_rv_t tr69_get_next_obj_by_oid_flag(tr69_oid oid, tr69_oid_stack_id *iidStack, tsl_u32_t getFlags,void **obj)
{
    tsl_rv_t rv = CMSRET_NO_MORE_INSTANCES;
    tr69_node_t *p_next_node = NULL;
    char full_name[MAX_FULL_NAME_LEN] = {0};

    if((rv = tr69_valid_param_by_oid(oid, iidStack)) != TSL_RV_SUC)
        return rv;

    /* check if the iid is valided */
    if(iidStack->currentDepth > 0)
    {
        if((rv = tr69_check_obj_by_oid(oid, iidStack,
                        full_name, &p_next_node)) == TSL_RV_SUC)
            rv = tr69_get_iter_by_oid(NULL, p_next_node,
                    oid, iidStack, getFlags, 0, obj, ITER_SAME_DEPTH);
    }
    else
    {
        memset(iidStack, 0x0, sizeof(tr69_oid_stack_id));
        rv = tr69_get_iter_by_oid(NULL, p_next_node,
                oid, iidStack, getFlags, 0, obj, ITER_NEXT_INSTOBJ);
    }


    if(rv != TSL_RV_SUC && rv != CMSRET_NO_MORE_INSTANCES)
        ctllog_debug("%s: oid = %s, full_name = %s, oid = %s, iidStack = %s, rv = %d\n",
                __FUNCTION__, oid, full_name, oid, tr69_dump_stackid(iidStack), rv);

    return rv;
}

tsl_rv_t tr69_get_next_obj_by_oid(tr69_oid oid, tr69_oid_stack_id *iidStack, void **obj)
{
    return tr69_get_next_obj_by_oid_flag(oid, iidStack,0, obj);
}

tsl_rv_t tr69_get_next_sub_obj_by_oid_flag(tr69_oid oid, tr69_oid_stack_id *ParentiidStack, tr69_oid_stack_id *iidStack, tsl_u32_t getFlags, void **obj)
{
    tsl_rv_t rv = CMSRET_NO_MORE_INSTANCES;
    char full_name[MAX_FULL_NAME_LEN] = {0},
         tmp_name[MAX_FULL_NAME_LEN] = {0},
         tmp_oid[MAX_FULL_NAME_LEN] = {0};
    tr69_node_t *p_next_node = NULL;

    if(((rv = tr69_valid_param_by_oid(oid, iidStack)) != TSL_RV_SUC)
            || ((rv = tr69_valid_param_by_oid(oid, ParentiidStack)) != TSL_RV_SUC))
        return rv;

    if(ParentiidStack->currentDepth <= 0)
    {
        ctllog_error("%s: ParentiidStack is invalid\n", __FUNCTION__);
        return TSL_RV_FAIL;
    }

    snprintf(tmp_oid,MAX_FULL_NAME_LEN,"%s", oid_transformer1(oid, ParentiidStack));

    oid_to_fullname_ByIid(tmp_name, tmp_oid, ParentiidStack);

    snprintf(tmp_oid,MAX_FULL_NAME_LEN,"%s",oid_transformer2(tmp_name, ParentiidStack));

    if(iidStack->currentDepth > 0)
    {
        if((rv = tr69_check_obj_by_oid(oid, iidStack,
                        full_name, &p_next_node)) == TSL_RV_SUC)
            rv = tr69_get_iter_by_oid(NULL, p_next_node,
                    tmp_oid, iidStack, getFlags, 0, obj, ITER_SAME_DEPTH);
    }
    else
    {
        //memcpy(iidStack, ParentiidStack, sizeof(tr69_oid_stack_id));
        memset(iidStack, 0x0, sizeof(tr69_oid_stack_id));
        rv = tr69_get_iter_by_oid(NULL, p_next_node,
                tmp_oid, iidStack, getFlags, 0, obj, ITER_NEXT_INSTOBJ);
    }

    if(rv != TSL_RV_SUC && rv != CMSRET_NO_MORE_INSTANCES)
        ctllog_debug("%s: tmp_oid = %s, full_name = %s, oid = %s, iidStack = %s, rv = %d\n",
                __FUNCTION__, tmp_oid, full_name, oid, tr69_dump_stackid(iidStack), rv);

    return rv;
}

tsl_rv_t tr69_get_next_sub_obj_by_oid(tr69_oid oid, tr69_oid_stack_id *ParentiidStack, tr69_oid_stack_id *iidStack, void **obj)
{
    return tr69_get_next_sub_obj_by_oid_flag(oid, ParentiidStack, iidStack,0, obj);
}

tsl_rv_t tr69_get_parent_obj_by_oid(tr69_oid parent_oid, tr69_oid oid, tr69_oid_stack_id *iidStack, void **obj)
{
    return tr69_get_parent_obj_by_oid_flag(parent_oid,oid,iidStack,0,obj);
}

tsl_rv_t tr69_get_parent_obj_by_oid_flag(tr69_oid parent_oid, tr69_oid oid, tr69_oid_stack_id *iidStack,tsl_u32_t getFlags, void **obj)
{
    tsl_rv_t rv = CMSRET_NO_MORE_INSTANCES;
    char comm_oid[MAX_FULL_NAME_LEN] = {0}, 
         root_name[MAX_FULL_NAME_LEN] = {0}, *ptr = NULL;
    tr69_node_t *p_next_node = NULL;

    if(((rv = tr69_valid_param_by_oid(oid, iidStack)) != TSL_RV_SUC)
            || ((rv = tr69_valid_param_by_oid(parent_oid, iidStack)) != TSL_RV_SUC))
        return rv;

    ptr = oid_get_comm_part(parent_oid, oid, comm_oid,sizeof(comm_oid));
    snprintf(root_name,MAX_FULL_NAME_LEN,"%s",oid_to_fullname(comm_oid, iidStack, ""));

    if(ptr < parent_oid + tsl_strlen(parent_oid))
    {
        tsl_strncat(root_name, ".",sizeof(root_name));
	//don't use tsl_strncat, because "parent_oid + tsl_strlen(parent_oid) - ptr" is not dest length
        strncat(root_name, ptr, parent_oid + tsl_strlen(parent_oid) - ptr);
    }

    memset(iidStack, 0x0, sizeof(tr69_oid_stack_id));

    rv = tr69_get_iter_by_oid(NULL, p_next_node,
            root_name, iidStack, getFlags, 0, obj, ITER_NEXT_INSTOBJ);

    if(rv != TSL_RV_SUC && rv != CMSRET_NO_MORE_INSTANCES)
        ctllog_error("%s: root_name = %s, parent_oid = %s, iidStack = %s, rv = %d\n",
                __FUNCTION__, root_name, parent_oid, tr69_dump_stackid(iidStack), rv);

    return rv;
}

tsl_rv_t tr69_add_inst_by_oid(tr69_oid oid, tr69_oid_stack_id *iidStack)
{
    char OID_fullname[256] = "\0";
    int inst_numb = 0;

    if(iidStack->currentDepth == 0)
    {
        oid_to_fullname_ext(OID_fullname, oid,1,1,1,1,1,1);
    }
    else
    {
        oid_to_fullname_ByIid(OID_fullname, oid, iidStack); 
    }

    tsl_rv_t ret = tr69_add_instance(OID_fullname, &inst_numb);

    if( ret == TSL_RV_SUC)
    {
        if(iidStack->currentDepth > 0)
        {
            iidStack->instance[iidStack->currentDepth++] = inst_numb;
        }
        else
        {
            iidStack->instance[0] = inst_numb;
            iidStack->currentDepth = 1;
        }
    }

    return ret;
}

tsl_rv_t tr69_del_inst_by_oid(tr69_oid oid, tr69_oid_stack_id *iidStack)
{
    char OID_fullname[256] = "\0";

    if(iidStack->currentDepth > 0)
    {
        char tmp_oid[256] = "\0";

        snprintf(tmp_oid,sizeof(tmp_oid),"%s.%s",oid,"%d");

        oid_to_fullname_ByIid(OID_fullname, tmp_oid, iidStack); 
    }
    else
    {
        oid_to_fullname_ByIid(OID_fullname, oid, iidStack); 
    }

    return tr69_del2_instance(OID_fullname);
}

char * tr69_oid_to_fullname(tr69_oid obj_oid, tr69_oid_stack_id *iidStack, char *param)
{
    static char full_name[MAX_FULL_NAME_LEN] = {0};
    char tmp_obj_oid[MAX_FULL_NAME_LEN] = {0};
    int obj_data_size = 0;
    tsl_rv_t ret;
    int d_num = oid_get_depth(obj_oid);

    memset(full_name, 0x0, MAX_FULL_NAME_LEN);

    snprintf(tmp_obj_oid, MAX_FULL_NAME_LEN, "%s", obj_oid);

    if(iidStack->currentDepth < d_num)
    {
        ctllog_error("%s: failed to get full_name for oid = %s, iidStack = %s, iidStack is not enough!\n",
                __FUNCTION__, tmp_obj_oid, tr69_dump_stackid(iidStack));
        return NULL;
    }

    oid_to_fullname_ByIid(full_name, tmp_obj_oid, iidStack);

    if((ret = tr69_get_obj_data_size(full_name, &obj_data_size)) != TSL_RV_SUC)
    {
        ctllog_error("%s: tr69_get_obj_data_size for %s error, oid = %s, iidStack = %s, ret = %d!\n",
                __FUNCTION__, full_name, tmp_obj_oid, tr69_dump_stackid(iidStack), ret);
        return NULL;
    }

    /* if the obj_data_size == 0, it means it is the parent 
     * of a multiple instance object, so append the %d at the end of oid*/
    if(obj_data_size == 0)
    {
        tsl_strncat(tmp_obj_oid, ".%d", MAX_FULL_NAME_LEN);
        d_num++;

        if(iidStack->currentDepth < d_num)
        {
            ctllog_error("%s: failed to get full_name for oid = %s, iidStack = %s, iidStack is not enough!!\n",
                    __FUNCTION__, tmp_obj_oid, tr69_dump_stackid(iidStack));
            return NULL;
        }

        oid_to_fullname_ByIid(full_name, tmp_obj_oid, iidStack);

        /* check the multiple instance existed or not */
        if((ret = tr69_get_obj_data_size(full_name, &obj_data_size)) != TSL_RV_SUC)
        {
            ctllog_error("%s: tr69_get_obj_data_size for %s error, oid = %s, iidStack = %s, ret = %d!!\n",
                    __FUNCTION__, full_name, tmp_obj_oid, tr69_dump_stackid(iidStack), ret);
            return NULL;
        }

        /* impossible */
        if(obj_data_size == 0)
        {
            ctllog_error("%s: tr69_get_obj_data_size for %s error, oid = %s, iidStack = %s, size is zero\n",
                    __FUNCTION__, full_name, tmp_obj_oid, tr69_dump_stackid(iidStack));
            return NULL;
        }
    }

    /* the object or multiple instance must be existed if get here,
     * append the parameter name if it is specified */
    if(param && *param != '\0')
    {
//Coverity comment 19472
        tsl_strncat(full_name, ".", MAX_FULL_NAME_LEN);
        tsl_strncat(full_name, param, MAX_FULL_NAME_LEN);
    }

    if( (!strstr(full_name, "Device")) && (!strstr(full_name, "InternetGatewayDevice")))
    {
        ctllog_error("%s: oid = %s, iidStack = %s, full_name = %s!\n",
                __FUNCTION__, obj_oid, tr69_dump_stackid(iidStack),
                full_name);
        return NULL;
    }
    return full_name;
}

/*
 * Function name: tr69_common_func_get_serial_number
 * Input paras:
 *             char* sn: buffer to store serial number
 *             int sn_len: buffer size
 * Return value:
 *             int: TSL_B_TRUE for success and TSL_B_FALSE for failure.
 */
int tr69_common_func_get_serial_number(char* sn, int sn_len)
{
    char* pValue = NULL;
    int type;
    int rv = TSL_B_FALSE;

    if(TSL_RV_SUC == (ACCESS_LEAF(tr69_get_unfresh_leaf_data, (void **)&pValue, &type, "%s.SerialNumber", TR69_OID_DEVICE_INFO)))
    {
        memset(sn, 0, sn_len);
        snprintf(sn, sn_len, "%s", pValue);
        CTLMEM_FREE_BUF_AND_NULL_PTR(pValue);
        rv = TSL_B_TRUE;
    }
    else
    {
        rv = TSL_B_FALSE;
    }

    return rv;
}

