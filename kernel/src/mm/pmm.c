#include <cpuid.h>
#include <stdint.h>

#include <efi.h>

#include <mm/kvmem_layout.h>
#include <mm/vmm.h>
#include <mm/pmm.h>

typedef struct {
    uint64_t physical_address;
    uint64_t available_pages;
} MemMapBlock;

static uint8_t MAXPHYADDR = 0;

static uint64_t _setup_filter_address(uint64_t _addr);

uint64_t(*filter_address)(uint64_t _addr) = &_setup_filter_address;

static uint64_t _filter_address(uint64_t _addr) {
    return _addr & (((uint64_t)1 << MAXPHYADDR) - 1);
}

static uint64_t _setup_filter_address(uint64_t _addr) {
    unsigned int eax = 0, unused = 0;
    __get_cpuid(0x80000008, &eax, &unused, &unused, &unused);    
    MAXPHYADDR = eax & 0xFF;
    filter_address = &_filter_address;
    return filter_address(_addr);
}

static uint64_t available_block_memory = 0; // memory available to store memory description blocks
static uint64_t available_memory = 0;       // memory available for allocation
static uint64_t current_free_block = 0;     // index of first descriptor with at least 1 free page
static uint64_t stored_blocks = 0;          // number of descriptors

static uint8_t* DMA_bitmap = NULL;

int pmm_setup(void) {
    const uint8_t* linfo = (uint8_t*)BOOT_DATA;

    const uint64_t mmap_size = *(uint64_t*)linfo;
    linfo += sizeof(uint64_t);
    
    const uint64_t desc_size = *(uint64_t*)linfo;
    linfo += sizeof(uint64_t);

    uint8_t* const mmap = (uint8_t*)linfo;
    const uint64_t desc_num = mmap_size / desc_size;

    DMA_bitmap = mmap + mmap_size + sizeof(uint8_t) + 3 * sizeof(uint64_t);

    MemMapBlock* current_block_ptr = (MemMapBlock*)PHYSICAL_MEM_MAP;      // pointer to current block

    for (size_t i = 0; i < desc_num; ++i) {
        EFI_MEMORY_DESCRIPTOR* descriptor = (EFI_MEMORY_DESCRIPTOR*)(mmap + i * desc_size);

        if (descriptor->Type == EfiConventionalMemory
                || descriptor->Type == EfiLoaderCode || descriptor->Type == EfiLoaderData
                || descriptor->Type == EfiBootServicesCode || descriptor->Type == EfiBootServicesData
                || descriptor->Type == EfiConventionalMemory || descriptor->Type == LoaderTemporaryMemory
        ) {
            if (descriptor->PhysicalStart < DMA_ZONE_SIZE) {
                int64_t end_dma_offset = descriptor->PhysicalStart + FRAME_SIZE * descriptor->NumberOfPages - DMA_ZONE_SIZE;

                if (end_dma_offset > 0) {
                    descriptor->NumberOfPages = end_dma_offset / FRAME_SIZE;
                    descriptor->PhysicalStart = DMA_ZONE_SIZE;
                }
                else {
                    continue;
                }
            }
            
            if (available_block_memory == 0) {
                if (descriptor->NumberOfPages == 0) {
                    continue;
                }

                // allocate more memory.
                uint64_t page_address = descriptor->PhysicalStart + FRAME_SIZE * (--descriptor->NumberOfPages);

                VIRTUAL_ADDRESS mapping = parse_virtual_address((uint64_t)current_block_ptr);

                PML4E* pml4e = get_pml4e_address(mapping.PML4_offset);

                if ((pml4e->raw & PML4E_PRESENT) == 0) {
                    // use the last page and try again
                    pml4e->raw = ((filter_address(page_address) >> 12) << 12) | PML4E_READWRITE | PML4E_PRESENT;

                    uint64_t* pdpt = (uint64_t*)get_pdpte_address(mapping.PML4_offset, 0);
                    for (size_t i = 0; i < 0x200; ++i) {
                        *(pdpt + i) = 0;
                    }
                    
                    if (descriptor->NumberOfPages == 0) {
                        continue;
                    }
                    page_address = descriptor->PhysicalStart + FRAME_SIZE * (--descriptor->NumberOfPages);
                }

                PDPTE* pdpte = get_pdpte_address(mapping.PML4_offset, mapping.PDPT_offset);

                if ((pdpte->raw & PDPTE_PRESENT) == 0) {
                    pdpte->raw = ((filter_address(page_address) >> 12) << 12) | PDPTE_READWRITE | PDPTE_PRESENT;

                    uint64_t* pd = (uint64_t*)get_pde_address(mapping.PML4_offset, mapping.PDPT_offset, 0);
                    for (size_t i = 0; i < 0x200; ++i) {
                        *(pd + i) = 0;
                    }

                    if (descriptor->NumberOfPages == 0) {
                        continue;
                    }
                    page_address = descriptor->PhysicalStart + FRAME_SIZE * (--descriptor->NumberOfPages);
                }

                PDE* pde = get_pde_address(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset);

                if ((pde->raw & PDE_PRESENT) == 0) {
                    pde->raw = ((filter_address(page_address) >> 12) << 12) | PDE_READWRITE | PDE_PRESENT;

                    uint64_t* pt = (uint64_t*)get_pte_address(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset, 0);
                    for (size_t i = 0; i < 0x200; ++i) {
                        *(pt + i) = 0;
                    }

                    if (descriptor->NumberOfPages == 0) {
                        continue;
                    }
                    page_address = descriptor->PhysicalStart + FRAME_SIZE * (--descriptor->NumberOfPages);
                }

                PTE* pte = get_pte_address(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset, mapping.PT_offset);
                pte->raw = PTE_XD | ((filter_address(page_address) >> 12) << 12) | PTE_READWRITE | PTE_PRESENT;

                if (descriptor->NumberOfPages == 0) {
                    continue;
                }

                available_block_memory += FRAME_SIZE;
            }

            current_block_ptr->physical_address = descriptor->PhysicalStart;
            current_block_ptr->available_pages = descriptor->NumberOfPages;
            ++current_block_ptr;
            ++stored_blocks;
            available_memory += FRAME_SIZE * descriptor->NumberOfPages;
            available_block_memory -= sizeof(MemMapBlock);
        }
    }

    if (current_block_ptr == (MemMapBlock*)PHYSICAL_MEM_MAP) {
        // not enough memory to setup PMM (Physical Memory Manager)
        return -1;
    }

    DMA_bitmap[0] |= 1; // reserve first dma page to make NULL pointers invalid.

    current_free_block = 0;
    return 0;
}

