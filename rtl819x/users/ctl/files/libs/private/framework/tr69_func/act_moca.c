#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"
#include "act_moca.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <stdio.h>

static struct clinkconf_t clink_conf_default[] = {
    {"hostId",1,"0"},									//line: 0
    {"ipaddr",0,""},
    {"netmask",0,""},
    {"gateWay",0,""},
    {"password",40,"c1f6ea8523102b1ba3628c81b64a5371ed372b74"},
    {"mocapassword",0,""},
    {"CMRatio",2,"20"},
    {"DistanceMode",1,"0"},
    {"TxPower",2,"10"},
    {"phyMargin",2,"10"},
    {"phyMBitMask",3,"127"},							// line: 10
    {"SwConfig",7,"0x107ff"},
    {"dhcp",1,"0"},
    {"admPswd",8,"entropic"},
    {"channelPlan",1,"2"},
#ifdef AEI_WECB_CUSTOMER_NCS
    {"scanMask",10,"0x15554000"},							// 1150 ~ 1500 MHz
    {"productMask",10,"0x15554000"},					// 1125MHz ~ 1525MHz 
    {"channelMask",10,"0x15554000"},						// 1150 ~ 1500 MHz
    {"lof",2,"-1"},
#else
    {"scanMask",5,"0x4000"},                                                   // default 1150 MHz
    {"productMask",10,"0x15554000"},					// 1125MHz ~ 1525MHz 
    {"channelMask",5,"0x4000"},                                                // default 1150 MHz
    {"lof",10,"1150000000"},
#endif
    {"bias",1,"2"},
    {"TargetPhyRate",3,"180"},							// line: 20
    {"PowerCtlPhyRate",3,"235"},
    {"BeaconPwrLevel",1,"7"},							// default -9db	
    {"MiiPausePriLevel",2,"-1"},
    {"PQoSClassifyMode",1,"0"},
    {"mfrVendorId",1,"0"},
    {"mfrHwVer",1,"0"},
    {"mfrSwVer",1,"0"},
    {"personality",1,"0"},
    {"feicProfileId",2,"-1"},
    {"dbgMask",3,"0x0"}								//line: 30

};

const moca_channel_t moca_channel_list[] = {
    {0,"0x0000000015554000", "auto"},
    {1150000000,"0x0000000000004000", "1150"},		
    {1200000000,"0x0000000000010000", "1200"},	
    {1250000000,"0x0000000000040000", "1250"},	
    {1300000000,"0x0000000000100000", "1300"},	
    {1350000000,"0x0000000000400000", "1350"},	
    {1400000000,"0x0000000001000000", "1400"},	
    {1450000000,"0x0000000004000000", "1450"},
    {1500000000,"0x0000000010000000", "1500"}
};

static unsigned long int ft(
	int t,
	unsigned long int x,
	unsigned long int y,
	unsigned long int z )
{
    unsigned long int a,b,c;

    c = 0;

    if (t < 20)
    {
        a = x & y;
        b = (~x) & z;
        c = a ^ b;
    }
    else if (t < 40)
    {
        c = x ^ y ^ z;
    }
    else if (t < 60)
    {
        a = x & y;
        b = a ^ (x & z);
        c = b ^ (y & z);
    }
    else if (t < 80)
    {
        c = x ^ y ^ z;
    }

    return c;
}

static unsigned long int k(int t)
{
    unsigned long int c;

    c = 0;

    if (t < 20)
    {
        c = 0x5a827999;
    }
    else if (t < 40)
    {
        c = 0x6ed9eba1;
    }
    else if (t < 60)
    {
        c = 0x8f1bbcdc;
    }
    else if (t < 80)
    {
        c = 0xca62c1d6;
    }

    return c;
}

static inline int rotl (int bits, unsigned int a)
{   
    // Rotate Left - Generates single instruction on ARM.
    return (a<<(bits)) | (a>>(32-bits));
}

