#include "timer.h"
#include <stdlib.h>

double get_time() {
    struct timeval tVal;
    gettimeofday(&tVal, NULL);
    double time_value = tVal.tv_sec + tVal.tv_usec / 1000000.0;
    return time_value;
}

