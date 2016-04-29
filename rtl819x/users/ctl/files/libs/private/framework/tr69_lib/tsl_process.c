#include "libtsl_process.h"
#include "tsl_process_imp.h"
#include "ctl_log.h"
#include <sys/wait.h>

#define _DEBUG_PROCESS

tsl_list_head_t global_process_list_head = {&global_process_list_head, &global_process_list_head};

#define MEM_FREE_BUF_AND_NULL_PTR(p) \
   do { \
      if ((p) != NULL) {free((p)); (p) = NULL;}   \
   } while (0)

static void list_all_process_status(void)
{
	tsl_process_t *p_process = NULL;
	tsl_list_head_t *p_list = NULL;

	ctllog_debug("/*************PMD: list all processes***********\n");
	for(p_list = global_process_list_head.prev; p_list != &global_process_list_head;)
	{
		p_process = tsl_list_entry(p_list, tsl_process_t, list);
		if (p_process == NULL)
		{
			ctllog_error("p_process is NULL!!! Why?\n");
		}
		else
		{
			ctllog_debug("path=%s, argv=%s, pid=%d, status=%d\n", p_process->path, p_process->argv, p_process->pid, (int)p_process->state);
		}
		p_list = p_list->prev;
	}
	ctllog_debug("********************************************/\n");
}

/** Free all the arg buffers in the argv, and the argv array itself.
 *
 */
void freeArgs(char **argv)
{
   tsl_u32_t i=0;

   while (argv[i] != NULL)
   {
      MEM_FREE_BUF_AND_NULL_PTR(argv[i]);
      i++;
   }

   MEM_FREE_BUF_AND_NULL_PTR(argv);
}

/** Give a single string, allocate and fill in an array of char *'s
 * each pointing to an individually malloc'd buffer containing a single
 * argument in the string; the array will end with a char * slot containing NULL.
 *
 * This array can be passed to execv.
 * This array must be freed by calling freeArgs.
 */
tsl_rv_t parseArgs(const char *cmd, const char *args, char ***argv)
{
   tsl_u32_t numArgs=3, i, len, argIndex=0;
   tsl_bool inSpace=TSL_B_TRUE;
   const char *cmdStr;
   char **array;

   len = (args == NULL) ? 0 : strlen(args);

   /*
    * First count the number of spaces to determine the number of args
    * there are in the string.
    */
   for (i=0; i < len; i++)
   {
      if ((args[i] == ' ') && (!inSpace))
      {
         numArgs++;
         inSpace = TSL_B_TRUE;
      }
      else
      {
         inSpace = TSL_B_FALSE;
      }
   }

   array = (char **) malloc((numArgs) * sizeof(char *));
   if (array == NULL)
   {
      ctllog_error( "%s@%d ===>>> malloc of %ld failed\n", __FUNCTION__,__LINE__,numArgs);
      return tsl_rv_fail_mem;
   }

   memset( array, 0x0, (numArgs) * sizeof(char *));
   /* locate the command name, last part of string */
   cmdStr = strrchr(cmd, '/');
   if (cmdStr == NULL)
   {
      cmdStr = cmd;
   }
   else
   {
      cmdStr++;  /* move past the / */
   }

   /* copy the command into argv[0] */
   array[argIndex] = malloc(strlen(cmdStr) + 1);
   if (array[argIndex] == NULL)
   {
      ctllog_error("%s@%d ===>>> malloc of %d failed\n", __FUNCTION__,__LINE__,strlen(cmdStr) + 1);
      freeArgs(array);
      return tsl_rv_fail_mem;
   }
   else
   {
      strcpy(array[argIndex], cmdStr);
      argIndex++;
   }


   /*
    * Wow, this is going to be incredibly messy.  I have to malloc a buffer
    * for each arg and copy them in individually.
    */
   inSpace = TSL_B_TRUE;
   for (i=0; i < len; i++)
   {
      if ((args[i] == ' ') && (!inSpace))
      {
         numArgs++;
         inSpace = TSL_B_TRUE;
      }
      else if ((args[i] != ' ') && (inSpace))
      {
            tsl_u32_t startIndex, endIndex;

            /*
             * this is the first character I've seen after a space.
             * Figure out how many letters this arg is, malloc a buffer
             * to hold it, and copy it into the buffer.
             */
            startIndex = i;
            endIndex = i;
            while ((endIndex < len) && (args[endIndex] != ' '))
            {
               endIndex++;
            }

            array[argIndex] = malloc(endIndex - startIndex + 1);
			memset(array[argIndex], 0, endIndex - startIndex + 1);
            if (array[argIndex] == NULL)
            {
               ctllog_error("malloc of %ld failed\n", endIndex - startIndex + 1);
               freeArgs(array);
               return tsl_rv_fail_mem;
            }

            memcpy(array[argIndex], &args[startIndex], endIndex - startIndex);
            
            //fprintf(stderr, "%s@%d ===>>> index=%d len=%d (%s)\n", __FUNCTION__,__LINE__,argIndex, endIndex - startIndex, &args[startIndex]);
            

            argIndex++;

            inSpace = TSL_B_FALSE;
      }
   }

   /* check what we did */
   i = 0;
   while (array[i] != NULL)
   {
      //fprintf(stderr, "argv[%d] = %s\n", i, array[i]);
      i++;
   }

   (*argv) = array;


   return tsl_rv_suc;
}

