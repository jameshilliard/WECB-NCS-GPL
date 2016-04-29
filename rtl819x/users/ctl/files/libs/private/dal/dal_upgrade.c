/*#######################################################################################
dal_upgrade.c for upgrade adapter
#######################################################################################
*/
#include <netdb.h>
#include <sys/wait.h>
#include <unistd.h>
#include "include/dal_api.h"
#include "ctl.h"

#if 0
typedef struct
{
	char status[BUFLEN_256];
	char filename[BUFLEN_256];
	char softwarever[BUFLEN_256];
	char action[BUFLEN_256];
	char imagecheck[BUFLEN_256];
} LanUpdateInfo;
LanUpdateInfo glbupdate;
#endif

//tsl_rv_t GetAvailableImageVersion(char *url);
//tsl_rv_t dal_sys_validateImage(char *file_name);
tsl_rv_t dal_sys_updateImage(char *file_name,int filesize,tsl_bool_t* b_restore_default);

static int progress = 0;

tsl_rv_t dal_tr69_download(dal_ret_t* download)
{
    return dal_tr69_download_ext( download, NULL );
}

tsl_rv_t dal_tr69_download_ext(dal_ret_t* download, tsl_bool_t* b_restore_default)
{   
	FILE *fp=NULL;
	char URL[256] = {0};
	char Username[256];
	char Password[256];
	char cmdstr[512];
	time_t StartTime;
	time_t CompleteTime;
	int DelaySeconds;
	pid_t pid;
	int retry_count = 0;
	int time_count = 0;
	size_t filesize = -1;
	int download_finish = 0;
	int error_count = 0;
	int fds[2];
	int file_type; 
	char download_file[256];
	int ret = TSL_RV_ERR;

	//prepare enough memory
	ret = AEI_checkmem(0);
	if( !ret  )
	{
		return ret;
	}

	snprintf(URL,sizeof(URL),"%s",download->param[TR69_DOWNLOAD_URL]);
	snprintf(Username,sizeof(Username),"%s", download->param[TR69_DOWNLOAD_USERNAME]);
	snprintf(Password,sizeof(Password),"%s", download->param[TR69_DOWNLOAD_PASSWORD]);
	file_type = atoi(download->param[TR69_DOWNLOAD_FILETYPE]);
	DelaySeconds = atoi(download->param[TR69_DOWNLOAD_DELAYSEC]);

	if (file_type==1)
		snprintf(download_file,sizeof(download_file),"%s",TR69_FIRMWARE_FILE);
	else
		snprintf(download_file,sizeof(download_file),"%s", TR69_CONFIG_FILE);

	if ( !strcmp(Username, "NULL") && !strcmp(Password, "NULL") ) 
	{
	    if ( !strncmp(URL, "https://", strlen("https://")) || 
	            !strncmp(URL, "HTTPS://", strlen("HTTPS://")))
	        snprintf(cmdstr,sizeof(cmdstr),"wget %s --no-check-certificate -q -O %s ",
	                        URL, download_file);
	    else
	        snprintf(cmdstr,sizeof(cmdstr),"wget %s -q -O %s ",
	                        URL, download_file);
	}
	else
	{
	    if ( !strncmp(URL, "https://", strlen("https://")) || 
	            !strncmp(URL, "HTTPS://", strlen("HTTPS://")))
	        snprintf(cmdstr,sizeof(cmdstr),"wget %s --no-check-certificate --user=%s --password=%s -q -O %s ",
	                        URL, Username, Password, download_file);
	    else
	        snprintf(cmdstr,sizeof(cmdstr),"wget %s --user=%s --password=%s -q -O %s ",
	                        URL, Username, Password, download_file);
	}

	ctllog_notice("%s: URL=%s\n username=%s password=%s \n type=%d DelaySeconds =%d\n cmdstr=%s",__func__,
		URL,Username,Password,file_type,DelaySeconds,cmdstr);

	unlink(download_file);
restart_download:
	time_count = 0;
	filesize = -1;
	download_finish = 0;
	if( pipe(fds) == -1 )
	{
		ctllog_error("%s:fail to create a pipe!!!",__func__);
		return CMSRET_INTERNAL_ERROR;
	}
	tsl_printf("\nDelay %d seconds...\n", DelaySeconds);
	sleep(DelaySeconds);
	StartTime = time(NULL);
	tsl_printf("\nDownload is beginning at %d...\n", (int)StartTime);

	//begin download
	pid = fork();
	if (pid < 0) {
		ctllog_error("%s:Error spawning a process to download file %s", __func__,download_file);
		return CMSRET_INTERNAL_ERROR;
	}

	if (pid == 0) //child process, download the file
	{
		umask(0);
		setsid();
		chdir("/tmp");
		close(fds[0]);
		char cmd[256]={0};
		tsl_printf("\nCHILD: Start Downloading %s,%ld\n\n", download_file, time (NULL));

		snprintf(cmd,sizeof(cmd),"echo begin to download\n");
		system(cmd);
		snprintf(cmd,sizeof(cmd),"echo %s\n",cmdstr);
		system(cmd);
		system(cmdstr);
		snprintf(cmd,sizeof(cmd),"echo end to download\n");
		system(cmd);
		tsl_printf("\nCHILD: Download completed at %ld\n\n", time(NULL));
		download_finish = 1;
		write(fds[1], &download_finish, sizeof(download_finish));
		exit(0);
	}
	else //parent process,  monitoring the download
	{
		close(fds[1]);
		while (!download_finish)
		{
			struct stat st;
			fd_set rfd;
			struct timeval to;
			int retval;

			FD_ZERO(&rfd);
			FD_SET(fds[0], &rfd);
			to.tv_sec = 2;	//2 second timeout
			to.tv_usec = 0;
			retval = select(fds[0] + 1, &rfd, NULL, NULL, &to);
			if (retval > 0)	//there is data to read
			{
				if (FD_ISSET(fds[0], &rfd))
				{
					read(fds[0], &download_finish, sizeof(download_finish));
					if( stat(download_file, &st) == -1 )
					{
                        /* CID 30802: Printf arg type mismatch (PW.PRINTF_ARG_MISMATCH) */
						ctllog_error("%s:fail to stat %d!!!!",__func__,download_finish);
						break;
					}
					
					close(fds[0]);
					if (download_finish)
					{
						ctllog_debug("%s:download_finish = %d, filesize = %d\n", __func__,download_finish, st.st_size);
						filesize = st.st_size;
						break;
					}
				}
			}
			if (++time_count == 15)	//Check the file size every 30 seconds
			{
				time_count = 0;
				st.st_size = -1;
				if (stat(download_file, &st) == -1)
				{
					tsl_printf("\n--Error getting file information %s\n", download_file);
					filesize = st.st_size; //make error read file same as filesize no change
				}
				if (filesize == st.st_size) //After 30 seconds and the file size is not changed, probably hang, retry
				{
					int tmp;

					if ((++retry_count < 3)) // 2 times retry is good enough
					{
						error_count = 0;
						system ("killall -9 wget");
						sleep(1);
						kill(pid, SIGKILL);
//						kill(getpid(), SIGKILL); /* getpid()  returns  the  process ID of the calling process. */
						wait(&tmp);
						ctllog_warn("Filesize no change for 30s, retries %d.Prepare to restart downloading file\n",retry_count);
						if (st.st_size > 0)
						{
							progress = 1;
						}
						unlink(download_file);
						sleep(5);
						close(fds[0]);
						goto restart_download;
					}
					else
					{
						ctllog_error("Error downloading %s after %d retries\n", download_file, retry_count);
						system ("killall -9 wget");
						sleep(1);
						kill(pid, SIGKILL);
//						kill(getpid(), SIGKILL);   /* getpid()  returns  the  process ID of the calling process. */
						wait(&tmp);
						if(progress == 0)
						{
							return 9015;
						}
						else
						{
							return 9017;
						}
					}
				}
				else
				{
					filesize = st.st_size;
					tsl_printf("Filesize now %lu\n", filesize);
					if (filesize > 0)
					{
						progress = 1;
					}
					retry_count = 0;
					error_count = 0;
				}
			}
		}
	}
	//////////////////////////////////////////
	fp=fopen(download_file, "r");
	if( !fp ) {
		if (progress == 0)
		{
			return 9015;
		}
		else
		{
			return 9017;
		}
	}

	CompleteTime = time(NULL);
	ctllog_notice("%s:Download is END at %d. load file complete=%d\n", __func__,(int)CompleteTime,filesize);

	fp=fopen(TR69_DOWNLOAD_COMPLETE,"w");
	if (!fp) {
		ctllog_error("%s:fail to write %s",__func__,TR69_DOWNLOAD_COMPLETE);
		return CMSRET_INTERNAL_ERROR; /*Internal Error*/
	} else {
		fprintf(fp, "%ld %ld\n", StartTime, CompleteTime);
	}
	fclose(fp);

	//download cfg.xml
	if (file_type == 3) {
		dal_ret_t conf_ret;
		int ret_size = sizeof(dal_ret_t);	
		memset(&conf_ret, 0, ret_size);

		conf_ret.param[LOCAL_CONFIG_FILE_SRC] = strdup(CFG_SRC_ACS);

		//get filename
		char *p = strrchr(URL,'/');
		if( p )
		{
			conf_ret.param[LOCAL_CONFIG_FILE] = strdup(p+1); 
		}
		else
		{
			conf_ret.param[LOCAL_CONFIG_FILE] = strdup(URL); 
		}

		tsl_printf("configuration_file=%s \n", conf_ret.param[LOCAL_CONFIG_FILE]);
		ret = dal_sys_load_cfg_file(&conf_ret);

		free_one_dal_ret(&conf_ret);
		return ret;
	}

	if (file_type !=1 )
	{
		ctllog_error("%s: file type=%d;can't get file data!!!",__func__,file_type);
		return CMSRET_INTERNAL_ERROR;
	}

	//update firmware
	ret = dal_sys_updateImage(download_file, filesize, b_restore_default);
	
	return  ret;
}
	
