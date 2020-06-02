#include <stdlib.h>
#include "vmsize.h"
#include "../../allocator.c"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Please supply an output file name\n");
        return 1;
    }

    void *a = malloc(4000);
    void *b = malloc(4000);
    void *c = malloc(4000);

    FILE *fp = fopen(argv[1], "w");
    write_memory(fp);
    fclose(fp);

    return 0;
}
