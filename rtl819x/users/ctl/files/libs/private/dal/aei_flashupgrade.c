/*
 *      Web server handler routines for management (password, save config, f/w update)
 *
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *
 *      $Id: fmmgmt.c,v 1.45 2009/09/03 05:04:42 keith_huang Exp $
 *
 */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/reboot.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/sysinfo.h>
#include <errno.h>

#include "tsl_common.h"
#include "ctl_log.h"
#include "../../../../../boa/apmib/apmib.h"
#include "../../../../../boa/apmib/mibtbl.h"

//////////////////////////////////////////////////////////////////////////////
#define READBUF_SIZE			4096
int fwMaxSize = 0x800000;				//8M

#ifdef CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE
#define GOOD_BANK_MARK_MASK 0x80000000  //goo abnk mark must set bit31 to 1
#define NO_IMAGE_BANK_MARK 0x80000000  
#define OLD_BURNADDR_BANK_MARK 0x80000001 
#define BASIC_BANK_MARK 0x80000002           
#define FORCEBOOT_BANK_MARK 0xFFFFFFF0  //means always boot/upgrade in this bank

//#define WEB_HEADER				((char *)"w6cg")
//#define FW_HEADER_WITH_ROOT	((char *)"cr6c")
//#define FW_HEADER				((char *)"cs6c")
//#define ROOT_HEADER			((char *)"r6cr")
//#define CODE_IMAGE_OFFSET		0x30000

char *Kernel_dev_name[2]=
 {
   "/dev/mtdblock0", "/dev/mtdblock2"
 };
char *Rootfs_dev_name[2]=
 {
   "/dev/mtdblock1", "/dev/mtdblock3"
 };
 char *webpage_dev_name[2]=
 {
   "/dev/mtdblock0", "/dev/mtdblock2"
 };
  char *bootcode_dev_name[2]=
 {
   "/dev/mtdblock0", "/dev/mtdblock0"
 };
#ifdef AEI_WECB // Save FW info on flash
char *flash_data_dev_name[2]=
{
"/dev/mtdblock2", "/dev/mtdblock2"
};
#endif


static int AEI_get_actvie_bank()
{
	FILE *fp;
	char buffer[2]={0};
	int bootbank;
	fp = fopen("/proc/bootbank", "r");
	
	if (!fp) {
		fprintf(stderr,"%s\n","Read /proc/bootbank failed!\n");
	}else
	{
		//fgets(bootbank, sizeof(bootbank), fp);
		fgets(buffer, sizeof(buffer), fp);
		fclose(fp);
	}
	bootbank = buffer[0] - 0x30;	
	if ( bootbank ==1 || bootbank ==2)
		return bootbank;
	else
		return 1;	
}

static void AEI_get_bank_info(int dual_enable,int *active,int *backup)
{
	int bootbank=0,backup_bank;
	
	bootbank = AEI_get_actvie_bank();	

	if(bootbank == 1 )
	{
		if( dual_enable ==0 )
			backup_bank =1;
		else
			backup_bank =2;
	}
	else if(bootbank == 2 )
	{
		if( dual_enable ==0 )
			backup_bank =2;
		else
			backup_bank =1;
	}
	else
	{
		bootbank =1 ;
		backup_bank =1 ;
	}	

	*active = bootbank;
	*backup = backup_bank;	
}

static unsigned long AEI_header_to_mark(int  flag, IMG_HEADER_Tp pHeader)
{
	unsigned long ret_mark=NO_IMAGE_BANK_MARK;
	//mark_dual ,  how to diff "no image" "image with no bank_mark(old)" , "boot with lowest priority"
	if(flag) //flag ==0 means ,header is illegal
	{
		if( (pHeader->burnAddr & GOOD_BANK_MARK_MASK) )
			ret_mark=pHeader->burnAddr;	
		else
			ret_mark = OLD_BURNADDR_BANK_MARK;
	}
	return ret_mark;
}

