#pragma once

#include <efi/efi.h>

#include <loader/kernel_loader.h>
#include <loader/basic_graphics.h>

typedef struct {
    UINTN mmap_size;
    EFI_MEMORY_DESCRIPTOR *mmap;
    UINTN mmap_key;
    UINTN desc_size;
    UINT32 desc_ver;
} EfiMemoryMap;

#define DMA_BITMAP_ENTRIES      512

typedef struct {
    uint8_t DMA_Bitmap[DMA_BITMAP_ENTRIES]; // 512 bytes, each 8 bits <=> 4096 4KB pages <=> 16MB
} PHYS_MEM_INFO;

typedef struct {
    EfiMemoryMap mmap;                  // EFI memory map
    KernelLocInfo KernelLI;             // tells where the kernel was mapped in memory, its image size...
    PHYS_MEM_INFO pmi;                  // describes the DMA Legacy memory region (first 16 MB)
    BasicGraphics gfxData;              // all the basic graphics data the kernel may need to know
    EFI_RUNTIME_SERVICES* rtServices;   // runtime services
    EFI_PHYSICAL_ADDRESS PCIe_ECAM_0;   // physical address of the first ECAM entry in the MCFG ACPI table
    uint64_t ACPIRevision;              // ACPI Revision
    void* RSDP;                         // ACPI RSDP
} LoaderInfo;
