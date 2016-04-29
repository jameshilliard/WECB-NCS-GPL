#include "ctl.h"
#include "libtr69_server.h"
#include "tr69_func.h"
#include "tr69_inst.h"
#include "tsl_common.h"
#include "ctl_log.h"

int CTL_LOG_LEVEL = LOG_LEVEL_WARN;
tsl_char_t st_version[128] = "2011.1.1.0";
#define  MAX_CMD_LEN	128
extern tsl_rv_t tr69_cfg_init(tsl_char_t *p_protype_xml, tsl_char_t *p_cfg_xml, tsl_char_t *p_prev_cfg_xml, tsl_char_t *p_cache_cfg_xml);

typedef tsl_int_t (*tr69_func_get_obj_t)(tsl_char_t *p_oid_name, tsl_void_t *p_cur_data);
typedef tsl_int_t (*tr69_func_set_obj_t)(tsl_char_t *p_oid_name, tsl_void_t *p_cur_data, tsl_void_t *p_new_data);
struct tr69_register_func{
	 tsl_char_t oid_name[256];
	tr69_func_get_obj_t get_func;
	tr69_func_set_obj_t set_func;
};


struct tr69_register_func tr69_regfunc_tb[]={};

//tsl_rv_t tr69_cfg_init(tsl_char_t *p_protype_xml, tsl_char_t *p_cfg_xml, tsl_char_t *p_prev_cfg_xml, tsl_char_t *p_cache_cfg_xml);


/*
/mnt/rt_conf/pre_cfg.xml.gz is the permanent file of cfg.xml
/mnt/rt_conf/cache_cfg.xml.gz is imported to avoid damage of /mnt/rt_conf/pre_cfg.xml.gz .
//
*/
tsl_int_t main(tsl_int_t argc, tsl_char_t *argv[] )
{
	tsl_char_t *protype_f=PROTYPE_XML_FILE;
	tsl_char_t *cfg_f=CFG_XML_FILE;
	tsl_char_t *pre_cfg_file=PRE_CFG_XML_FILE;
	tsl_char_t *new_cfg_file=CUR_CFG_XML_FILE;
	tsl_char_t *cache_cfg_file=CACHE_CFG_XML_FILE;

	tsl_char_t cmd[MAX_CMD_LEN] = {"0"};
	tsl_int_t 	ret;
    tsl_bool_t needSave = TSL_B_FALSE;

	if(argc > 6)
	{
		ctllog_debug("==tr69_cfg: Too Much Arguments==%d\n",argc);
		return TSL_RV_ERR;
	}

	if(argc > 1 && argv[1])
		protype_f=argv[1];
	if(argc > 2 && argv[2])
		cfg_f=argv[2];
	if(argc > 3 && argv[3])
		pre_cfg_file=argv[3];
	if(argc > 4 && argv[4])
		new_cfg_file=argv[4];

	if( argc == 6)
	{
			ctllog_debug("===protype_f=%s,cfg_f=%s,pre_cfg_file=%s,new_cfg_file=%s, argv[5] = %s====\n",
				protype_f,cfg_f,pre_cfg_file,new_cfg_file, argv[5]);
			ret = tr69_cfg_init(protype_f, cfg_f, pre_cfg_file, argv[5]);
	}
	else
	{
		ctllog_debug("===protype_f=%s,cfg_f=%s,pre_cfg_file=%s,new_cfg_file=%s====\n",
				protype_f,cfg_f,pre_cfg_file,new_cfg_file);
		ret = tr69_cfg_init(protype_f, cfg_f, pre_cfg_file, NULL);
	}

	ctllog_debug("==tr69_cfg_init %d==\n",ret);

	//update cfg.xml
	if( TSL_RV_SUC == ret )
	{
	       tr69_register(tr69_inst_numb_tb, sizeof(tr69_inst_numb_tb)/sizeof(struct tr69_inst_numb_of_entries_s), 
	                      tr69_regfunc_tb, sizeof(tr69_regfunc_tb)/sizeof(struct tr69_register_func));	

	       tr69_save_xml(new_cfg_file);
		ctllog_debug("==update cfg file %s==\n",new_cfg_file);
        needSave = TSL_B_TRUE;
	}
	//no need merge cfg.xml, use last cfg.xml
	else if( TSL_RV_FAIL_FUNC == ret )
	{
		snprintf(cmd,MAX_CMD_LEN-1,"cp %s %s",pre_cfg_file,new_cfg_file);
		system(cmd);
		ctllog_debug("==tr69 cfg cp %s %sl==\n",pre_cfg_file,new_cfg_file);
	}
	//no need merge cfg.xml, use cached cfg.xml
	else if( TSL_RV_FAIL_NONE == ret )
	{
		snprintf(cmd,MAX_CMD_LEN-1,"cp %s %s",cache_cfg_file,new_cfg_file);
		system(cmd);
		ctllog_debug("==tr69 cfg cp %s %sl==\n",cache_cfg_file,new_cfg_file);
	}
	//must use default cfg.xml
	else// if( TSL_RV_FAIL == ret  )
	{
	       //tr69_save_xml(new_cfg_file);
		snprintf(cmd,MAX_CMD_LEN-1,"cp %s %s",cfg_f,new_cfg_file);
		system(cmd);
	       
		ctllog_debug("==tr69 cfg use default cfg.xml==\n");

        /* For some cases, need backup instance list and default cfg file  here.
         * e.g.: Upgrade to an image of different CUSTOMER_ID
        */
        //save current instance list file
        tsl_char_t cmd_line[128]={"0"};
        snprintf(cmd_line,sizeof(cmd_line)-1,"cp %s %s",DEF_INST_LIST_FILE,PRE_INST_LIST_FILE);
        system( cmd_line);

#ifdef AEI_PARAM_FORCEUPDATE
        //save current /etc/cfg.xml as previous version cfg file for upgrade using
        system(ZIP_COMMAND " -o " PREV_VERSION_CFG_XML_FILE " " CFG_XML_FILE);
#endif
        system("sync");
	}

       tr69_cleanup();	
		   
	ctllog_debug("==tr69 cfg done==\n");

    if( needSave ) {
        // create file /tmp/cfgsave to notice the caller tr69_lib to save cfg to flash
        system( "touch /tmp/cfgsave" );
    }
    return 0;
}
