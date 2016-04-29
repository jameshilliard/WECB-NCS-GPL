#include "tsl_common.h"
#include "tsl_strconv.h"
#include "ctl_log.h"
#include "dbussend_msg.h"
#include "dbus_define.h"
#include "ctl_msg.h"
#include "tr69_cms_object.h"
#include "accesscfg.h"
#include "ctl.h"

static tsl_int_t 	upload_cfg = 0;
static tsl_void_t processCfgDownLoading(CtlMsgHeader *p_msg);
static tsl_rv_t prepareCfgBackup(tsl_char_t * filein,tsl_char_t *fileout);

tsl_rv cfg_save()
{
	tsl_int_t ret;
	ctllog_debug( "enter cfg_save()\n" );
	if( 0 == upload_cfg )
	{
	    	tsl_char_t cmd_line[128]={"0"};
			
       	snprintf(cmd_line,sizeof(cmd_line)-1,"cp %s %s",CUR_CFG_XML_FILE,PRE_CFG_XML_FILE);
		ret = system( cmd_line);
		if( ret != -1 )
		{
			ret = system( ZIP_COMMAND" "PRE_CFG_XML_FILE );
			if( ret != -1 )
			{
				//save cfg.xml temporarily
				snprintf(cmd_line,sizeof(cmd_line)-1,"mv %s %s",TMPZIP_CFG_FULLPATH,ZIP_CACHE_CFG_FULLPATH);
				ret = system(cmd_line);
				if( ret != -1 )
				{
					system("sync");	
			
					//save it to permanent file
		       		snprintf(cmd_line,sizeof(cmd_line)-1,"cp %s %s",ZIP_CACHE_CFG_FULLPATH,ZIP_CFG_FULLPATH);
					system(cmd_line);
					system("sync");	
					
					ctllog_debug( "cfg_save():%d \n",ret );
				}
				else
				{
					ctllog_error( "cfg_save() fail to mv %s\n",ZIP_CFG_FULLPATH);
				}
			}
			else
			{
				ctllog_error( "cfg_save() fail to zip %s\n",PRE_CFG_XML_FILE);
			}
		}
		else
		{
			ctllog_error( "cfg_save() fail to %s\n", cmd_line);
		}
        //snprintf(cmd_line, sizeof(cmd_line), "rm -f %s", CUR_CFG_XML_FILE);
        //system(cmd_line);
        snprintf(cmd_line, sizeof(cmd_line), "rm -f %s", PRE_CFG_XML_FILE);
        system(cmd_line);
        snprintf(cmd_line, sizeof(cmd_line), "rm -f %s", TMPZIP_CFG_FULLPATH);
        system(cmd_line);
	}
	else
	{
		ctllog_debug( "cfg_save() do nothing\n" );
	}

	return TSL_RV_SUC;
}

tsl_rv cfg_restore_default()
{
    	tsl_char_t cmd_line[128]={"0"};
       snprintf(cmd_line,sizeof(cmd_line)-1,"cp %s %s",CFG_XML_FILE,CUR_CFG_XML_FILE);
	system( cmd_line);
	ctllog_notice( "restore default cfg.xml!" );
	return TSL_RV_SUC;
}

//update cfg.xml when system started
tsl_rv cfg_restore()
{
	ctllog_debug( "enter cfg_restore() \n" );
    	tsl_char_t cmd_line[BUFLEN_256]={"0"};
	tsl_rv_t rv;
    tsl_bool_t needSave = TSL_B_FALSE;

	//prepare last saving cfg.xml
	rv = prepareCfgBackup(ZIP_CFG_FULLPATH,TMPZIP_CFG_FULLPATH);

	if( TSL_RV_SUC == rv )
	{
		//prepare last cache cfg.xml
		rv = prepareCfgBackup(ZIP_CACHE_CFG_FULLPATH,TMPZIP_CACHE_CFG_FULLPATH);

		if( TSL_RV_SUC == rv )
		{
	  		snprintf(cmd_line,sizeof(cmd_line)-1,"%s %s %s %s %s %s",
		        		TR69_CFG,PROTYPE_XML_FILE,CFG_XML_FILE,PRE_CFG_XML_FILE,CUR_CFG_XML_FILE,CACHE_CFG_XML_FILE);
		}
		else
		{
	  		snprintf(cmd_line,sizeof(cmd_line)-1,"%s %s %s %s %s",
		        		TR69_CFG,PROTYPE_XML_FILE,CFG_XML_FILE,PRE_CFG_XML_FILE,CUR_CFG_XML_FILE);
		}
		//call tr69_cfg to generate cfg.xml
        system(cmd_line );

        // If tr69_cfg decide to save cfg to flash now, it will create a file /tmp/cfgsave
        // to notice the caller tr69_lib to do cfg_save().
        // e.g. for forceupdate case, need save cfg to flash.
        if( access( "/tmp/cfgsave", F_OK) ) {
            needSave = TSL_B_TRUE;
            unlink( "/tmp/cfgsave" );
        }
	}
	else
	{
       	cfg_restore_default();
	}
    snprintf(cmd_line, sizeof(cmd_line), "rm -f %s", TMPZIP_CFG_FULLPATH);
    system(cmd_line);
    snprintf(cmd_line, sizeof(cmd_line), "rm -f %s", TMPZIP_CACHE_CFG_FULLPATH);
    system(cmd_line);
    snprintf(cmd_line, sizeof(cmd_line), "rm -f %s", PRE_CFG_XML_FILE);
    system(cmd_line);
    //snprintf(cmd_line, sizeof(cmd_line), "rm -f %s", CUR_CFG_XML_FILE);
    //system(cmd_line);
    snprintf(cmd_line, sizeof(cmd_line), "rm -f %s", CACHE_CFG_XML_FILE);
    system(cmd_line);    
    ctllog_debug("========end restore==========\n");

    if( needSave ) {
        return cfg_save();
    }

	return TSL_RV_SUC;
}

