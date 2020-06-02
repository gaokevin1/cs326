#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>

/* Function prototypes */
void initialize(void) __attribute__((constructor));
int open(const char *pathname, int flags);

/* Function pointers */
int (*orig_open)(const char * pathname, int flags) = NULL;

/* Preprocessor Directives */
#ifndef DEBUG
#define DEBUG 1
#endif
/**
 * Logging functionality. Set DEBUG to 1 to enable logging, 0 to disable.
 */
#define LOG(fmt, ...) \
    do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
            __LINE__, __func__, __VA_ARGS__); } while (0)

void initialize(void)
{
    orig_open = (int (*)(const char *, int)) dlsym(RTLD_NEXT, "open");
    LOG("Original open() location: %p\n", orig_open);
    LOG("New open() location: %p\n", open);
}

int open(const char *pathname, int flags)
{
    LOG("Opening file: %s\n", pathname);
    return (*orig_open)(pathname, flags);
}
