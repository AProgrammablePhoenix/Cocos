#include <stddef.h>
#include <stdint.h>

#include <mm/pmm.h>
#include <mm/vmm.h>

typedef struct malloc_block_t malloc_block_t;
struct malloc_block_t {
    malloc_block_t* prev;   // Address of the previous block of free memory, NULL if none ; or if allocated, the address of the block itself
    malloc_block_t* next;   // Address of the next block of free memory, NULL if none ; cannot contain the address of an allocated block
    size_t size;            // Full size of the current block, including the block header
};

static struct {
    malloc_block_t* first_block;
} memctx = {
    .first_block = NULL
};

void* malloc(size_t size) {
    // align on a natural boundary
    if (size % sizeof(max_align_t) != 0) {
        size += sizeof(max_align_t) - size % sizeof(max_align_t);
    }
    
    if (memctx.first_block == NULL) {
        void* pages = NULL;

        size_t allocated_pages = 16;

        if (size + sizeof(malloc_block_t) < allocated_pages) {
            pages = kheap_vmalloc(16);
        }
        else {
            allocated_pages = (size + sizeof(malloc_block_t) + FRAME_SIZE - 1) / FRAME_SIZE;
            pages = kheap_vmalloc(allocated_pages);
        }

        if (pages == NULL) {
            return NULL;
        }

        malloc_block_t* allocated = (malloc_block_t*)pages;
        allocated->prev = allocated;
        allocated->size = size + sizeof(malloc_block_t);

        if (allocated_pages * FRAME_SIZE - size - sizeof(malloc_block_t) >= sizeof(malloc_block_t) + sizeof(max_align_t)) {
            memctx.first_block = (malloc_block_t*)((uint8_t*)pages + size + sizeof(malloc_block_t));
            memctx.first_block->prev = NULL;
            memctx.first_block->next = NULL;
            memctx.first_block->size = allocated_pages * FRAME_SIZE - size - sizeof(malloc_block_t);
            allocated->next = memctx.first_block;
        }
        else {
            allocated->next = NULL;
        }

        return (uint8_t*)pages + sizeof(malloc_block_t);
    }

    malloc_block_t* current_block = memctx.first_block;
    while (current_block != NULL) {
        if (current_block->size < size + sizeof(malloc_block_t)) {
            current_block = current_block->next;
            continue;
        }

        malloc_block_t* allocated = current_block;
        malloc_block_t* prev = allocated->prev;
        allocated->prev = allocated;
        allocated->size = size + sizeof(malloc_block_t);
        size_t remaining_size = allocated->size - allocated->size;
        
        if (remaining_size >= sizeof(malloc_block_t) + sizeof(max_align_t)) {
            current_block = (malloc_block_t*)((uint8_t*)allocated + allocated->size);
            current_block->prev = prev;
            current_block->next = allocated->next;
            current_block->size = remaining_size;
            allocated->next = current_block;
            if (current_block->next != NULL) {
                current_block->next->prev = current_block;
            }
            if (memctx.first_block == allocated) {
                memctx.first_block = current_block;
            }
        }
        else {
            allocated->next = NULL;
        }

        return (uint8_t*)allocated + sizeof(malloc_block_t);
    }
    
    return NULL;
}

void free(void* ptr) {
    if (ptr == NULL) {
        return;
    }

    malloc_block_t* blkptr = (malloc_block_t*)((uint8_t*)ptr - sizeof(malloc_block_t));
    uint8_t* blkendptr = (uint8_t*)blkptr + blkptr->size;
    
    if (blkptr->next != NULL) {
        if ((malloc_block_t*)blkendptr == blkptr->next) {
            blkptr->size += blkptr->next->size;
            blkptr->prev = blkptr->next->prev;
            blkptr->next = blkptr->next->next;

            if (blkptr->next != NULL) {
                blkptr->next->prev = blkptr;
            }
            if (blkptr->prev != NULL) {
                blkptr->prev->next = blkptr;
            }
        }
        else {
            blkptr->prev = blkptr->next->prev;
            blkptr->next->prev = blkptr;
            blkptr->prev->next = blkptr;
        }
        
        while (blkptr->prev != NULL && (malloc_block_t*)blkendptr == (malloc_block_t*)((uint8_t*)blkptr->prev + blkptr->prev->size)) {
            blkptr->prev->size += blkptr->size;
            blkptr->prev->next = blkptr->next;
            blkptr->next->prev = blkptr->prev;
            blkptr = blkptr->prev;
        }

        if (blkptr->prev == NULL) {
            memctx.first_block = blkptr;
        }
    }
    else {
        if (memctx.first_block == NULL) {
            memctx.first_block = blkptr;
            blkptr->next = NULL;
            blkptr->prev = NULL;
        }
        else {
            malloc_block_t* ptr = memctx.first_block;
            while (ptr->next != NULL) {
                ptr = ptr->next;
            }
            ptr->next = blkptr;
            blkptr->prev = ptr;
        }
    }
}
