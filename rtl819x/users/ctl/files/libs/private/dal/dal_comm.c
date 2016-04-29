#include <stdio.h>
#include <stdarg.h>
#include "include/dal_api.h"
#include "libtr69_func.h"
#include "ctl_validstrings.h"

/****************************************************************/
#define PRINTF_CAT    1
#define PRINTF_ESCAPE 2

#define ctllog_debug(arg...) do { } while(0)

char *itoa(int num)
{
	static char ret[16];

	*ret = 0;
	snprintf(ret, sizeof(ret), "%d", num);
	return ret;
}

char **str_free(char **s)
{
	free(*s);
	*s = NULL;
	return s;
}

char **str_vprintf_full(char **s, int flags, char *fmt, va_list ap)
{
	int rc, rc2;
	int cat_len = 0;
	char *old_s = NULL;
	va_list ap2;

	if ((flags & PRINTF_CAT) && *s)
		cat_len = strlen(*s);
	/* New version of vsnprintf returns the number of characters that would
	 * have been written if the buffer was big enough, excluding the trailing
	 * NULL.
	 */
	va_copy(ap2, ap);
#if defined(__SUNOS__)
	/* a hack for Sun OS - vsnprintf(bufsize=0) returns -1, instead of
	 * the expected formatted length. Giving it a buffer of 1 char solves
	 * this problem.
	 */
	{
		char b;
		rc = vsnprintf(&b, 1, fmt, ap2);
	}
#else
	rc = vsnprintf(NULL, 0, fmt, ap2);
#endif
	va_end(ap2);

	if (rc<0)
		ctllog_error("Error in printf format\n");
	if (!cat_len)
	{
		old_s = *s;
		*s = NULL;
	}
	*s = realloc(*s, cat_len+rc+1);
	rc2 = vsnprintf(*s+cat_len, rc+1, fmt, ap);
	free(old_s);
	if (rc2 < 0)
		ctllog_error("Failed vnsprintf\n"); /* this should never happen */
	return s;
}

char **str_printf(char **s, char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	str_vprintf_full(s, 0, fmt, arg);
	va_end(arg);
	return s;
}

char **str_cat(char **s, char *str)
{
	char *news;
	int len;
	if (!*s)
	{
		*s = strdup(str);
		return s;
	}
	len = strlen(*s);
	news = malloc(len+strlen(str)+1);
	strcpy(news, *s);
	strcpy(news+len, str);
	free(*s);
	*s = news;
	return s;
}

int init_dal_ret_t(dal_ret_t* t)
{
	int i;

	if (t == NULL)
		return TSL_B_FALSE;
	// Coverity CID 19236
	//if (t->param == NULL)
	//	return TSL_B_FALSE;
	t->errorMsg = NULL;
	for (i = 0 ; i < DAL_MAX_PARAM_NUM; i++)
		t->param[i] = NULL;

	return TSL_B_TRUE;
}

void free_one_dal_ret(dal_ret_t *dal_ret)
{
    int i = 0;

    if (dal_ret==NULL)
        return;

    ctllog_debug("%s: enter!\n", __FUNCTION__);

    if(NULL != dal_ret->errorMsg)
    {
        ctllog_debug("%s: Freeing Msg %s\n", __FUNCTION__, dal_ret->errorMsg);
        free(dal_ret->errorMsg);
        dal_ret->errorMsg = NULL;
    }

    for ( ; i < DAL_MAX_PARAM_NUM; i++)
    {
        if(NULL != dal_ret->param[i])
        {
            ctllog_debug("%s: Freeing Param[%d] %s in %p\n",
                    __FUNCTION__, i, dal_ret->param[i], dal_ret->param[i]);
            free(dal_ret->param[i]);
            dal_ret->param[i] = NULL;
        }
    }

    ctllog_debug("%s: exit!\n", __FUNCTION__);

    return;
}

void free_dal_ret(dal_ret_t **dal_ret)
{
    ctllog_debug("%s: enter!\n", __FUNCTION__);

    if(dal_ret == NULL || *dal_ret == NULL)
        return;

    free_one_dal_ret(*dal_ret);

    ctllog_debug("%s: %p\n", __FUNCTION__, *dal_ret);
    free(*dal_ret);
    *dal_ret = NULL;

    ctllog_debug("%s: exit!\n", __FUNCTION__);

    return;
}

