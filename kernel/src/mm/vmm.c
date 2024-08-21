#include <stdint.h>

#include <mm/pmm.h>
#include <mm/vmm.h>

#define NULL ((void*)0)

VIRTUAL_ADDRESS parse_virtual_address(uint64_t address) {
    VIRTUAL_ADDRESS vaddr = {
        .PML4_offset    = (address >> 39) & 0x1FF,
        .PDPT_offset    = (address >> 30) & 0x1FF,
        .PD_offset      = (address >> 21) & 0x1FF,
        .PT_offset      = (address >> 12) & 0x1FF,
        .offset         = address & 0xFFF
    };

    return vaddr;
}

uint64_t build_virtual_address(const VIRTUAL_ADDRESS* address) {
    return  (((address->PML4_offset & 0x100) != 0) ? 0xFFFF000000000000 : 0)
            | ((uint64_t)address->PML4_offset << 39)
            | ((uint64_t)address->PDPT_offset << 30)
            | ((uint64_t)address->PD_offset << 21)
            | ((uint64_t)address->PT_offset << 12)
            | address->offset;
}

static inline PTE* get_pt_address(uint64_t pml4_offset, uint64_t pdpt_offset, uint64_t pd_offset) {
    return (PTE*)(PAGING_LOOP1 | (pml4_offset << 30) | (pdpt_offset << 21) | (pd_offset << 12));
}

static inline PDE* get_pd_address(uint64_t pml4_offset, uint64_t pdpt_offset) {
    return (PDE*)(PAGING_LOOP2 | (pml4_offset << 21) | (pdpt_offset << 12));
}

static inline PDPTE* get_pdpt_address(uint64_t pml4_offset) {
    return (PDPTE*)(PAGING_LOOP3 | (pml4_offset << 12));
}

static inline PML4E* get_pml4_address(void) {
    return (PML4E*)(PAGING_LOOP4);
}

PTE* get_pte_address(uint64_t pml4_offset, uint64_t pdpt_offset, uint64_t pd_offset, uint64_t pt_offset) {
    return (PTE*)((uint64_t)get_pt_address(pml4_offset, pdpt_offset, pd_offset) | (pt_offset * sizeof(PTE)));
}

PDE* get_pde_address(uint64_t pml4_offset, uint64_t pdpt_offset, uint64_t pd_offset) {
    return (PDE*)((uint64_t)get_pd_address(pml4_offset, pdpt_offset) | (pd_offset * sizeof(PDE)));
}

PDPTE* get_pdpte_address(uint64_t pml4_offset, uint64_t pdpt_offset) {
    return (PDPTE*)((uint64_t)get_pdpt_address(pml4_offset) | (pdpt_offset * sizeof(PDPTE)));
}

PML4E* get_pml4e_address(uint64_t pml4_offset) {
    return (PML4E*)((uint64_t)get_pml4_address() | (pml4_offset * sizeof(PML4E)));
}