tsl_rv_t dal_sys_updateImage(char *file_name,int filesize,tsl_bool_t* b_restore_default)
{
	int ret = CMSRET_INTERNAL_ERROR;	
	FILE *fp = NULL;
	char *filedata = NULL;	
	char cmd[256] = {0};
	struct stat st;
	time_t StartTime = time(NULL);
	int need_reboot = 0;
	
	do
	{
		if( file_name == NULL )
		{
			ctllog_error("Invalid filename !!! can't upgrade firmware!!!");
			break;
		}
	
		if( filesize == 0 )
		{
			if( stat(file_name, &st) == -1 )
			{
				ctllog_error("%s:fail to stat file %s!!!",__func__,file_name);
				break;
			}
			filesize = st.st_size;
		}

		fprintf(stdout,"%s:Begin to upgrade image file %s, size %d b ..........\n",__func__,file_name,filesize);

		//check current memory
		ret = AEI_checkmem(filesize);
		if( !ret )
		{
			ctllog_error("%s:There is no enough memory to upgrade firmware!!!",__func__);
			break;
		}
		//kill some daemons to free memory, need reboot CPE whatever
		else if( ret != 1 )
		{
			need_reboot = 1;
		}
		
		snprintf(cmd,sizeof(cmd),"rm %s",file_name);

		//check image file
		ret  = AEI_validateImage(file_name, b_restore_default);
		if( !ret )
		{
			ctllog_error("%s: %s is a invalid image!!",__func__,file_name);
			//rm invalid file
			system(cmd);
			ret = CMSRET_INVALID_ARGUMENTS;
			break;
		}
		
		//load image file
		fp=fopen(file_name, "r");
		if( !fp ) {
			//rm invalid file
			system(cmd);
			ctllog_error("%s:fail to load firmware file %s\n",__func__,file_name);
			break;
		}
		
		filedata = (char *)malloc(filesize+1);
		if( filedata )
		{
			memset(filedata,0,filesize+1);
			if(fread( filedata, filesize, 1, fp ) != 1)
			{
				ctllog_error("%s:fail to load file %s!!!",__func__,file_name);
				free(filedata);
				filedata = NULL;
				fclose(fp);
				break;
			}
		}
		else
		{
			//rm invalid file
			system(cmd);
			fclose(fp);
			fp = NULL;
			ctllog_error("%s:can't malloc memory for  %s!!!",__func__,file_name);
			break;
		}
		fclose(fp);

		//write flash
		int update_ret = AEI_FirmwareUpgrade(filedata,filesize);
		if( update_ret  )
		{
			ret = TSL_RV_SUC;
			time_t CompleteTime = time(NULL);
			FILE *fp=fopen(TR69_UPDATE_COMPLETE,"w");
			if (fp) 
			{
				fprintf(fp, "%ld %ld\n", StartTime,CompleteTime);
				fclose(fp);
			}
			free(filedata);
			filedata = NULL;
			ctllog_notice("%s:Upgrade firmware from TR69 successfully!",__func__);
		}
		else
		{
			//due to stop some daemons, need reboot CPE also
			if( need_reboot )
				ret = CMSRET_RESOURCE_EXCEEDED;
				
			//rm firmware file
			system(cmd);
			
			free(filedata);
			filedata = NULL;
			ctllog_error("%s:Upgrade firmware from TR69 fail!!!",__func__);
		}
	}while(0);		
	
	return ret;
}

