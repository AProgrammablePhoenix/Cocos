#include <stdint.h>

#include <efi/efi_misc.hpp>
#include <efi/efi.h>

#include <loader/paging.hpp>
#include <loader/loader_info.hpp>

#include <ldstdio.hpp>
#include <ldstdlib.hpp>

#define PAGING_VLOC         0xFFFFFF0000000000
#define PAGING_LOOP_MASK    ((PAGING_VLOC >> 39) & 0x1FF)

typedef struct {
    size_t size;
    size_t desc_size;
    EFI_MEMORY_DESCRIPTOR* mmap;
} SimpleMMAP;

namespace {
    static inline RawVirtualAddress parse_virtual_address(void* _addr) {
        uint64_t addr = reinterpret_cast<uint64_t>(_addr);

        return {
            .PML4_offset    = static_cast<uint16_t>((addr >> 39) & 0x1FF),
            .PDPT_offset    = static_cast<uint16_t>((addr >> 30) & 0x1FF),
            .PD_offset      = static_cast<uint16_t>((addr >> 21) & 0x1FF),
            .PT_offset      = static_cast<uint16_t>((addr >> 12) & 0x1FF),
            .offset         = static_cast<uint16_t>(addr & 0xFFF)
        };
    }

    static inline constexpr uint64_t filter_address(uint64_t address, const PagingInformation* PI) {
        return (address & (((uint64_t)1 << PI->MAXPHYADDR) - 1)) & ~0xFFF;
    }

    static inline constexpr PML4E make_PML4E(uint64_t address, const PagingInformation* PI) {
        return {
            .raw = filter_address(address, PI) | PML4E_READWRITE | PML4E_PRESENT
        };
    }

    static inline constexpr PDPTE make_PDPTE(uint64_t address, const PagingInformation* PI) {
        return {
            .raw = filter_address(address, PI) | PDPTE_READWRITE | PDPTE_PRESENT
        };
    }

    static inline constexpr PDE make_PDE(uint64_t address, const PagingInformation* PI) {
        return {
            .raw = filter_address(address, PI) | PDE_READWRITE | PDE_PRESENT
        };
    }

    static inline constexpr PTE make_PTE(uint64_t address, const PagingInformation* PI, bool xd) {
        return {
            .raw = (xd ? PTE_XD : 0) | filter_address(address, PI) | PTE_READWRITE | PTE_PRESENT
        };
    }

    static inline SimpleMMAP getSimpleMMAP(void) {
        SimpleMMAP smmap = {
            .size = 0,
            .desc_size = 0,
            .mmap = nullptr
        };

        UINTN mmap_key;
        UINT32 desc_ver;

        EFI::sys->BootServices->GetMemoryMap(&smmap.size, smmap.mmap, &mmap_key, &smmap.desc_size, &desc_ver);
        smmap.size += 2 * smmap.desc_size;
        EFI::sys->BootServices->AllocatePool(EfiLoaderData, smmap.size, reinterpret_cast<VOID**>(&smmap.mmap));
        EFI::sys->BootServices->GetMemoryMap(&smmap.size, smmap.mmap, &mmap_key, &smmap.desc_size, &desc_ver);

        return smmap;
    }

    static inline void updateRemapRVA(RawVirtualAddress* remap_rva) {
        if (++remap_rva->PD_offset >= PD_ENTRIES) {
            remap_rva->PD_offset = 0;
            if (++remap_rva->PDPT_offset >= PDPT_ENTRIES) {
                remap_rva->PDPT_offset = 0;
                ++remap_rva->PML4_offset;
            }
        }
    }

    static inline void fullUpdateRemapRVA(RawVirtualAddress* remap_rva) {
        if (++remap_rva->PT_offset >= PT_ENTRIES) {
            remap_rva->PT_offset = 0;
            updateRemapRVA(remap_rva);
        }
    }

