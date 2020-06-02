#include <stdlib.h>
#include "vmsize.h"
#include "../../allocator.c"

int main(void)
{
    unsigned long vm_start = vmsize();

    void *a = malloc(500);
    void *b = malloc(1000);
    void *c = malloc(250);
    void *d = malloc(250);
    void *e = malloc(500);

    free(b);
    free(d);

    void *f = malloc(600);
    void *g = malloc(150);
    void *h = malloc(50);

    unsigned long vm_end = vmsize();

    printf("Pages at start = %lu; pages after allocations = %lu\n",
            vm_start, vm_end);
    if (vm_end - vm_start > 1) {
        printf("Allocated too many pages!\n");
        return 1;
    }

    return 0;
}