#if 0
tsl_rv_t dal_sys_do_router_update(dal_ret_t* dal_set)
{
	char imagefile[256]={0};
	int rv = TSL_RV_SUC;

	tsl_printf("Enter %s line %d\n", __FUNCTION__, __LINE__);
	TSL_VASSERT(dal_set != NULL);

#ifdef MY_DEBUG
	tsl_printf("update status: %s\n", dal_set->param[ROUTER_UPDATE_STATUS]);
	tsl_printf("update filename: %s\n", dal_set->param[ROUTER_UPDATE_FILENAME]);
	tsl_printf("update version: %s\n", dal_set->param[ROUTER_UPDATE_SWVERSION]);
	tsl_printf("update action: %s\n", dal_set->param[ROUTER_UPDATE_ACTION]);
#endif

	if ( dal_set->param[ROUTER_UPDATE_ACTION] )	{		
		/*if get action order here, no need to do image check*/
		char action[256]={0};
		sprintf(action, "%s", dal_set->param[ROUTER_UPDATE_ACTION]);

		tsl_printf("flash updating action %s==========================\n", dal_set->param[ROUTER_UPDATE_ACTION]);
		if (!tsl_strcmp(action, "startdatacenter"))	{
			/* start datacenter here */
			system("sh /etc/upgrade_end.sh");
		}
		else if (!tsl_strcmp(action, "stopdatacenter"))	 {	
			/*  kill datacenter here */
			system("sh /etc/prepare_upgrade.sh");
		}
		else if (!tsl_strcmp(action, "startupdate")) {
			/* start firmware update here */	
			if ( glbupdate.filename )
			{
				rv = dal_sys_updateImage(glbupdate.filename,0);
			}
			else
			{
				ctllog_error("update file haven't gotted, please input file name first!");				
				rv = TSL_RV_FAIL;
			}	
		} else	{
			ctllog_error("Unsupported action type %s\n", action);			
		}
	}	else if ( dal_set->param[ROUTER_UPDATE_FILENAME] ) {		
		AEI_IMG_TAG tag;
		FILE *in_fp;

		sprintf(imagefile, "%s",  dal_set->param[ROUTER_UPDATE_FILENAME]);		
		ctllog_debug("do upgrade image check===================\n");
		memset(&glbupdate, 0, sizeof(LanUpdateInfo));

		do	{
			if ((in_fp = fopen (imagefile,"rb")) == NULL)  {
				ctllog_error("fopen %s for read error !\n", dal_set->param[ROUTER_UPDATE_FILENAME]);
				sprintf(glbupdate.imagecheck,"%s","Invalid FileName");
				fclose(in_fp);
				break;
			}	

			if (fread((void *)&tag, sizeof(AEI_IMG_TAG), 1, in_fp) != 1)  {
				ctllog_error ("%s: Read File TAG Error \n", __FUNCTION__);
				sprintf(glbupdate.imagecheck,"%s","Invalid Image");
				fclose (in_fp);
				break;
			}

			sprintf(glbupdate.softwarever,"%s",tag.softwareRev);
			sprintf(glbupdate.filename,"%s",imagefile);

			rv = dal_sys_validateImage(imagefile);	
		}while(0);	

		if (rv == TSL_RV_SUC)	
			sprintf(glbupdate.imagecheck,"%s","Pass");			
	}	

	tsl_printf("LEAVE %s line %d\n", __FUNCTION__, __LINE__);

	return rv;
}
#endif