    static inline void prepareRemap(PML4E* pml4, PagingInformation* PI, void* _addr, uint64_t pages) {
        RawVirtualAddress remap_rva = parse_virtual_address(_addr);
        uint64_t required_PTs = (pages + PT_ENTRIES - 1) / PT_ENTRIES;

        for (size_t i = 0; i < required_PTs; ++i) {
            PML4E* pml4e = pml4 + remap_rva.PML4_offset;
            PDPTE* pdpt = nullptr;

            if ((pml4e->raw & PML4E_PRESENT) == 0) {
                EFI::sys->BootServices->AllocatePages(
                    AllocateAnyPages,
                    LoaderPersistentMemory,
                    1,
                    reinterpret_cast<EFI_PHYSICAL_ADDRESS*>(&pdpt)
                );
                EFI::sys->BootServices->SetMem(reinterpret_cast<VOID*>(pdpt), PAGE_SIZE, 0);
                *pml4e = make_PML4E(reinterpret_cast<uint64_t>(pdpt), PI);
            }
            else {
                pdpt = reinterpret_cast<PDPTE*>(pml4e->raw & PML4E_ADDRESS);
            }

            PDPTE* pdpte = pdpt + remap_rva.PDPT_offset;
            PDE* pd = nullptr;

            if ((pdpte->raw & PDPTE_PRESENT) == 0) {
                EFI::sys->BootServices->AllocatePages(
                    AllocateAnyPages,
                    LoaderPersistentMemory,
                    1,
                    reinterpret_cast<EFI_PHYSICAL_ADDRESS*>(&pd)
                );
                EFI::sys->BootServices->SetMem(reinterpret_cast<VOID*>(pd), PAGE_SIZE, 0);
                *pdpte = make_PDPTE(reinterpret_cast<uint64_t>(pd), PI);
            }
            else {
                pd = reinterpret_cast<PDE*>(pdpte->raw & PDPTE_ADDRESS);
            }

            PDE* pde = pd + remap_rva.PD_offset;
            PTE* pt = nullptr;

            if ((pde->raw & PDE_PRESENT) == 0) {
                EFI::sys->BootServices->AllocatePages(
                    AllocateAnyPages,
                    LoaderPersistentMemory,
                    1,
                    reinterpret_cast<EFI_PHYSICAL_ADDRESS*>(&pt)
                );
                EFI::sys->BootServices->SetMem(reinterpret_cast<VOID*>(pt), PAGE_SIZE, 0);
                *pde = make_PDE(reinterpret_cast<uint64_t>(pt), PI);
            }

            // ignore PT_offset as we only care about the number of page tables required
            // (= how many PDEs we need), not the number of pages required.

            updateRemapRVA(&remap_rva);
        }
    }

    static inline void directRemap(
        size_t pages,
        PML4E* pml4,
        RawVirtualAddress* remap_rva,
        uint64_t physical_start,
        unsigned int execute_disable,
        const PagingInformation* PI
    ) {
        for (size_t i = 0; i < pages; ++i) {
            PML4E* pml4e = pml4 + remap_rva->PML4_offset;
            PDPTE* pdpte = reinterpret_cast<PDPTE*>(pml4e->raw & PML4E_ADDRESS) + remap_rva->PDPT_offset;
            PDE* pde = reinterpret_cast<PDE*>(pdpte->raw & PDPTE_ADDRESS) + remap_rva->PD_offset;
            PTE* pte = reinterpret_cast<PTE*>(pde->raw & PDE_ADDRESS) + remap_rva->PT_offset;

            *pte = make_PTE(physical_start + i * PAGE_SIZE, PI, execute_disable);

            fullUpdateRemapRVA(remap_rva);
        }
    }

    static unsigned int PAT_enable = 0;
    static unsigned int PWT_enable = 0;
    static unsigned int PCD_enable = 0;

