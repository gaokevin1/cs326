#ifndef _WORKLOAD_H_
#define _WORKLOAD_H_

#include <limits.h>

/* This "dummy function" is used to simulate a real CPU workload. It is
 * basically just incrementing/decrementing a couple of counters. */
#define RUN_WORKLOAD(count) \
{ unsigned long i, j; for (i = 0, j = UINT_MAX; i < count; ++i, --j); }

unsigned long calibrate_workload(void);
double get_time(void);

#endif