tsl_rv do_cfg_access( DBusConnection *connection, DBusMessage *message, void *user_data, CtlMsgHeader * p_msg,
		const tsl_char_t * methodType,  tsl_bool bMethodCall )
{
	//DBusMessage *reply = NULL;
	//char *reply_str = NULL;
	tsl_rv result = TSL_RV_ERR;

	do {
		//ctllog_debug( "enter do_cfg_access()\n");

		if( !tsl_strcmp( methodType, CTL_METHOD_SAVE) ) {
			result = cfg_save();
		} else if( !tsl_strcmp(methodType, CTL_METHOD_RESTORE )) {
			result = cfg_restore();
		} else if( !tsl_strcmp(methodType, CTL_METHOD_RESTORE_DEFAULT )) {
			result = cfg_restore_default();
			ctllog_debug( "RTD recv CTL_METHOD_RESTORE_DEFAULT from Data-Center." );
		}

#if 0
		if( TSL_RV_SUC != result ) {
			reply_str = DBUS_REPLY_FAIL;
		} else {
			reply_str = DBUS_REPLY_SUCC;
		}

		// if method call, reply msg
		// if signal, no reply
		if( TSL_B_TRUE == bMethodCall ) {
			reply = dbus_message_new_method_return (message);
			if (NULL == reply) {
				ctllog_error( "No memory, dbus_message_new_method_return\n" );
				break;
			}
			if (!dbus_message_append_args (reply,
						DBUS_TYPE_STRING, &reply_str,
						DBUS_TYPE_INVALID))
			{
				ctllog_error( "No memory, dbus_message_append_args\n" );
				break;
			}
			if (!dbus_connection_send (connection, reply, NULL))
			{
				ctllog_error( "No memory, dbus_connection_send\n" );
				break;
			}
		}
#endif
	} while(0);
	
//	if(reply) dbus_message_unref (reply);
	return result;
}

void reboot_seq(void)
{
	ctllog_debug( "enter reboot_seq()\n" );
	
	//save cfg.xml
	cfg_save();

	//reboot
    	tsl_char_t cmd_line[128]={"0"};
       snprintf(cmd_line,sizeof(cmd_line)-1,"reboot");
	system( cmd_line);
}

void do_reboot( CtlMsgType msgtype,CtlMsgHeader *p_msg)
{
	if(  CTL_MSG_CONFIG_LOAD == msgtype )
	{
		upload_cfg = 1;
		processCfgDownLoading(p_msg);
	}
	else if( CTL_MSG_REBOOT == msgtype )
	{
		reboot_seq();
	}
}

//InternetGatewayDevice.DeviceInfo.VendorConfigFile
//Save cfg history into flash firstby downloading from ACS
tsl_void_t processCfgDownLoading(CtlMsgHeader *p_msg)
{
	if( p_msg == NULL ) 
		return;

	CFGDownloadInfo *cfginfo = (CFGDownloadInfo *) (p_msg + 1);
	if (p_msg->data_length != sizeof(CFGDownloadInfo))
   	{
      		ctllog_error("%s:bad data length, got %ld expected %d, drop msg",
                   __func__,p_msg->data_length, sizeof(CFGDownloadInfo));
      		return;
   	}

	if( tsl_strcmp(cfginfo->src,CFG_SRC_RESTORE) == 0 )
	{
		tsl_char_t cmdbuf[BUFLEN_64] = {0};
		snprintf(cmdbuf,BUFLEN_64,"rm -f %s",ZIP_CFG_FULLPATH);
		system(cmdbuf);
	    	ctllog_debug( "restor default\n");
		return;
	}
	else if(  tsl_strcmp(cfginfo->src,CFG_SRC_LOCAL) == 0 )
	{
		ctllog_debug( "local upload cfg file %s\n",cfginfo->filename);
		return;
	}

	//only record when download cfg.xml from ACS
	FILE * pfile = NULL;

	//save to flash
	pfile = fopen( CFG_DOWNLOAD_LIST, "wa+");
	if( NULL == pfile ) {
		ctllog_error("fail to open file %s", CFG_DOWNLOAD_LIST );
		return;
	}

        fprintf(pfile, "%s\n",cfginfo->filename);
        fprintf(pfile, "%s\n",cfginfo->version);
        fprintf(pfile, "%s\n",cfginfo->datetime);
        fflush(pfile);

	fclose(pfile);
	pfile = NULL;

	return ;
}

