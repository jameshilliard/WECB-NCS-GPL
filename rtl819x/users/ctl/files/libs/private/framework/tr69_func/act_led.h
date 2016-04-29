// Added bySW-Task#101588
#ifndef ACT_LED_H
#define ACT_LED_H

#define SIOWIFILEDSTATUS 0x8bc6
struct _ioctl_led_status
{
    unsigned int led_mode;
    unsigned int led_interval;
    unsigned int led_on_interval;
    unsigned int led_off_interval;
    int led_cycles;
    unsigned int led_toggle;
    unsigned int led_toggle_start;
};

typedef enum _LED_MODE{
    LED_MODE_OFF,
    LED_MODE_ON,
    LED_MODE_BLINK,
    LED_MODE_BLINKSTOP,
    LED_MODE_MAX
}LED_MODE;

#define INTERFACE_WIFI2G "wlan1-va0"
#define INTERFACE_WIFI5G "wlan0-va0"

#ifdef AEI_WECB_CUSTOMER_COMCAST
#define SLOW_BLINK_INTERVAL_ON  500
#define SLOW_BLINK_INTERVAL_OFF 500
#define FAST_BLINK_INTERVAL_ON  166
#define FAST_BLINK_INTERVAL_OFF 166
#define FAST_BLINK_CYCLES       6
#else
#define SLOW_BLINK_INTERVAL_ON  250
#define SLOW_BLINK_INTERVAL_OFF 250
#define FAST_BLINK_INTERVAL_ON  125
#define FAST_BLINK_INTERVAL_OFF 125
#define FAST_BLINK_CYCLES       8
#endif
#define SLOW_BLINK_CYCLES       1
#define SLOW_BLINK_ALWAYS       0
#define FAST_BLINK_ALWAYS       0

#define HZ 100
#define BLINK_INTERVAL_JIFFIES(interval) ((interval*HZ)/1000)
#endif
