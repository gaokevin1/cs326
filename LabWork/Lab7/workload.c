#include "workload.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/time.h>

/**
 * Attempts to create a standardized "workload" for the processes to run,
 * regardless of the host CPU speed.
 */
unsigned long calibrate_workload(void)
{
    fprintf(stderr, "%s\n", "Calibrating workload...");
    uint32_t workload = UINT32_MAX / 100;
    double run_time = 0.0;
    double start, end;

    while (true) {
        start = get_time();
        RUN_WORKLOAD(workload);
        end = get_time();
        run_time = end - start;
        fprintf(stderr, "%"PRIu32" -> %.2fs\n", workload, run_time);
        if (run_time > .25) {
            break;
        }
        workload *= 2;
    }

    workload = workload / run_time * .25;
    start = get_time();
    RUN_WORKLOAD(workload);
    end = get_time();
    fprintf(stderr, "%"PRIu32" -> %.2fs\n", workload, end - start);
    return workload;
}

/**
 * Retrieves the current time, in seconds.
 */
double get_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

