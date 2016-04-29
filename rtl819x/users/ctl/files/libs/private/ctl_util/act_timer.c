/*****************e
 * rtSamples
SINT32 cmsUtl_strcmp(const char *s1, const char *s2) 
{
   char emptyStr = '\0';
   char *str1 = (char *) s1;
   char *str2 = (char *) s2;

   if (str1 == NULL)
   {
      str1 = &emptyStr;
   }
   if (str2 == NULL)
   {
      str2 = &emptyStr;
   }

   return strcmp(str1, str2);
}************************************************************

                               Copyright (c) 2010
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#include "timer.h"

#define DBprintf(args...)
timer_callback *timerTable[50];

int getSigno();
int setSigno(unsigned int signo);

timer_t timerCreate(unsigned char periodicity, timerFuncPtr t_handler,
		    unsigned long param)
{
	struct sigevent event;	/* event specification */
	struct sigaction action;	/* signal specification */
	timer_callback *callback;
	timer_t timer_id;
	unsigned char timers_available;
	unsigned int mySignal;
	unsigned int infi_rt_timers;

	//puts("timer_start");
	timers_available = SIGRTMAX - SIGRTMIN;
	infi_rt_timers = getSigno();
	if (infi_rt_timers >= timers_available)
		return 0;

	mySignal = SIGRTMIN + infi_rt_timers + IFX_OFFSET;
	if (mySignal > SIGRTMAX) {
		printf("Timers Not available \n");
		return 0;
	}

	/* copy function and param for callback function */
	callback = (timer_callback *) malloc(sizeof(timer_callback));
	if (!callback) {
		return 0;
	}

	callback->func = (timerFuncPtr) t_handler;
	callback->param = param;
	callback->periodicity = periodicity;

	/* Set up and register a signal handler for mySignal */

	if (sigemptyset(&action.sa_mask))
		err_quit("sigemptyset");

	action.sa_flags = SA_SIGINFO;
	action.sa_sigaction = timer_handler;
	if (sigaction(mySignal, &action, NULL))
		err_quit("sigaction");

	/* Set up and register the event handler */
	event.sigev_signo = mySignal;
	event.sigev_notify = SIGEV_SIGNAL;
	event.sigev_value.sival_ptr = (void *)timer_handler;

	/* Initialize the timer */
	if (timer_create(CLOCK_REALTIME, &event, &timer_id))
		err_quit("timer_create");

	callback->timer_id = timer_id;
	timerTable[infi_rt_timers] = callback;

	DBprintf("Signal : %d   timer id: %d\n", mySignal, timer_id);
	// OAM timerbug: Wrong calculation of timer value been passed to start 
	// and delete function.
	//timer_id = timer_id |((infi_rt_timers <<16) & 0xffff0000); 

	//sigsuspend(&action.sa_mask);
	// OAM timerbug return timer_id;
	return (timer_t *) infi_rt_timers;
}

unsigned int timerDelete(timer_t timer_id)
{
	timer_t timerId;

	if (!timer_id) {
		return 0;
	}
	timerId = timerTable[(int)timer_id]->timer_id;
	DBprintf(" timer delete signal:%d  timer:%d\n", signo, timerId);
	setSigno((int)timer_id);;
	free(timerTable[(int)timer_id]);
	return timer_delete(timerId);
#if 0				/* [ ******************** */
      Neeraj:Removed a bug that was causing system crash
	    signo = (timer_id >> 16) & 0x0000ffff;
	timerId = timer_id & 0x0000ffff;
	setSigno(signo);;
	DBprintf(" timer delete signal:%d  timer:%d\n", signo, timerId);
	free(timerTable[signo]);
	return timer_delete(timerId);
#endif				/* ] *********************** */
}

