#include <stdint.h>

#include <efi/efi_datatypes.h>
#include <efi/efi_misc.h>
#include <efi/efi_image_services.h>
#include <efi/efi_fs.h>
#include <efi/efi.h>

#include <stdiok.h>

#include <loader/kernel_loader.h>
#include <loader/paging_loader.h>

#define NOTIFY_CORRUPTED_KERNEL(code) \
    kprintf(u"Invalid or corrupted kernel (error %#.2x)", code); \
    Terminate()

#define KERNEL_FILE_PATH u"\\EFI\\BOOT\\kernel.exe\0"

static inline KernelLocInfo load_kernel_pe(const uint8_t* kernel_data, size_t kernel_size) {
    if (kernel_size < 0x3C + sizeof(COFF_FH)) {
        NOTIFY_CORRUPTED_KERNEL(0x0);
    }
    
    const uint8_t* pe_signature_ptr = kernel_data + kernel_data[0x3C];

    if ((*(uint32_t*)pe_signature_ptr) != 0x4550) {
        NOTIFY_CORRUPTED_KERNEL(0x1);
    }

    const COFF_FH* cfh = (COFF_FH*)(pe_signature_ptr + 4);

    if (cfh->Machine != 0x8664) {
        NOTIFY_CORRUPTED_KERNEL(0x2);
    }

    const OPT_HEADER* opthdr = (OPT_HEADER*)(pe_signature_ptr + 4 + sizeof(COFF_FH));

    if (opthdr->StandardFields.Magic != 0x20B) {
        NOTIFY_CORRUPTED_KERNEL(0x3);
    }

    const uint32_t RVAs = opthdr->WindowsSpecificFields.NumberOfRvaAndSizes;
    const DATA_DIRETORY* PDD = (DATA_DIRETORY*)((uint8_t*)opthdr + sizeof(OPT_HEADER));

    for (size_t i = 0; i < RVAs; ++i, ++PDD) {
        if (PDD->Size != 0) {
            NOTIFY_CORRUPTED_KERNEL(0x4); // Not supposed to happen for now
        }
    }

    SECTION_HEADER* SHDR = (SECTION_HEADER*)PDD;

    uint8_t* kernel_image_buffer = NULL;
    EFI_STATUS KernelImageBufferStatus = est->BootServices->AllocatePages(
        AllocateAnyPages,
        LoaderPersistentMemory,
        (opthdr->WindowsSpecificFields.SizeOfImage + PAGE_SIZE - 1) / PAGE_SIZE,
        (EFI_PHYSICAL_ADDRESS*)&kernel_image_buffer
    );

    if (KernelImageBufferStatus != EFI_SUCCESS) {
        kputs(u"Could not allocate enough memory to load the kernel image\n\r");
        Terminate();
    }

    for (size_t i = 0; i < cfh->NumberOfSections; ++i, ++SHDR) {
        est->BootServices->CopyMem(
            kernel_image_buffer + SHDR->VirtualAddress,
            (VOID*)(kernel_data + SHDR->PointerToRawData),
            SHDR->SizeOfRawData
        );

        if (SHDR->SizeOfRawData < SHDR->VirtualSize) {
            est->BootServices->SetMem(
                kernel_image_buffer + SHDR->VirtualAddress + SHDR->SizeOfRawData,
                SHDR->VirtualSize - SHDR->SizeOfRawData,
                0
            );
        }
    }

    KernelLocInfo kernel_loc = {
        .CurrentAddress = kernel_image_buffer,
        .SizeOfImage = opthdr->WindowsSpecificFields.SizeOfImage,
        .ImageBase = opthdr->WindowsSpecificFields.ImageBase,
        .RelativeEntryPoint = opthdr->StandardFields.AddressOfEntryPoint
    };

    return kernel_loc;
}

KernelLocInfo loadKernel(EFI_HANDLE ImageHandle) {
    EFI_LOADED_IMAGE_PROTOCOL* efi_lip = getLoadedImageProtocol(ImageHandle);
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* efi_sfsp = getDeviceSFSP(ImageHandle, efi_lip->DeviceHandle);
    EFI_FILE_PROTOCOL* efi_root_fsp = openDeviceVolume(efi_sfsp);
    EFI_FILE_PROTOCOL* efi_kernel_fsp = openReadOnlyFile(efi_root_fsp, KERNEL_FILE_PATH);

    if (efi_kernel_fsp == NULL) {
        kputs(u"Kernel image was either not found, or no suitable protocol was found to locate/open it\n\r");
        Terminate();
    }

    EFI_FILE_INFO* KernelInfo = getFileInfo(efi_kernel_fsp);
    UINT64 KernelSize = KernelInfo->FileSize;
    est->BootServices->FreePool(KernelInfo);

    UINT8* KernelData = NULL;
    est->BootServices->AllocatePool(EfiLoaderData, KernelSize, (VOID**)&KernelData);

    if (efi_kernel_fsp->Read(efi_kernel_fsp, &KernelSize, KernelData) != EFI_SUCCESS) {
        kputs(u"Error reading kernel\n\r");
        Terminate();
    }
    efi_kernel_fsp->Close(efi_kernel_fsp);

    KernelLocInfo kli = load_kernel_pe(KernelData, KernelSize);
    est->BootServices->FreePool(KernelData);

    return kli;
}