// return,  0: not found, 1: linux found, 2:linux with root found
static int AEI_check_system_image(int fh,IMG_HEADER_Tp pHeader)
{
	// Read header, heck signature and checksum
	int ret=0;		
	char image_sig[4]={0};
	char image_sig_root[4]={0};
	
        /*check firmware image.*/
	if ( read(fh, pHeader, sizeof(IMG_HEADER_T)) != sizeof(IMG_HEADER_T)) 
     		return 0;	
	
	memcpy(image_sig, FW_HEADER, SIGNATURE_LEN);
	memcpy(image_sig_root, FW_HEADER_WITH_ROOT, SIGNATURE_LEN);

	if (!memcmp(pHeader->signature, image_sig, SIGNATURE_LEN))
		ret=1;
	else if  (!memcmp(pHeader->signature, image_sig_root, SIGNATURE_LEN))
		ret=2;
	else{
		printf("no sys signature at !\n");
	}				
       //mark_dual , ignore checksum() now.(to do) 
	return (ret);
}

static int AEI_get_image_header(int fh,IMG_HEADER_Tp header_p)
{
	int ret=0;
	//check 	CODE_IMAGE_OFFSET2 , CODE_IMAGE_OFFSET3 ?
	//ignore check_image_header () for fast get header , assume image are same offset......	
	// support CONFIG_RTL_FLASH_MAPPING_ENABLE ? , scan header ...
// Coverity CID 29509
	if( lseek(fh, CODE_IMAGE_OFFSET, SEEK_SET) != -1 )
	{
		ret = AEI_check_system_image(fh,header_p);
		//assume , we find the image header in CODE_IMAGE_OFFSET
		if( lseek(fh, CODE_IMAGE_OFFSET, SEEK_SET) == -1 )
			ret = 0;
	}
	
	return ret;	
}

// return,  0: not found, 1: linux found, 2:linux with root found
static unsigned long AEI_get_next_bankmark(char *kernel_dev,int dual_enable)
{
    unsigned long bankmark=NO_IMAGE_BANK_MARK;
    int ret=0,fh;
    IMG_HEADER_T header; 	
	
	fh = open(kernel_dev, O_RDONLY);
	if ( fh == -1 ) {
      		fprintf(stderr,"%s\n","Open file failed!\n");
		return NO_IMAGE_BANK_MARK;
	}
	ret = AEI_get_image_header(fh,&header);	

	bankmark= AEI_header_to_mark(ret, &header);	
	close(fh);
	//get next boot mark

	if( bankmark < BASIC_BANK_MARK)
		return BASIC_BANK_MARK;
	else if( (bankmark ==  FORCEBOOT_BANK_MARK) || (dual_enable == 0)) //dual_enable = 0 ....	 	
		return FORCEBOOT_BANK_MARK;
	else
		return bankmark+1;  
	
}
#endif

static int AEI_compare_version(const char * old_version, const char * new_version)
{
	int ret = 0;
	int old_num, new_num;
	int i = 0;
	const char *old_p = NULL, *new_p = NULL;

	if (old_version == NULL || new_version == NULL)
	{
		printf("Compare_version error\n");
		ret = -2;
		goto DONE;
	}

	// same version
	if (strncmp(old_version, new_version, SOFTWARE_VERSION_MAX-1) == 0)
	{
		ret = 0;
		goto DONE;
	}

	for(i = 0; i < strlen(old_version); i++)
	{
		if (old_version[i] <= '9' && old_version[i] >= '0')
		{
			old_p = old_version + i;
			break;
		}
	}

	for(i = 0; i < strlen(new_version); i++)
	{
		if (new_version[i] <= '9' && new_version[i] >= '0')
		{
			new_p = new_version + i;
			break;
		}
	}

	i = 0;
	while(new_p && new_p[0] != '\0' && old_p && old_p[0] != '\0')
	{
		new_num = strtol(new_p, &new_p, 0);
		old_num = strtol(old_p, &old_p, 0);

		if(new_num < old_num)
		{
			ret = -1;
			break;
		}
		else if(new_num > old_num)
		{
			ret = 1;
            break;
		}

		if(old_p == NULL && new_p == NULL)
		{
			break;
		}

		if(old_p == NULL && new_p != NULL)
		{
			ret = 1;
			break;
		}

		if(old_p != NULL && new_p == NULL)
		{
			ret = -1;
			break;
		}

		while(1)
		{
			if(old_p[0] == '\0' && new_p[0] == '\0')
			{
				goto DONE;
			}

			if(old_p[0] != '\0' && new_p[0] == '\0')
			{
				ret = -1;
				goto DONE;
			}

			if(old_p[0] == '\0' && new_p[0] != '\0')
			{
				ret = 1;
				goto DONE;
			}

			if(old_p[0] == '.' && new_p[0] == '.')
			{
				break;
			}

			if(old_p[0] != '.' && new_p[0] == '.')
			{
				ret = -1;
				goto DONE;
			}

			if(old_p[0] == '.' && new_p[0] != '.')
			{
				ret = 1;
				goto DONE;
			}

			if(old_p[0] > new_p[0])
			{
				ret = -1;
				goto DONE;
			}

			if(old_p[0] < new_p[0])
			{
				ret = 1;
				goto DONE;
			}

			old_p++;
			new_p++;
		}

		old_p++;
		new_p++;
	}

DONE:
	return ret;
}


