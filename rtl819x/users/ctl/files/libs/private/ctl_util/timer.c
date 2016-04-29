#include "ctl.h"
#include "ctl_mem.h"
#include "ctl_tms.h"
#include "ctl_tmr.h"
#include "ctl_log.h"
#include "tr69_oal.h"


/** Internal event timer structure
 */
typedef struct cms_timer_event
{
   struct cms_timer_event *next;      /**< pointer to the next timer. */
   CmsTimestamp            expireTms; /**< Timestamp (in the future) of when this
                                       *   timer event will expire. */
   CmsEventHandler         func;      /**< handler func to call when event expires. */
   void *                  ctxData;   /**< context data to pass to func */
   char name[CMS_EVENT_TIMER_NAME_LENGTH]; /**< name of this timer */
} CmsTimerEvent;


/** Internal timer handle. */
typedef struct
{
   CmsTimerEvent *events;     /**< Singly linked list of events */
   tsl_u32_t         numEvents;  /**< Number of events in this handle. */
} CmsTimerHandle;


tsl_rv_t ctl_timer_init(void **tmrHandle)
{

   (*tmrHandle) = calloc(sizeof(CmsTimerHandle), 1);
   if ((*tmrHandle) == NULL)
   {
      ctllog_error("could not malloc mem for tmrHandle");
      return CMSRET_RESOURCE_EXCEEDED;
   }

   return TSL_RV_SUC;
}

/** This macro will evaluate TSL_B_TRUE if a is earlier than b */
#define IS_EARLIER_THAN(a, b) (((a)->sec < (b)->sec) || \
                               (((a)->sec == (b)->sec) && ((a)->nsec < (b)->nsec)))

tsl_rv_t ctl_timer_set(void *handle, CmsEventHandler func, void *ctxData, tsl_u32_t ms, const char *name)
{
   CmsTimerHandle *tmrHandle = (CmsTimerHandle *) handle;
   CmsTimerEvent *currEvent, *prevEvent, *newEvent;

   /*
    * First verify there is not a duplicate event.
    * (The original code first deleted any existing timer,
    * which is a "side-effect", bad style, but maybe tr69c requires
    * that functionality?)
    */
   if (ctl_timer_is_event_present(handle, func, ctxData))
   {
      ctllog_error("There is already an event func 0x%x ctxData 0x%x",
                   func, ctxData);
      return CMSRET_INVALID_ARGUMENTS;
   }

   /* make sure name is not too long */
   if ((name != NULL) && (strlen(name) >= CMS_EVENT_TIMER_NAME_LENGTH))
   {
      ctllog_error("name of timer event is too long, max %d", CMS_EVENT_TIMER_NAME_LENGTH);
      return CMSRET_INVALID_ARGUMENTS;
   }


   /*
    * Allocate a structure for the timer event.
    */
   newEvent = calloc(1,sizeof(CmsTimerEvent));
   if (newEvent == NULL)
   {
      ctllog_error("malloc of new timer event failed");
      return CMSRET_RESOURCE_EXCEEDED;
   }

   /* fill in fields of new event timer structure. */
   newEvent->func = func;
   newEvent->ctxData = ctxData;

   ctl_timer_get(&(newEvent->expireTms));
   ctl_timer_add_ms(&(newEvent->expireTms), ms);

   if (name != NULL)
   {
      sprintf(newEvent->name, "%s", name);
   }


   /* 
    * Now we just need to insert it in the correct place in the timer handle.
    * We just insert the events in absolute order, i.e. smallest expire timer
    * at the head of the queue, largest at the end of the queue.  If the
    * modem is up long enough where timestamp rollover is an issue (139 years!)
    * ctl_timer_exec_expire_event and ctl_timer_get_time_to_next_event will have to
    * be careful about where they pick the next timer to expire.
    */
   if (tmrHandle->numEvents == 0)
   {
      tmrHandle->events = newEvent;
   }
   else 
   {
      currEvent = tmrHandle->events;

      if (IS_EARLIER_THAN(&(newEvent->expireTms), &(currEvent->expireTms)))
      {
         /* queue at the head */
         newEvent->next = currEvent;
         tmrHandle->events = newEvent;
      }
      else
      {
         tsl_bool_t done = TSL_B_FALSE;

         while (!done)
         {
            prevEvent = currEvent;
            currEvent = currEvent->next;

            if ((currEvent == NULL) ||
                (IS_EARLIER_THAN(&(newEvent->expireTms), &(currEvent->expireTms))))
            {
               newEvent->next = prevEvent->next;
               prevEvent->next = newEvent;
               done = TSL_B_TRUE;
            }
         }
      }
   }

   tmrHandle->numEvents++;

/*
   ctllog_debug("added event %s, expires in %ums (at %u.%03u), func=0x%x data=%p count=%d",
                newEvent->name,
                ms,
                newEvent->expireTms.sec,
                newEvent->expireTms.nsec/NSECS_IN_MSEC,
                func,
                ctxData,
                tmrHandle->numEvents);
*/

   return TSL_RV_SUC;
}  


