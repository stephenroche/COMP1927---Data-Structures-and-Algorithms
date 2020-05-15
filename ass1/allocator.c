//
//  COMP1927 Assignment 1 - Vlad: the memory allocator
//  allocator.c ... implementation
//
//  Stephen Roche
//  z5059764
//  10/08/16-24/08/16
//

#include "allocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define FREE_HEADER_SIZE  sizeof(struct free_list_header)  
#define ALLOC_HEADER_SIZE sizeof(struct alloc_block_header)  
#define MAGIC_FREE     0xDEADBEEF
#define MAGIC_ALLOC    0xBEEFDEAD

#define MIN_MALLOC_SIZE 1024
#define MIN_ALLOC_SIZE 8

#define BEST_FIT       1
#define WORST_FIT      2
#define RANDOM_FIT     3

typedef unsigned char byte;
typedef u_int32_t vsize_t;
typedef u_int32_t vlink_t;
typedef u_int32_t vaddr_t;

typedef struct free_list_header {
   u_int32_t magic;  // ought to contain MAGIC_FREE
   vsize_t size;     // # bytes in this block (including header)
   vlink_t next;     // memory[] index of next free block
   vlink_t prev;     // memory[] index of previous free block
} free_header_t;

typedef struct alloc_block_header {
   u_int32_t magic;  // ought to contain MAGIC_ALLOC
   vsize_t size;     // # bytes in this block (including header)
} alloc_header_t;

// Global data

static byte *memory = NULL;   // pointer to start of allocator memory
static vaddr_t free_list_ptr; // index in memory[] of first block in free list
static vsize_t memory_size;   // number of bytes malloc'd in memory[]
static u_int32_t strategy;    // allocation strategy (by default BEST_FIT)

// Private functions
int checkFree(free_header_t *ptr)
{
    if (ptr->magic != MAGIC_FREE) {
        fprintf(stderr, "vlad_malloc: Memory corruption\n");
        exit(EXIT_FAILURE);
    }
    return 1;
}

int checkAlloc(alloc_header_t *ptr)
{
    if (ptr->magic != MAGIC_ALLOC) {
        fprintf(stderr, "vlad_malloc: Memory corruption\n");
        exit(EXIT_FAILURE);
    }
    return 1;
}

static void vlad_merge();


// Input: size - number of bytes to make available to the allocator
// Output: none              
// Precondition: Size >= 1024
// Postcondition: `size` bytes are now available to the allocator
// 
// (If the allocator is already initialised, this function does nothing,
//  even if it was initialised with different size)

void vlad_init(u_int32_t size)
{
    // If vlad has already been initialised
    if (memory != NULL) {
        return;
    }

    int nBytes = MIN_MALLOC_SIZE;
    while (nBytes < size) {
        nBytes *= 2;
    }

    memory = malloc(nBytes);
    if (memory == NULL) {
        fprintf(stderr, "vlad_init: Insufficient memory\n");
        exit(EXIT_FAILURE);
    }

    memory_size = nBytes;
    free_list_ptr = (vaddr_t)0;
    strategy = BEST_FIT;

    free_header_t *initial = (free_header_t *)memory;

    initial->magic = MAGIC_FREE;
    initial->size = memory_size;
    initial->next = 0;
    initial->prev = 0;

    checkFree(initial);
}


// Input: n - number of bytes requested
// Output: p - a pointer, or NULL
// Precondition: n < size of largest available free block
// Postcondition: If a region of size n or greater cannot be found, p = NULL 
//                Else, p points to a location immediately after a header block
//                      for a newly-allocated region of some size >= 
//                      n + header size.

