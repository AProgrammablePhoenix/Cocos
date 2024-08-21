#pragma once

#include <stdint.h>

#include "kvmem_layout.h"

#define PAGING_LOOP_MASK        ((RECURSIVE_MAPPING >> 39) & 0x1FF)
#define PAGING_LOOP1            (RECURSIVE_MAPPING)
#define PAGING_LOOP2            (PAGING_LOOP1 | (PAGING_LOOP_MASK << 30))
#define PAGING_LOOP3            (PAGING_LOOP2 | (PAGING_LOOP_MASK << 21))
#define PAGING_LOOP4            (PAGING_LOOP3 | (PAGING_LOOP_MASK << 12))

// standard values

#define PML4_ENTRIES            512
#define PDPT_ENTRIES            512
#define PD_ENTRIES              512
#define PT_ENTRIES              512

#define PML4E_COVERAGE          0x0000008000000000
#define PDPTE_COVERAGE          0x0000000040000000
#define PDE_COVERAGE            0x0000000000200000
#define PTE_COVERAGE            0x0000000000001000

#define PML4E_PRESENT           0x0000000000000001
#define PML4E_READWRITE         0x0000000000000002
#define PML4E_USERMODE          0x0000000000000004
#define PML4E_PWT               0x0000000000000008
#define PML4E_PCD               0x0000000000000010
#define PML4E_ACCESSED          0x0000000000000020
#define PML4E_ADDRESS           0x000FFFFFFFFFF000
#define PML4E_XD                0x8000000000000000

#define PDPTE_PRESENT           0x0000000000000001
#define PDPTE_READWRITE         0x0000000000000002
#define PDPTE_USERMODE          0x0000000000000004
#define PDPTE_PWT               0x0000000000000008
#define PDPTE_PCD               0x0000000000000010
#define PDPTE_ACCESSED          0x0000000000000020
#define PDPTE_DIRTY             0x0000000000000040
#define PDPTE_PAGE_SIZE         0x0000000000000080
#define PDPTE_GLOBAL            0x0000000000000100
#define PDPTE_ADDRESS           0x000FFFFFFFFFF000
#define PDPTE_XD                0x8000000000000000

#define PDE_PRESENT             0x0000000000000001
#define PDE_READWRITE           0x0000000000000002
#define PDE_USERMODE            0x0000000000000004
#define PDE_PWT                 0x0000000000000008
#define PDE_PCD                 0x0000000000000010
#define PDE_ACCESSED            0x0000000000000020
#define PDE_DIRTY               0x0000000000000040
#define PDE_PAGE_SIZE           0x0000000000000080
#define PDE_GLOBAL              0x0000000000000100
#define PDE_ADDRESS             0x000FFFFFFFFFF000
#define PDE_PK                  0x7800000000000000
#define PDE_XD                  0x8000000000000000

#define PTE_PRESENT             0x0000000000000001
#define PTE_READWRITE           0x0000000000000002
#define PTE_USERMODE            0x0000000000000004
#define PTE_PWT                 0x0000000000000008
#define PTE_PCD                 0x0000000000000010
#define PTE_ACCESSED            0x0000000000000020
#define PTE_DIRTY               0x0000000000000040
#define PTE_GLOBAL              0x0000000000000100
#define PTE_ADDRESS             0x000FFFFFFFFFF000
#define PTE_PK                  0x7800000000000000
#define PTE_XD                  0x8000000000000000

// custom values

#define PTE_LOCK                0x0000000000000200

typedef struct {
    uint64_t raw;
} PTE;

typedef struct {
    uint64_t raw;
} PDE;

typedef struct {
    uint64_t raw;
} PDPTE;

typedef struct {
    uint64_t raw;
} PML4E;

typedef struct {
    uint16_t PML4_offset;
    uint16_t PDPT_offset;
    uint16_t PD_offset;
    uint16_t PT_offset;
    uint16_t offset;
} VIRTUAL_ADDRESS;

VIRTUAL_ADDRESS parse_virtual_address(uint64_t address);

PTE* get_pte_address(uint64_t pml4_offset, uint64_t pdpt_offset, uint64_t pd_offset, uint64_t pt_offset);
PDE* get_pde_address(uint64_t pml4_offset, uint64_t pdpt_offset, uint64_t pd_offset);
PDPTE* get_pdpte_address(uint64_t pml4_offset, uint64_t pdpt_offset);
PML4E* get_pml4e_address(uint64_t pml4_offset);

int vmm_setup(void);

void* dma_vmalloc(uint64_t pages);
void* kheap_vmalloc(uint64_t pages);
void* user_vmalloc(uint64_t pages);
void* user_vmalloc_at(void* _ptr, uint64_t pages);

void dma_vfree(void* _address, uint64_t pages);
void kheap_vfree(void* _address, uint64_t pages);
void user_vfree(void* _address, uint64_t pages);

void* map_pci_configuration(void* _config_addr);