tsl_void_t tsl_wait_time(tsl_int_t wait_time)
{
        tsl_uint_t lasp_time = wait_time; 
        
        if (wait_time){
                do{
                        lasp_time = sleep(lasp_time);
                }while(lasp_time != 0);
                
        }
        return;
}


tsl_int_t tsl_process_state(pid_t pid, tsl_process_state_t *p_state, tsl_int_t wait_time)
{
	tsl_int_t w;
	tsl_int_t status;
	
	tsl_wait_time(wait_time);
	
	ctllog_debug("waitpid %d ", pid);
	w = waitpid(pid, &status, WNOHANG | WUNTRACED);

    if (w == -1) {
            ctllog_error("waitpid error\n");
            return w; 
    }
    if (w == 0){
            ctllog_debug(" run %d succ \n", pid);
            *p_state = tsl_process_state_running;
    }else if (w > 0){
            if (WIFEXITED(status)) {
                    ctllog_debug("Aexited, status=%d\n", WEXITSTATUS(status));
                    *p_state = tsl_process_state_stoping;
            } else if (WIFSIGNALED(status)) {
                    ctllog_debug("Akilled by signal %d\n", WTERMSIG(status));
                    *p_state = tsl_process_state_stoping;
            }else if (WIFSTOPPED(status)) {
                    ctllog_debug("Astopped by signal %d\n", WSTOPSIG(status));
            }
#if 0   //BCM maybe couldn't support the function WIFCONTINUED               
            else if (WIFCONTINUED(status)) {
                    printf("Acontinued\n");
            }
#endif                
    }
	
    return w;
}


tsl_rv_t tsl_process_launch(tsl_char_t *p_path, tsl_char_t *p_argv, pid_t *rpid, tsl_process_state_t *p_state)
{
	pid_t pid;
	tsl_rv_t ret;
	char **argv=NULL;     
	if ((ret = parseArgs(p_path, p_argv, &argv)) != tsl_rv_suc)
	{
	  ctllog_error( "%s@%d =========>>>>>>>>>>>> parseArgs error\n", __FUNCTION__, __LINE__);
	  return ret;
	}

	ctllog_debug( "p_path = %s , p_argv = %s\n", p_path, p_argv);
	pid = fork();
	if (pid == 0){
	        execv(p_path, argv);
	        ctllog_error( "fork failed!!!!!\n");
	        exit(-1);
	}
	freeArgs(argv);

	*rpid = pid;

    if (tsl_process_state(pid, p_state, 2) == 0)        
        return TSL_RV_SUC;
	return TSL_RV_ERR;
}

pid_t tsl_process_start(tr69_process_t *p_process_tb)
{            
    tsl_process_t *p_process = NULL;
    tsl_process_state_t state;        
    pid_t pid = 0;
	tsl_char_t *p_path = NULL;
	tsl_char_t *p_argv = NULL;

	ctllog_debug("tsl_process_start entered\n");
	if (NULL == p_process_tb)
	{
		ctllog_error("%s[%d]: invalid parameter! p_process_tb\n", __FILE__, __LINE__);
        return 0;
	}
	p_path = p_process_tb->path;
	p_argv = p_process_tb->argv;
    if(access(p_path, F_OK) != 0)
    {
		ctllog_debug("%s[%d]: return here access fail\n", __FILE__, __LINE__);
        return 0;
    }
    if(tsl_process_launch(p_path, p_argv, &pid, &state) != TSL_RV_SUC)
    {
        ctllog_debug("%s[%d]: launch failed\n", __FILE__, __LINE__);
        return 0;
    }
    
    p_process = _CALLOC(1, sizeof(tsl_process_t));
	if (NULL == p_process)
	{
		ctllog_debug("%s[%d]: malloc failed! p_process\n", __FILE__, __LINE__);
        return 0;
	}
    sprintf(p_process->path, "%s", p_path);
    sprintf(p_process->argv, "%s", p_argv);
    p_process->pid = pid;
    p_process->state = state;
    tsl_list_add(&(p_process->list), &global_process_list_head);
	
    ctllog_debug("%s[%d]: tsl_list_add process successfully!\n", __FILE__, __LINE__);
	
#ifdef _DEBUG_PROCESS
	list_all_process_status();
#endif

	return pid;
}

