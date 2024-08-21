#include <stdint.h>

#include <efi/efi.h>
#include <efi/efi_misc.h>

#include <loader/paging_loader.h>
#include <loader/loader_info.h>

#include <stdiok.h>

#define PAGING_VLOC         0xFFFFFF0000000000                          
#define PAGING_LOOP_MASK    ((PAGING_VLOC >> 39) & 0x1FF)               // 0x00000000000001FE

static inline RawVirtualAddress parse_virtual_address(void* _addr) {
    uint64_t addr = (uint64_t)_addr;

    RawVirtualAddress raw_va;

    raw_va.PML4_offset    = (addr >> 39) & 0x1FF;
    raw_va.PDPT_offset    = (addr >> 30) & 0x1FF;
    raw_va.PD_offset      = (addr >> 21) & 0x1FF;
    raw_va.PT_offset      = (addr >> 12) & 0x1FF;
    raw_va.offset         = addr & 0xFFF;

    return raw_va;
}

static inline uint64_t filter_address(uint64_t address, const PagingInformation* PI) {
    return address & (((uint64_t)1 << PI->MAXPHYADDR) - 1);
}

static inline PML4E make_PML4E(uint64_t address, const PagingInformation* PI) {
    PML4E entry = {
        .present = 1,
        .read_write = 1,
        .user_mode = 0,
        .write_through = 0,
        .cache_disable = 0,
        .fixed_0 = 0,
        .address = filter_address(address, PI) >> 12,
        .execute_disable = 0
    };

    return entry;
}

static inline PDPTE make_PDPTE(uint64_t address, const PagingInformation* PI) {
    PDPTE entry = {
        .present = 1,
        .read_write = 1,
        .user_mode = 0,
        .write_through = 0,
        .cache_disable = 0,
        .page_size = 0,
        .address = filter_address(address, PI) >> 12,
        .execute_disable = 0
    };

    return entry;
}

static inline PDE make_PDE(uint64_t address, const PagingInformation* PI) {
    PDE entry = {
        .present = 1,
        .read_write = 1,
        .user_mode = 0,
        .write_through = 0,
        .cache_disable = 0,
        .page_size = 0,
        .address = filter_address(address, PI) >> 12,
        .execute_disable = 0
    };

    return entry;
}

static inline PTE make_PTE(uint64_t address, const PagingInformation* PI) {
    PTE entry = {
        .present = 1,
        .read_write = 1,
        .user_mode = 0,
        .write_through = 0,
        .cache_disable = 0,
        .pat = 0,
        .address = filter_address(address, PI) >> 12,
        .protection_key = 0,
        .execute_disable = 1
    };

    return entry;
}

PML4E* setupBasicPaging(const PagingInformation* PI) {
    // maps the PML4 onto itself as defined by PAGING_VLOC, PAGING_LOOP_MASK and PAGING_LOOP4

    PML4E* pml4 = NULL;
    est->BootServices->AllocatePages(AllocateAnyPages, LoaderPersistentMemory, 1, (EFI_PHYSICAL_ADDRESS*)&pml4);
    est->BootServices->SetMem((VOID*)pml4, PAGE_SIZE, 0);

    if ((uint64_t)pml4 % 0x1000 != 0) {
        return NULL; // ensures the pml4 is 4 KB aligned
    }

    PML4E* pml4e = pml4 + PAGING_LOOP_MASK;

    *pml4e = make_PML4E((uint64_t)pml4, PI);    // recursive mapping

    return pml4;
}

typedef struct {
    size_t size;
    size_t desc_size;
    EFI_MEMORY_DESCRIPTOR* mmap;
} SimpleMMAP;

static SimpleMMAP getSimpleMMAP(void) {
    SimpleMMAP smmap;
    smmap.size = 0;
    smmap.desc_size = 0;
    smmap.mmap = NULL;
    
    UINTN mmap_key;
    UINT32 desc_ver;

    est->BootServices->GetMemoryMap(&smmap.size, smmap.mmap, &mmap_key, &smmap.desc_size, &desc_ver);
    smmap.size += 2 * smmap.desc_size;
    est->BootServices->AllocatePool(EfiLoaderData, smmap.size, (VOID**)&smmap.mmap);
    est->BootServices->GetMemoryMap(&smmap.size, smmap.mmap, &mmap_key, &smmap.desc_size, &desc_ver);

    return smmap;
}