    static inline void indirectRemap(
        PML4E* pml4,
        RawVirtualAddress* remap_rva,
        EFI_MEMORY_TYPE mem_type,
        uint64_t* current_source,
        unsigned int execute_disable,
        const PagingInformation* PI
    ) {
        PML4E* pml4e = pml4 + remap_rva->PML4_offset;
        PDPTE* pdpt = nullptr;

        if ((pml4e->raw & PML4E_PRESENT) == 0) {            
            EFI::sys->BootServices->AllocatePages(
                AllocateAnyPages,
                mem_type,
                1,
                reinterpret_cast<EFI_PHYSICAL_ADDRESS*>(&pdpt)
            );
            EFI::sys->BootServices->SetMem(reinterpret_cast<VOID*>(pdpt), PAGE_SIZE, 0);
            *pml4e = make_PML4E(reinterpret_cast<uint64_t>(pdpt), PI);
        }
        else {
            pdpt = reinterpret_cast<PDPTE*>(pml4e->raw & PML4E_ADDRESS);
        }

        PDPTE* pdpte = pdpt + remap_rva->PDPT_offset;
        PDE* pd = nullptr;

        if ((pdpte->raw & PDPTE_PRESENT) == 0) {
            EFI::sys->BootServices->AllocatePages(
                AllocateAnyPages,
                mem_type,
                1,
                reinterpret_cast<EFI_PHYSICAL_ADDRESS*>(&pd)
            );
            EFI::sys->BootServices->SetMem(reinterpret_cast<VOID*>(pd), PAGE_SIZE, 0);
            *pdpte = make_PDPTE(reinterpret_cast<uint64_t>(pd), PI);
        }
        else {
            pd = reinterpret_cast<PDE*>(pdpte->raw & PDPTE_ADDRESS);
        }

        PDE* pde = pd + remap_rva->PD_offset;
        PTE* pt = nullptr;

        if ((pde->raw & PDE_PRESENT) == 0) {
            EFI::sys->BootServices->AllocatePages(
                AllocateAnyPages,
                mem_type,
                1,
                reinterpret_cast<EFI_PHYSICAL_ADDRESS*>(&pt)
            );
            EFI::sys->BootServices->SetMem(reinterpret_cast<VOID*>(pt), PAGE_SIZE, 0);
            *pde = make_PDE(reinterpret_cast<uint64_t>(pt), PI);
        }
        else {
            pt = reinterpret_cast<PTE*>(pde->raw & PDE_ADDRESS);
        }

        PTE* pte = pt + remap_rva->PT_offset;
        *pte = make_PTE(*current_source, PI, execute_disable);
        pte->raw |= (PAT_enable == 1 && PI->PAT_support != 0) ? PTE_PAT : 0;
        pte->raw |= PWT_enable ? PTE_PWT : 0;
        pte->raw |= PCD_enable ? PTE_PCD : 0;

        *current_source += PAGE_SIZE;

        fullUpdateRemapRVA(remap_rva);
    }

    // called only once preferably, tolerance of two calls,
    // as should be anticipated by the function that retreives the EFI memory map
    // so make sure to allocate space for +4 memory descriptors when retrieving the memory map (instead of the usual +2)
    static void* makeshiftMalloc(EfiMemoryMap* mmap, size_t pages) {
        const size_t desc_num = mmap->mmap_size / mmap->desc_size;

        for (size_t i = 0; i < desc_num; ++i) {
            EFI_MEMORY_DESCRIPTOR* mm_descriptor = reinterpret_cast<EFI_MEMORY_DESCRIPTOR*>(
                reinterpret_cast<uint8_t*>(mmap->mmap) + i * mmap->desc_size
            );

            if (mm_descriptor->Type == EfiConventionalMemory && mm_descriptor->NumberOfPages >= pages) {
                mm_descriptor->NumberOfPages -= pages;

                void* range_start = reinterpret_cast<void*>(
                    mm_descriptor->PhysicalStart + mm_descriptor->NumberOfPages * PAGE_SIZE
                );
                
                EFI_MEMORY_DESCRIPTOR newBlock = {
                    .Type = LoaderPersistentMemory,
                    ._padding = 0,
                    .PhysicalStart = reinterpret_cast<EFI_PHYSICAL_ADDRESS>(range_start),
                    .VirtualStart = 0, // don't care about this for now
                    .NumberOfPages = pages,
                    .Attribute = mm_descriptor->Attribute
                };

                // append new block at the end
                *reinterpret_cast<EFI_MEMORY_DESCRIPTOR*>(
                    reinterpret_cast<uint8_t*>(mmap->mmap) + desc_num * mmap->desc_size
                ) = newBlock;
                mmap->mmap_size += mmap->desc_size;

                return range_start;
            }
        }

        return nullptr;
    }
}

EfiMemoryMap Loader::getEfiMemoryMap(void) {
    EfiMemoryMap efi_mmap = {
        .mmap_size  = 0,
        .mmap       = nullptr,
        .mmap_key   = 0,
        .desc_size  = 0,
        .desc_ver   = 0
    };
    EFI::sys->BootServices->GetMemoryMap(
        &efi_mmap.mmap_size,
        efi_mmap.mmap,
        &efi_mmap.mmap_key,
        &efi_mmap.desc_size,
        &efi_mmap.desc_ver
    );
    efi_mmap.mmap_size += 2 * efi_mmap.desc_size;
    // another +2 because mapping the loader info requires memory allocations that may fragment the memory map
    EFI::sys->BootServices->AllocatePool(
        EfiLoaderData,
        efi_mmap.mmap_size + 2 * efi_mmap.desc_size,
        reinterpret_cast<VOID**>(&efi_mmap.mmap)
    );
    EFI::sys->BootServices->GetMemoryMap(
        &efi_mmap.mmap_size,
        efi_mmap.mmap,
        &efi_mmap.mmap_key,
        &efi_mmap.desc_size,
        &efi_mmap.desc_ver
    );
    return efi_mmap;
}