static void sha1 (	unsigned char *message,		// NOTE: input, not const, extended
					int message_length,
					unsigned char *digest )		// 20 bytes
{
    int i;
    int padded_length;

    unsigned long int l;
    unsigned long int t;
    unsigned long int h[5];
    unsigned long int a,b,c,d,e;
    unsigned long int w[80];
    unsigned long int temp;

    /* Calculate the number of 512 bit blocks */

    padded_length = message_length + 8; /* Add length for l */
    padded_length = padded_length + 1; /* Add the 0x01 bit postfix */

    l = message_length * 8;

    /* Round up to multiple of 64 */
    padded_length = (padded_length + 63) & ~63;

    /* clear the padding field */
    memset((char *)(message+message_length), 0, padded_length - message_length);

    /* Insert b1 padding bit */
    message[message_length] = 0x80;

    /* Insert length */
    message[padded_length-1] = (unsigned char)( l        & 0xff);
    message[padded_length-2] = (unsigned char)((l >> 8)  & 0xff);
    message[padded_length-3] = (unsigned char)((l >> 16) & 0xff);
    message[padded_length-4] = (unsigned char)((l >> 24) & 0xff);

    /* Set initial hash state */
    h[0] = 0x67452301;
    h[1] = 0xefcdab89;
    h[2] = 0x98badcfe;
    h[3] = 0x10325476;
    h[4] = 0xc3d2e1f0;

    for (i = 0; i < padded_length; i += 64)
    {
        /* Prepare the message schedule */
        for (t=0; t < 16; t++)
        {
            w[t]  = (256*256*256) * message[i + (t*4)    ];
            w[t] += (256*256    ) * message[i + (t*4) + 1];
            w[t] += (256        ) * message[i + (t*4) + 2];
            w[t] +=                 message[i + (t*4) + 3];
        }
        for ( ; t < 80; t++)
        {
            w[t] = rotl(1,(w[t-3] ^ w[t-8] ^ w[t-14] ^ w[t-16]));
        }

        /* Initialize the five working variables */
        a = h[0];
        b = h[1];
        c = h[2];
        d = h[3];
        e = h[4];

        /* iterate a-e 80 times */

        // NOTE: split into 4 loops by 20 to simplify ft() and k()
        for (t = 0; t < 80; t++)
        {
            temp = (rotl(5,a) + ft(t,b,c,d)) & 0xffffffff;
            temp = (temp + e) & 0xffffffff;
            temp = (temp + k(t)) & 0xffffffff;
            temp = (temp + w[t]) & 0xffffffff;
            e = d;
            d = c;
            c = rotl(30,b);
            b = a;
            a = temp;
        }

        /* compute the ith intermediate hash value */
        h[0] = (a + h[0]) & 0xffffffff;
        h[1] = (b + h[1]) & 0xffffffff;
        h[2] = (c + h[2]) & 0xffffffff;
        h[3] = (d + h[3]) & 0xffffffff;
        h[4] = (e + h[4]) & 0xffffffff;
    }

    digest[3]  = (unsigned char) ((h[0]      ) & 0xff);
    digest[2]  = (unsigned char) ((h[0] >>  8) & 0xff);
    digest[1]  = (unsigned char) ((h[0] >> 16) & 0xff);
    digest[0]  = (unsigned char) ((h[0] >> 24) & 0xff);

    digest[7]  = (unsigned char) ((h[1]      ) & 0xff);
    digest[6]  = (unsigned char) ((h[1] >>  8) & 0xff);
    digest[5]  = (unsigned char) ((h[1] >> 16) & 0xff);
    digest[4]  = (unsigned char) ((h[1] >> 24) & 0xff);

    digest[11] = (unsigned char) ((h[2]      ) & 0xff);
    digest[10] = (unsigned char) ((h[2] >>  8) & 0xff);
    digest[9]  = (unsigned char) ((h[2] >> 16) & 0xff);
    digest[8]  = (unsigned char) ((h[2] >> 24) & 0xff);

    digest[15] = (unsigned char) ((h[3]      ) & 0xff);
    digest[14] = (unsigned char) ((h[3] >>  8) & 0xff);
    digest[13] = (unsigned char) ((h[3] >> 16) & 0xff);
    digest[12] = (unsigned char) ((h[3] >> 24) & 0xff);

    digest[19] = (unsigned char) ((h[4]      ) & 0xff);
    digest[18] = (unsigned char) ((h[4] >>  8) & 0xff);
    digest[17] = (unsigned char) ((h[4] >> 16) & 0xff);
    digest[16] = (unsigned char) ((h[4] >> 24) & 0xff);

}