typedef enum {
    DMA,
    PMM,
    GENERAL,
} P_ALLOCATION_TYPE;

uint64_t query_memory_usage() {
    return available_memory;
}

// returns 0 if the address belongs to a free page, 1 if it belongs to an allocated page, -1 if the address is invalid.
int query_dma_address(uint64_t address) {
    uint64_t page = address / FRAME_SIZE;
    return address < DMA_ZONE_SIZE ? ((DMA_bitmap[page / 8] & (1 << (page % 8))) != 0) : -1;
}

/* General physical memory frame allocator, the requested number of pages only matters for DMA allocation,
 * for any other allocation, a single page is returned, no matter what value is stored in pages (even if it is 0).
 * This is why you should not use this allocator directly, but instead use helper function such as
 * dma_pmalloc, pmm_pmalloc and pmalloc.
 */
static inline void* kpmalloc(P_ALLOCATION_TYPE type, uint64_t pages) {
    if (type == DMA) {
        uint64_t start_page = 0;
        uint64_t found_pages = 0;

        for (size_t i = 0; i < DMA_BITMAP_SIZE; ++i) {
            uint8_t byte = DMA_bitmap[i];

            for (size_t j = 0; j < 8; ++j) {
                if ((byte & (1 << j)) != 0) {
                    found_pages = 0;
                }
                else {
                    if (found_pages++ == 0) {
                        start_page = 8 * i + j;
                    }
                }

                if (found_pages >= pages) {
                    for (size_t p = start_page; p < start_page + pages; ++p) {
                        size_t x = p / 8;
                        size_t y = p % 8;

                        DMA_bitmap[x] |= 1 << y;
                    }
                    return (void*)start_page;
                }
            }
        }

        return NULL;
    }

    if (available_memory < pages * FRAME_SIZE || stored_blocks == 0) {
        return NULL;
    }
    
    MemMapBlock* mmb = (MemMapBlock*)PHYSICAL_MEM_MAP + current_free_block;

    void* page = (void*)(mmb->physical_address + FRAME_SIZE * (--mmb->available_pages));
    
    if (mmb->available_pages == 0) {
        ++current_free_block;
        --stored_blocks;
        available_block_memory += sizeof(MemMapBlock);
    }

    available_memory -= FRAME_SIZE;

    if (type == PMM) {
        available_block_memory += FRAME_SIZE;
    }

    return page;
}