static void AEI_fwChecksum(unsigned short*sum,char *data, int len)
{
	unsigned short checksum=0;
	int i;

	for (i=0; i<len; i+=2) {
#ifdef _LITTLE_ENDIAN_
		checksum += WORD_SWAP( *((unsigned short *)&data[i]) );
#else
		checksum += *((unsigned short *)&data[i]);
#endif

	}
	(*sum) += checksum;
}

 int AEI_checkmem(int memsize/*0:download file ; >0 upgrade*/)
{
	int ret =  0;
	char cmd[256] = {0};
	
	//free
	system("echo 3 >/proc/sys/vm/drop_caches");
	//check memory
	struct sysinfo info;
	sysinfo(&info);

	if( memsize == 0 )
		memsize = fwMaxSize;

	do
	{
		//there is not enough memory for upgrade
		if(info.freeram < memsize)
		{
			//for file download, there must be fwMaxSize free
			if( memsize == fwMaxSize)
			{
				ctllog_error("%s:There is only %lu b free,but need %d b. Can't download image file!!!\n",
					__func__,info.freeram,memsize);
				break;
			}
			//kill some daemons,put them into a shell script later
			snprintf(cmd,sizeof(cmd),"/etc/cleandaemons4upgrade.sh");
			system(cmd);
			
			//check again
			sysinfo(&info);
			if( info.freeram < memsize )
			{
				ctllog_error("%s:There is only %lu free memory, need %d memory.Can't continue to upgrade firmware!!",
					__func__,info.freeram,memsize);
			}
			else
			{
				ret  = info.freeram;
				ctllog_debug("%s:there is %lu free memory",__func__,info.freeram);
			}
		}
		else
		{
			ret  = 1;
			ctllog_notice("%s:there is %lu b free memory",__func__,info.freeram);
		}
	}while(0);

	return ret;
}