#define TR69_UPLOAD_COMPLETE "/tmp/tr69_upload_complete"
#define TR69_UPLOAD_TMPFILE "/tmp/cfg.xml.upload"
tsl_rv_t dal_tr69_upload(dal_ret_t* upload)
{
	FILE *fp=NULL;
	char URL[256] = {0};
	char Username[256] = {0};
	char Password[256] = {0};
	int DelaySeconds = 0;
	char cmdstr[512] = {0};
	time_t StartTime = 0;
	time_t CompleteTime = 0;
	dal_ret_t *conf_ret = NULL;
    char https_args[32] = {0};
    int file_len = 0;

	conf_ret = dal_sys_get_cfg_file();
    if (conf_ret == NULL || conf_ret->ret == TSL_RV_FAIL)
		return CMSRET_INTERNAL_ERROR;

	//buflen = atoi(ret->param[LOCAL_CONFIG_FILE_LENGTH]);
	//uh_http_send(cl, req, ret->param[LOCAL_CONFIG_FILE], buflen);

	tsl_printf("\nconf_file=%s %sBytes\n", conf_ret->param[LOCAL_CONFIG_FILE], conf_ret->param[LOCAL_CONFIG_FILE_LENGTH]);

	snprintf(URL,sizeof(URL),"%s", upload->param[TR69_UPLOAD_URL]);
	snprintf(Username,sizeof(Username),"%s", upload->param[TR69_UPLOAD_USERNAME]);
	snprintf(Password,sizeof(Password),"%s", upload->param[TR69_UPLOAD_PASSWORD]);
	DelaySeconds = atoi(upload->param[TR69_UPLOAD_DELAYSEC]);

    file_len = atoi(conf_ret->param[LOCAL_CONFIG_FILE_LENGTH]);
    fp = fopen(TR69_UPLOAD_TMPFILE, "w");
    if (!fp) {
    	ctllog_error("could not open %s to write\n", TR69_UPLOAD_TMPFILE);
    	free_dal_ret(&conf_ret);
    	return CMSRET_INTERNAL_ERROR;
    }
    fwrite(conf_ret->param[LOCAL_CONFIG_FILE], file_len, 1, fp);
    fclose(fp);
    fp = NULL;

    if (!strncasecmp(URL, "https://", strlen ("https://")))
    {
        strcpy(https_args, "-k ");
        //strcpy(https_args, "-k -v ");
    }
	if (URL[0]=='H' || URL[0]=='h') {
		if ( strcmp(Username, "NULL") && strcmp(Password, "NULL") ) {
                snprintf(cmdstr,sizeof(cmdstr),"curl %s-u %s:%s --anyauth -T %s %s",
                        https_args, Username, Password, TR69_UPLOAD_TMPFILE, URL);
		}
		else {
              snprintf(cmdstr,sizeof(cmdstr),"curl %s-T %s %s", https_args, TR69_UPLOAD_TMPFILE, URL);
		}
	}
	else {
        if ( strcmp(Username, "NULL") && strcmp(Password, "NULL") ) {
                snprintf(cmdstr,sizeof(cmdstr),"curl %s-u %s:%s --anyauth -T %s %s",
                        https_args, Username, Password, TR69_UPLOAD_TMPFILE, URL);
		}
		else {
              snprintf(cmdstr,sizeof(cmdstr),"curl %s-T %s %s", https_args, TR69_UPLOAD_TMPFILE, URL);
		}
	}
	free_dal_ret(&conf_ret);

	tsl_printf("\nDelay %d seconds...\n", DelaySeconds);
	sleep(DelaySeconds);
	tsl_printf("\ncmdstr=%s\n",cmdstr);
	StartTime = time(NULL);
	tsl_printf("\nUpload is beginning at %d...\n", StartTime);
	system(cmdstr);
	CompleteTime = time(NULL);
	tsl_printf("\nUpload is END at %d.\n", CompleteTime);
	tsl_printf("**Check %d %d", StartTime, CompleteTime);

    unlink(TR69_UPLOAD_TMPFILE);
	fp=fopen(TR69_UPLOAD_COMPLETE,"w");
	if (!fp) {
		return CMSRET_INTERNAL_ERROR; /*Internal Error*/
	} else {
        fprintf(fp, "%lu %lu\n", StartTime, CompleteTime);
	}
	fclose(fp);

	return  TSL_RV_SUC;
}