/*******************************************************************************
*
* Public method:      InitSecurityKey(char*password, SYS_UCHAR* seed[20])
*
********************************************************************************
*
* Description:	Generate binary seed for storage, from the incoming ascii pwd.
*               
* Inputs:    17 byte ascii password
*
* Outputs:   20 byte binary seed
*      
*
********************************************************************************/
int InitSecurityKey(char *password, char *binary_seed)
{
    int  len = 0;
    char   ascii_key[64]={0};  // sha1() extends 17 to 64

    // Restrict Key to 17 characters
    if (!password || !binary_seed)
        return -1;

    len = strlen(password);
    if (len > MAX_PASSWORD_LENGTH)
    {
        password += len - MAX_PASSWORD_LENGTH;
        len = MAX_PASSWORD_LENGTH;
    }
    // If less than 17 Chars, prefix with leading ascii zeros.
    memset((char *)ascii_key, '0', MAX_PASSWORD_LENGTH);
    /* null terminate string so it can be printed later */
    ascii_key[MAX_PASSWORD_LENGTH] = '\0';
    memcpy((char *)(ascii_key + MAX_PASSWORD_LENGTH - len), (char *)password, len);

    // Hash ascii_key to get binary_seed
    memset((char *)binary_seed, 0, PASSWD_SEED_LENGTH);
    sha1(ascii_key, MAX_PASSWORD_LENGTH, binary_seed );
    return 0;
}

void read_moca_conf(struct clinkconf_t clink_conf[])
{
	FILE *fp=NULL;
	char line[128]={0};
	int count = sizeof(clink_conf_default)/sizeof(struct clinkconf_t);
	int i=0;

	if ((fp=fopen(CLINK_CONF_FILE,"r"))!=NULL)	{
		while (fgets(line, sizeof(line)-1, fp))	{
			sscanf(line,"%s %d %s", &(clink_conf[i].name[0]), &(clink_conf[i].len), &(clink_conf[i].val[0]));
			i++;
			if (i>=count)
				break;
		}
		fclose(fp);
	}	
}

int write_moca_conf(const char *fname)
{
    FILE *fp=NULL;
    int count = sizeof(clink_conf_default)/sizeof(struct clinkconf_t);
    int i=0;
    int ret = -1;

    if (!fname)
        return -1;

    if ((fp=fopen(fname, "w"))!=NULL)	{
        tsl_printf("======== write clink file %s \n", fname);
        for (i=0; i<count; i++)		{
            fprintf(fp, "%s %d %s\n", clink_conf_default[i].name, clink_conf_default[i].len, clink_conf_default[i].val);
            tsl_printf("========  %s %d %s\n", clink_conf_default[i].name, 
                    clink_conf_default[i].len, clink_conf_default[i].val);			
        }

        fclose(fp);
        ret = 0;
    }	else	{
        tsl_printf("write file %s failed\n", fname);
        ret = -1;
    }

    return ret;
}

int reconfig_moca_daemon()
{
    system("killall -1 clinkd");
    return 0;
}

int stop_moca_daemon()
{
    system("killall clinkd");	
    return 0;
}
	
int launch_moca_daemon(const char *clink_file, const char *macaddr, const char *clink_dev)
{
    int ret = -1;
    char cmdstr[512]={0};

    if (!clink_file || !clink_dev || !macaddr)
        return ret;

    snprintf(cmdstr, sizeof(cmdstr), CLINK_CMD_FORMAT, clink_file, macaddr, clink_dev);
    system(cmdstr);	
    ret = 0;

    return ret;
}