int AEI_validateImage(char *file_name, tsl_bool_t* b_restore_default)
{
	int ret = 0;
	FW_HEADER_T *allHeader = NULL;	
	int checkversion = 0,size = 0,fileszie = 0,totalsize = 0,leftsize = 0;
    /* CID 29767: String not null terminated (STRING_NULL) */
	char buf[READBUF_SIZE+1] ={0};
	unsigned short read_chksum = 0;
#ifdef AEI_WECB // protect bootcoder
    unsigned short bootloader_safe = 1;
    IMG_HEADER_T header;
#endif

	int fd = open(file_name, O_RDONLY);
	if (fd < 0)
	{
		ctllog_error("%s: read %s error",__func__,file_name);
		return ret;
	}
	while (1) {
		size = read(fd, buf, READBUF_SIZE);
		if (size <= 0)
		{
			if (size == 0)
			{
				break;
			}

			ctllog_error("%s:Read [%s]  error errorno=%s",__func__,file_name,strerror(errno));
			break;
		}
		//need check version 
		if( !checkversion )
		{
			allHeader = (FW_HEADER_T *) buf;

			//checksum header
			if( !AEI_fwChecksumOk(buf,sizeof(FW_HEADER_T)) )
			{
				ctllog_error("%s:Header is invalid!!!",__func__);
				break;
			}
			allHeader->hdr_chksum = 0;
			
			fileszie = allHeader->len;
			leftsize = fileszie;
			//check product flag
			if (memcmp(allHeader->signature, ALL_HEADER, SIGNATURE_LEN))
			{
				ctllog_error("%s:Header[%s] is invalid!!!It should be [%s]",
					__func__,allHeader->signature,ALL_HEADER);
				break;
			}

			fprintf(stdout,"%s:chek product flag ok[%s]\n",__func__,allHeader->signature);

			//check hw version
			if (allHeader->hw_version != AEI_BOARD_ID)
			{
				ctllog_error("%s:HW version [%d] is invalid!!!It should be %d",
					__func__,allHeader->hw_version,AEI_BOARD_ID);
				break;
			}
			fprintf(stdout,"%s:chek hw version ok[%d]\n",__func__,allHeader->hw_version);

			//check file version
			if(!allHeader->force_upgrade)
            {
                int ret;
                ret = AEI_compare_version(SOFTWARE_VERSION, (const char *)allHeader->version);
                if(ret < 0)
                {
#ifdef AEI_ENABLE_DOWNGRADE
                    ctllog_warn("%s:Downgrade from version [%s] to [%s]",
                            __func__, SOFTWARE_VERSION, allHeader->version );
#ifdef AEI_RESTORE_ON_DOWNGRADE
                    if( b_restore_default ) {
                        * b_restore_default = TSL_B_TRUE;
                    }
#endif
#else
                    ctllog_error("%s:File version [%s] is invalid!!! Current file version is [%s]",
                            __func__,allHeader->version,SOFTWARE_VERSION);
                    break;
#endif
                }
            }			
			fprintf(stdout,"%s:chek file version ok[%s]\n",__func__,allHeader->version);

			//check header is ok
			checkversion = 1;
		}
		
		//checksum whole file
		if( leftsize < 0)
			break;
		else if( leftsize < size )
			AEI_fwChecksum(&read_chksum, buf, leftsize);
		else
			AEI_fwChecksum(&read_chksum, buf, size);
		leftsize -= size;
		totalsize +=size;
	}

	//it is a valid image file
	if( checkversion && read_chksum == 0 )
    {
#ifdef AEI_WECB // protect bootcoder
        if( checkversion && read_chksum == 0 ) {
            // check burn address, confirm image will not recover bootcoder
            if(lseek(fd, sizeof(FW_HEADER_T), SEEK_SET) < 0) {
                ctllog_error( "lseek fail" );
                bootloader_safe = 0;
            } else {
                while(1) {
                    if(read(fd, &header, sizeof(header)) != sizeof(header)) {
                        // end of header
                        break;
                    }

                    if((header.burnAddr < CONFIG_BOOT_CODE_SIZE) && memcmp(header.signature, BOOT_HEADER, SIGNATURE_LEN)) {
                        ctllog_error( "This image will recover bootcoder, plese double check!" );
                        bootloader_safe = 0;
                        break;
                    }

                    if(lseek(fd, header.len, SEEK_CUR) < 0) {
                        ctllog_error( "lseek fail" );
                        bootloader_safe = 0;
                        break;
                    }
                }
            }
        }
        if( bootloader_safe == 1)
#endif
        {
            fprintf(stdout,"%s:chek sum ok\n",__func__);
            ret = 1;
        }
    }
	else
	{
		ctllog_error("%s:chek sum fail[%d] filelen=%d readlen=%d \n",
			__func__,read_chksum,fileszie,totalsize);
	}

	close(fd);
	return ret;
}


static int AEI_fwChecksumOk(char *data, int len)
{
	unsigned short sum=0;
	int i;

	for (i=0; i<len; i+=2) {
#ifdef _LITTLE_ENDIAN_
		sum += WORD_SWAP( *((unsigned short *)&data[i]) );
#else
		sum += *((unsigned short *)&data[i]);
#endif

	}
	
	return( (sum==0) ? 1 : 0);
}

