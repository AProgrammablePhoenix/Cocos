#pragma once

#include <stdint.h>

// Size of a physical frame (or page), in bytes
#define FRAME_SIZE  4096

#define DMA_PAGES       (DMA_ZONE_SIZE / FRAME_SIZE)
#define DMA_BITMAP_SIZE (DMA_PAGES / 8)

extern uint64_t(*filter_address)(uint64_t _addr);

int pmm_setup(void);

uint64_t query_memory_usage();
int query_dma_address(uint64_t address);

void* dma_pmalloc(uint64_t pages);
void* pmm_pmalloc(void);
void* pmalloc(void);

void dma_pfree(void* _address, uint64_t pages);
void pmm_pfree(void* _address);
void pfree(void* _address);