#if 0
//read cfg change log from flash and write into data_model
tsl_void_t AddCfgChangelog(tsl_void_t)
{
	tsl_int_t ret = access(CFG_DOWNLOAD_LIST, R_OK);
	if( ret != 0)
	{
		ctllog_error( "cfg file was not downloaded before\n");
		return;	
	}

	FILE * pfile = NULL;
	tsl_char_t buf[BUFLEN_256] = {0};
	int index = 0;
	CFGDownloadInfo cfginfo;
	memset(&cfginfo,0,sizeof(CFGDownloadInfo));
	
	pfile = fopen( CFG_DOWNLOAD_LIST, "r");
	if( NULL == pfile ) {
		ctllog_error("fail to open file %s", CFG_DOWNLOAD_LIST );
		return;
	}

	while(fgets(buf, BUFLEN_256, pfile))
	{
		buf[strlen(buf)-1] = '\0';
		switch(index)
		{
			// file name
			case 0:
				{
					strncpy(cfginfo.filename,buf,BUFLEN_256-1);
				}
				break;
			// file version
			case 1:
				{
					strncpy(cfginfo.version,buf,BUFLEN_64-1);
				}				
				break;
			// file datetime
			case 2:
				{
					strncpy(cfginfo.datetime,buf,BUFLEN_128-1);
				}								
				break;
			case 3:
				{
				}								
				break;
		}
		index  =  index + 1;
		memset(buf,0,BUFLEN_256);
	}

	fclose(pfile);
	pfile = NULL;

	tr69_oid_stack_id iidStack = EMPTY_INSTANCE_ID_STACK;
	tsl_char_t *p_val = NULL;
	tsl_bool_t found = TSL_B_FALSE;
	int intval;

	INIT_INSTANCE_ID_STACK(&iidStack);
	while (dal_ParamValueGetNext(TR69_OID_VENDOR_CONFIG_FILE, &iidStack, "Name", &p_val) == TSL_RV_SUC) 
	{
		//
		if( tsl_strcmp(p_val,cfginfo.filename) == 0 )
		{
			found = TSL_B_TRUE;
			CTLMEM_FREE_BUF_AND_NULL_PTR(p_val);
			break;
		}
		CTLMEM_FREE_BUF_AND_NULL_PTR(p_val);
	}

	//add new
	if( !found )
	{
		INIT_INSTANCE_ID_STACK(&iidStack);
		if ((ret = tr69_add_instance(TR69_OID_VENDOR_CONFIG_FILE, &intval)) != TSL_RV_SUC)
		{
			ctllog_error("add instance of Vendor Cfg  returned %d", ret);
			return ret;
		}
		
		PUSH_INSTANCE_ID(&iidStack,intval);

		ctllog_debug( "oid fullname [%s]\n",oid_to_fullname(TR69_OID_VENDOR_CONFIG_FILE,&iidStack,"Name"));
	}

        ret = dal_ParamValueSet(TR69_OID_VENDOR_CONFIG_FILE, &iidStack, "Name", 1, cfginfo.filename);
        ret = dal_ParamValueSet(TR69_OID_VENDOR_CONFIG_FILE, &iidStack, "Version", 1, cfginfo.version);
        ret = dal_ParamValueSet(TR69_OID_VENDOR_CONFIG_FILE, &iidStack, "Date", 1, cfginfo.datetime);


	// clear history
	tsl_char_t cmdbuf[BUFLEN_64] = {0};
	snprintf(cmdbuf,BUFLEN_64,"rm -f %s",CFG_DOWNLOAD_LIST);
	system(cmdbuf);
	
        ctllog_debug("%s.%d %s %s %s", TR69_OID_VENDOR_CONFIG_FILE, 
			iidStack.instance[0],cfginfo.filename,cfginfo.version,cfginfo.datetime);
}
#endif

tsl_rv_t prepareCfgBackup(tsl_char_t * filein,tsl_char_t *fileout)
{
	tsl_rv_t rv = TSL_RV_ERR;

	ctllog_debug( "%s: filein %s fileout %s\n",__func__,filein,fileout);
    	tsl_char_t cmd_line[BUFLEN_128]={"0"};

	//check pre_cfg.xml
	tsl_int_t ret = access(filein, R_OK);
	if( 0 == ret )
	{
	       snprintf(cmd_line,sizeof(cmd_line)-1,"cp %s %s",filein,fileout);
		ret = system( cmd_line);
		if( ret != -1 )
		{
		       snprintf(cmd_line,sizeof(cmd_line)-1,"%s -d %s",ZIP_COMMAND,fileout);
			ret = system( cmd_line );
			if( ret != -1 )
			{
				rv = TSL_RV_SUC;
			}
			else
			{
				ctllog_error( "%s: fail to unzip %s\n", __func__,fileout);			
			}
		}
		else
		{
			ctllog_error( "%s: fail to %s\n", __func__,cmd_line);
		}	
	}

	return rv;
}