tsl_rv_t tsl_process_stop(tr69_process_t *p_process_tb)
{            
    tsl_process_t *p_process = NULL;
    tsl_list_head_t  *p_list = NULL;
	tsl_char_t *p_path = NULL;
	
	ctllog_debug("tsl_process_stop entered!!\n");
    if (NULL == p_process_tb)
	{
		ctllog_error("%s[%d]: invalid parameter! p_process_tb\n", __FILE__, __LINE__);
        return 0;
	}
	p_path = p_process_tb->path;

	if (p_process_tb->is_lowerver)
	{
		for(p_list = global_process_list_head.prev; p_list != &global_process_list_head;){
		        p_process = tsl_list_entry(p_list, tsl_process_t, list);
		        
		        if (!strcmp(p_process->path, p_path)){
		                if (p_process->state == tsl_process_state_running){
							ctllog_debug( "tsl_process_stop: path=%s, pid=%d\n", p_process->path, p_process->pid);
	                            if (strstr(p_path, "pppd")){
	                                kill(p_process->pid, SIGKILL);
	                            }
	                            else{
			                        kill(p_process->pid, SIGTERM);
	                            }
		                }
		                p_process->state = tsl_process_state_stoping;
		                return TSL_RV_SUC;
		        }
		        p_list = p_list->prev;
		}
	}
	else
	{
		for(p_list = global_process_list_head.prev; p_list != &global_process_list_head;)
		{
			p_process = tsl_list_entry(p_list, tsl_process_t, list);
			if ((p_process->pid == p_process_tb->pid) && (!strcmp(p_process->path, p_path)))
			{		
				if (p_process->state == tsl_process_state_running)
				{
				    	if (strstr(p_path, "pppd")){
						kill(p_process->pid, SIGKILL);
                    		}else{
    						kill(p_process->pid, SIGTERM);
                   			}
							
					p_process->state = tsl_process_state_stoping;
					return TSL_RV_SUC;
				}
			}
			p_list = p_list->prev;
		}
	}	
#ifdef _DEBUG_PROCESS
	list_all_process_status();
#endif

    return TSL_RV_ERR;
}

pid_t tsl_process_restart(tr69_process_t *p_process_tb)
{
	tsl_char_t *p_path = NULL;

	if (NULL == p_process_tb)
	{
		ctllog_error("%s[%d]: invalid parameter! p_process_tb\n", __FILE__, __LINE__);
        return 0;
	}
	p_path = p_process_tb->path;
	
	tsl_process_stop(p_process_tb);
    //tsl_wait_time(2);
	
    return tsl_process_start(p_process_tb);
}

#include "libtsl_process.h"

void process_signal_func(int signum, siginfo_t* info, void*ptr)
{
	tsl_process_state_t state;
	tsl_process_t *p_process = NULL;
	tsl_list_head_t *p_list = NULL;
	tsl_int_t pid = 0;

	while ((pid = tsl_process_state(-1, &state, 0)) > 0)
	{
		ctllog_debug("%s[%d]: signal, pid=%d\n", __FILE__, __LINE__, pid);
		for(p_list = global_process_list_head.prev; p_list != &global_process_list_head; p_list = p_list->prev)
		{
	        p_process = tsl_list_entry(p_list, tsl_process_t, list);			
	        if (p_process->pid == pid)
			{
                ctllog_debug( "signal_func pid %d state %d\n", pid, state);
				tsl_list_del(p_list);
				_FREE(p_process);
				break;
	        }
		}
	}
	
#ifdef _DEBUG_PROCESS
	list_all_process_status();
#endif

	return;
}

void process_sig_handle_init()
{
        struct sigaction action;
        
        memset(&action, 0, sizeof(action));
        action.sa_sigaction = process_signal_func;
        action.sa_flags = SA_SIGINFO;
        sigaction(SIGCLD, &action, NULL);
        
        signal(SIGKILL, SIG_DFL);
        signal(SIGINT, SIG_DFL);
        signal(SIGTERM, SIG_DFL);
}

tsl_rv_t tsl_process_init(struct tr69_process_s *p_process_tb, tsl_int_t numb) 
{
	tsl_int_t i;

	process_sig_handle_init();        
	for (i = 0; i < numb; i++)
	{
		tsl_process_start(&p_process_tb[i]);
	}

	return TSL_RV_SUC;
}

tsl_rv_t tsl_process_cleanup()
{
       tsl_process_t *p_process = NULL;
       tsl_list_head_t     *p_list = NULL;
       tsl_process_state_t state = tsl_process_state_running;       

       for(p_list = global_process_list_head.prev; p_list != &global_process_list_head;){
               p_process = tsl_list_entry(p_list, tsl_process_t, list);
               
               p_list = p_list->prev;
               
               if (state == tsl_process_state_running){
                       kill(p_process->pid, SIGTERM);
               }
               _FREE(p_process);
       }
       
       return TSL_RV_SUC;
}