void *vlad_malloc(u_int32_t n)
{
    if (strategy != BEST_FIT) {
        return NULL;
    }

    // Sets minimum size for allocated blocks
    int size = MIN_ALLOC_SIZE + ALLOC_HEADER_SIZE;
    
    while (size < n + ALLOC_HEADER_SIZE) {
        size += 4;
    }

    free_header_t *freeBlockPtr = (free_header_t *)(memory + free_list_ptr);
    free_header_t *newBlockPtr = NULL;

    // Searches free list for smallest block of adequate size
    int firstLoop = 1;
    while (freeBlockPtr != (free_header_t *)(memory + free_list_ptr) || firstLoop == 1) {
        firstLoop = 0;

        if (checkFree(freeBlockPtr) && freeBlockPtr->size >= size && (newBlockPtr == NULL || freeBlockPtr->size < newBlockPtr->size)) {
            newBlockPtr = freeBlockPtr;
        }

        freeBlockPtr = (free_header_t *)(memory + freeBlockPtr->next);
    }

    // If no block large enough was found, or if allocating block would result in an empty free list
    if (newBlockPtr == NULL || (newBlockPtr == (free_header_t *)(memory + newBlockPtr->next) && newBlockPtr->size < (size + 2 * FREE_HEADER_SIZE))) {
        return NULL;
    }

    checkFree(newBlockPtr);
    checkFree((free_header_t *)(memory + newBlockPtr->prev));
    checkFree((free_header_t *)(memory + newBlockPtr->next));
    free_header_t *firstFree;

    if (newBlockPtr->size < (size + 2 * FREE_HEADER_SIZE)) { // Don't split block
        ((free_header_t *)(memory + newBlockPtr->prev))->next = newBlockPtr->next;
        ((free_header_t *)(memory + newBlockPtr->next))->prev = newBlockPtr->prev;

        // Size of allocated block will be larger than requested
        size = newBlockPtr->size;

        // Needed for later, firstFree must point somewhere in the free list
        firstFree = (free_header_t *)(memory + newBlockPtr->prev);

    } else { // Split block
        free_header_t *newFreeBlockPtr = (free_header_t *)((byte *)newBlockPtr + size);
        // Needed for later, firstFree must point somewhere in the free list
        firstFree = newFreeBlockPtr;

        newFreeBlockPtr->magic = MAGIC_FREE;
        newFreeBlockPtr->size = newBlockPtr->size - size;

        // If there are other free regions in the free list
        if ((free_header_t *)(memory + newBlockPtr->next) != newBlockPtr) {
            newFreeBlockPtr->next = newBlockPtr->next;
            newFreeBlockPtr->prev = newBlockPtr->prev;
            ((free_header_t *)(memory + newBlockPtr->prev))->next = (byte *)newFreeBlockPtr - memory;
            ((free_header_t *)(memory + newBlockPtr->next))->prev = (byte *)newFreeBlockPtr - memory;

        // Unless we are splitting the last free region:
        } else {
            newFreeBlockPtr->next = (byte *)newFreeBlockPtr - memory;
            newFreeBlockPtr->prev = (byte *)newFreeBlockPtr - memory;
        }

        checkFree(newFreeBlockPtr);
    }

    // Reset free_list_ptr to first region in free list
    while (checkFree(firstFree) && firstFree->prev < (byte *)firstFree - memory) {
        firstFree = (free_header_t *)(memory + firstFree->prev);
    }

    free_list_ptr = (byte *)firstFree - memory;

    alloc_header_t *allocBlockPtr = (alloc_header_t *)newBlockPtr;
    allocBlockPtr->magic = MAGIC_ALLOC;
    allocBlockPtr->size = size;

    checkAlloc(allocBlockPtr);

    return ((void *)allocBlockPtr + ALLOC_HEADER_SIZE);
}


// Input: object, a pointer.
// Output: none
// Precondition: object points to a location immediately after a header block
//               within the allocator's memory.
// Postcondition: The region pointed to by object has been placed in the free
//                list, and merged with any adjacent free blocks; the memory
//                space can be re-allocated by vlad_malloc