void* dma_pmalloc(uint64_t pages) {
    return kpmalloc(DMA, pages);
}

void* pmm_pmalloc(void) {
    return kpmalloc(PMM, 1);
}

void* pmalloc(void) {
    return kpmalloc(GENERAL, 1);
}

/* General physical memory frame deallocator, the requested number of pages only matters for DMA allocation,
 * for any other deallocation, a single page is freed, no matter what value is stored in pages (even if it is 0).
 * This is why you should not use this deallocator directly, but instead use helper function such as
 * dma_free, pmm_free and pfree.
 */
static inline void kpfree(P_ALLOCATION_TYPE type, void* _address, uint64_t pages) {
    const uint64_t address = (uint64_t)_address;

    // check if the address is correctly aligned
    if (address % FRAME_SIZE != 0) {
        return;
    }

    if (type == DMA) {
        // check for invalid parameters
        if (address >= DMA_ZONE || address + FRAME_SIZE * pages > DMA_ZONE || pages > DMA_PAGES) {
            return;
        }

        for (size_t p = 0; p < pages; ++p) {
            size_t x = p / 8;
            size_t y = p % 8;

            DMA_bitmap[x] &= ~(1 << y);
        }

        return;
    }

    if (type == PMM) {
        available_block_memory -= FRAME_SIZE;
    }

    if (current_free_block > 0) {
        MemMapBlock* block_ptr = (MemMapBlock*)PHYSICAL_MEM_MAP + (--current_free_block);
        block_ptr->available_pages = pages;
        block_ptr->physical_address = address;
    }
    else {
        MemMapBlock* block_ptr = (MemMapBlock*)PHYSICAL_MEM_MAP + stored_blocks;

        if (available_block_memory == 0) {
            VIRTUAL_ADDRESS mapping = parse_virtual_address((uint64_t)block_ptr);

            PML4E* pml4e = get_pml4e_address(mapping.PML4_offset);

            if ((pml4e->raw & PML4E_PRESENT) == 0) {
                void* _page = pmalloc();

                if (_page == NULL) {
                    return;
                }

                pml4e->raw = ((filter_address((uint64_t)_page) >> 12) << 12) | PML4E_READWRITE | PML4E_PRESENT;
            }

            PDPTE* pdpte = get_pdpte_address(mapping.PML4_offset, mapping.PDPT_offset);

            if ((pdpte->raw & PDPTE_PRESENT) == 0) {
                void* _page = pmalloc();

                if (_page == NULL) {
                    return;
                }

                pdpte->raw = ((filter_address((uint64_t)_page) >> 12) << 12) | PDPTE_READWRITE | PDPTE_PRESENT;
            }

            PDE* pde = get_pde_address(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset);

            if ((pde->raw & PDE_PRESENT) == 0) {
                void* _page = pmalloc();

                if (_page == NULL) {
                    return;
                }

                pde->raw = ((filter_address((uint64_t)_page) >> 12) << 12) | PDE_READWRITE | PDE_PRESENT;
            }

            PTE* pte = get_pte_address(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset, mapping.PT_offset);
            void* _page = NULL;

            if (type == PMM) {
                _page = _address;
                available_block_memory += FRAME_SIZE;
            }
            else {
                _page = pmm_pmalloc();
            }

            if (_page == NULL) {
                return;
            }

            pte->raw = PTE_XD | ((filter_address((uint64_t)_page) >> 12) << 12) | PTE_READWRITE | PTE_PRESENT;
        }

        block_ptr->available_pages = 1;
        block_ptr->physical_address = address;
    }

    ++stored_blocks;
    available_block_memory -= sizeof(MemMapBlock);
    available_memory += FRAME_SIZE;
}

void dma_pfree(void* _address, uint64_t pages) {
    kpfree(DMA, _address, pages);
}

void pmm_pfree(void* _address) {
    kpfree(PMM, _address, 1);
}

void pfree(void* _address) {
    kpfree(GENERAL, _address, 1);
}