EfiMemoryMap getEfiMemoryMap(void) {
    EfiMemoryMap efi_mmap;

    efi_mmap.mmap_size  = 0;
    efi_mmap.mmap       = NULL;
    efi_mmap.mmap_key   = 0;
    efi_mmap.desc_size  = 0;
    efi_mmap.desc_ver   = 0;

    est->BootServices->GetMemoryMap(&efi_mmap.mmap_size, efi_mmap.mmap, &efi_mmap.mmap_key, &efi_mmap.desc_size, &efi_mmap.desc_ver);
    efi_mmap.mmap_size += 2 * efi_mmap.desc_size;

    // another +2 because mapping the loader info requires memory allocations that may fragment the memory map
    est->BootServices->AllocatePool(
        EfiLoaderData,
        efi_mmap.mmap_size + 2 * efi_mmap.desc_size, 
        (VOID**)&efi_mmap.mmap
    );

    est->BootServices->GetMemoryMap(&efi_mmap.mmap_size, efi_mmap.mmap, &efi_mmap.mmap_key, &efi_mmap.desc_size, &efi_mmap.desc_ver);

    return efi_mmap;
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
        PDPTE* pdpt = NULL;

        if (!pml4e->present) {
            est->BootServices->AllocatePages(AllocateAnyPages, LoaderPersistentMemory, 1, (EFI_PHYSICAL_ADDRESS*)&pdpt);
            est->BootServices->SetMem((VOID*)pdpt, PAGE_SIZE, 0);
            *pml4e = make_PML4E((uint64_t)pdpt, PI);
        }
        else {
            pdpt = (PDPTE*)(pml4e->address << 12);
        }

        PDPTE* pdpte = pdpt + remap_rva.PDPT_offset;
        PDE* pd = NULL;

        if (!pdpte->present) {
            est->BootServices->AllocatePages(AllocateAnyPages, LoaderPersistentMemory, 1, (EFI_PHYSICAL_ADDRESS*)&pd);
            est->BootServices->SetMem((VOID*)pd, PAGE_SIZE, 0);
            *pdpte = make_PDPTE((uint64_t)pd, PI);
        }
        else {
            pd = (PDE*)(pdpte->address << 12);
        }

        PDE* pde = pd + remap_rva.PD_offset;
        PTE* pt = NULL;

        if (!pde->present) {
            est->BootServices->AllocatePages(AllocateAnyPages, LoaderPersistentMemory, 1, (EFI_PHYSICAL_ADDRESS*)&pt);
            est->BootServices->SetMem((VOID*)pt, PAGE_SIZE, 0);
            *pde = make_PDE((uint64_t)pt, PI);
        }

        // ignore PT_offset as we only care about the number of page tables required
        // (= how many PDEs we need), not the number of pages required.

        updateRemapRVA(&remap_rva);
    }
}

void prepareEFIRemap(PML4E* pml4, PagingInformation* PI) {
    SimpleMMAP smmap = getSimpleMMAP();

    uint64_t efi_services_required_pages = 0;
    uint64_t efi_acpi_data_required_pages = 0;

    size_t desc_num = smmap.size / smmap.desc_size;

    for (size_t i = 0; i < desc_num; ++i) {
        EFI_MEMORY_DESCRIPTOR* current_descriptor = (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)smmap.mmap + i * smmap.desc_size);
        
        if (current_descriptor->Type == EfiRuntimeServicesCode || current_descriptor->Type == EfiRuntimeServicesData) {
            efi_services_required_pages += current_descriptor->NumberOfPages;
        }
        else if (current_descriptor->Type == EfiACPIReclaimMemory  || current_descriptor->Type == EfiACPIMemoryNVS) {
            efi_acpi_data_required_pages += current_descriptor->NumberOfPages;
        }
    }

    if (efi_services_required_pages * PAGE_SIZE >= EFI_RT_SVC_MAX_REMAP) {
        kputs(u"Not enough memory to map all runtime services.\n\r");
        Terminate();
    }
    else if (efi_acpi_data_required_pages * PAGE_SIZE >= EFI_ACPI_MAX_REMAP) {
        kputs(u"Not enough memory to map all ACPI services.\n\r");
        Terminate();
    }

    // allocate enough pages to create enough Page Tables to map enough EFI pages

    prepareRemap(pml4, PI, (void*)EFI_RT_SVC_REMAP, efi_services_required_pages);
    prepareRemap(pml4, PI, (void*)EFI_ACPI_REMAP, efi_acpi_data_required_pages);

    est->BootServices->FreePool((VOID*)smmap.mmap);
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
        PDPTE* pdpte = (PDPTE*)(pml4e->address << 12) + remap_rva->PDPT_offset;
        PDE* pde = (PDE*)(pdpte->address << 12) + remap_rva->PD_offset;
        PTE* pte = (PTE*)(pde->address << 12) + remap_rva->PT_offset;

        *pte = make_PTE(physical_start + i * PAGE_SIZE, PI);
        pte->execute_disable = execute_disable;

        fullUpdateRemapRVA(remap_rva);
    }
}

