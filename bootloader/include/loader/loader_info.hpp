#pragma once

#include <stdint.h>

#include <efi/efi.h>

#include <loader/kernel_loader.hpp>
#include <loader/basic_graphics.hpp>

struct EfiMemoryMap {
    UINTN mmap_size;
    EFI_MEMORY_DESCRIPTOR* mmap;
    UINTN mmap_key;
    UINTN desc_size;
    UINT32 desc_ver;
};

#define DMA_BITMAP_ENTRIES 512

struct DMAZoneInfo {
    uint8_t bitmap[DMA_BITMAP_ENTRIES];
};

struct LoaderInfo {
    EfiMemoryMap mmap;                  //  EFI memory map
    KernelLocInfo KernelLI;             //  tells where the kernel was mapped in memory, its image size...
    DMAZoneInfo pmi;                    //  describes the DMA Legacy memory region (first 16 MB)
    BasicGraphics gfxData;              //  all the basic graphics data the kernel may need to know
    EFI_RUNTIME_SERVICES* rtServices;   //  EFI runtime services table location
    EFI_PHYSICAL_ADDRESS PCIe_ECAM_0;   //  physical address of the first ECAM entry in the MCFG ACPI table
    uint64_t ACPIRevision;              //  ACPI Revision
    void* RSDP;                         //  ACPI RSDP
};

