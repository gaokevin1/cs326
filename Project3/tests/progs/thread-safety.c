#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#define THREADS_PER_ROUND 2500
#define NUM_ROUNDS 10
#define MAX_ALLOC 2050

void *thread_proc(void *arg)
{
    int i, r;

    r = rand() % MAX_ALLOC;
    int *mem = malloc(sizeof(int) * r);
    for (i = 0; i < r; ++i) {
        mem[i] = (long) arg;
    }

    unsigned long total = 0;
    for (i = 0; i < r; ++i) {
        total += mem[i];
    }

    if (total != ((long) arg) * r) {
        printf("Mismatch of array total! Memory corruption?\n");
    }

    free(mem);
    return 0;
}

double get_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

int main(void)
{
    srand(time(0));

    double start = get_time();
    pthread_t threads[THREADS_PER_ROUND];
    int i, j;

    for (i = 0; i < NUM_ROUNDS; ++i) {
        for (j = 0; j < THREADS_PER_ROUND; ++j) {
            pthread_create(&threads[j], NULL, thread_proc, (void *) (long) j);
        }

        for (j = 0; j < THREADS_PER_ROUND; ++j) {
            pthread_join(threads[j], NULL);
        }
    }

    double end = get_time();
    printf("Performed %d multi-threaded allocations in %.2fs\n",
            THREADS_PER_ROUND * NUM_ROUNDS, (end - start));

    return 0;
}