// maps a physical frame to a virtual page
//  0: OK
// -1: one of the parameters is not aligned on a 4KB boundary
// -2: out of memory
static int map_4KB(uint64_t physical_frame, uint64_t virtual_frame) {
    if (physical_frame % FRAME_SIZE != 0 || virtual_frame % FRAME_SIZE != 0) {
        return -1;
    }

    VIRTUAL_ADDRESS virt = parse_virtual_address(virtual_frame);

    PML4E* pml4e = get_pml4e_address(virt.PML4_offset);
    if ((pml4e->raw & PML4E_PRESENT) == 0) {
        void* page = pmalloc();
        if (page == NULL) {
            return -2;
        }
        pml4e->raw = ((filter_address((uint64_t)page) >> 12) << 12) | PML4E_READWRITE | PML4E_PRESENT;

        uint64_t* pdpt = (uint64_t*)get_pdpt_address(virt.PML4_offset);
        for (size_t i = 0; i < FRAME_SIZE / sizeof(uint64_t); ++i) {
            *(pdpt + i) = 0;
        }
    }

    PDPTE* pdpte = get_pdpte_address(virt.PML4_offset, virt.PDPT_offset);
    if ((pdpte->raw & PDPTE_PRESENT) == 0) {
        void* page = pmalloc();
        if (page == NULL) {
            return -2;
        }
        pdpte->raw = ((filter_address((uint64_t)page) >> 12) << 12) | PDPTE_READWRITE | PDPTE_PRESENT;

        uint64_t* pd = (uint64_t*)get_pd_address(virt.PML4_offset, virt.PDPT_offset);
        for (size_t i = 0; i < FRAME_SIZE / sizeof(uint64_t); ++i) {
            *(pd + i) = 0;
        }
    }

    PDE* pde = get_pde_address(virt.PML4_offset, virt.PDPT_offset, virt.PD_offset);
    if ((pde->raw & PDE_PRESENT) == 0) {
        void* page = pmalloc();
        if (page == NULL) {
            return -2;
        }
        pde->raw = ((filter_address((uint64_t)page) >> 12) << 12) | PDE_READWRITE | PDE_PRESENT;

        uint64_t* pt = (uint64_t*)get_pt_address(virt.PML4_offset, virt.PDPT_offset, virt.PD_offset);
        for (size_t i = 0; i < FRAME_SIZE / sizeof(uint64_t); ++i) {
            *(pt + i) = 0;
        }
    }

    PTE* pte = get_pte_address(virt.PML4_offset, virt.PDPT_offset, virt.PD_offset, virt.PT_offset);
    pte->raw = filter_address(physical_frame) | PTE_READWRITE | PTE_PRESENT;

    return 0;
}

typedef struct {
    uint64_t virtual_start;
    uint64_t available_pages;
} VMemMapBlock;

static uint64_t available_kernel_heap = KERNEL_HEAP_SIZE;
static uint64_t available_user_memory = 0x800000000000 - DMA_ZONE_SIZE;

static struct {
    uint64_t kernel_legacy_heap;
    uint64_t user_memory;
} available_block_memory;

static struct {
    uint64_t kernel_legacy_heap;
    uint64_t user_memory;
} stored_blocks;

// initializes the virtual memory manager
//  0: OK
// -1: FAILED
int vmm_setup(void) {
    // setup identity paging for the DMA zone
    for (size_t i = DMA_ZONE; i < DMA_ZONE + DMA_ZONE_SIZE; i += FRAME_SIZE) {
        if (query_dma_address(i) == 1) {
            map_4KB(i, i);
        }
    }

    // make the NULL memory page reserved and unusable
    PTE* null_pte = get_pte_address(0, 0, 0, 0);    
    uint64_t null_pte_address = null_pte->raw & PTE_ADDRESS;
    dma_pfree((void*)null_pte_address, 1);
    null_pte->raw = 0;

    // setup legacy heap (kernel heap zone 0)
    void* base_page = pmalloc();
    if (base_page == NULL) {
        return -1;
    }

    if (map_4KB((uint64_t)base_page, KERNEL_VMEM_MAP) != 0) {
        return -1;
    }

    available_block_memory.kernel_legacy_heap = FRAME_SIZE - sizeof(VMemMapBlock);
    stored_blocks.kernel_legacy_heap = 1;

    VMemMapBlock* kvmem_map_block = (VMemMapBlock*)KERNEL_VMEM_MAP;
    kvmem_map_block->virtual_start = KERNEL_HEAP;
    kvmem_map_block->available_pages = available_kernel_heap / FRAME_SIZE;

    // setup user address space (first 128 TB of virtual address space, without the first 16MB, reserved for DMA)
    base_page = pmalloc();
    if (base_page == NULL) {
        return -1;
    }

    if (map_4KB((uint64_t)base_page, USER_VMEM_MAP) != 0) {
        return -1;
    }

    available_block_memory.user_memory = FRAME_SIZE - sizeof(VMemMapBlock);
    stored_blocks.user_memory = 1;

    VMemMapBlock* umem_map_block = (VMemMapBlock*)USER_VMEM_MAP;
    umem_map_block->virtual_start = DMA_ZONE + DMA_ZONE_SIZE;
    umem_map_block->available_pages = available_user_memory / FRAME_SIZE;

    return 0;
}