void ctl_timer_cancel(void *handle, CmsEventHandler func, void *ctxData)
{
   CmsTimerHandle *tmrHandle = (CmsTimerHandle *) handle;
   CmsTimerEvent *currEvent, *prevEvent;

   if ((currEvent = tmrHandle->events) == NULL)
   {
      ctllog_debug("no events to delete (func=0x%x data=%p)", func, ctxData);
      return;
   }

   if (currEvent->func == func && currEvent->ctxData == ctxData)
   {
      /* delete from head of the queue */
      tmrHandle->events = currEvent->next;
      currEvent->next = NULL;
   }
   else
   {
      tsl_bool_t done = TSL_B_FALSE;

      while ((currEvent != NULL) && (!done))
      {
         prevEvent = currEvent;
         currEvent = currEvent->next;

         if (currEvent != NULL && currEvent->func == func && currEvent->ctxData == ctxData)
         {
            prevEvent->next = currEvent->next;
            currEvent->next = NULL;
            done = TSL_B_TRUE;
         }
      }
   }

   if (currEvent != NULL)
   {
      tmrHandle->numEvents--;

      ctllog_debug("canceled event %s, count=%d", currEvent->name, tmrHandle->numEvents);

      CTLMEM_FREE_BUF_AND_NULL_PTR(currEvent);
   }
   else
   {
      ctllog_debug("could not find requested event to delete, func=0x%x data=%p count=%d",
                   func, ctxData, tmrHandle->numEvents);
   }

   return;
}

tsl_rv_t ctl_timer_get_time_to_next_event(const void *handle, tsl_u32_t *ms)
{
   CmsTimerHandle *tmrHandle = (CmsTimerHandle *) handle;
   CmsTimerEvent *currEvent;
   CmsTimestamp nowTms;

   ctl_timer_get(&nowTms);
   currEvent = tmrHandle->events;

   if (currEvent == NULL)
   {
      *ms = MAX_UINT32;
      return CMSRET_NO_MORE_INSTANCES;
   }

   /* this is the same code as in dumpEvents, integrate? */
   if (IS_EARLIER_THAN(&(currEvent->expireTms), &nowTms))
   {
      /*
       * the next event is past due (nowTms is later than currEvent),
       * so time to next event is 0.
       */
      *ms = 0;
   }
   else
   {
      /*
       * nowTms is earlier than currEvent, so currEvent is still in
       * the future.  
       */
      (*ms) = ctl_timer_deltea_ms(&(currEvent->expireTms), &nowTms);
   }

   return TSL_RV_SUC;
}


void ctl_timer_exec_expire_event(void *handle)
{
   CmsTimerHandle *tmrHandle = (CmsTimerHandle *) handle;
   CmsTimerEvent *currEvent;
   CmsTimestamp nowTms;

   ctl_timer_get(&nowTms);
   currEvent = tmrHandle->events;

   while ((currEvent != NULL) && (IS_EARLIER_THAN(&(currEvent->expireTms), &nowTms)))
   {
      /*
       * first remove the currEvent from the tmrHandle because
       * when we execute the callback function, it might call the
       * cmsTmr API again.
       */
      tmrHandle->events = currEvent->next;
      currEvent->next = NULL;
      tmrHandle->numEvents--;

/*      ctllog_debug("executing timer event %s func 0x%x data 0x%x",
                   currEvent->name, currEvent->func, currEvent->ctxData);
*/
      /* call the function */
      (*currEvent->func)(currEvent->ctxData);

      /* free the event struct */
      free(currEvent);

      currEvent = tmrHandle->events;
   }

   return;
}


tsl_bool_t ctl_timer_is_event_present(const void *handle, CmsEventHandler func, void *ctxData)
{
   const CmsTimerHandle *tmrHandle = (const CmsTimerHandle *) handle;
   CmsTimerEvent *tmrEvent;
   tsl_bool_t found=TSL_B_FALSE;

   tmrEvent = tmrHandle->events;

   while ((tmrEvent != NULL) && (!found))
   {
      if (tmrEvent->func == func && tmrEvent->ctxData == ctxData)
      {
         found = TSL_B_TRUE;
      }
      else
      {
         tmrEvent = tmrEvent->next;
      }
   }

   return found;
}