int find_moca_channel(char *currentOperFreq)
{
    int i=0;
    int chan = 0;
    if (!currentOperFreq)
        return chan;

    for (i=0; i<sizeof(moca_channel_list)/sizeof(moca_channel_t); i++)
    {
        if (strcmp(moca_channel_list[i].channel, currentOperFreq)==0)
        {
            chan = moca_channel_list[i].mid;
            break;
        }
    }

    return chan;
}

int set_moca_channel(char *currentOperFreq)
{
    int channel = 0;

    if (!currentOperFreq)
        return -1;

    channel = find_moca_channel(currentOperFreq);
    switch (channel)
    {
        case 1150000000:
            tsl_printf("=========== currentOperFreq is 1150MHz\n");
            clink_conf_default[15].len = 6;
            strcpy(clink_conf_default[15].val , "0x4000");
            clink_conf_default[17].len = 6;
            strcpy(clink_conf_default[17].val , "0x4000");
            break;

        case 1200000000:
            tsl_printf("=========== currentOperFreq is 1200MHz\n");
            clink_conf_default[15].len = 7;
            strcpy(clink_conf_default[15].val , "0x10000");
            clink_conf_default[17].len = 7;
            strcpy(clink_conf_default[17].val , "0x10000");
            break;

        case 1250000000:
            tsl_printf("=========== currentOperFreq is 1250MHz\n");
            clink_conf_default[15].len = 7;
            strcpy(clink_conf_default[15].val , "0x40000");
            clink_conf_default[17].len = 7;
            strcpy(clink_conf_default[17].val , "0x40000");
            break; 

        case 1300000000:
            tsl_printf("=========== currentOperFreq is 1300MHz\n");
            clink_conf_default[15].len = 8;
            strcpy(clink_conf_default[15].val , "0x100000");
            clink_conf_default[17].len = 8;
            strcpy(clink_conf_default[17].val , "0x100000");
            break;        

        case 1350000000:
            tsl_printf("=========== currentOperFreq is 1350MHz\n");
            clink_conf_default[15].len = 8;
            strcpy(clink_conf_default[15].val , "0x400000");
            clink_conf_default[17].len = 8;
            strcpy(clink_conf_default[17].val , "0x400000");
            break;         

        case 1400000000:
            tsl_printf("=========== currentOperFreq is 1400MHz\n");
            clink_conf_default[15].len = 9;
            strcpy(clink_conf_default[15].val , "0x1000000");
            clink_conf_default[17].len = 9;
            strcpy(clink_conf_default[17].val , "0x1000000");
            break;

        case 1450000000:
            tsl_printf("=========== currentOperFreq is 1450MHz\n");
            clink_conf_default[15].len = 9;
            strcpy(clink_conf_default[15].val , "0x4000000");
            clink_conf_default[17].len = 9;
            strcpy(clink_conf_default[17].val , "0x4000000");
            break;    

        case 1500000000:
            tsl_printf("=========== currentOperFreq is 1500MHz\n");
            clink_conf_default[15].len = 10;
            strcpy(clink_conf_default[15].val , "0x10000000");
            clink_conf_default[17].len = 10;
            strcpy(clink_conf_default[17].val , "0x10000000");
            break;            

        default: // auto
            tsl_printf("=========== currentOperFreq auto\n");
            clink_conf_default[15].len = 10;
            strcpy(clink_conf_default[15].val , "0x15554000");
            clink_conf_default[17].len = 10;
            strcpy(clink_conf_default[17].val , "0x15554000");
            break;    	  
    }
    return 0;
}