void vlad_free(void *object)
{
    if ((byte *)object < memory || (byte *)object >= memory + memory_size) {
        fprintf(stderr, "vlad_free: Attempt to free via invalid pointer\n");
        exit(EXIT_FAILURE);
    }

    if ((byte *)object - ALLOC_HEADER_SIZE < memory || ((alloc_header_t *)(object - ALLOC_HEADER_SIZE))->magic != MAGIC_ALLOC) {
        fprintf(stderr, "vlad_free: Attempt to free non-allocated memory\n");
        exit(EXIT_FAILURE);
    }

    // Make object point to start of allocated region
    object -= ALLOC_HEADER_SIZE;
    free_header_t *newFree = (free_header_t *)object;
    newFree->magic = MAGIC_FREE;
    newFree->size = ((alloc_header_t *)object)->size;

    // Find the region in the free list immediately following the region to be freed
    free_header_t *nextFree = (free_header_t *)(memory + free_list_ptr);
    int firstLoop = 1;

    while (nextFree < newFree && (firstLoop == 1 || nextFree != (free_header_t *)(memory + free_list_ptr))) {
        firstLoop = 0;
        nextFree = (free_header_t *)(memory + nextFree->next);
    }

    newFree->prev = nextFree->prev;
    newFree->next = (byte *)nextFree - memory;
    ((free_header_t *)(memory + nextFree->prev))->next = (byte *)newFree - memory;
    nextFree->prev = (byte *)newFree - memory;

    // If the newly freed region is the first in free list
    if ((byte *)newFree < memory + free_list_ptr) {
        free_list_ptr = (byte *)newFree - memory;
    }

    vlad_merge();
}

// Input: current state of the memory[]
// Output: new state, where any adjacent blocks in the free list
//            have been combined into a single larger block; after this,
//            there should be no region in the free list whose next
//            reference is to a location just past the end of the region

static void vlad_merge()
{
    free_header_t *currFreePtr = (free_header_t *)(memory + free_list_ptr);
    free_header_t *nextFreePtr;

    while (currFreePtr->next != free_list_ptr) {
        nextFreePtr = (free_header_t *)(memory + currFreePtr->next);

        // If the two free regions are immediately adjacent
        if (nextFreePtr == (free_header_t *)((byte *)currFreePtr + currFreePtr->size)) {
            ((free_header_t *)(memory + nextFreePtr->next))->prev = (byte *)currFreePtr - memory;
            currFreePtr->next = nextFreePtr->next;
            currFreePtr->size += nextFreePtr->size;
        } else {
            currFreePtr = nextFreePtr;
        }
    }
}

// Stop the allocator, so that it can be init'ed again:
// Precondition: allocator memory was once allocated by vlad_init()
// Postcondition: allocator is unusable until vlad_int() executed again

void vlad_end(void)
{
    if (memory != NULL) {
        free(memory);
        // Set memory to NULL to be able to test whether vlad has been ended
        memory = NULL;
    }
}


// Precondition: allocator has been vlad_init()'d
// Postcondition: allocator stats displayed on stdout

void vlad_stats(void)
{
    printf("Memory = %p\n", memory);
    printf("Memory size = %d\n", memory_size);
    printf("Free list pointer = %d\n", free_list_ptr);
    if (strategy == BEST_FIT) {
        printf("Strategy = BEST_FIT\n\n");
    } else {
        printf("Strategy = WORST_FIT or RANDOM_FIT\n\n");
    }

    // Scans free list and prints out information about free regions
    printf("Free memory blocks:\n");
    free_header_t *freeBlockPtr = (free_header_t *)(memory + free_list_ptr);
    int first = 1;
    int i = 1;

    while (first == 1 || ((byte *)freeBlockPtr - memory) != free_list_ptr) {
        first = 0;

        printf("%d:\n", i++);
        printf("Address = %d\n", ((byte *)freeBlockPtr - memory));
        printf("Size = %d\n", freeBlockPtr->size);
        printf("Next = %d\n", freeBlockPtr->next);
        printf("Prev = %d\n\n", freeBlockPtr->prev);

        freeBlockPtr = (free_header_t *)(memory + freeBlockPtr->next);   
    }

    return;
}