#if 0
tsl_u32_t getCrc32(const tsl_u8_t *pdata, tsl_u32_t size, tsl_u32_t crc)
{
	while (size-- > 0)
		crc = (crc >> 8) ^ Crc32_table[(crc ^ *pdata++) & 0xff];

	return crc;
}

// verify the tag of the image
static byte buffer[BUF_SIZE];
static tsl_rv_t verifyAEIFileTag(AEI_IMG_TAG *pTag)
{
	tsl_u32_t crc;
	tsl_u32_t tokenCrc, tagCrc;
	char *pStrTmp1=NULL;
	tsl_u8_t random_num=0;
	tsl_u8_t subtag[TAG_LEN-2*TOKEN_LEN-CRC_LEN];
	tsl_u8_t *pInt=NULL;
	tsl_u8_t *crcPtr;
	int i=0;

	pStrTmp1= pTag->signiture;    
	if(pStrTmp1)
		random_num=pStrTmp1[0];
	else
		return CMSRET_INVALID_IMAGE;

	pInt=(tsl_u8_t *) pTag;
	tokenCrc=0;
	for(i=0;i<4;i++)
	{
		tokenCrc |=((pInt[TAG_LEN-CRC_LEN-2*TOKEN_LEN+i])<<(8*(3-i))) ;
	}

	for(i=0;i<(TAG_LEN-2*TOKEN_LEN-CRC_LEN);i++)
	{
		subtag[i]=pInt[(i+random_num)%(TAG_LEN-CRC_LEN-2*TOKEN_LEN)];
	}

	ctllog_debug("##random_num(%d)\n",random_num);              
	// check tag validate token first
	crc = CRC32_INIT_VALUE;
	crc = getCrc32(&subtag, (tsl_u32_t)TAG_LEN-CRC_LEN-2*TOKEN_LEN, crc);
	ctllog_debug("###111calculated crc=0x%x tokenCrc=0x%x\n", crc, tokenCrc);
	if (crc != tokenCrc)
	{
		/* this function is called even when we are not sure the image is
		 * a Actiontec image.  So if crc fails, it is not a big deal.  It just
		 * means this is not a Actiontec image.
		 */
		ctllog_error("token crc failed, this is not a valid Actiontec image");
		ctllog_error("###calculated crc=0x%x tokenCrc=0x%x\n", crc, tokenCrc);
		return CMSRET_INVALID_IMAGE;
	}    
	ctllog_debug("Actiontec CRC is OK.\n");


	crcPtr = pInt + TAG_LEN-2*TOKEN_LEN;
	/*
	 * CRC may not be word aligned, so extract the bytes out one-by-one.
	 * Whole image CRC is calculated, then htonl, then written out using
	 * fwrite (see AEIImageBuilder.c in AEIImageBuilder).  Because of the way we are
	 * extracting the CRC here, we don't have to swap for endieness when
	 * doing compares on desktop Linux and modem (?).
	 */
	tagCrc = (crcPtr[0] << 24) | (crcPtr[1] << 16) | (crcPtr[2] << 8) | crcPtr[3];


	// check tag validate token first
	crc = CRC32_INIT_VALUE;
	crc = getCrc32((tsl_u8_t *) pTag, (tsl_u32_t)TAG_LEN-2*TOKEN_LEN, crc);
	if (crc != tagCrc)
	{
		/* this function is called even when we are not sure the image is
		 * an actiontec image.  So if crc fails, it is not a big deal.  It just
		 * means this is not an actiontec image.
		 */
		ctllog_error("tag crc failed, this is not a valid Actiontec image");
		ctllog_error("calculated crc=0x%x tagCrc=0x%x", crc, tagCrc);
		return CMSRET_INVALID_IMAGE;
	}
	ctllog_debug("header CRC is OK.");

	return tsl_rv_suc;
}