void ctl_timer_get(CmsTimestamp *tms)
{
   struct timespec ts;
   tsl_32_t rc;

   if (tms == NULL)
   {
      return;
   }

   rc = clock_gettime(CLOCK_MONOTONIC, &ts);
   if (rc == 0)
   {
      tms->sec = ts.tv_sec;
      tms->nsec = ts.tv_nsec;
   }
   else
   {
      ctllog_error("clock_gettime failed, set timestamp to 0");
      tms->sec = 0;
      tms->nsec = 0;
   }
}


void ctl_timer_delta(const CmsTimestamp *newTms,
                  const CmsTimestamp *oldTms,
                  CmsTimestamp *deltaTms)
{
   if (newTms->sec >= oldTms->sec)
   {
      if (newTms->nsec >= oldTms->nsec)
      {
         /* no roll-over in the sec and nsec fields, straight subtract */
         deltaTms->nsec = newTms->nsec - oldTms->nsec;
         deltaTms->sec = newTms->sec - oldTms->sec;
      }
      else
      {
         /* no roll-over in the sec field, but roll-over in nsec field */
         deltaTms->nsec = (NSECS_IN_SEC - oldTms->nsec) + newTms->nsec;
         deltaTms->sec = newTms->sec - oldTms->sec - 1;
      }
   }
   else
   {
      if (newTms->nsec >= oldTms->nsec)
      {
         /* roll-over in the sec field, but no roll-over in the nsec field */
         deltaTms->nsec = newTms->nsec - oldTms->nsec;
         deltaTms->sec = (MAX_UINT32 - oldTms->sec) + newTms->sec + 1; /* +1 to account for time spent during 0 sec */
      }
      else
      {
         /* roll-over in the sec and nsec fields */
         deltaTms->nsec = (NSECS_IN_SEC - oldTms->nsec) + newTms->nsec;
         deltaTms->sec = (MAX_UINT32 - oldTms->sec) + newTms->sec;
      }
   }
}


tsl_u32_t ctl_timer_deltea_ms(const CmsTimestamp *newTms,
										const CmsTimestamp *oldTms)
{
	CmsTimestamp deltaTms;
	tsl_u32_t ms;

	ctl_timer_delta(newTms, oldTms, &deltaTms);

	if (deltaTms.sec > MAX_UINT32 / MSECS_IN_SEC)
	{
		/* the delta seconds is larger than the tsl_u32_t return value, so return max value */
		ms = MAX_UINT32;
	}
	else
	{
		ms = deltaTms.sec * MSECS_IN_SEC;

		if ((MAX_UINT32 - ms) < (deltaTms.nsec / NSECS_IN_MSEC))
		{
			/* overflow will occur when adding the nsec, return max value */
			ms = MAX_UINT32;
		}
		else
		{
			ms += deltaTms.nsec / NSECS_IN_MSEC;
		}
	}

	return ms;
}


tsl_rv_t ctl_timer_get_time(tsl_u32_t t, char *buf, tsl_u32_t bufLen)
{
	int          c;
	time_t       now;
	struct tm   *tmp;

	if (t == 0)
	{
	  now = time(NULL);
	}
	else
	{
	  now = t;
	}

	tmp = localtime(&now);
	memset(buf, 0, bufLen);
	c = strftime(buf, bufLen, "%Y-%m-%dT%H:%M:%S%z", tmp);
	if ((c == 0) || (c+1 > bufLen))
	{
	  /* buf was not long enough */
	  return CMSRET_RESOURCE_EXCEEDED;
	}

	/* fix missing : in time-zone offset-- change -500 to -5:00 */
	buf[c+1] = '\0';
	buf[c] = buf[c-1];
	buf[c-1] = buf[c-2];
	buf[c-2]=':';

	return TSL_RV_SUC;
}


void ctl_timer_add_ms(CmsTimestamp *tms, tsl_u32_t ms)
{
   tsl_u32_t addSeconds;
   tsl_u32_t addNano;

   addSeconds = ms / MSECS_IN_SEC;
   addNano = (ms % MSECS_IN_SEC) * NSECS_IN_MSEC;

   tms->sec += addSeconds;
   tms->nsec += addNano;

   /* check for carry-over in nsec field */
   if (tms->nsec > NSECS_IN_SEC)
   {
      /* we can't have carried over by more than 1 second */
      tms->sec++;
      tms->nsec -= NSECS_IN_SEC;
   }

   return;
}