typedef enum {
    DMA,
    KERN_HEAP,
    USER
} V_ALLOCATION_TYPE;

// sorts (to the right) by descending number of pages, returns the new address of start
static inline VMemMapBlock* sort_vmem_map(VMemMapBlock* start, size_t n) {
    if (n == 0 || n == 1) {
        return start;
    }

    VMemMapBlock temp;
    for (size_t i = 0; i < n - 1 && (start + 1)->available_pages > start->available_pages; ++i) {
        temp = *start;
        *start = *(start + 1);
        *(++start) = temp;
    }

    return start;
}

// reverse sorts (sorts to the left) by descending number of pages, returns the new address of start
static inline VMemMapBlock* rsort_vmem_map(VMemMapBlock* start, size_t n) {
    if (n == 0 || n == 1) {
        return start;
    }

    VMemMapBlock temp;
    for (size_t i = 0; i < n - 1 && (start - 1)->available_pages < start->available_pages; ++i) {
        temp = *start;
        *start = *(start - 1);
        *(--start) = temp;
    }

    return start;
}

#define KERNEL_ACCESS   0
#define USER_ACCESS     1

#define KERNEL_ON_DEMAND    0x7FFFFFFFFFFFFFFE
#define USER_ON_DEMAND      0xFFFFFFFFFFFFFFFE

// maps the given range on-demand (physical frame allocation is reserved to the page-fault handler)
// BE CAUTIOUS when using this function as it ERASES ANY INFORMATION stored in the given range,
// make sure the latter is free before calling this function.
static inline int map_on_demand(void* address, uint64_t pages, int privilege) {
    for (size_t i = 0; i < pages; ++i) {
        VIRTUAL_ADDRESS virt = parse_virtual_address((uint64_t)address);

        PML4E* pml4e = get_pml4e_address(virt.PML4_offset);
        if ((pml4e->raw & PML4E_PRESENT) == 0) {
            void* page = pmalloc();
            if (page == NULL) {
                return -1;
            }
            pml4e->raw = ((filter_address((uint64_t)page) >> 12) << 12) | (privilege == USER_ACCESS ? PML4E_USERMODE : 0) | PML4E_READWRITE | PML4E_PRESENT;

            uint64_t* pdpt = (uint64_t*)get_pdpt_address(virt.PML4_offset);
            for (size_t i = 0; i < FRAME_SIZE / sizeof(uint64_t); ++i) {
                *(pdpt + i) = 0;
            }
        }

        PDPTE* pdpte = get_pdpte_address(virt.PML4_offset, virt.PDPT_offset);
        if ((pdpte->raw & PDPTE_PRESENT) == 0) {
            void* page = pmalloc();
            if (page == NULL) {
                return -1;
            }
            pdpte->raw = ((filter_address((uint64_t)page) >> 12) << 12) | (privilege == USER_ACCESS ? PDPTE_USERMODE : 0) | PDPTE_READWRITE | PDPTE_PRESENT;

            uint64_t* pd = (uint64_t*)get_pd_address(virt.PML4_offset, virt.PDPT_offset);
            for (size_t i = 0; i < FRAME_SIZE / sizeof(uint64_t); ++i) {
                *(pd + i) = 0;
            }
        }

        PDE* pde = get_pde_address(virt.PML4_offset, virt.PDPT_offset, virt.PD_offset);
        if ((pde->raw & PDE_PRESENT) == 0) {
            void* page = pmalloc();
            if (page == NULL) {
                return -1;
            }
            pde->raw = ((filter_address((uint64_t)page) >> 12) << 12) | (privilege == USER_ACCESS ? PDE_USERMODE : 0) | PDE_READWRITE | PDE_PRESENT;

            uint64_t* pt = (uint64_t*)get_pt_address(virt.PML4_offset, virt.PDPT_offset, virt.PD_offset);
            for (size_t i = 0; i < FRAME_SIZE / sizeof(uint64_t); ++i) {
                *(pt + i) = 0;
            }
        }

        PTE* pte = get_pte_address(virt.PML4_offset, virt.PDPT_offset, virt.PD_offset, virt.PT_offset);
        pte->raw = privilege == USER_ACCESS ? USER_ON_DEMAND : KERNEL_ON_DEMAND;

        address = (uint8_t*)address + FRAME_SIZE;
    }

    return 0;
}