#if 0

int main()
{
        tsl_process_init(tr69_process_tb, sizeof(tr69_process_tb)/sizeof(struct tr69_process_s));
        
        printf("1time %u\n", time(NULL));

        tsl_wait_time(10);
        
        printf("2time %u\n", time(NULL));
        
        tsl_process_restart(APP_DATA_CENTER);
        
        tsl_wait_time(10);
        
        printf("3time %u\n", time(NULL));

        tsl_wait_time(10);
        tsl_process_cleanup();

        printf("4time %u\n", time(NULL));        
        
        tsl_wait_time(3);
        return 0;
}

#endif


pid_t tsl_app_start(tsl_char_t * apppath,tsl_char_t * argv)
{
	pid_t pid = 0;
	char **p_argv=NULL;     
	int ret = 0;
	tsl_process_t *p_process = NULL;
	
	if( !apppath || !argv) 
	{
		ctllog_error("%s[%d]: return here param errors\n", __FILE__, __LINE__);
		return pid;
	}
	
	if ((ret = parseArgs(apppath, argv, &p_argv)) != tsl_rv_suc)
	{
		ctllog_error( "%s@%d =========>>>>>>>>>>>> parseArgs error\n", __FUNCTION__, __LINE__);
		return ret;
	}

	if( access(apppath, F_OK) != 0)
	{
		ctllog_error("%s[%d]: return here access [%s] fail\n", __FILE__, __LINE__,apppath);
		/* Coverity #19791 */
		freeArgs(p_argv);		
	    	return pid;
	}

	pid = fork();
	if (pid == 0){
	        execv(apppath, p_argv);
		 exit(-1);
	}
	freeArgs(p_argv);

	p_process = _CALLOC(1, sizeof(tsl_process_t));
	if (NULL == p_process)
	{
		ctllog_error("%s[%d]: malloc failed! p_process\n", __FILE__, __LINE__);
		return 0;
	}
	sprintf(p_process->path, "%s", apppath);
	sprintf(p_process->argv, "%s", argv);
	p_process->pid = pid;
	tsl_list_add(&(p_process->list), &global_process_list_head);
	

	ctllog_debug("%s[%d]: start app [%s %s ] successfully pid[%d]!\n", 
		__FILE__, __LINE__,apppath,argv,pid);

	return pid;
}

int tsl_app_stop(char * apppath,char * argv,pid_t stop_pid)
{
	tsl_process_t *p_process = NULL;
	tsl_list_head_t     *p_list = NULL;
	pid_t pid = stop_pid;
	int ret = TSL_RV_ERR;
	
	do
	{
		for(p_list = global_process_list_head.prev; p_list != &global_process_list_head;)
		{
		        p_process = tsl_list_entry(p_list, tsl_process_t, list);

			if ( ((p_process->pid == stop_pid) && (!strcmp(p_process->path, apppath))) ||
				 (!strcmp(p_process->path, apppath) &&  (!strcmp(p_process->argv, argv))) )
				{
					pid = p_process->pid;
					tsl_list_del(p_list);
					_FREE(p_process);
					break;
				}

			ctllog_debug("****stop app [%s] argv[%s] : list app[%s] list argv[%s]\n",
				apppath,argv,p_process->path,p_process->argv);
			p_list = p_list->prev;
		}

		if( pid == 0 )
		{
			break;
		}
		
		if (strstr(apppath, "pppd")){
		   kill(pid, SIGKILL);
		}
		else{
		    kill(pid, SIGTERM);
		}
		ret = TSL_RV_SUC;
	}
	while(0);

	ctllog_debug("%s[%d]: stop app [%s %s ] pid[%d]!\n", 
	__FILE__, __LINE__,apppath,argv,pid);

	return ret;
}

pid_t tsl_app_restart(char * apppath,char * argv,pid_t stop_pid)
{
//Coverity comment 18533
	if( tsl_app_stop(apppath,argv,stop_pid) != TSL_RV_SUC ) {
        ctllog_warn( "Fail to tsl_app_stop(%s, %s, %d)", apppath, argv, stop_pid );
    }

	return tsl_app_start( apppath,argv);
}

int tsl_app_stop_all()
{
	tsl_process_t *p_process = NULL;
	tsl_list_head_t     *p_list = NULL;

	for(p_list = global_process_list_head.prev; p_list != &global_process_list_head;){

		p_process = tsl_list_entry(p_list, tsl_process_t, list);
		p_list = p_list->prev;
		kill(p_process->pid, SIGTERM);
		_FREE(p_process);
	}
       
       return TSL_RV_SUC;
}

