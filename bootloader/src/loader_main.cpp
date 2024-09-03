#include <cpuid.h>

#include <efi/efi_datatypes.h>
#include <efi/efi_fs.hpp>
#include <efi/efi_image_services.hpp>
#include <efi/efi_misc.hpp>
#include <efi/efi.h>

#include <ldstdio.hpp>
#include <ldstdlib.hpp>

#include <loader/kernel_loader.hpp>
#include <loader/paging.hpp>
#include <loader/basic_graphics.hpp>
#include <loader/psf_font.hpp>
#include <loader/acpi_check.hpp>
#include <loader/system_config.hpp>
#include <loader/pci_setup.hpp>
#include <loader/loader_info.hpp>

EFI_SYSTEM_TABLE* EFI::sys;

extern "C" int efi_loader_setup(void);

#define PAT_SUPPORT_MASK    0x0000000000000001

extern "C" int EFIAPI efi_main(EFI_HANDLE handle, EFI_SYSTEM_TABLE* _sys) {
    EFI::sys = _sys;
    EFI::sys->ConOut->ClearScreen(EFI::sys->ConOut);

    static PagingInformation PI;

    uint32_t init_status = 0;
    efi_loader_setup();
    __asm__ volatile("mov %%eax, %0" : "=r"(init_status));
    __asm__ volatile("mov %%r10b, %0" :"=r"(PI.MAXPHYADDR));
    switch (init_status) {
        case static_cast<uint32_t>(-1): {
            uint32_t command_code = 0;
            __asm__ volatile("mov %%r9d, %0" : "=r"(command_code));
            Loader::printf(u"LOADER PANIC: CPUID DOES NOT SUPPORT COMMAND 0x%.8x.\n\r", command_code);
            EFI::Terminate();
        }
        case static_cast<uint32_t>(-2): {
            Loader::puts(u"LOADER PANIC: CPU DOES NOT SUPPORT NXE PROTECTION.\n\r");
            EFI::Terminate();
        }
    }

    if (init_status & PAT_SUPPORT_MASK) {
        PI.PAT_support = 1;
    }
    else {
        PI.PAT_support = 0;
    }

    // all static variables are locally persistent after boot services have been exited.
    // this simplifies code as only the loader has to be mapped, and not its stack
    // (which might reside in boot services data for some reason...)

    static PML4E* pml4;
    pml4 = Loader::setupBasicPaging(&PI);

    static KernelLocInfo KernelLI;
    KernelLI = Loader::loadKernel(handle, pml4, &PI);

    static BasicGraphics BasicGFX = {};
    BasicGFX = Loader::loadGraphics();

    Loader::prepareEFIRemap(pml4, &PI);
    Loader::mapLoader(pml4, &PI);
    Loader::remapGOP(pml4, &BasicGFX, &PI);

    Loader::loadFont(handle, pml4, &PI);

    EFI_SYSTEM_CONFIGURATION sysconfig;
    Loader::detectSystemConfiguration(&sysconfig);

    EFI_PHYSICAL_ADDRESS ecam_0_ptr = Loader::locatePCI(&sysconfig);

    // no code unrelated to memory mapping beyond this point

    static DMAZoneInfo* pmi;
    EFI::sys->BootServices->AllocatePages(
        AllocateAnyPages,
        LoaderTemporaryMemory,
        sizeof(DMAZoneInfo) / 4096,
        reinterpret_cast<EFI_PHYSICAL_ADDRESS*>(&pmi)
    );
    EFI::sys->BootServices->SetMem(pmi, sizeof(DMAZoneInfo), 0);

    static LoaderInfo linfo;
    linfo.KernelLI = KernelLI;
    linfo.gfxData = BasicGFX;
    linfo.PCIe_ECAM_0 = ecam_0_ptr;

    if (sysconfig.ACPI_20 != 0) {
        linfo.ACPIRevision = 2;
        linfo.RSDP = sysconfig.ACPI_20;
    }
    else if (sysconfig.ACPI_10 != 0) {
        linfo.ACPIRevision = 1;
        linfo.RSDP = sysconfig.ACPI_10;
    }

    // no EFI memory allocations beyond this point (so no printf)
    
    // memory map for the kernel, will be modified and given to SetVirtualAddressMap
    EfiMemoryMap mmap = Loader::getEfiMemoryMap();
    // memory map for BootServices, not modified
    EfiMemoryMap cmmap;

    // checks ACPI tables memory layout
    Loader::check_acpi(&sysconfig, &mmap);

    UINTN desc_num = mmap.mmap_size / mmap.desc_size;

    EFI_MEMORY_DESCRIPTOR* previous_descriptor = nullptr;

    for (size_t i = 0; i < desc_num; ++i) {
        EFI_MEMORY_DESCRIPTOR* current_descriptor = reinterpret_cast<EFI_MEMORY_DESCRIPTOR*>(
            reinterpret_cast<uint8_t*>(mmap.mmap) + i * mmap.desc_size
        );

        if (current_descriptor->PhysicalStart < DMA_ZONE_LIMIT) {
            const size_t start_page = current_descriptor->PhysicalStart / PAGE_SIZE;
            const size_t end_page = start_page + current_descriptor->NumberOfPages;

            if (current_descriptor->Type != EfiConventionalMemory) {
                for (size_t page = start_page; page < end_page; ++page) {
                    size_t byte = page / 8;
                    size_t bit = page % 8;
                    pmi->bitmap[byte] |= 1 << bit;
                }
            }
            else {
                for (size_t page = start_page; page < end_page; ++page) {
                    size_t byte = page / 8;
                    size_t bit = page % 8;
                    pmi->bitmap[byte] &= ~(1 << bit);
                }
            }
        }

        if (current_descriptor->Type == EfiRuntimeServicesCode || current_descriptor->Type == EfiRuntimeServicesData) {
            Loader::remapRuntimeServices(pml4, current_descriptor, &PI);
        }
        else if (current_descriptor->Type == EfiACPIMemoryNVS) {
            Loader::remapACPINVS(pml4, current_descriptor, &PI);
        }

        while (current_descriptor->PhysicalStart < previous_descriptor->PhysicalStart && previous_descriptor != nullptr) {
            EFI_MEMORY_DESCRIPTOR temp = *current_descriptor;
            *current_descriptor = *previous_descriptor;
            *previous_descriptor = temp;
            current_descriptor = previous_descriptor;
            previous_descriptor = i >= 1 ? reinterpret_cast<EFI_MEMORY_DESCRIPTOR*>(
                reinterpret_cast<uint8_t*>(mmap.mmap) + (i - 1) * mmap.desc_size
            ) : nullptr;
        }

        previous_descriptor = reinterpret_cast<EFI_MEMORY_DESCRIPTOR*>(
            reinterpret_cast<uint8_t*>(mmap.mmap) + i * mmap.desc_size
        );
    }

    linfo.mmap = mmap;

    cmmap = Loader::getEfiMemoryMap();

    EFI_STATUS Status = EFI::sys->BootServices->ExitBootServices(handle, cmmap.mmap_key);
    if (Status != EFI_SUCCESS) {
        Loader::puts(u"Could not exit boot services.\n\r");
        EFI::Terminate();
    }

    Status = EFI::sys->RuntimeServices->SetVirtualAddressMap(mmap.mmap_size, mmap.desc_size, mmap.desc_ver, mmap.mmap);
    if (Status != EFI_SUCCESS) {
        EFI::sys->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, nullptr);
    }

    // very basic stack for the kernel, aligned on an 8-byte boundary, 4KB
    static uint64_t temporary_stack[512] = { 0 };
    
    __asm__ volatile("mov %0, %%rsp\n\tmov %1, %%rbp"
        :: "r"(temporary_stack + 512),"r"(temporary_stack + 512));

    linfo.pmi = *pmi;
    linfo.rtServices = EFI::sys->RuntimeServices;

    Loader::setupLoaderInfo(pml4, &linfo, &PI);

    __asm__ volatile("mov %0, %%cr3" :: "r"(pml4) : "memory");

    static size_t r = 0;
    static size_t c = 0;
    for (r = 0; r < BasicGFX.ResY; ++r) {
        for (c = 0; c < BasicGFX.ResX; ++c) {
            BasicGFX.FBADDR[r * BasicGFX.PPSL + c] = 0;
        }
    }

    const static uint64_t boot_data_version = 1;

    // TODO: fix the loader to load the new Loader Info at the right place.

    __asm__ volatile("mov %0, %%rax" :: "m"(KernelLI.EntryPoint));
    __asm__ volatile("mov %0, %%rdx" :: "m"(boot_data_version) : "rdx");
    __asm__ volatile("callq *%rax");
    __asm__ volatile("jmp .");

    return 0;
}