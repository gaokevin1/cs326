#include <stdlib.h>
#include "../../allocator.c"

int main(void)
{
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

    print_memory();

    return 0;
}