/** Validate the image from the stream.
 * 
 * @return CmsRet enum from among the following: CMSRET_SUCCESS,
 *         CMSRET_RESOURCE_EXCEEDED (out of memory),
 *         CMSRET_INVALID_FILENAME
 *         CMSRET_INVALID_IMAGE
 *         CMSRET_DOWNLOAD_FAILURE
 */
tsl_rv_t dal_sys_validateImage(char *file_name)
{
	AEI_IMG_TAG tag;
	FILE *in_fp;
	tsl_rv_t rc = tsl_rv_suc;

	ctllog_debug("file_name is %s\n", file_name);
	if ((in_fp = fopen (file_name,"rb")) == NULL)   
	{
		ctllog_error("fopen %s for read error !\n", file_name);
		fclose(in_fp);
		return CMSRET_INVALID_FILENAME;
	}

	if (fread((void *)&tag, sizeof(AEI_IMG_TAG), 1, in_fp) != 1)  
	{
		ctllog_error ("%s: Read File TAG Error \n", __FUNCTION__);
		fclose (in_fp);
		return  CMSRET_INVALID_IMAGE;
	}

	//Debug INFO
#ifdef UPDATE_DEBUG
	{	
		ctllog_debug ("tagVersion %s\n", tag.tagVersion);
		ctllog_debug ("signiture %s\n", tag.signiture);
		ctllog_debug ("companyId %s\n", tag.companyId);
		ctllog_debug ("productId %s\n", tag.productId);
		// printf ("bigEndian %s\n", tag.bigEndian);
		ctllog_debug ("hardwareRev %s\n", tag.hardwareRev);
		ctllog_debug ("softwareRev %s\n", tag.softwareRev);
		ctllog_debug ("imageTimeStamp %s\n", tag.imageTimeStamp);
		ctllog_debug ("tag crc %x\n", tag.tagValidationToken);
		ctllog_debug ("image crc %x\n", tag.imageValidationToken);
	}   
#endif

	if(rc = verifyAEIFileTag(&tag) != tsl_rv_suc) 
	{
		fclose (in_fp);
		return  CMSRET_INVALID_IMAGE;
	}


	// check image Crc
	unsigned int count = 0;
	tsl_u32_t crc, imageCrc;
	tsl_u8_t *crcPtr;
	crc = CRC32_INIT_VALUE;
	while (!feof( in_fp )) 
	{
		count = fread( buffer, sizeof(byte), BUF_SIZE, in_fp);
		if (ferror( in_fp)) {
			ctllog_error( "Read error" );
			fclose (in_fp);
			return  CMSRET_INVALID_IMAGE;
		}
		crc = getCrc32(buffer, count, crc);
	}

	//imageCrc = tag.imageValidationToken;       
	crcPtr = ( tsl_u8_t* )&tag + TAG_LEN - TOKEN_LEN;
	imageCrc = (crcPtr[0] << 24) | (crcPtr[1] << 16) | (crcPtr[2] << 8) | crcPtr[3];

	if (crc != imageCrc)
	{
		/*
		 * This should not happen.  We already passed the crc check on the header,
		 * so we should pass the crc check on the image.  If this fails, something
		 * is wrong.
		 */
		ctllog_error("image crc failed after actiontec header crc succeeded");
		ctllog_error("calculated crc=0x%x imageCrc=0x%x\n", crc, imageCrc); 
		fclose (in_fp);
		return CMSRET_INVALID_IMAGE;
	}
	ctllog_debug("image crc is OK");

	if(in_fp) fclose(in_fp);

	return rc;
}
#endif


