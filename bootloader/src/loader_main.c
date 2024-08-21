#include <cpuid.h>
#include <stdarg.h>
#include <limits.h>
#include <wchar.h>

#include <efi/efi_datatypes.h>
#include <efi/efi_misc.h>
#include <efi/efi_image_services.h>
#include <efi/efi_fs.h>
#include <efi/efi.h>

#include <stdiok.h>
#include <stdlibk.h>

#include <loader/kernel_loader.h>
#include <loader/paging_loader.h>
#include <loader/basic_graphics.h>
#include <loader/tty_font.h>
#include <loader/check_acpi.h>
#include <loader/system_config.h>
#include <loader/pci_setup.h>

EFI_SYSTEM_TABLE* est;

extern int efi_loader_setup(void);

#define PAT_SUPPORT_MASK    0x0000000000000001

int EFIAPI efi_main(EFI_HANDLE handle, EFI_SYSTEM_TABLE* _est) {
    est = _est;
    est->ConOut->ClearScreen(est->ConOut);

    static PagingInformation PI;
    
    uint32_t init_status = 0;
    efi_loader_setup();
    __asm__ volatile("mov %%eax, %0" : "=r"(init_status));
    switch (init_status) {
        case -1:
            uint32_t command_code = 0;
            __asm__ volatile("mov %%r9d, %0" : "=r"(command_code));
            kprintf(u"LOADER PANIC: CPUID DOES NOT SUPPORT COMMAND 0x%.8x.\n\r", command_code);
            Terminate();
        case -2:
            kputs(u"LOADER PANIC: CPU DOES NOT SUPPORT NXE PROTECTION.\n\r");
            Terminate();
    }

    if (init_status & PAT_SUPPORT_MASK) {
        PI.PAT_support = 1;
    }
    else {
        PI.PAT_support = 0;
    }

    __asm__ volatile("mov %%r10b, %0" : "=r"(PI.MAXPHYADDR));

    // all static variables are locally persistent after boot services have been exited.
    // this simplifies code as only the loader has to be mapped, and not its stack
    // (which might reside in boot services data for some reason...)

    static KernelLocInfo KernelLI;
    KernelLI = loadKernel(handle);

    static BasicGraphics BasicGFX = {};
    BasicGFX = loadGraphics();

    static PML4E* pml4;
    pml4 = setupBasicPaging(&PI);
    prepareEFIRemap(pml4, &PI);
    remapKernel(pml4, (void*)KernelLI.CurrentAddress, (void*)KernelLI.ImageBase, KernelLI.SizeOfImage, &PI);
    mapLoader(pml4, &PI);
    remapGOP(pml4, &BasicGFX, &PI);

    loadFont(handle, pml4, &PI);

    EFI_SYSTEM_CONFIGURATION sysconfig;
    detectSystemConfiguration(&sysconfig);

    EFI_PHYSICAL_ADDRESS ecam_0_ptr = locatePCI(&sysconfig);

    // no code unrelated to memory mapping beyond this point

    static PHYS_MEM_INFO* pmi;
    est->BootServices->AllocatePages(AllocateAnyPages, LoaderTemporaryMemory, sizeof(PHYS_MEM_INFO) / 4096, (EFI_PHYSICAL_ADDRESS*)&pmi);
    est->BootServices->SetMem(pmi, sizeof(PHYS_MEM_INFO), 0);

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
    
    // no EFI memory allocations beyond this point (so no kprintf)
    
    // memory map for the kernel, will be modified and given to SetVirtualAddressMap
    EfiMemoryMap mmap = getEfiMemoryMap();
    // memory map for BootServices, not modified
    EfiMemoryMap cmmap;

    // checks ACPI tables memory layout
    check_acpi(&sysconfig, &mmap);

    UINTN desc_num = mmap.mmap_size / mmap.desc_size;

    EFI_MEMORY_DESCRIPTOR* previous_descriptor = NULL;

    for (size_t i = 0; i < desc_num; ++i) {
        EFI_MEMORY_DESCRIPTOR* current_descriptor = (EFI_MEMORY_DESCRIPTOR*)(((uint8_t*)mmap.mmap) + i * mmap.desc_size);

        if (current_descriptor->PhysicalStart < DMA_ZONE_LIMIT) {
            const size_t start_page = current_descriptor->PhysicalStart / PAGE_SIZE;
            const size_t end_page = start_page + current_descriptor->NumberOfPages;

            if (current_descriptor->Type != EfiConventionalMemory) {
                for (size_t page = start_page; page < end_page; ++page) {
                    size_t byte = page / 8;
                    size_t bit = page % 8;
                    pmi->DMA_Bitmap[byte] |= 1 << bit;
                }
            }
            else {
                for (size_t page = start_page; page < end_page; ++page) {
                    size_t byte = page / 8;
                    size_t bit = page % 8;
                    pmi->DMA_Bitmap[byte] &= ~(1 << bit);
                }
            }
        }

        if (current_descriptor->Type == EfiRuntimeServicesCode || current_descriptor->Type == EfiRuntimeServicesData) {
            remapRuntimeServices(pml4, current_descriptor, &PI);
        }
        else if (current_descriptor->Type == EfiACPIMemoryNVS) {
            remapACPINVS(pml4, current_descriptor, &PI);
        }

        while (current_descriptor->PhysicalStart < previous_descriptor->PhysicalStart && previous_descriptor != NULL) {
            EFI_MEMORY_DESCRIPTOR temp = *current_descriptor;
            *current_descriptor = *previous_descriptor;
            *previous_descriptor = temp;
            current_descriptor = previous_descriptor;
            previous_descriptor = i >= 1 ? (EFI_MEMORY_DESCRIPTOR*)(((uint8_t*)mmap.mmap) + (i - 1) * mmap.desc_size) : NULL;
        }

        previous_descriptor = (EFI_MEMORY_DESCRIPTOR*)(((uint8_t*)mmap.mmap) + i * mmap.desc_size);
    }

    linfo.mmap = mmap;

    cmmap = getEfiMemoryMap();

    EFI_STATUS Status = est->BootServices->ExitBootServices(handle, cmmap.mmap_key);
    if (Status != EFI_SUCCESS) {
        kputs(u"Could not exit boot services.\n\r");
        Terminate();
    }

    Status = est->RuntimeServices->SetVirtualAddressMap(mmap.mmap_size, mmap.desc_size, mmap.desc_ver, mmap.mmap);
    if (Status != EFI_SUCCESS) {
        est->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
    }

    // very basic stack for the kernel, aligned on an 8-byte boundary, 4KB
    static uint64_t temporary_stack[512] = { 0 };
    
    __asm__ volatile("mov %0, %%rsp\n\tmov %1, %%rbp"
        :: "r"(temporary_stack + 512),"r"(temporary_stack + 512) : "rbp","rsp"
    );

    linfo.pmi = *pmi;
    linfo.rtServices = est->RuntimeServices;

    setupLoaderInfo(pml4, &linfo, &PI);

    __asm__ volatile("mov %0, %%cr3" :: "r"(pml4) : "memory");

    static size_t r = 0;
    static size_t c = 0;
    for (r = 0; r < BasicGFX.ResY; ++r) {
        for (c = 0; c < BasicGFX.ResX; ++c) {
            BasicGFX.FBADDR[r * BasicGFX.PPSL + c] = 0;
        }
    }

    static uint64_t kernel_ep_loc = 0;
    kernel_ep_loc = KernelLI.ImageBase + KernelLI.RelativeEntryPoint;

    const static uint64_t boot_data_version = 1;

    __asm__ volatile("mov %0, %%rax" :: "m"(kernel_ep_loc));
    __asm__ volatile("mov %0, %%rdx" :: "m"(boot_data_version) : "rdx");
    __asm__ volatile("callq *%rax");
    __asm__ volatile("jmp .");

    return 0;
}
