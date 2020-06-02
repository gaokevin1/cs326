#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define NUM_ROUNDS 50
#define NUM_ALLOCATIONS 10
#define ALLOC_SZ 5000000

unsigned long vm_start = 0;
unsigned long vm_limit = 0;

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

int main(void)
{
    int i, j;
    int *allocs[NUM_ALLOCATIONS];

    vm_limit = (ALLOC_SZ / getpagesize()) * 2;

    printf("Virtual Memory Size at startup (pages): %lu\n", vmsize());

    for (i = 0; i < NUM_ROUNDS; ++i) {

        for (j = 0; j < NUM_ALLOCATIONS; ++j) {
            unsigned int alloc_sz = rand() % ALLOC_SZ;
            int *a = malloc(alloc_sz * sizeof(int));
            if (a == NULL) {
                exit(1);
            }

            /* If we don't actually do anything with the memory, this all
             * essentially becomes no-ops. */
            memset(a, 0x00, alloc_sz * sizeof(int));
            allocs[j] = a;
        }

        /* Free each allocation */
        for (j = 0; j < NUM_ALLOCATIONS; ++j) {
            free(allocs[j]);
        }

        unsigned long vm = vmsize();
        if (vm_start == 0) {
            vm_start = vm;
        }
        printf("[%d] Virtual Memory Size (pages): %lu\n", i, vm);
        if (vm - vm_start > vm_limit) {
            printf("ERROR: Virtual memory has increased more than %lu pages\n"
                    "since the first round of execution!\n", vm_limit);
            printf("free() is not functioning correctly.\n");
            exit(1);
        }
    }

    return 0;
}