static inline void* kvmalloc_user_hint_core(VMemMapBlock* vmem_block, uint64_t pages, size_t map_offset) {
    void* pages_start = (void*)vmem_block->virtual_start;

    vmem_block->available_pages -= pages;
    vmem_block->virtual_start += FRAME_SIZE * pages;

    if (map_on_demand(pages_start, pages, USER_ACCESS) != 0) {
        vmem_block->available_pages += pages;
        vmem_block->virtual_start -= FRAME_SIZE * pages;
        return NULL;
    }

    sort_vmem_map(vmem_block, stored_blocks.user_memory - map_offset);

    available_user_memory -= FRAME_SIZE * pages;
    return pages_start;
}

static inline int expand_vmem_map(uint64_t start, uint64_t* blocks) {
    VIRTUAL_ADDRESS pte_virt = parse_virtual_address(start + *blocks * sizeof(VMemMapBlock));
                            
    PDPTE* pdpte = get_pdpte_address(pte_virt.PML4_offset, pte_virt.PDPT_offset);
    if ((pdpte->raw & PDPTE_PRESENT) == 0) {
        void* new_page = pmalloc();
        if (new_page == NULL) {
            return -1;
        }
        pdpte->raw = ((filter_address((uint64_t)new_page) >> 12) << 12) | PDPTE_READWRITE | PDPTE_PRESENT;

        uint64_t* pd = (uint64_t*)get_pd_address(pte_virt.PML4_offset, pte_virt.PDPT_offset);
        for (size_t i = 0; i < FRAME_SIZE / sizeof(uint64_t); ++i) {
            *(pd + i) = 0;
        }
    }

    PDE* pde = get_pde_address(pte_virt.PML4_offset, pte_virt.PDPT_offset, pte_virt.PD_offset);
    if ((pde->raw & PDE_PRESENT) == 0) {
        void* new_page = pmalloc();
        if (new_page == NULL) {
            return -1;
        }
        pde->raw = ((filter_address((uint64_t)new_page) >> 12) << 12) | PDE_READWRITE | PDE_PRESENT;

        uint64_t* pt = (uint64_t*)get_pt_address(pte_virt.PML4_offset, pte_virt.PDPT_offset, pte_virt.PD_offset);
        for (size_t i = 0; i < FRAME_SIZE / sizeof(uint64_t); ++i) {
            *(pt + i) = 0;
        }
    }

    PTE* pte = get_pte_address(pte_virt.PML4_offset, pte_virt.PDPT_offset, pte_virt.PD_offset, pte_virt.PT_offset);            
    void* new_page = pmalloc();
    if (new_page == NULL) {
        return -1;
    }
    pte->raw = ((filter_address((uint64_t)new_page) >> 12) << 12) | PTE_READWRITE | PTE_PRESENT;

    return 0;
}

