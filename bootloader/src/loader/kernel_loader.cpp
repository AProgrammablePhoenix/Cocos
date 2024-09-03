#include <stdint.h>

#include <efi/efi_datatypes.h>
#include <efi/efi_fs.hpp>
#include <efi/efi_image_services.hpp>
#include <efi/efi_misc.hpp>
#include <efi/efi.h>

#include <ldstdio.hpp>
#include <ldstdlib.hpp>

#include <loader/kernel_loader.hpp>
#include <loader/paging.hpp>

namespace {
    [[noreturn]] static inline constexpr notify_corrupted_kernel(unsigned int code) {
        Loader::printf(u"Invalid or corrupted kernel (error 0x%.2x)", code);
        EFI::Terminate();
    }

    static const CHAR16* kernel_file_path = u"\\EFI\\BOOT\\kernel.img\0";

    static constexpr uint8_t ELF_MAGIC[4] = {0x7F, 0x45, 0x4C, 0x46};
    static constexpr uint8_t ELF_64 = 0x02;
    static constexpr uint8_t ELF_LE = 0x01;
    static constexpr uint16_t ELF_EXEC_TYPE = 0x02;
    static constexpr uint16_t ELF_MARCH_x86_64 = 0x003E;
    static constexpr uint32_t ELF_LOAD_SEGMENT = 0x00000001;

    static inline KernelLocInfo load_kernel_pe(const uint8_t* kernel_data, PML4E* pml4, const PagingInformation* PI) {
        const ELF_HEADER* elf_hdr = reinterpret_cast<const ELF_HEADER*>(kernel_data);

        if (!Loader::memcmp(&elf_hdr->Magic, ELF_MAGIC, sizeof(ELF_MAGIC))) {
            notify_corrupted_kernel(0x0);
        }
        else if (elf_hdr->Format != ELF_64 || elf_hdr->Endianness != ELF_LE || elf_hdr->MArch != ELF_MARCH_x86_64) {
            notify_corrupted_kernel(0x1);
        }
        else if (elf_hdr->Type != ELF_EXEC_TYPE) {
            notify_corrupted_kernel(0x2);
        }
        
        const ELF_PROGRAM_HEADER* elf_phdr_table = reinterpret_cast<const ELF_PROGRAM_HEADER*>(kernel_data + elf_hdr->ProgramHeaderTableOffset);

        for (size_t i = 0; i < elf_hdr->ProgramHeadersCount; ++i) {
            if (elf_phdr_table[i].SegmentType == ELF_LOAD_SEGMENT) {
                size_t segment_pages = (elf_phdr_table[i].SegmentMemorySize + PAGE_SIZE - 1) / PAGE_SIZE;

                EFI_PHYSICAL_ADDRESS segment_memory = 0;

                EFI::sys->BootServices->AllocatePages(
                    AllocateAnyPages,
                    LoaderPersistentMemory,
                    segment_pages,
                    &segment_memory
                );
                EFI::sys->BootServices->SetMem(reinterpret_cast<VOID*>(segment_memory), segment_pages * PAGE_SIZE, 0);
                EFI::sys->BootServices->CopyMem(
                    reinterpret_cast<VOID*>(segment_memory),
                    reinterpret_cast<VOID*>(const_cast<uint8_t*>(kernel_data) + elf_phdr_table[i].FileOffset),
                    elf_phdr_table[i].SegmentFileSize
                );

                Loader::mapKernel(
                    pml4,
                    reinterpret_cast<void*>(segment_memory),
                    reinterpret_cast<void*>(elf_phdr_table[i].SegmentVirtualAddress),
                    segment_pages * PAGE_SIZE,
                    PI
                );
            }
        }

        KernelLocInfo kernel_loc = {
            .EntryPoint = elf_hdr->EntryPoint
        };

        return kernel_loc;
    }
}

KernelLocInfo Loader::loadKernel(EFI_HANDLE ImageHandle, PML4E* pml4, const PagingInformation* PI) {
    EFI_LOADED_IMAGE_PROTOCOL* efi_lip = EFI::getLoadedImageProtocol(ImageHandle);
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* efi_sfsp = EFI::getDeviceSFSP(ImageHandle, efi_lip->DeviceHandle);
    EFI_FILE_PROTOCOL* efi_root_fsp = EFI::openDeviceVolume(efi_sfsp);
    EFI_FILE_PROTOCOL* efi_kernel_fsp = EFI::openReadOnlyFile(efi_root_fsp, const_cast<CHAR16*>(kernel_file_path));

    if (efi_kernel_fsp == nullptr) {
        Loader::puts(u"Kernel image was either not found, or no suitable protocol was found to locate/open it\n\r");
        EFI::Terminate();
    }

    EFI_FILE_INFO* KernelInfo = EFI::getFileInfo(efi_kernel_fsp);
    UINT64 KernelSize = KernelInfo->FileSize;
    EFI::sys->BootServices->FreePool(KernelInfo);

    UINT8* KernelData = nullptr;
    EFI::sys->BootServices->AllocatePool(
        EfiLoaderData,
        KernelSize,
        reinterpret_cast<VOID**>(&KernelData)
    );

    if (efi_kernel_fsp->Read(efi_kernel_fsp, &KernelSize, KernelData) != EFI_SUCCESS) {
        Loader::puts(u"Error reading kernel\n\r");
        EFI::Terminate();
    }
    efi_kernel_fsp->Close(efi_kernel_fsp);

    KernelLocInfo kli = load_kernel_pe(KernelData, pml4, PI);
    EFI::sys->BootServices->FreePool(KernelData);

    return kli;
}