#if 0
int http_upgrade_get(const char *url, char *buff, int nbytes, int *content_length) 
{
	struct hostent *server_info;
	struct sockaddr_in server_addr;
	char *ptr,*ptr1, server[100], request[200];
	int n, sock, first = 1;;
	int port = 80;

	char *host;

	if (!strncmp(url, "http://", 7))
	{
		host = url + 7;
	}
	else
		host=url;

	ptr = strpbrk(host, "/");	
	if ( ptr == NULL )
	{	
		ctllog_debug("incorrect url address %s\n", url);
		return 0;
	}	

	*ptr = '\0';
	ptr1 = strpbrk(host, ":");	

	if ( ptr1 != NULL )
	{
		bzero(server, sizeof(server));
		strncpy(server, host, ptr1 - host);	
		port = atoi(ptr1+1);			
	}
	else
	{	// default using 80 http port
		bzero(server, sizeof(server));
		strncpy(server, host, ptr - host);		
	}				
	*ptr = '/';

	ctllog_debug("update image locate at server %s port %d\n", server, port);

	server_info = gethostbyname(server);
	if (server_info == NULL)
		return 0;                /* error: can not resolve host name */

	if (server_info->h_addrtype != AF_INET)
		return 0;                /* error: wrong address type */

	bzero((char *)&server_addr, sizeof(server_addr));
	memcpy(&server_addr.sin_addr.s_addr, server_info->h_addr, server_info->h_length);

	server_addr.sin_family = AF_INET;
	//	server_addr.sin_port = htons(80);
	server_addr.sin_port = htons(port);

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return 0;                 /* error: socket error */

	if (connect(sock, (struct sockadd *)&server_addr, sizeof(server_addr)) < 0)
		return 0;                 /* error: fail to connect */

	sprintf(request, "GET %s HTTP/1.0\r\nHOST:%s \r\n\r\n", ptr, server);
	n = write(sock, request, strlen(request));

	if (n != strlen(request))
		return 0;                 /* error: fail to send request */

	while (nbytes > 0) 
	{
		char temp[1000], *content;
		int actual;

		n = recv(sock, temp, sizeof(temp), 0);

		if (n <= 0) {
			break;
		}

		if (first)
		{                            /* get proper info from the first block of the response data */
			char *tmpptr = temp;

			strsep(&tmpptr, "\r\n");

			if (strcmp(temp, "HTTP/1.1 200 OK"))
				return 0;                             /* error: no valid response of 200 OK */

			content = strstr(tmpptr, "Content-Length: ");   /* get the content length */
			if (content != NULL)
				sscanf(content, "Content-Length: %d\r\n", content_length);
			else
				return 0;                              /* error: fail to get the length of the content */

			content = strstr(tmpptr, "\r\n\r\n") + 4;    /* content start after \r\n\r\n */

			n -= (content - temp);                       /* remove the size of the html header */
			first = 0;
		}

		actual = (n > nbytes) ? nbytes : n;    
		memcpy(buff, content, actual);
		buff += actual;
		nbytes -= actual;
		content = temp;
	}

	close(sock);
	return 1;
}

tsl_rv_t GetAvailableImageVersion(char *url)
{
	tsl_rv_t rv = TSL_RV_SUC;

	return rv;
}


/**************************************************************************
 *	[FUNCTION NAME]:
 *		dal_check_router_upgrade
 *
 *	[DESCRIPTION]:
 *		get all router's basic information
 *		key: InternetGatewayDevice.ManagementServer.UpgradesManaged
 *
 *	[return]:
	TSL_RV_SUC : local upgrade enable
	TSL_RV_FAIL : local upgrade disabled
 **************************************************************************/
tsl_rv_t dal_check_router_upgrade(void)
{
	tsl_rv_t ret = TSL_RV_SUC;


	return ret;
}
#endif