void remapRuntimeServices(PML4E* pml4, EFI_MEMORY_DESCRIPTOR* rt_desc, const PagingInformation* PI) {
    static uint64_t current_remap = EFI_RT_SVC_REMAP;

    if (rt_desc != NULL) {
        rt_desc->VirtualStart = current_remap;
        RawVirtualAddress remap_rva = parse_virtual_address((void*)current_remap);

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

void remapACPINVS(PML4E* pml4, EFI_MEMORY_DESCRIPTOR* acpi_desc, const PagingInformation* PI) {
    static uint64_t current_remap = EFI_ACPI_REMAP;

    if (acpi_desc != NULL) {
        acpi_desc->VirtualStart = current_remap;
        RawVirtualAddress remap_rva = parse_virtual_address((void*)current_remap);

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
    PDPTE* pdpt = NULL;

    if (!pml4e->present) {            
        est->BootServices->AllocatePages(AllocateAnyPages, mem_type, 1, (EFI_PHYSICAL_ADDRESS*)&pdpt);
        est->BootServices->SetMem((VOID*)pdpt, PAGE_SIZE, 0);
        *pml4e = make_PML4E((uint64_t)pdpt, PI);
    }
    else {
        pdpt = (PDPTE*)(pml4e->address << 12);
    }

    PDPTE* pdpte = pdpt + remap_rva->PDPT_offset;
    PDE* pd = NULL;

    if (!pdpte->present) {
        est->BootServices->AllocatePages(AllocateAnyPages, mem_type, 1, (EFI_PHYSICAL_ADDRESS*)&pd);
        est->BootServices->SetMem((VOID*)pd, PAGE_SIZE, 0);
        *pdpte = make_PDPTE((uint64_t)pd, PI);
    }
    else {
        pd = (PDE*)(pdpte->address << 12);
    }

    PDE* pde = pd + remap_rva->PD_offset;
    PTE* pt = NULL;

    if (!pde->present) {
        est->BootServices->AllocatePages(AllocateAnyPages, mem_type, 1, (EFI_PHYSICAL_ADDRESS*)&pt);
        est->BootServices->SetMem((VOID*)pt, PAGE_SIZE, 0);
        *pde = make_PDE((uint64_t)pt, PI);
    }
    else {
        pt = (PTE*)(pde->address << 12);
    }

    PTE* pte = pt + remap_rva->PT_offset;
    *pte = make_PTE(*current_source, PI);
    pte->execute_disable = execute_disable;

    if (PAT_enable == 1 && PI->PAT_support != 0) {
        pte->pat = 1;
    }
    else {
        pte->pat = 0;
    }

    if (PWT_enable == 1) {
        pte->write_through = 1;
    }
    else {
        pte->write_through = 0;
    }

    if (PCD_enable == 1) {
        pte->cache_disable = 1;
    }
    else {
        pte->cache_disable = 0;
    }

    *current_source += PAGE_SIZE;

    fullUpdateRemapRVA(remap_rva);
}

void remapKernel(PML4E* pml4, void* _source, void* _dest, size_t size, const PagingInformation* PI) {
    size_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    uint64_t current_src = (uint64_t)_source;
    RawVirtualAddress remap_rva = parse_virtual_address(_dest);

    for (size_t i = 0; i < pages; ++i) {
        indirectRemap(pml4, &remap_rva, LoaderPersistentMemory, &current_src, 0, PI);
    }
}

void mapLoader(PML4E* pml4, const PagingInformation* PI) {
    SimpleMMAP smmap = getSimpleMMAP();

    size_t desc_num = smmap.size / smmap.desc_size;

    for (size_t i = 0; i < desc_num; ++i) {
        EFI_MEMORY_DESCRIPTOR* current_descriptor = (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)smmap.mmap + i * smmap.desc_size);
        
        if (current_descriptor->Type == EfiLoaderCode || current_descriptor->Type == EfiLoaderData) {
            RawVirtualAddress remap_rva = parse_virtual_address((void*)current_descriptor->PhysicalStart);

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

    est->BootServices->FreePool((VOID*)smmap.mmap);
}

void remapGOP(PML4E* pml4, BasicGraphics* BasicGFX, const PagingInformation* PI) {
    if (BasicGFX->FBSIZE > EFI_GOP_MAX_REMAP) {
        kputs(u"LOADER PANIC: FRAMEBUFFER TOO LARGE TO FIT IN MEMORY");
        Terminate();
    }

    size_t pages = (BasicGFX->FBSIZE + PAGE_SIZE - 1) / PAGE_SIZE;

    uint64_t current_src = (uint64_t)BasicGFX->FBADDR;
    RawVirtualAddress remap_rva = parse_virtual_address((void*)EFI_GOP_REMAP);

    PAT_enable = 1;
    for (size_t i = 0; i < pages; ++i) {
        indirectRemap(pml4, &remap_rva, LoaderPersistentMemory, &current_src, 1, PI);
    }
    PAT_enable = 0;

    BasicGFX->FBADDR = (uint32_t*)EFI_GOP_REMAP;
}

void remapTTYFont(PML4E* pml4, const void** tty_font, size_t size, const PagingInformation* PI) {
    if (size > TTY_FONT_MAX_REMAP) {
        kputs(u"LOADER PANIC: TTY FONT TOO LARGE TO FIT IN MEMORY");
        Terminate();
    }

    size_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    uint64_t current_src = (uint64_t)*tty_font;
    RawVirtualAddress remap_rva = parse_virtual_address((void*)TTY_FONT_REMAP);

    for (size_t i = 0; i < pages; ++i) {
        indirectRemap(pml4, &remap_rva, LoaderPersistentMemory, &current_src, 1, PI);
    }

    *tty_font = (const void*)TTY_FONT_REMAP;
}

// called only once preferably, tolerance of two calls,
// as should be anticipated by the function that retreives the EFI memory map
// so make sure to allocate space for +4 memory descriptors when retrieving the memory map (instead of the usual +2)
static void* makeshiftMalloc(EfiMemoryMap* mmap, size_t pages) {
    const size_t desc_num = mmap->mmap_size / mmap->desc_size;

    for (size_t i = 0; i < desc_num; ++i) {
        EFI_MEMORY_DESCRIPTOR* mm_descriptor = (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)mmap->mmap + i * mmap->desc_size);

        if (mm_descriptor->Type == EfiConventionalMemory && mm_descriptor->NumberOfPages >= pages) {
            mm_descriptor->NumberOfPages -= pages;

            void* range_start = (void*)(mm_descriptor->PhysicalStart + mm_descriptor->NumberOfPages * PAGE_SIZE);
            
            EFI_MEMORY_DESCRIPTOR newBlock = {
                .Type = LoaderPersistentMemory,
                .PhysicalStart = (EFI_PHYSICAL_ADDRESS)range_start,
                .VirtualStart = 0, // don't care about this for now
                .NumberOfPages = pages,
                .Attribute = mm_descriptor->Attribute
            };

            // append new block at the end
            *(EFI_MEMORY_DESCRIPTOR*)((uint8_t*)(mmap->mmap + desc_num * mmap->desc_size)) = newBlock;
            mmap->mmap_size += mmap->desc_size;

            return range_start;
        }
    }

    return NULL;
}

// Please setup a temporary stack before calling this function.
// Call this function after SetVirtualMap (otherwise it's impossible to know where runtime services are mapped),
// but before setting up paging
void* setupLoaderInfo(PML4E* pml4, LoaderInfo* linfo, const PagingInformation* PI) {
    const size_t total_size = sizeof(*linfo)
        + linfo->mmap.mmap_size
        - sizeof(linfo->mmap.mmap_key)
        - sizeof(linfo->mmap.desc_ver)
        - sizeof(linfo->mmap.mmap);

    if (total_size > LOADER_INFO_MAX_REMAP) {
        return NULL;
    }

    // TTY font already mapped, so this means the corresponding PML4, PDPT and PD have already been allocated and mapped
    // since we have 32MB - 512KB, we need at most 32 / 2 - 1 = 16 - 1 = 15 page tables
    // the real number of pages we need is required_pages + required_pages / PT_ENTRIES
    // notice we don't need to round up the division result here, however,
    // we need to add 0x80 to take into account the 512KB of space reserved to the TTY font.

    const size_t required_pages = (total_size + PAGE_SIZE - 1) / PAGE_SIZE;
    const size_t total_pages = required_pages + (required_pages + 0x80) / PT_ENTRIES;

    void* linfo_buffer = makeshiftMalloc(&linfo->mmap, total_pages);
    if (linfo_buffer == NULL) {
        return NULL;
    }

    uint8_t* ptr = linfo_buffer;

    *((uint64_t*)ptr) = (uint64_t)linfo->mmap.mmap_size;
    ptr += sizeof(uint64_t);
    
    *((uint64_t*)ptr) = (uint64_t)linfo->mmap.desc_size;
    ptr += sizeof(uint64_t);

    memcpy(ptr, linfo->mmap.mmap, linfo->mmap.mmap_size);
    ptr += linfo->mmap.mmap_size;

    // might be mis-aligned, so be careful with how you access the following data from now on.
    *(KernelLocInfo*)ptr = linfo->KernelLI;
    ptr += sizeof(KernelLocInfo);

    *(PHYS_MEM_INFO*)ptr = linfo->pmi;
    ptr += sizeof(PHYS_MEM_INFO);

    *(BasicGraphics*)ptr = linfo->gfxData;
    ptr += sizeof(BasicGraphics);

    *(EFI_RUNTIME_SERVICES**)ptr = linfo->rtServices;
    ptr += sizeof(EFI_RUNTIME_SERVICES*);

    *(void**)ptr = (void*)(linfo->PCIe_ECAM_0);
    ptr += sizeof(void*);

    *(uint64_t*)ptr = linfo->ACPIRevision;
    ptr += sizeof(uint64_t);

    *(void**)ptr = linfo->RSDP;

    // Now we remap this buffer, first get a pointer to the next page
    uint64_t misalignment = (uint64_t)ptr % PAGE_SIZE;
    if (misalignment != 0) {
        ptr += PAGE_SIZE - misalignment;
    }

    uint64_t current_source = (uint64_t)linfo_buffer;
    RawVirtualAddress remap_rva = parse_virtual_address((void*)LOADER_INFO_REMAP);

    for (size_t i = 0; i < required_pages; ++i) {
        PML4E* pml4e = pml4 + remap_rva.PML4_offset;
        PDPTE* pdpt = (PDPTE*)(pml4e->address << 12);
        PDPTE* pdpte = pdpt + remap_rva.PDPT_offset;
        PDE* pd = (PDE*)(pdpte->address << 12);
        PDE* pde = pd + remap_rva.PD_offset;
        PTE* pt = (PTE*)(pde->address << 12);

        if (!pde->present) {
            pt = (PTE*)ptr;
            ptr += PAGE_SIZE;
            memset(pt, 0, PAGE_SIZE);
            *pde = make_PDE((uint64_t)pt, PI);
        }
        else {
            pt = (PTE*)(pde->address << 12);
        }

        PTE* pte = pt + remap_rva.PT_offset;
        *pte = make_PTE(current_source, PI);

        current_source += PAGE_SIZE;

        fullUpdateRemapRVA(&remap_rva);
    }

    return linfo_buffer;
}
