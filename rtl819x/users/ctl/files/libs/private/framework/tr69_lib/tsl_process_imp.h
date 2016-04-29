#ifndef TSL_PROCESS_IMP_H
#define TSL_PROCESS_IMP_H

#include "tsl_common.h"
#include "tsl_list.h"

typedef enum{
        tsl_process_event_start = 1,
        tsl_process_event_stop,
        tsl_process_event_restart,
} tsl_process_event_t;

typedef enum{
        tsl_process_state_running = 11,
        tsl_process_state_stoping,
} tsl_process_state_t;

typedef struct tsl_process_s{
        tsl_char_t path[32];
        tsl_char_t argv[128];
        pid_t pid;
        tsl_process_state_t state;
        tsl_list_head_t list;
}tsl_process_t;



#endif
