#ifndef _FILE_FORMAT_H_
#define _FILE_FORMAT_H_

#include "scheduler.h"

char *next_token(char **str_ptr, const char *delim);
void read_spec(char *file, struct scheduler_state *sstate);

#endif