// this function is obviously poorly written, a lot of code is duplicated...
// could do map_on_demand before sorting the map, this would remove a lot of unnecessary calls to rsort.
static inline void* kvmalloc(V_ALLOCATION_TYPE type, void* _ptr, uint64_t pages) {
    if (pages == 0) {
        return NULL;
    }

    if (type == DMA) {
        void* dma_page = dma_pmalloc(pages);
        if (dma_page == NULL) {
            return NULL;
        }
        for (size_t i = 0; i < pages; ++i) {
            if (map_4KB((uint64_t)dma_page + i * FRAME_SIZE, (uint64_t)dma_page + i * FRAME_SIZE) != 0) {
                return NULL;
            }
        }

        return dma_page;
    }
    else if (type == KERN_HEAP) {
        VMemMapBlock* vmem_block = (VMemMapBlock*)KERNEL_VMEM_MAP;

        if (available_kernel_heap < pages * FRAME_SIZE || vmem_block->available_pages < pages) {
            return NULL;
        }

        size_t i = 0;

        // find the best fit
        for (++i, ++vmem_block;; ++i, ++vmem_block) {
            if (i >= stored_blocks.kernel_legacy_heap || vmem_block->available_pages < pages) {
                --i;
                --vmem_block;
                break;
            }
        }

        void* pages_start = (void*)(vmem_block->virtual_start + (vmem_block->available_pages - pages) * FRAME_SIZE);

        int remove = vmem_block->available_pages == 0;
        
        // on-demand paging
        if (map_on_demand(pages_start, pages, KERNEL_ACCESS) != 0) {
            return NULL;
        }

        vmem_block->available_pages -= pages;

        sort_vmem_map(vmem_block, stored_blocks.kernel_legacy_heap - i);

        if (remove) {
            --stored_blocks.kernel_legacy_heap;
            available_block_memory.kernel_legacy_heap += sizeof(VMemMapBlock);

            if (available_block_memory.kernel_legacy_heap % FRAME_SIZE == 0) {
                available_block_memory.kernel_legacy_heap -= FRAME_SIZE;
                VIRTUAL_ADDRESS pte_virt = parse_virtual_address((uint64_t)KERNEL_VMEM_MAP + available_block_memory.kernel_legacy_heap);
                PTE* map_pte = get_pte_address(pte_virt.PML4_offset, pte_virt.PDPT_offset, pte_virt.PD_offset, pte_virt.PT_offset);
                pfree((void*)(map_pte->raw & PTE_ADDRESS));
                map_pte->raw = 0;
            }
        }

        available_kernel_heap -= pages * FRAME_SIZE;

        return pages_start;
    }
    else if (type == USER) {
        VMemMapBlock* vmem_block = (VMemMapBlock*)USER_VMEM_MAP;

        if (available_user_memory < pages * FRAME_SIZE || vmem_block->available_pages < pages) {
            return NULL;
        }

        size_t i = 0;
            
        // find the best fit in terms of pages
        for (++i, ++vmem_block;; ++i, ++vmem_block) {
            if (i >= stored_blocks.user_memory || vmem_block->available_pages < pages) {
                --i;
                --vmem_block;
                break;
            }
        }

        // takes the value of ptr into account ; ptr is only a HINT, not a condition to be met
        // loops in reverse, each time a closer address AFTER ptr is found, change i and vmem_block with this new block
        // then split this block into two new blocks, sort the two w.r.t the amount of pages they reference, and then
        // sort the entire free blocks list, as this process is relatively slow, it is advised not to use this method
        // and to only rely on the default address returned by this allocator without supplying a hint.
        if (_ptr != NULL && ((uint64_t)_ptr % FRAME_SIZE) == 0) {
            const uint64_t ptr = (uint64_t)_ptr;
            size_t i_offset = 0;
            unsigned int fit_found = 0;

            for (size_t j = 0; j <= i; ++j) {
                VMemMapBlock* new_block = vmem_block - j;

                if (new_block->virtual_start <= ptr && ptr + FRAME_SIZE * pages <= new_block->virtual_start + FRAME_SIZE * new_block->available_pages) {
                    // found perfect fit
                    i_offset = j;
                    fit_found = 1;
                    break;
                }
                else if (ptr - new_block->virtual_start < ptr - vmem_block->virtual_start && new_block->virtual_start >= ptr) {
                    if (ptr + FRAME_SIZE * pages <= new_block->virtual_start + FRAME_SIZE * new_block->available_pages) {
                        // found a better fit
                        i_offset = j;
                        fit_found = 1;
                        continue;
                    }
                }

                // no better fit was found
            }

            // if a better fit was found, then split, sort, map and return the address
            // if the number of pages are equal, then it is just the normal process, as the block is removed...
            if (fit_found != 0 && vmem_block->available_pages > pages) {
                vmem_block -= i_offset;
                i -= i_offset;

                // the following code is a pain to explain so good luck :), but it's mostly a bunch of offsets

                // a perfect fit was found
                if (vmem_block->virtual_start <= ptr) {
                    VMemMapBlock prev_block = {
                        .virtual_start = vmem_block->virtual_start,
                        .available_pages = (ptr - vmem_block->virtual_start) / FRAME_SIZE
                    };
                    VMemMapBlock next_block = {
                        .virtual_start = ptr + FRAME_SIZE * pages,
                        .available_pages = (vmem_block->virtual_start + FRAME_SIZE * vmem_block->available_pages - (ptr + FRAME_SIZE * pages)) / FRAME_SIZE
                    };

                    if (prev_block.available_pages == 0) {
                        // you asked, we deliver :)
                        return kvmalloc_user_hint_core(vmem_block, pages, i);
                    }
                    else if (next_block.available_pages == 0) {
                        // this case is trickier as the allocation point is... in the middle of the block :D
                        *vmem_block = prev_block;

                        if (map_on_demand(_ptr, pages, USER_ACCESS) != 0) {
                            vmem_block->available_pages += pages;
                            return NULL;
                        }

                        sort_vmem_map(vmem_block, stored_blocks.user_memory - i);

                        return _ptr; // :D
                    }
                    else {
                        // the worst case scenario, we need to split...
                        if (available_block_memory.user_memory == 0) {
                            if (expand_vmem_map(USER_VMEM_MAP, &stored_blocks.user_memory) != 0) {
                                return NULL;
                            }
                            available_block_memory.user_memory += FRAME_SIZE; 
                        }

                        VMemMapBlock* split_residue = (VMemMapBlock*)USER_VMEM_MAP + stored_blocks.user_memory++;

                        if (prev_block.available_pages > next_block.available_pages) {
                            *vmem_block = prev_block;
                            *split_residue = next_block;
                        }
                        else {
                            *vmem_block = next_block;
                            *split_residue = prev_block;
                        }

                        if (map_on_demand(_ptr, pages, USER_ACCESS) != 0) {
                            return NULL;
                        }

                        sort_vmem_map(vmem_block, stored_blocks.user_memory - i);
                        rsort_vmem_map(split_residue, stored_blocks.user_memory);

                        return _ptr;
                    }
                }
                // a better fit was found
                else {
                    // don't need to split, just need to modify the start of the block...
                    return kvmalloc_user_hint_core(vmem_block, pages, i);
                }
            }
        }

        vmem_block->available_pages -= pages;
        void* pages_start = (void*)(vmem_block->virtual_start + vmem_block->available_pages * FRAME_SIZE);

        int remove = vmem_block->available_pages == 0;

        if (map_on_demand(pages_start, pages, USER_ACCESS) != 0) {
            vmem_block->available_pages += pages;
            return NULL;
        }

        sort_vmem_map(vmem_block, stored_blocks.user_memory - i);

        if (remove) {
            --stored_blocks.user_memory;
            available_block_memory.user_memory += sizeof(VMemMapBlock);

            if (available_block_memory.user_memory % FRAME_SIZE == 0) {
                available_block_memory.user_memory -= FRAME_SIZE;
                VIRTUAL_ADDRESS pte_virt = parse_virtual_address((uint64_t)USER_VMEM_MAP + available_block_memory.user_memory);
                PTE* map_pte = get_pte_address(pte_virt.PML4_offset, pte_virt.PDPT_offset, pte_virt.PD_offset, pte_virt.PT_offset);
                pfree((void*)(map_pte->raw & PTE_ADDRESS));
                map_pte->raw = 0;
            }
        }

        available_user_memory -= pages * FRAME_SIZE;

        return pages_start;
    }

    return NULL;
}

