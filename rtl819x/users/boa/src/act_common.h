/*
 *      Common Function Define
 *
 *      Authors: Erishen  <lsun@actiontec.com>
 *
 */

#ifndef ACT_COMMON_H
#define ACT_COMMON_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include "act_log.h"

int safe_strcmp(const char *s1, const char *s2);
int safe_strlen(const char *s1);
void gui_trim(char *s);
void tolower_string (char *string);
int agets(FILE *f, char *buff);
void convert_bin_to_str(unsigned char *bin, int len, char *out);
char* speciCharEncode(char *s, int len);
void changeString(char *buf);
void convert_hex_to_ascii(unsigned long code, char *out);
int compute_pin_checksum(unsigned long int PIN);


#define CMSMEM_FREE_BUF_AND_NULL_PTR(p) \
   do { \
      if ((p) != NULL) {free((p)); (p) = NULL;}   \
   } while (0)
   

#endif