int set_moca_LastOperFreq(tsl_uint_t lof)
{
    if(lof != 0){
        sprintf(clink_conf_default[18].val, "%d", lof);
        clink_conf_default[18].len = strlen(clink_conf_default[18].val);
    }
    else
    {
        sprintf(clink_conf_default[18].val, "%d", -1);
        clink_conf_default[18].len = strlen(clink_conf_default[18].val);
    }

    return 0;
}

int set_moca_password(char *moca_pwd)
{
    unsigned char pwdseed[20] = {0};
    char tmphex[3]={0};
    int i=0;

    if (!moca_pwd)
        return -1;

    if (strlen(moca_pwd) < MIN_PASSWORD_LENGTH ||
            strlen(moca_pwd) > MAX_PASSWORD_LENGTH) {
        tsl_printf("error moca pwd parameter\n");
        return -1;
    }

    if (! InitSecurityKey(moca_pwd, pwdseed) )
    {
        memset(clink_conf_default[4].val, 0, sizeof(clink_conf_default[4].val));

        for (i = 0; i < PASSWD_SEED_LENGTH; i++)
        {
            sprintf(tmphex,"%02x", pwdseed[i]);
            strncat(clink_conf_default[4].val, tmphex, 2);
        }

        if (moca_pwd && strcmp(moca_pwd, clink_conf_default[5].val) !=0)	{
            clink_conf_default[5].len = strlen(moca_pwd);
            snprintf(clink_conf_default[5].val, sizeof(clink_conf_default[5].val), moca_pwd); 
        }
    }

    return -1;
}

int set_moca_swconfig(int privacy, int nc, char *freq)
{
    if (find_moca_channel(freq) == 0)      { // auto channel Frequency Scanning enable
        if (privacy)	{
            if (nc) {
                clink_conf_default[11].len = 9;
                snprintf(clink_conf_default[11].val , sizeof(clink_conf_default[11].val), "0x40017ff");
            } else {
                clink_conf_default[11].len = 6;
                snprintf(clink_conf_default[11].val , sizeof(clink_conf_default[11].val), "0x17ff");	
            }
        }	else 	{
            if (nc) {
                clink_conf_default[11].len = 9; 
                snprintf(clink_conf_default[11].val , sizeof(clink_conf_default[11].val), "0x40007ff");	
            } else {
                clink_conf_default[11].len = 5;
                snprintf(clink_conf_default[11].val , sizeof(clink_conf_default[11].val), "0x7ff");	
            }
        }
    } else  {   // Frequency Scanning disable
        if (privacy)	{
            if (nc) {
                clink_conf_default[11].len = 9;
                snprintf(clink_conf_default[11].val , sizeof(clink_conf_default[11].val), "0x40117ff");
            } else {
                clink_conf_default[11].len = 7;
                snprintf(clink_conf_default[11].val , sizeof(clink_conf_default[11].val), "0x117ff");	
            }
        }	else 	{
            if (nc) {
                clink_conf_default[11].len = 9; 
                snprintf(clink_conf_default[11].val , sizeof(clink_conf_default[11].val), "0x40107ff");	
            } else {
                clink_conf_default[11].len = 7;
                snprintf(clink_conf_default[11].val , sizeof(clink_conf_default[11].val), "0x107ff");	
            }
        }
    }

    return 0;
}

int set_moca_BeaconPwrLevel(int beacon)
{
    if (beacon>0 && beacon<10) {
        clink_conf_default[22].len = 1;
        sprintf(clink_conf_default[22].val, "%d", beacon);
    } else if (beacon==10) {
        clink_conf_default[22].len = 2;
        sprintf(clink_conf_default[22].val, "%d", beacon);
    } else {	// default is 7
        clink_conf_default[22].len = 1;
        sprintf(clink_conf_default[22].val, "%d", 7);
    }

    return 0;
}

int set_moca_TxPowerLimit(int txpower)
{
    if (txpower>=0 && txpower <10) {
        clink_conf_default[8].len = 1;
        sprintf(clink_conf_default[8].val, "%d", txpower);
    } else if (txpower == 10) 	{
        clink_conf_default[8].len = 2;
        sprintf(clink_conf_default[8].val, "%d", txpower);
    }

    return 0;
}