void* dma_vmalloc(uint64_t pages) {
    return kvmalloc(DMA, NULL, pages);
}

void* kheap_vmalloc(uint64_t pages) {
    return kvmalloc(KERN_HEAP, NULL, pages);
}

void* user_vmalloc(uint64_t pages) {
    return kvmalloc(USER, NULL, pages);
}

void* user_vmalloc_at(void* _ptr, uint64_t pages) {
    return kvmalloc(USER, _ptr, pages);
}

static inline void kvfree(V_ALLOCATION_TYPE type, void* _address, uint64_t pages) {
    uint64_t address = (uint64_t)_address;

    if (type == DMA) {
        dma_pfree(_address, pages);
        for (size_t i = 0; i < pages; ++i, address += FRAME_SIZE) {
            VIRTUAL_ADDRESS virt = parse_virtual_address(address);
            PTE* pte = get_pte_address(virt.PML4_offset, virt.PDPT_offset, virt.PD_offset, virt.PT_offset);
            pte->raw = 0;
        }
    }
    else if (type == KERN_HEAP) {
        for (size_t i = 0; i < pages; ++i, address += FRAME_SIZE) {
            VIRTUAL_ADDRESS virt = parse_virtual_address(address);
            PTE* pte = get_pte_address(virt.PML4_offset, virt.PDPT_offset, virt.PD_offset, virt.PT_offset);

            if (pte->raw != KERNEL_ON_DEMAND) {
                void* paddr = (void*)(pte->raw & PTE_ADDRESS);
                pfree(paddr);
            }
            pte->raw = 0;
        }

        VMemMapBlock vmem_block = {
            .virtual_start = (uint64_t)_address,
            .available_pages = pages
        };

        if (available_block_memory.kernel_legacy_heap == 0) {
            void* new_page = pmalloc();
            if (new_page == NULL) {
                return;
            }

            VIRTUAL_ADDRESS pte_virt = parse_virtual_address((uint64_t)KERNEL_VMEM_MAP + stored_blocks.kernel_legacy_heap * sizeof(VMemMapBlock));
            PTE* map_pte = get_pte_address(pte_virt.PML4_offset, pte_virt.PDPT_offset, pte_virt.PD_offset, pte_virt.PT_offset);

            map_pte->raw = ((filter_address((uint64_t)new_page) >> 12) << 12) | PTE_READWRITE | PTE_PRESENT;
            available_block_memory.kernel_legacy_heap += FRAME_SIZE;
        }

        VMemMapBlock* vmem_block_ptr = (VMemMapBlock*)KERNEL_VMEM_MAP + stored_blocks.kernel_legacy_heap++;
        *vmem_block_ptr = vmem_block;
        available_block_memory.kernel_legacy_heap -= sizeof(VMemMapBlock);
        available_kernel_heap += pages * FRAME_SIZE;

        rsort_vmem_map(vmem_block_ptr, stored_blocks.kernel_legacy_heap);
    }
    else if (type == USER) {
        for (size_t i = 0; i < pages; ++i, address += FRAME_SIZE) {
            VIRTUAL_ADDRESS virt = parse_virtual_address(address);
            PTE* pte = get_pte_address(virt.PML4_offset, virt.PDPT_offset, virt.PD_offset, virt.PT_offset);

            if (pte->raw != USER_ON_DEMAND) {
                void* paddr = (void*)(pte->raw & PTE_ADDRESS);
                pfree(paddr);
            }
            pte->raw = 0;
        }

        VMemMapBlock vmem_block = {
            .virtual_start = (uint64_t)_address,
            .available_pages = pages
        };

        if (available_block_memory.user_memory == 0) {
            if (expand_vmem_map(USER_VMEM_MAP, &stored_blocks.user_memory) != 0) {
                return;
            }
            available_block_memory.user_memory += FRAME_SIZE;
        }

        VMemMapBlock* vmem_block_ptr = (VMemMapBlock*)USER_VMEM_MAP + stored_blocks.user_memory++;
        *vmem_block_ptr = vmem_block;
        available_block_memory.user_memory -= sizeof(VMemMapBlock);
        available_user_memory += pages * FRAME_SIZE;

        rsort_vmem_map(vmem_block_ptr, stored_blocks.user_memory);
    }
}