static void AEI_kill_processes(void)
{
	printf("upgrade: killing tasks...\n");
	
	kill(1, SIGTSTP);		/* Stop init from reforking tasks */
	kill(1, SIGSTOP);		
	kill(2, SIGSTOP);		
	kill(3, SIGSTOP);		
	kill(4, SIGSTOP);		
	kill(5, SIGSTOP);		
	kill(6, SIGSTOP);		
	kill(7, SIGSTOP);		
	//atexit(restartinit);		/* If exit prematurely, restart init */
	sync();

	signal(SIGTERM,SIG_IGN);	/* Don't kill ourselves... */
	setpgrp(); 			/* Don't let our parent kill us */
	sleep(1);
	signal(SIGHUP, SIG_IGN);	/* Don't die if our parent dies due to
					 * a closed controlling terminal */
}

#ifdef AEI_WECB // Save FW info on flash

static unsigned short aei_check_sum_calc(unsigned long addr, unsigned long len)
{
    int i;
    unsigned short sum = 0;
    unsigned short *p = (unsigned short *)addr;

    for (i = 0; i < len/2; i++)
    {
        sum += p[i];
    }

    return sum;
}

static int aei_write_flash_data(AEI_FLASH_DATA_T *data, unsigned int burn_bank)
{
    int ret = 0;
    unsigned long burn_addr = 0;
    int fd;

    if (data == NULL)
    {
        printf("Error argument\n");
        return -1;
    }

    if(burn_bank == 0)
    {
        burn_addr = 0;
    }
    else
    {
        burn_addr = AEI_FLASH_DATA_SIZE;
    }

    if(memcmp(data->signature, AEI_FLASH_SIGNATURE, SIGNATURE_LEN) == 0)
    {
        data->chksum = 0;
        data->chksum = ~ aei_check_sum_calc(data, sizeof(AEI_FLASH_DATA_T)) + 1;
    }
    else
    {
        memset(data, 0, sizeof(AEI_FLASH_DATA_T));
    }

    fd = open(flash_data_dev_name[burn_bank], O_RDWR);
    if(fd < 0)
    {
        return fd;
    }


    if(burn_addr != lseek(fd, burn_addr, SEEK_SET))
    {
        ret = -1;
        goto L1;
    }

    ret = write(fd, data, sizeof(AEI_FLASH_DATA_T));
    if(ret != sizeof(AEI_FLASH_DATA_T))
    {
        goto L1;
    }

    sync();
    ret = 0;

L1:
    close(fd);
    return ret;
}
#endif

