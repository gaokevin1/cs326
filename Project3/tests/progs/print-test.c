#include <stdlib.h>
#include "vmsize.h"
#include "../../allocator.c"

int main(void)
{
    void *a = malloc(4000);
    void *b = malloc(4000);
    void *c = malloc(4000);

    print_memory();

    return 0;
}
