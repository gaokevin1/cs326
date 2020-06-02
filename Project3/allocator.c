/**
 * allocator.c
 *
 * Explores memory management at the C runtime level.
 *
 * Author: Kevin and Angel 
 *
 * To use (one specific command):
 * LD_PRELOAD=$(pwd)/allocator.so command
 * ('command' will run with your allocator)
 *
 * To use (all following commands):
 * export LD_PRELOAD=$(pwd)/allocator.so
 * (Everything after this point will use your custom allocator -- be careful!)
 */

#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>
#include <pthread.h>

#ifndef DEBUG
#define DEBUG 0
#endif

#define LOG(fmt, ...) \
        do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)

#define LOGP(str) \
    do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): %s", __FILE__, \
            __LINE__, __func__, str); } while (0)

struct mem_block {
    /* Each allocation is given a unique ID number. If an allocation is split in
     * two, then the resulting new block will be given a new ID. */
    unsigned long alloc_id;

    /* Size of the memory region */
    size_t size;

    /* Space used; if usage == 0, then the block has been freed. */
    size_t usage;

    /* Pointer to the start of the mapped memory region. This simplifies the
     * process of finding where memory mappings begin. */
    struct mem_block *region_start;

    /* If this block is the beginning of a mapped memory region, the region_size
     * member indicates the size of the mapping. In subsequent (split) blocks,
     * this is undefined. */
    size_t region_size;

    /* Next block in the chain */
    struct mem_block *next;
};

/* Start (head) of our linked list: */
struct mem_block *g_head = NULL;
struct mem_block *g_tail = NULL;

/* Allocation counter: */
unsigned long g_allocations = 0;
pthread_mutex_t g_alloc_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * print_memory
 *
 * Prints out the current memory state, including both the regions and blocks.
 * Entries are printed in order, so there is an implied link from the topmost
 * entry to the next, and so on.
 */
void print_memory(void)
{
    puts("-- Current Memory State --");
    struct mem_block *current_block = g_head;
    struct mem_block *current_region = NULL;
    while (current_block != NULL) {
        if (current_block->region_start != current_region) {
            current_region = current_block->region_start;
            printf("[REGION] %p-%p %zu\n",
                    current_region,
                    (void *) current_region + current_region->region_size,
                    current_region->region_size);
        }
        printf("[BLOCK]  %p-%p (%ld) %zu %zu %zu\n",
                current_block,
                (void *) current_block + current_block->size,
                current_block->alloc_id,
                current_block->size,
                current_block->usage,
                current_block->usage == 0
                    ? 0 : current_block->usage - sizeof(struct mem_block));
        current_block = current_block->next;
    }
}

/**
 * write_memory
 *
 * Prints out the current memory state, including both the regions and blocks.
 * However, instead of a void, it prints from a FILE passed through the method
 * argument, *fp.
 */
void *write_memory(FILE *fp) {
    puts("-- Current Memory State --");
    struct mem_block *current_block = g_head;
    struct mem_block *current_region = NULL;
    
    while (current_block != NULL) {
        if (current_block-> region_start != current_region) {
            current_region = current_block-> region_start;
            fprintf(fp,"[REGION] %p-%p %zu\n",
                    current_region,
                    (void *) current_region + current_region-> region_size,
                    current_region-> region_size);
        }

        fprintf(fp,"[BLOCK]  %p-%p (%ld) %zu %zu %zu\n",
                current_block,
                (void *) current_block + current_block-> size,
                current_block-> alloc_id,
                current_block-> size,
                current_block-> usage,
                current_block-> usage == 0
                    ? 0 : current_block->usage - sizeof(struct mem_block));
        current_block = current_block-> next;
    }
 }

/**
 * reuse
 *
 * This method contains all of the free memory algorithms used
 * in the program. When it is called it will find a suitable
 * block of memory to reuse and reallocate for new stuff. If no
 * suitable block is found the method will return NULL.
 */
void *reuse(size_t size) {
    char *algo = getenv("ALLOCATOR_ALGORITHM");
    if (algo == NULL) {
        algo = "first_fit";
    }
    
    struct mem_block *current_block = g_head;

    /* First Fit algorithm, 
     * will find the first block that's big enough
     */
    if (strcmp(algo, "first_fit") == 0) {
        while (current_block != NULL) {
            if (current_block->size - current_block->usage > size + sizeof(struct mem_block)) {
                return current_block;
            }
            current_block = current_block->next;
        }
        return current_block;

    /* Best fit algorithm, 
     * will find the best possible sized block to reuse
     */
    } else if (strcmp(algo, "best_fit") == 0) {
        struct mem_block *return_block = NULL;
        size_t usage = INT_MAX;
        while (current_block != NULL) {
            if (current_block-> size - current_block-> usage >= size + sizeof(struct mem_block)) {
                if (current_block-> size - current_block-> usage <= usage) {
                    usage = current_block-> size - current_block-> usage;
                    return_block = current_block;
                }
            }
            current_block = current_block-> next;
        }
        return return_block;

    /* Worst fit algorithm, 
     * will find the largest empty space and segment it further to make room
     */
    } else if (strcmp(algo, "worst_fit") == 0) {
        struct mem_block *return_block = NULL;
        size_t usage = 0;
        while (current_block != NULL) {
            if (current_block-> size - current_block-> usage >= size + sizeof(struct mem_block)) {
                if(current_block-> size - current_block-> usage >= usage) {
                    usage = current_block-> size - current_block-> usage;
                    return_block = current_block;
                }
            }
            current_block = current_block-> next;
        }
        return return_block;
    }
    return NULL;
}