PML4E* Loader::setupBasicPaging(const PagingInformation* PI) {
    // maps the PML4 onto itself
    PML4E* pml4 = nullptr;
    EFI::sys->BootServices->AllocatePages(
        AllocateAnyPages,
        LoaderPersistentMemory,
        1,
        reinterpret_cast<EFI_PHYSICAL_ADDRESS*>(&pml4)
    );
    EFI::sys->BootServices->SetMem(reinterpret_cast<VOID*>(pml4), PAGE_SIZE, 0);

    if ((uint64_t)pml4 % PAGE_SIZE != 0) {
        return nullptr; // ensures the pml4 is 4 KB aligned
    }

    PML4E* pml4e = pml4 + PAGING_LOOP_MASK; // recursive pml4 entry
    *pml4e = make_PML4E((uint64_t)pml4, PI);
    pml4e->raw |= PML4E_XD;
    return pml4;
}

void Loader::prepareEFIRemap(PML4E* pml4, PagingInformation* PI) {
    SimpleMMAP smmap = getSimpleMMAP();

    uint64_t efi_services_required_pages = 0;
    uint64_t efi_acpi_required_pages = 0;

    size_t desc_num = smmap.size / smmap.desc_size;

    for (size_t i = 0; i < desc_num; ++i) {
        EFI_MEMORY_DESCRIPTOR* current_descriptor = reinterpret_cast<EFI_MEMORY_DESCRIPTOR*>(
            reinterpret_cast<uint8_t*>(smmap.mmap) + i * smmap.desc_size
        );

        if (current_descriptor->Type == EfiRuntimeServicesCode || current_descriptor->Type == EfiRuntimeServicesData) {
            efi_services_required_pages += current_descriptor->NumberOfPages;
        }
        else if (current_descriptor->Type == EfiACPIMemoryNVS) {
            efi_acpi_required_pages += current_descriptor->NumberOfPages;
        }
    }

    if (efi_services_required_pages * PAGE_SIZE >= EFI_RT_SVC_LIMIT) {
        Loader::puts(u"Not enough memory to map all runtime services.\n\r");
        EFI::Terminate();
    }
    else if (efi_acpi_required_pages * PAGE_SIZE >= EFI_ACPI_LIMIT) {
        Loader::puts(u"Not enough memory to map all ACPI memory.\n\r");
        EFI::Terminate();
    }

    // llocate enough pages to create enough pages tables to map enough EFI pages

    prepareRemap(pml4, PI, reinterpret_cast<void*>(EFI_RT_SVC_REMAP), efi_services_required_pages);
    prepareRemap(pml4, PI, reinterpret_cast<void*>(EFI_ACPI_REMAP), efi_acpi_required_pages);

    EFI::sys->BootServices->FreePool(reinterpret_cast<VOID*>(smmap.mmap));
}

void Loader::remapRuntimeServices(PML4E* pml4, EFI_MEMORY_DESCRIPTOR* rt_desc, const PagingInformation* PI) {
    static uint64_t current_remap = EFI_RT_SVC_REMAP;

    if (rt_desc != nullptr) {
        rt_desc->VirtualStart = current_remap;
        RawVirtualAddress remap_rva = parse_virtual_address(reinterpret_cast<void*>(current_remap));

        directRemap(
            rt_desc->NumberOfPages,
            pml4,
            &remap_rva,
            rt_desc->PhysicalStart,
            rt_desc->Type == EfiRuntimeServicesData,
            PI
        );

        current_remap += rt_desc->NumberOfPages * PAGE_SIZE;
    }
}

void Loader::remapACPINVS(PML4E* pml4, EFI_MEMORY_DESCRIPTOR* acpi_desc, const PagingInformation* PI) {
    static uint64_t current_remap = EFI_ACPI_REMAP;

    if (acpi_desc != nullptr) {
        acpi_desc->VirtualStart = current_remap;
        RawVirtualAddress remap_rva = parse_virtual_address(reinterpret_cast<void*>(current_remap));

        directRemap(
            acpi_desc->NumberOfPages,
            pml4,
            &remap_rva,
            acpi_desc->PhysicalStart,
            0,
            PI
        );

        current_remap += acpi_desc->NumberOfPages * PAGE_SIZE;
    }
}