void dma_vfree(void* _address, uint64_t pages) {
    kvfree(DMA, _address, pages);
}

void kheap_vfree(void* _address, uint64_t pages) {
    kvfree(KERN_HEAP, _address, pages);
}

void user_vfree(void* _address, uint64_t pages) {
    kvfree(USER, _address, pages);
}

void* map_pci_configuration(void* _config_addr) {
    VIRTUAL_ADDRESS map_space = parse_virtual_address(PCI_CONFIG_SPACE_MAP);

    for (size_t i = 0; i < (PCI_CONFIG_SPACE_MAP_SIZE / FRAME_SIZE); ++i) {
        PDPTE* pdpte = get_pdpte_address(map_space.PML4_offset, map_space.PDPT_offset);

        if ((pdpte->raw & PDPTE_PRESENT) == 0) {
            void* pd_page = pmalloc();

            if (pd_page == NULL) {
                return NULL;
            }
            pdpte->raw = ((filter_address((uint64_t)pd_page) >> 12) << 12) | PDPTE_READWRITE | PDPTE_PRESENT;

            uint64_t* pd = (uint64_t*)get_pd_address(map_space.PML4_offset, map_space.PDPT_offset);
            for (size_t i = 0; i < FRAME_SIZE / sizeof(uint64_t); ++i) {
                *(pd + i) = 0;
            }
        }

        PDE* pde = get_pde_address(map_space.PML4_offset, map_space.PDPT_offset, map_space.PD_offset);

        if ((pde->raw & PDE_PRESENT) == 0) {
            void* pt_page = pmalloc();

            if (pt_page == NULL) {
                return NULL;
            }
            pde->raw = ((filter_address((uint64_t)pt_page) >> 12) << 12) | PDE_READWRITE | PDE_PRESENT;

            uint64_t* pt = (uint64_t*)get_pt_address(map_space.PML4_offset, map_space.PDPT_offset, map_space.PD_offset);
            for (size_t i = 0; i < FRAME_SIZE / sizeof(uint64_t); ++i) {
                *(pt + i) = 0;
            }
        }
        
        PTE* pte = get_pte_address(map_space.PML4_offset, map_space.PDPT_offset, map_space.PD_offset, map_space.PT_offset);

        if ((pte->raw & PTE_PRESENT) == 0) {
            pte->raw = ((filter_address((uint64_t)_config_addr) >> 12) << 12) | PTE_PCD | PTE_PWT | PTE_READWRITE | PTE_PRESENT;
            return (void*)build_virtual_address(&map_space);
        }

        if (++map_space.PT_offset == PT_ENTRIES) {
            map_space.PT_offset = 0;
            if (++map_space.PD_offset == PD_ENTRIES) {
                map_space.PD_offset = 0;
                if (++map_space.PDPT_offset == 0) {
                    map_space.PDPT_offset = 0;
                    ++map_space.PML4_offset;
                }
            }
        }
    }

    return NULL;
}
