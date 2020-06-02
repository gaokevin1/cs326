#define _GNU_SOURCE

#include <ctype.h>
#include <stdbool.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "dirstream.h"
#include "dirent.h"
#include <unistd.h>
#include <dlfcn.h>
#include <stdio.h>

/* Function prototypes */
void initialize(void) __attribute__((constructor));
DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);

/* Function pointers */
int (*orig_open)(const char * pathname, int flags) = NULL;
struct dirent *(*orig_readdir)(DIR *dirp) = NULL;
DIR *(*orig_opendir)(const char *name) = NULL;


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


/* We'll keep track of the open file descriptors here. If the file descriptor
 * being opened matches our target directory ('/' in this case), then we will
 * store in the list. */
int proc_fds[65535];
int num_fds = 0;


void initialize(void)
{
    orig_readdir = (struct dirent *(*)(DIR *dirp)) dlsym(RTLD_NEXT, "readdir");
    orig_opendir = (DIR * (*)(const char *name)) dlsym(RTLD_NEXT, "opendir");
}

DIR *opendir(const char *name)
{
    DIR *dir = orig_opendir(name);

    if (strcmp(name, "/") == 0) {
        /* It's the root directory ... */
        proc_fds[dir-> fd] = true;
    }

    return dir;
}

struct dirent *readdir(DIR *dirp)
{
    struct dirent *entry = orig_readdir(dirp);
    if (entry == NULL) {
        return NULL;
    }

    char name[50];
    strcpy(namef, entry-> d_name);

    if (strcmp(namef, entry-> d_name))
    {
        entry = orig_readdir(dirp);
    }

    return entry;
}