void Loader::mapKernel(PML4E* pml4, void* _source, void* _dest, size_t size, const PagingInformation* PI) {
    size_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    uint64_t current_src = (uint64_t)_source;
    RawVirtualAddress map_rva = parse_virtual_address(_dest);

    for (size_t i = 0; i < pages; ++i) {
        indirectRemap(pml4, &map_rva, LoaderPersistentMemory, &current_src, 0, PI);
    }
}

void Loader::mapLoader(PML4E* pml4, const PagingInformation* PI) {
    SimpleMMAP smmap = getSimpleMMAP();

    size_t desc_num = smmap.size / smmap.desc_size;

    for (size_t i = 0; i < desc_num; ++i) {
        EFI_MEMORY_DESCRIPTOR* current_descriptor = reinterpret_cast<EFI_MEMORY_DESCRIPTOR*>(
            reinterpret_cast<uint8_t*>(smmap.mmap) + i * smmap.desc_size
        );
        
        if (current_descriptor->Type == EfiLoaderCode || current_descriptor->Type == EfiLoaderData) {
            RawVirtualAddress remap_rva = parse_virtual_address(
                reinterpret_cast<void*>(current_descriptor->PhysicalStart)
            );

            uint64_t current_src = current_descriptor->PhysicalStart;
            
            for (size_t i = 0; i < current_descriptor->NumberOfPages; ++i) {
                indirectRemap(
                    pml4,
                    &remap_rva,
                    LoaderTemporaryMemory,
                    &current_src,
                    current_descriptor->Type == EfiLoaderData,
                    PI
                );
            }
        }
    }

    EFI::sys->BootServices->FreePool(reinterpret_cast<VOID*>(smmap.mmap));
}

void Loader::remapGOP(PML4E* pml4, BasicGraphics* BasicGFX, const PagingInformation* PI) {
    if (BasicGFX->FBSIZE > EFI_GOP_LIMIT) {
        Loader::puts(u"Not enough memory to map the framebuffer\n\r");
        EFI::Terminate();
    }

    size_t pages = (BasicGFX->FBSIZE + PAGE_SIZE - 1) / PAGE_SIZE;

    uint64_t current_src = reinterpret_cast<uint64_t>(BasicGFX->FBADDR);
    RawVirtualAddress remap_rva = parse_virtual_address(reinterpret_cast<void*>(EFI_GOP_REMAP));

    PAT_enable = 1;
    for (size_t i = 0; i < pages; ++i) {
        indirectRemap(pml4, &remap_rva, LoaderPersistentMemory, &current_src, 1, PI);
    }
    PAT_enable = 0;

    BasicGFX->FBADDR = reinterpret_cast<uint32_t*>(EFI_GOP_REMAP);
}

void Loader::mapPSFFont(PML4E* pml4, const void** pcf_font, size_t size, const PagingInformation* PI) {
    if (size > PSF_FONT_LIMIT) {
        Loader::puts(u"PCF Font too large to fit in memory\n\r");
        EFI::Terminate();
    }

    size_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    uint64_t current_src = reinterpret_cast<uint64_t>(*pcf_font);
    RawVirtualAddress remap_rva = parse_virtual_address(reinterpret_cast<void*>(PSF_FONT_REMAP));

    for (size_t i = 0; i < pages; ++i) {
        indirectRemap(pml4, &remap_rva, LoaderPersistentMemory, &current_src, 1, PI);
    }

    *pcf_font = reinterpret_cast<void*>(PSF_FONT_REMAP);
}

