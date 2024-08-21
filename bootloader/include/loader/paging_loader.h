#pragma once

#include <stdint.h>

#include <loader/basic_graphics.h>
#include <loader/loader_info.h>

#define PAGE_SIZE       0x1000

#define PML4_ENTRIES    0x200
#define PDPT_ENTRIES    0x200
#define PD_ENTRIES      0x200
#define PT_ENTRIES      0x200

#define ENTRY_SIZE      8

#define PML4_PAGE_SIZE  ((ENTRY_SIZE * PML4_ENTRIES) / PAGE_SIZE)
#define PDPT_PAGE_SIZE  ((ENTRY_SIZE * PDPT_ENTRIES) / PAGE_SIZE)
#define PD_PAGE_SIZE    ((ENTRY_SIZE * PD_ENTRIES) / PAGE_SIZE)
#define PT_PAGE_SIZE    ((ENTRY_SIZE * PT_ENTRIES) / PAGE_SIZE)

#define DMA_ZONE_START          0x0000000000000000
#define DMA_ZONE_LIMIT          0x0000000001000000

#define EFI_RT_SVC_REMAP        0xFFFF800022000000
#define EFI_RT_SVC_MAX_REMAP    0x0000000004000000

#define EFI_ACPI_REMAP          0xFFFF800026000000
#define EFI_ACPI_MAX_REMAP      0x0000000010000000

#define EFI_MMIO_REMAP          0xFFFF800036000000
#define EFI_MMIO_MAX_REMAP      0x0000000400000000

#define EFI_GOP_REMAP           0xFFFF8004B6000000
#define EFI_GOP_MAX_REMAP       0x0000000002000000

#define TTY_FONT_REMAP          0xFFFF8004B8000000
#define TTY_FONT_MAX_REMAP      0x0000000000080000
#define LOADER_INFO_REMAP       0xFFFF8004B8080000
#define LOADER_INFO_MAX_REMAP   0x0000000001F80000

typedef struct {
    unsigned int PAT_support;
    uint8_t MAXPHYADDR;
} PagingInformation;

typedef struct {
    uint64_t present            : 1;
    uint64_t read_write         : 1;
    uint64_t user_mode          : 1;
    uint64_t write_through      : 1;
    uint64_t cache_disable      : 1;
    uint64_t accessed           : 1;
    uint64_t dirty              : 1;
    uint64_t pat                : 1;
    uint64_t global             : 1;
    uint64_t reserved_3         : 3;
    uint64_t address            : 40;
    uint64_t reserved_7         : 7;
    uint64_t protection_key     : 4;
    uint64_t execute_disable    : 1;
} PTE;

typedef struct {
    uint64_t present            : 1;
    uint64_t read_write         : 1;
    uint64_t user_mode          : 1;
    uint64_t write_through      : 1;
    uint64_t cache_disable      : 1;
    uint64_t accessed           : 1;
    uint64_t dirty              : 1;
    uint64_t page_size          : 1;
    uint64_t global             : 1;
    uint64_t reserved_3         : 3;
    uint64_t address            : 40;
    uint64_t reserved_11        : 11;
    uint64_t execute_disable    : 1;
} PDE, PDPTE;

typedef struct {
    uint64_t present            : 1;
    uint64_t read_write         : 1;
    uint64_t user_mode          : 1;
    uint64_t write_through      : 1;
    uint64_t cache_disable      : 1;
    uint64_t accessed           : 1;
    uint64_t reserved_1         : 1;
    uint64_t fixed_0            : 1;
    uint64_t reserved_4         : 4;
    uint64_t address            : 40;
    uint64_t reserved_11        : 11;
    uint64_t execute_disable    : 1;
} PML4E;

typedef struct {
    uint16_t PML4_offset;
    uint16_t PDPT_offset;
    uint16_t PD_offset;
    uint16_t PT_offset;
    uint16_t offset;
} RawVirtualAddress;

EfiMemoryMap getEfiMemoryMap(void);

PML4E* setupBasicPaging(const PagingInformation* PI);
void prepareEFIRemap(PML4E* pml4, PagingInformation* PI);

void remapRuntimeServices(PML4E* pml4, EFI_MEMORY_DESCRIPTOR* rt_desc, const PagingInformation* PI);
void remapACPINVS(PML4E* pml4, EFI_MEMORY_DESCRIPTOR* rt_desc, const PagingInformation* PI);
void* mapACPITable(PML4E* pml4, void* buffer, size_t size, const PagingInformation* PI);
void remapKernel(PML4E* pml4, void* _source, void* _dest, size_t size, const PagingInformation* PI);
void mapLoader(PML4E* pml4, const PagingInformation* PI);
void remapGOP(PML4E* pml4, BasicGraphics* BasicGFX, const PagingInformation* PI);
void remapTTYFont(PML4E* pml4, const void** tty_font, size_t size, const PagingInformation* PI);
void* setupLoaderInfo(PML4E* pml4, LoaderInfo* linfo, const PagingInformation* PI);