int AEI_FirmwareUpgrade(char *upload_data, int upload_len )
{
	char buffer[256] = {0};
	int head_offset=0 ;
	int isIncludeRoot=0;
	int		 len;
	int          locWrite;
	int          numLeft;
	int          numWrite;
	FW_HEADER_T *allHeader;
	IMG_HEADER_Tp pHeader;
	int flag=0, startAddr=-1, startAddrWeb=-1;
	int update_fw=0, update_cfg=0;
	int fh = 0;

#ifdef CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE
	int active_bank,backup_bank;
	int dual_enable =0;
#endif
	unsigned char isValidfw = 0;
#ifdef AEI_WECB // Save FW info on flash
    AEI_FLASH_DATA_T aei_data;
    int write_aei_data = 0;
#endif

#ifdef CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE
//	apmib_get(MIB_DUALBANK_ENABLED,(void *)&dual_enable);
	dual_enable = 1; // set dual_enable == 1 then make sure DUAL IMAGE are available
	AEI_get_bank_info(dual_enable,&active_bank,&backup_bank);        
#endif

	allHeader = (FW_HEADER_T *) &upload_data[head_offset];
	head_offset += allHeader->offset;
	fprintf(stdout,"####%s:%d %d  allHeader->offset=%d###\n",  __FILE__, __LINE__ , head_offset, allHeader->offset);

	while ((head_offset+sizeof(IMG_HEADER_T)) < upload_len) {
		locWrite = 0;
		pHeader = (IMG_HEADER_Tp) &upload_data[head_offset];
		len = pHeader->len;
#ifdef _LITTLE_ENDIAN_
		len  = DWORD_SWAP(len);
#endif
		numLeft = len + sizeof(IMG_HEADER_T) ;
		// check header and checksum
		if (!memcmp(&upload_data[head_offset], FW_HEADER, SIGNATURE_LEN) ||
			!memcmp(&upload_data[head_offset], FW_HEADER_WITH_ROOT, SIGNATURE_LEN)) {
			isValidfw = 1;
			flag = 1;
#ifdef AEI_WECB // Save FW info on flash
            write_aei_data = 1;
#endif
		}
		else if (!memcmp(&upload_data[head_offset], WEB_HEADER, SIGNATURE_LEN)) {
			isValidfw = 1;
			flag = 2;
		}
		else if (!memcmp(&upload_data[head_offset], ROOT_HEADER, SIGNATURE_LEN)) {
			isValidfw = 1;
			flag = 3;
			isIncludeRoot = 1;
		}else if (!memcmp(&upload_data[head_offset], BOOT_HEADER, SIGNATURE_LEN)) {
			isValidfw = 1;
			flag = 4;
		} 
		else {
			if (isValidfw == 1)
				break;
			strcpy(buffer, ("Invalid file format9!"));
			goto ret_upload;
		}

		if ( (flag == 1) || (flag == 3) || (flag == 4) ) {
			if ( !AEI_fwChecksumOk(&upload_data[sizeof(IMG_HEADER_T)+head_offset], len)) {
				sprintf(buffer, ("Image checksum mismatched! len=0x%x, checksum=0x%x</b><br>"), len,
					*((unsigned short *)&upload_data[len-2]) );
				goto ret_upload;
			}
		}
		else {
			char *ptr = &upload_data[sizeof(IMG_HEADER_T)+head_offset];
			if ( !CHECKSUM_OK((unsigned char *)ptr, len) ) {
				sprintf(buffer, ("Image checksum mismatched! len=0x%x</b><br>"), len);
				goto ret_upload;
			}
		}

#ifndef CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE
		if (flag == 3)
			fh = open(FLASH_DEVICE_NAME1, O_RDWR);
		else
			fh = open(FLASH_DEVICE_NAME, O_RDWR);
#else
		if (flag == 3) //rootfs
			fh = open(Rootfs_dev_name[backup_bank-1], O_RDWR);
		else if (flag == 1) //linux
			fh = open(Kernel_dev_name[backup_bank-1], O_RDWR);
		else if (flag == 4) // bootcode
			fh = open(bootcode_dev_name[backup_bank-1], O_RDWR);		
		else //web
			fh = open(webpage_dev_name[backup_bank-1], O_RDWR);
#endif

		if ( fh == -1 ) {
			strcpy(buffer, ("File open failed!"));
		} else {
			if (flag == 1) {
				if (startAddr == -1) {
					//startAddr = CODE_IMAGE_OFFSET;
					startAddr = pHeader->burnAddr ;
#ifdef _LITTLE_ENDIAN_
					startAddr = DWORD_SWAP(startAddr);
#endif
				}
			}
			else if (flag == 3) {
				if (startAddr == -1) {
					startAddr = 0; // always start from offset 0 for 2nd FLASH partition
				}
			}
			else if (flag == 4) {
				if (startAddr == -1) {
					startAddr = pHeader->burnAddr ;
#ifdef _LITTLE_ENDIAN_
					startAddr = DWORD_SWAP(startAddr);
#endif
				}
			}			
			else {
				if (startAddrWeb == -1) {
					//startAddr = WEB_PAGE_OFFSET;
					startAddr = pHeader->burnAddr ;
#ifdef _LITTLE_ENDIAN_
					startAddr = DWORD_SWAP(startAddr);
#endif
				}
				// Coverity CID 29530
				//else
				//	startAddr = startAddrWeb;
			}
			// Coverity CID 29508
			if ( lseek(fh, startAddr, SEEK_SET) == -1)
			{
				strcpy(buffer, ("lseek failed!"));
				close(fh);
				goto ret_upload;
			}
			if (flag == 3) {
				locWrite += sizeof(IMG_HEADER_T); // remove header
				numLeft -=  sizeof(IMG_HEADER_T);
//need response ACS server
#if 0				
				system("ifconfig br0 down 2> /dev/null");
				system("ifconfig eth0 down 2> /dev/null");
				system("ifconfig eth1 down 2> /dev/null");
				system("ifconfig ppp0 down 2> /dev/null");
				system("ifconfig wlan0 down 2> /dev/null");
				system("ifconfig wlan0-vxd down 2> /dev/null");		
				system("ifconfig wlan0-va0 down 2> /dev/null");		
				system("ifconfig wlan0-va1 down 2> /dev/null");		
				system("ifconfig wlan0-va2 down 2> /dev/null");		
				system("ifconfig wlan0-va3 down 2> /dev/null");
				system("ifconfig wlan0-wds0 down 2> /dev/null");
				system("ifconfig wlan0-wds1 down 2> /dev/null");
				system("ifconfig wlan0-wds2 down 2> /dev/null");
				system("ifconfig wlan0-wds3 down 2> /dev/null");
				system("ifconfig wlan0-wds4 down 2> /dev/null");
				system("ifconfig wlan0-wds5 down 2> /dev/null");
				system("ifconfig wlan0-wds6 down 2> /dev/null");
				system("ifconfig wlan0-wds7 down 2> /dev/null");
#if defined(CONFIG_RTL_92D_SUPPORT)	
				system("ifconfig wlan1 down 2> /dev/null");
				system("ifconfig wlan1-vxd down 2> /dev/null");		
				system("ifconfig wlan1-va0 down 2> /dev/null");		
				system("ifconfig wlan1-va1 down 2> /dev/null");		
				system("ifconfig wlan1-va2 down 2> /dev/null");		
				system("ifconfig wlan1-va3 down 2> /dev/null");
				system("ifconfig wlan1-wds0 down 2> /dev/null");
				system("ifconfig wlan1-wds1 down 2> /dev/null");
				system("ifconfig wlan1-wds2 down 2> /dev/null");
				system("ifconfig wlan1-wds3 down 2> /dev/null");
				system("ifconfig wlan1-wds4 down 2> /dev/null");
				system("ifconfig wlan1-wds5 down 2> /dev/null");
				system("ifconfig wlan1-wds6 down 2> /dev/null");
				system("ifconfig wlan1-wds7 down 2> /dev/null");
#endif			
				AEI_kill_processes();
				sleep(2);
#endif				
			}
			else if(flag == 4)
			{
				locWrite += sizeof(IMG_HEADER_T); // remove header
				numLeft -=  sizeof(IMG_HEADER_T);
			}			
#ifdef CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE
			if (flag == 1) {  //kernel image
				pHeader->burnAddr = AEI_get_next_bankmark(Kernel_dev_name[active_bank-1],dual_enable);	//replace the firmware header with new bankmark //mark_debug		
			}
#endif
			numWrite = write(fh, &(upload_data[locWrite+head_offset]), numLeft);
			if (numWrite < numLeft) {
				sprintf(buffer, ("File write failed. locWrite=%d numLeft=%d numWrite=%d Size=%d bytes."), locWrite, numLeft, numWrite, upload_len);
				close(fh);
				goto ret_upload;
			}
			locWrite += numWrite;
			numLeft -= numWrite;
			sync();
			close(fh);

			head_offset += len + sizeof(IMG_HEADER_T) ;
			startAddr = -1 ; //by sc_yang to reset the startAddr for next image
			update_fw = 1;
		}
	} //while //sc_yang   

#ifdef AEI_WECB // Save FW info on flash
    if(write_aei_data) {
        memcpy(&aei_data.signature, AEI_FLASH_SIGNATURE, SIGNATURE_LEN);
        aei_data.len = sizeof(AEI_FLASH_DATA_T);
        memcpy(&aei_data.version, allHeader->version, SOFTWARE_VERSION_MAX);
        aei_data.hw_version = allHeader->hw_version;

        if(aei_write_flash_data(&aei_data, backup_bank-1) < 0) {
        }
    }
#endif

	return 1;
ret_upload:	
	fprintf(stderr, "%s\n", buffer);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////