// Please setup a temporary stack before calling this function.
// Call this function after SetVirtualMap (otherwise it's impossible to know where runtime services are mapped),
// but before setting up paging
void* Loader::setupLoaderInfo(PML4E* pml4, LoaderInfo* linfo, const PagingInformation* PI) {
    const size_t total_size = sizeof(*linfo)
        + linfo->mmap.mmap_size
        - sizeof(linfo->mmap.mmap_key)
        - sizeof(linfo->mmap.desc_ver)
        - sizeof(linfo->mmap.mmap);

    // TTY font already mapped, so this means the corresponding PML4, PDPT and PD have already been allocated and mapped
    // since we have 32MB - 512KB, we need at most 32 / 2 - 1 = 16 - 1 = 15 page tables
    // the real number of pages we need is required_pages + required_pages / PT_ENTRIES
    // notice we don't need to round up the division result here, however,
    // we need to add 0x80 to take into account the 512KB of space reserved to the TTY font.

    const size_t required_pages = (total_size + PAGE_SIZE - 1) / PAGE_SIZE;
    const size_t total_pages = required_pages + (required_pages + 0x80) / PT_ENTRIES;

    if (total_pages * PAGE_SIZE > LOADER_INFO_LIMIT) {
        return nullptr;
    }

    // contains the loader info data and the paging structures to map it
    void* linfo_buffer = makeshiftMalloc(&linfo->mmap, total_pages);
    if (linfo_buffer == nullptr) {
        return nullptr;
    }

    uint8_t* ptr = reinterpret_cast<uint8_t*>(linfo_buffer);

    *(reinterpret_cast<uint64_t*>(ptr)) = reinterpret_cast<uint64_t>(linfo->mmap.mmap_size);
    ptr += sizeof(uint64_t);
    
    *(reinterpret_cast<uint64_t*>(ptr)) = reinterpret_cast<uint64_t>(linfo->mmap.desc_size);
    ptr += sizeof(uint64_t);

    Loader::memcpy(ptr, linfo->mmap.mmap, linfo->mmap.mmap_size);
    ptr += linfo->mmap.mmap_size;

    // might be mis-aligned, so be careful with how you access the following data from now on.
    *(reinterpret_cast<KernelLocInfo*>(ptr)) = linfo->KernelLI;
    ptr += sizeof(KernelLocInfo);
    ptr += sizeof(uint64_t); // reserved 8 bytes field

    *(reinterpret_cast<DMAZoneInfo*>(ptr)) = linfo->pmi;
    ptr += sizeof(DMAZoneInfo);

    *(reinterpret_cast<BasicGraphics*>(ptr)) = linfo->gfxData;
    ptr += sizeof(BasicGraphics);

    *(reinterpret_cast<EFI_RUNTIME_SERVICES**>(ptr)) = linfo->rtServices;
    ptr += sizeof(EFI_RUNTIME_SERVICES*);

    *(reinterpret_cast<void**>(ptr)) = reinterpret_cast<void*>(linfo->PCIe_ECAM_0);
    ptr += sizeof(void*);

    *(reinterpret_cast<uint64_t*>(ptr)) = linfo->ACPIRevision;
    ptr += sizeof(uint64_t);

    *(reinterpret_cast<void**>(ptr)) = linfo->RSDP;

    // Now we remap this buffer, first get a pointer to the next page
    uint64_t misalignment = reinterpret_cast<uint64_t>(ptr) % PAGE_SIZE;
    if (misalignment != 0) {
        ptr += PAGE_SIZE - misalignment;
    }

    uint64_t current_source = reinterpret_cast<uint64_t>(linfo_buffer);
    RawVirtualAddress remap_rva = parse_virtual_address(reinterpret_cast<void*>(LOADER_INFO_REMAP));

    for (size_t i = 0; i < required_pages; ++i) {
        PML4E* pml4e = pml4 + remap_rva.PML4_offset;
        PDPTE* pdpt = reinterpret_cast<PDPTE*>(pml4e->raw & PML4E_ADDRESS);
        PDPTE* pdpte = pdpt + remap_rva.PDPT_offset;
        PDE* pd = reinterpret_cast<PDE*>(pdpte->raw & PDPTE_ADDRESS);
        PDE* pde = pd + remap_rva.PD_offset;
        PTE* pt = reinterpret_cast<PTE*>(pde->raw & PDE_ADDRESS);

        if ((pde->raw & PDE_PRESENT) == 0) {
            pt = reinterpret_cast<PTE*>(ptr);
            ptr += PAGE_SIZE;
            Loader::memset(pt, 0, PAGE_SIZE);
            *pde = make_PDE(reinterpret_cast<uint64_t>(pt), PI);
        }
        else {
            pt = reinterpret_cast<PTE*>(pde->raw & PDE_ADDRESS);
        }

        PTE* pte = pt + remap_rva.PT_offset;
        *pte = make_PTE(current_source, PI, 1);

        current_source += PAGE_SIZE;

        fullUpdateRemapRVA(&remap_rva);
    }

    return linfo_buffer;
}