char *get_moca_status()
{
    FILE *pfp = NULL;
    tsl_char_t pipeline[128]={0};
    tsl_char_t cmdstr[128]={0};
    char* moca_status = "Down";

    snprintf(cmdstr, sizeof(cmdstr), "clnkstat -i %s -d", CLINK_DEV);

    if ((pfp = popen(cmdstr, "r"))!=NULL) {
        while (fgets(pipeline, sizeof(pipeline)-1, pfp)) {		

            if (strstr(pipeline, "LINK UP")) {
                moca_status = "Up";
            }
        }
        pclose(pfp);
    }

    return moca_status;
}

char *get_moca_mac(char *mac, int len)
{
    int s;
    struct ifreq buffer;

    if ((s=socket(PF_INET, SOCK_DGRAM, 0))!=-1)
    {
        memset(&buffer, 0x00, sizeof(buffer));
        strcpy(buffer.ifr_name, CLINK_DEV);
        /* coverity #29512 */
        if (ioctl(s, SIOCGIFHWADDR, &buffer)!=-1)	{		
            snprintf(mac, len,"%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
                    (unsigned char)buffer.ifr_hwaddr.sa_data[0],
                    (unsigned char)buffer.ifr_hwaddr.sa_data[1],
                    (unsigned char)buffer.ifr_hwaddr.sa_data[2],
                    (unsigned char)buffer.ifr_hwaddr.sa_data[3],
                    (unsigned char)buffer.ifr_hwaddr.sa_data[4],
                    (unsigned char)buffer.ifr_hwaddr.sa_data[5] );	
        }

        close(s);
    }

    return mac;
}

int dev_stats(const char *devName, int* byteRx, int* packetRx, 
                      int* errRx, int* dropRx,
                      int* byteTx, int* packetTx, 
                      int* errTx, int* dropTx)
{
    char line[512]={0}/*, buf[512]={0}*/;
    char dummy[32]={0};
    char rxByte[32]={0};
    char rxPacket[32]={0};
    char rxErr[32]={0};
    char rxDrop[32]={0};
    char txByte[32]={0};
    char txPacket[32]={0};
    char txErr[32]={0};
    char txDrop[32]={0};
    int count=0;
    char if_mark[8] = {0};

    /* getstats put device statistics int o this file, read the stats */
    FILE* fs = fopen("/proc/net/dev", "r");
    if ( fs == NULL ) 	
        return -1;

    while ( fgets(line, sizeof(line), fs) ) 	{
        /* read pass 2 header lines */
        if ( count++ < 2 ) 	
            continue;

        /* if int erface is found then store statistic values */
        sprintf(if_mark, "%s:", devName);
        if ( strstr(line, if_mark) != NULL )	 {
            char *pos=strchr(line,':');
            /* coverity #29587 */
            if (pos != NULL)
                pos[0]=' ';

            sscanf(line, "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
                    dummy, rxByte, rxPacket, rxErr, rxDrop, dummy, dummy, dummy, dummy,
                    txByte, txPacket, txErr, txDrop, dummy, dummy, dummy, dummy);

            *byteRx= atol(rxByte);
            *packetRx = atol(rxPacket);
            *errRx = atol(rxErr);
            *dropRx = atol(rxDrop);
            *byteTx = atol(txByte);
            *packetTx = atol(txPacket);
            *errTx = atol(txErr);
            *dropTx = atol(txDrop);

            break;
        } /* devName */
    } /* while */

    fclose(fs);
    return 0;
}


int get_moca_stats(_MOCAInterfaceStatObject *p_moca_stats)
{
    int ret = -1;

    ret =  dev_stats(CLINK_DEV,  
            &(p_moca_stats->bytesReceived),
            &(p_moca_stats->packetsReceived),	
            &(p_moca_stats->errorsReceived),
            &(p_moca_stats->discardPacketsReceived),	

            &(p_moca_stats->bytesSent),
            &(p_moca_stats->packetsSent),		
            &(p_moca_stats->errorsSent),
            &(p_moca_stats->discardPacketsSent));

    return ret;
}