unsigned int timerReset(timer_t timer_id, unsigned int t_sec,
			unsigned char periodicity)
{

	struct itimerspec timer_spec;	/* highres interval timer */
	timer_t timerId;

	if (!timer_id) {
		return 0;
	}
	//Neeraj: timerId = timer_id & 0x0000ffff;
	timerId = timerTable[(int)timer_id]->timer_id;
	timer_spec.it_value.tv_sec = t_sec / INFI_TICK;
	timer_spec.it_value.tv_nsec = (t_sec % 100) * INFI_MSEC;

	if (periodicity)
		timer_spec.it_interval = timer_spec.it_value;
	else {
		timer_spec.it_interval.tv_sec = 0;
		timer_spec.it_interval.tv_nsec = 0;
	}

	return timer_settime(timerId, 0, &timer_spec, NULL);
}

unsigned int timerStart(timer_t timer_id, unsigned int t_sec,
			unsigned char periodicity)
{

	struct itimerspec timer_spec;	/* highres interval timer */
	timer_t timerId;

	if (!timer_id) {
		return 0;
	}
	//Neeraj: timerId = timer_id & 0x0000ffff;
	timerId = timerTable[(int)timer_id]->timer_id;
	timer_spec.it_value.tv_sec = t_sec / INFI_TICK;
	timer_spec.it_value.tv_nsec = (t_sec % 100) * INFI_MSEC;

	if (periodicity)
		timer_spec.it_interval = timer_spec.it_value;
	else {
		timer_spec.it_interval.tv_sec = 0;
		timer_spec.it_interval.tv_nsec = 0;
	}

	return timer_settime(timerId, 0, &timer_spec, NULL);
}

unsigned int timerStop(timer_t timer_id)
{

	struct itimerspec timer_spec;	/* highres interval timer */
	timer_t timerId;

	if (!timer_id) {
		return 0;
	}
	//Neeraj: timerId = timer_id & 0x0000ffff;
	timerId = timerTable[(int)timer_id]->timer_id;
	timer_spec.it_value.tv_sec = 0;
	timer_spec.it_value.tv_nsec = 0;

	return timer_settime(timerId, 0, &timer_spec, NULL);
}

unsigned int timerPending(timer_t timer_id, struct itimerspec *ispec)
{

	timer_t timerId;

	if (!timer_id) {
		return 0;
	}
	//Neeraj: timerId = timer_id & 0x0000ffff;
	timerId = timerTable[(int)timer_id]->timer_id;

	timer_gettime(timerId, ispec);
	DBprintf("\ntimer_gettime: value=%ld.%09ld, interval=%ld.%09ld\n",
			ispec->it_value.tv_sec, ispec->it_value.tv_nsec,
			ispec->it_interval.tv_sec, ispec->it_interval.tv_nsec);
	DBprintf("TIMEID = %d %s %d\n",timer_id,__FUNCTION__,__LINE__ );

	if (((long int)ispec->it_value.tv_sec > 0)) {
		DBprintf(" Pending:%d\n", timerId);
		return 1;
	} else {
		DBprintf(" Not Pending:%d\n", timerId);
		return 0;
	}
}

int err_quit(char *msg)
{
	perror(msg);
	return 0;
}

void timer_handler(int signo, siginfo_t * sig_info, void *extra)
{
	timer_callback *callback;

//      puts("\n\ntimer_handler");
	DBprintf("\t signal: %d\n", sig_info->si_signo);
	callback = timerTable[signo - (SIGRTMIN + IFX_OFFSET)];
	(callback->func) (callback->param);
	if (callback->periodicity == INFI_TIMER_ONESHOT) {
		//printf("\nOneshot");
		//timerStop(callback->timer_id);
	}
	return;
}

int getSigno()
{
	int i, j, found = 0;
	unsigned int timerVal;

	timerArr[0] |= 0x01;

	for (i = 0; i < 10; i++) {
		for (j = 0; j < 31; j++) {
			if (!((timerArr[i] >> j) & 1)) {
				found = 1;
				break;
			}
		}
		if (found)
			break;
	}
	if (found) {
		timerVal = (i * 32) + j;
		timerArr[i] |= (1 << j);
		DBprintf("TimerID %d\n", timerVal);
		return timerVal;
	} else {
		return -1;
	}

	return 0;
}

int setSigno(unsigned int signo)
{
	int i, j;

	i = signo / 32;
	j = signo % 32;

	timerArr[i] &= ~(1 << j);

	return 0;
}
