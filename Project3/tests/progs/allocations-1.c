#include <stdlib.h>
#include "../../allocator.c"

int main(void)
{
    void *a = malloc(100);
    void *b = malloc(100); /* Will be deleted */
    void *c = malloc(100);
    void *d = malloc(10);  /* Will be deleted */
    void *e = malloc(100);
    void *f = malloc(100);

    free(b);
    free(d);

    /* This will split:
     * - b with first fit
     * - d with best fit
     * - f with worst fit
     */
    void *g = malloc(10);

    print_memory();

    return 0;
}