/**
 * malloc
 *
 * This method dynamically allocates a block of memory on the heap. The method
 * returns a pointer that the program accesses for a block of memory. When the 
 * memory is no longer needed, the pointer is passed to free where it deallocates
 * the physical space required for other information.
 */
void *malloc(size_t size) {
    /* Realign memory requests to 8 bytes (64 bits) */
    if (size % 8 != 0) {
        size = size + (8 - size % 8);                      
    } 
    
    struct mem_block *reusable_block = NULL;
    reusable_block = reuse(size);

    if (reusable_block != NULL) {
        size_t real_sz = size + sizeof(struct mem_block);

        if (reusable_block->usage == 0) {
            reusable_block-> alloc_id = g_allocations++;

            /* Space used */
            reusable_block-> usage = real_sz;

            if (getenv("ALLOCATOR_SCRIBBLE")){
                memset(reusable_block+1, 0xAA, size);
            }
            return reusable_block + 1;

        } else {
            struct mem_block *block = (void *) reusable_block + reusable_block->usage;
            block-> alloc_id = g_allocations++;
            /* Size of the memory region */
            block-> size = reusable_block->size - reusable_block->usage;
            /* Space used */
            block-> usage = real_sz;
            /* Pointer to the start of the mapped memory region. */
            block-> region_start = reusable_block->region_start;
            /* Indicates the size of the mapping. */
            block-> region_size = reusable_block->region_size;
            /* Next block in the chain */
            block-> next = reusable_block->next;

            reusable_block->size = reusable_block->usage;

            reusable_block->next = block;

            if (getenv("ALLOCATOR_SCRIBBLE")){
                memset(block+1, 0xAA, size);
            }
            return block + 1;
        }
    }

    size_t real_sz = size + sizeof(struct mem_block);
    int page_sz = getpagesize();

    size_t pages = real_sz / page_sz; 

    /* Make sure page won't be left over */
    if (real_sz % page_sz != 0) {
        pages++;
    }

    size_t region_sz = pages * page_sz;

    struct mem_block *block = mmap(
        NULL, /* Address (we use NULL to let the kernel decide) */
        region_sz, /* Size of memory block to allocate */
        PROT_READ | PROT_WRITE, /* Memory protection flags */
        MAP_PRIVATE | MAP_ANONYMOUS, /* Type of mapping */
        -1, /* file descriptor */
        0); /* offset to start at within the mapping */

    if (block == MAP_FAILED){
        perror("mmap");
        return NULL;
    }

    /* Each allocation is given a unique ID number. */
    block-> alloc_id = g_allocations++;
    /* Size of the memory region */
    block-> size = region_sz;
    /* Space used */
    block-> usage = real_sz;
    /* Pointer to the start of the mapped memory region. */
    block-> region_start = block;
    /* Indicates the size of the mapping. */
    block-> region_size = region_sz;
    /* Next block in the chain */
    block-> next = NULL;

    /* Linked list implementation */
    if (g_head == NULL){
        g_head = block;
    } else {
        struct mem_block *current = g_head;       
        while (current != NULL) {  
            if (current-> next == NULL){
                current-> next = block;   
                break; 
            }     
            current = current-> next;                 
        }                                      
    }

    if (getenv("ALLOCATOR_SCRIBBLE")) {
        memset(block + 1, 0xAA, size);
    }
    return block + 1;
}

/**
 * free
 *
 * This method frees blocks of memory, requested by the address given from
 * the pointer passed through the method argument.
 */
void free(void *ptr)
{
    /* Freeing a NULL pointer does nothing */
    if (ptr == NULL) {
        return;
    }

    struct mem_block *blk = (struct mem_block*) ptr - 1;
    
    /* Go to region start */
    blk-> usage = 0;
    bool free = true;
    struct mem_block * current = blk-> region_start;

    /* Traverse through linked list */
    while (current != NULL && current-> region_start == blk-> region_start) {
        /* Stop when finding something that is not free */
        if (current-> usage != 0) {
            free = false;
            break;
        }
        current = current-> next;
    }

    if (free) {
        /* Update the linked list */
        if (g_head == blk-> region_start) {
            g_head = current;
        } else {
            struct mem_block *tmp = g_head;
            while (tmp != NULL && tmp-> next != blk-> region_start) {
                tmp = tmp-> next;
            }
            tmp-> next = current;
        }

        /* Catch munmap error */
        int ret = munmap(blk-> region_start, blk-> region_size);
        if (ret == -1) {
            perror("munmap");
        }
    } 
}

/**
 * calloc
 *
 * This method is used to allocate a block of memory of a particular size using
 * malloc. However, unlike malloc it sets the newly allocated region to 0.
 */
void *calloc(size_t nmemb, size_t size) {
    /* Malloc a pointer of size (nmemb * size) */
    void *ptr = malloc(nmemb * size);
    /* Set everything to 0 */
    memset(ptr, 0, nmemb * size);
    return ptr;
}

/**
 * realloc
 *
 * This method takes in a pointer and a size_t size variable and reallocates
 * precious memory without losing the original data. Assuming memory blocks have
 * already been created to house information.
 */
void *realloc(void *ptr, size_t size) {
    /* Realign memory requests to 8 bytes (64 bits) */
    if (size % 8 != 0) {                              
        size = size + (8 - size % 8);                 
        LOG("Aligned size: %zu\n", size);             
    } 
    /* If the pointer is NULL, then we simply malloc a new block */
    if (ptr == NULL) {
        return malloc(size);
    }

    /* Realloc to 0 is often the same as freeing the memory block... But the
     * C standard doesn't require this. We will free the block and return
     * NULL here. */
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    return NULL;
}