int get_moca_maxbw(int *moca_rx, int *moca_tx)
{
    FILE *fp=NULL;
    char line[BUFLEN_128] = {0};
    char buff[BUFLEN_32] = {0};
    char mac[BUFLEN_8][BUFLEN_32] = {{0}};
    int  tx[BUFLEN_8] = {0}, rx[BUFLEN_8] = {0};
    char *p=NULL;
    int step=0, macnode=0;

    system("clnkstat -a > "TMP_CLNKBW_FILE);
    fp = fopen(TMP_CLNKBW_FILE, "r");

    if (fp == NULL)
        return -1;

    macnode = -1;
    step = 0;

    for (;;) {
        if (fgets(line,sizeof(line),fp) == NULL)
            break;

        if (macnode == 8) {
            macnode--;
            break;
        }

        if (step == 0) {
            p = strstr(line,"MAC Address:");
            if (p) {
                macnode++;
                sscanf(p+strlen("MAC Address:"), "%s", mac[macnode]);
                step++;
            }
            continue;
        }

        if (step == 1) {
            p = strstr(line, "TxBitRate:");
            if (p) {
                sscanf(p + strlen("TxBitRate:"), "%s", buff);
                buff[sizeof(buff)-1] = 0;
                tx[macnode] = atoi(buff)/(1024*1024);
                step = 0;
            }

            p = strstr(line,"RxBitRate:");
            if (p) {
                sscanf(p + strlen("RxBitRate:"), "%s", buff);
                buff[sizeof(buff)-1] = 0;
                rx[macnode] = atoi(buff)/(1024*1024);
            }
            continue;
        }
    }

    * moca_rx = rx[1];
    * moca_tx = tx[1];	

    fclose(fp);
    unlink(TMP_CLNKBW_FILE);
    return 0;
}

int get_moca_nodeinfo(int *nodeid, int *cmnodeid, int *backupnc)
{
    FILE *fp=NULL;
    char line[BUFLEN_128] = {0};
    char buff[BUFLEN_32] = {0};
    char *p=NULL;

    system("clnkstat -d > "TMP_CLNKNODE_FILE);
    fp = fopen(TMP_CLNKNODE_FILE, "r");

    if (fp == NULL)
        return -1;


    for (;;) {
        if (fgets(line,sizeof(line),fp) == NULL)
            break;

        p = strstr(line," NodeID:");
        if (p) {
            sscanf(p+strlen(" NodeID:"), "%s", buff);
            * nodeid = atoi(buff);            
        }

        p = strstr(line,"CMNodeID:");
        if (p) {
            sscanf(p+strlen("CMNodeID:"), "%s", buff);
            * cmnodeid = atoi(buff);            
        }

        p = strstr(line,"BackupCMNodeID:");
        if (p) {
            sscanf(p+strlen("BackupCMNodeID:"), "%s", buff);
            * backupnc = atoi(buff);
            break;
        } 
    }

    fclose(fp);
    unlink(TMP_CLNKNODE_FILE);
    return 0;
}
    

/*   Function Name: get_moca_led
 *   Return value: led status got by accessing /proc/act_config/MoCALED
 */
int get_moca_led()
{
    FILE *pipe;
    char cmd[32] = {0};
    char pipeline[16] = {0};
    char* stop_pot = NULL;
    int rv = -1;
    strcpy(cmd, "cat /proc/act_config/MoCALED");
    if((pipe = popen(cmd, "r")) != NULL)
    {
        if(fgets(pipeline, sizeof(pipeline)-1, pipe))
        {
            ctllog_debug("%s get moca %s \n", __func__,pipeline);
            rv = strtol(pipeline, &stop_pot, 16);
        }

        pclose(pipe);
    }
    
    return rv;
}
