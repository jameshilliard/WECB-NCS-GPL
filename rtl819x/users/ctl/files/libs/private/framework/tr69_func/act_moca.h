#ifndef ACT_MOCA_H
#define ACT_MOCA_H

#define CLINK_CONF_FILE 		"/var/clink.conf"
#define CLINK_DEV				"eth0"
#define MIN_PASSWORD_LENGTH		12
#define MAX_PASSWORD_LENGTH		17
#define PASSWD_SEED_LENGTH		20

#define MOCA_STATUS_UP 	"Up"
#define MOCA_STATUS_DOWN	"Down"

#define TMP_CLNKBW_FILE    "/tmp/clnkstat.txt"
#define TMP_CLNKNODE_FILE   "/tmp/clnknode.txt"

#define CLINK_CMD_FORMAT "clinkd -Dvtf %s --microcode /etc/EN2510.1.10.01.21_dic_gphy.bin --firmware  /etc/EN2510.1.10.01.21_soc.bin  --mac-addr %s %s &"

struct clinkconf_t {
	char name[32];
	int len;
	char val[64];
};

typedef struct _moca_channel_t {
	int mid;
	char channel[32];
	char desc[32];
} moca_channel_t;

void read_moca_conf(struct clinkconf_t clink_conf[]);

int write_moca_conf(const char *fname);

int reconfig_moca_daemon();

int stop_moca_daemon();

int launch_moca_daemon(const char *clink_file, const char *macaddr, const char *clink_dev);

int find_moca_channel(char *currentOperFreq);

int set_moca_channel(char *currentOperFreq);

int set_moca_LastOperFreq(tsl_uint_t lof);

int set_moca_password(char *moca_pwd);

int set_moca_swconfig(int privacy, int nc, char *freq);

int set_moca_BeaconPwrLevel(int beacon);

int set_moca_TxPowerLimit(int txpower);

char *get_moca_status();

char *get_moca_mac(char *mac, int len);

int get_moca_stats(_MOCAInterfaceStatObject *p_moca_stats);

int get_moca_maxbw(int *moca_rx, int *moca_tx);
int get_moca_nodeinfo(int *nodeid, int *cmnodeid,  int *backupnc);

typedef enum _LED_STATE{
    LED_ON   = 1 << 0,
    LED_OFF   = 1 << 1,
    LED_BLINK  = 1 << 2,
    LED_UNBLINK  = 1 << 3,
    LED_SLOW_BLINK  = 1 << 4,
    LED_FAST_BLINK  = 1 << 5
}LED_STATE;

#if defined(AEI_WECB_CUSTOMER_COMCAST)

#define MOCA_START_UP_SUCCESS_LED LED_ON
#define MOCA_START_UP_FAIL_LED LED_OFF
#define MOCA_CLINKD_STOP (LED_OFF | LED_UNBLINK)
#define MOCA_LINK_UP_LED LED_SLOW_BLINK
#define MOCA_LINK_DOWN_LED LED_UNBLINK

#else // For Customer Verizon, default

#define MOCA_START_UP_SUCCESS_LED LED_OFF
#define MOCA_START_UP_FAIL_LED LED_OFF
#define MOCA_CLINKD_STOP (LED_OFF | LED_UNBLINK)
#define MOCA_LINK_UP_LED LED_ON
#define MOCA_LINK_DOWN_LED LED_OFF

#endif


int get_moca_led();
#endif

