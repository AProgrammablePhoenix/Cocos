#pragma once

#include <stdint.h>

struct EfiMemoryMap;
struct LoaderInfo;

#include <loader/basic_graphics.hpp>
#include <loader/loader_info.hpp>

#define PAGE_SIZE               0x1000

#define PML4_ENTRIES            0x200
#define PDPT_ENTRIES            0x200
#define PD_ENTRIES              0x200
#define PT_ENTRIES              0x200

#define ENTRY_SIZE              8

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
#define PTE_PAT                 0x0000000000000080
#define PTE_GLOBAL              0x0000000000000100
#define PTE_ADDRESS             0x000FFFFFFFFFF000
#define PTE_PK                  0x7800000000000000
#define PTE_XD                  0x8000000000000000

#define DMA_ZONE_START          0x0000000000000000
#define DMA_ZONE_LIMIT          0x0000000001000000

#define EFI_RT_SVC_REMAP        0xFFFF800100000000
#define EFI_RT_SVC_LIMIT        0x0000000004000000

#define EFI_GOP_REMAP           0xFFFF800104000000
#define EFI_GOP_LIMIT           0x0000000004000000

#define EFI_ACPI_REMAP          0xFFFF800108000000
#define EFI_ACPI_LIMIT          0x0000000010000000

#define PSF_FONT_REMAP          0xFFFF800138000000
#define PSF_FONT_LIMIT          0x0000000000080000

#define LOADER_INFO_REMAP       0xFFFF800138080000
#define LOADER_INFO_LIMIT       0x0000000001F80000

struct PagingInformation {
    unsigned int PAT_support;
    uint8_t MAXPHYADDR;
};

struct PTE {
    uint64_t raw;
};

struct PDE {
    uint64_t raw;
};

struct PDPTE {
    uint64_t raw;
};

struct PML4E {
    uint64_t raw;
};

struct RawVirtualAddress {
    uint16_t PML4_offset;
    uint16_t PDPT_offset;
    uint16_t PD_offset;
    uint16_t PT_offset;
    uint16_t offset;
};

namespace Loader {
    EfiMemoryMap getEfiMemoryMap(void);

    PML4E* setupBasicPaging(const PagingInformation* PI);
    void prepareEFIRemap(PML4E* pml4, PagingInformation* PI);

    void remapRuntimeServices(PML4E* pml4, EFI_MEMORY_DESCRIPTOR* rt_desc, const PagingInformation* PI);
    void remapACPINVS(PML4E* pml4, EFI_MEMORY_DESCRIPTOR* acpi_desc, const PagingInformation* PI);
    void mapKernel(PML4E* pml4, void* _source, void* _dest, size_t size, const PagingInformation* PI);
    void mapLoader(PML4E* pml4, const PagingInformation* PI);
    void remapGOP(PML4E* pml4, BasicGraphics* BasicGFX, const PagingInformation* PI);
    void mapPSFFont(PML4E* pml4, const void** pcf_font, size_t size, const PagingInformation* PI);
    void* setupLoaderInfo(PML4E* pml4, LoaderInfo* linfo, const PagingInformation* PI);
}
