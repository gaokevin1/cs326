#ifndef _VMSIZE_H_
#define _VMSIZE_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * Retrieves the virtual memory size of the process, in pages.
 */
unsigned long vmsize(void)
{
    pid_t pid = getpid();
    char path[4096] = {0};
    sprintf(path, "/proc/%d/statm", pid);
    FILE *f = fopen(path, "r");
    unsigned long mem;
    fscanf(f, "%lu ", &mem);
    fclose(f);
    return mem;
}

#endif
