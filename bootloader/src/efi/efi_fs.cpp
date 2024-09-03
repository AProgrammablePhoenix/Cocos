#include <efi/efi_datatypes.h>
#include <efi/efi_fs.hpp>
#include <efi/efi_misc.hpp>
#include <efi/efi.h>

#include <ldstdio.hpp>

namespace {
    static constexpr EFI_GUID EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID = {
        .Data1 = 0x964E5B22,
        .Data2 = 0x6459,
        .Data3 = 0x11D2,
        .Data4 = { 0x8E, 0x39, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B }
    };

    static constexpr EFI_GUID EFI_FILE_INFO_GUID = {
        .Data1 = 0x09576E92,
        .Data2 = 0x6D3F,
        .Data3 = 0x11D2,
        .Data4 = { 0x8E, 0x39, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B }
    };
}

EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* EFI::getDeviceSFSP(EFI_HANDLE ImageHandle, EFI_HANDLE DeviceHandle) {
    EFI_GUID** BootDeviceProtocols;
    UINTN DeviceProtocolsCount;

    if (EFI::sys->BootServices->ProtocolsPerHandle(
        DeviceHandle,
        &BootDeviceProtocols,
        &DeviceProtocolsCount
    ) != EFI_SUCCESS) {
        Loader::puts(u"Error retrieving device protocols\n\r");
        EFI::Terminate();
    }

    for (UINTN k = 0; k < DeviceProtocolsCount; ++k) {
        if (Loader::guidcmp(BootDeviceProtocols[k], &EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID)) {
            EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* efi_sfsp = nullptr;

            if (EFI::sys->BootServices->OpenProtocol(
                DeviceHandle,
                BootDeviceProtocols[k],
                reinterpret_cast<VOID**>(&efi_sfsp),
                ImageHandle,
                nullptr,
                EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL
            ) != EFI_SUCCESS) {
                Loader::puts(u"Error openining device file system protocol\n\r");
                EFI::Terminate();
            }

            EFI::sys->BootServices->FreePool(BootDeviceProtocols);
            return efi_sfsp;
        }
    }

    sys->BootServices->FreePool(BootDeviceProtocols);
    return nullptr;
}

EFI_FILE_PROTOCOL* EFI::openDeviceVolume(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* SFSP) {
    EFI_FILE_PROTOCOL* efi_root_fsp = nullptr;

    if (SFSP == nullptr || SFSP->OpenVolume(SFSP, &efi_root_fsp) != EFI_SUCCESS) {
        Loader::puts(u"Error opening root volume on device\n\r");
        EFI::Terminate();
    }

    return efi_root_fsp;
}

EFI_FILE_PROTOCOL* EFI::openReadOnlyFile(EFI_FILE_PROTOCOL* Volume, CHAR16* FilePath) {
    EFI_FILE_PROTOCOL* efi_file_fsp = nullptr;
    EFI_STATUS Status = EFI_INVALID_PARAMETER;

    if (Volume == nullptr || (Status = Volume->Open(
        Volume,
        &efi_file_fsp,
        FilePath,
        0x0000000000000001,
        0)
    ) != EFI_SUCCESS) {
        Loader::printf(u"Error opening file (error %llx)\n\r", Status);
        EFI::Terminate();
    }

    return efi_file_fsp;
}

EFI_FILE_INFO* EFI::getFileInfo(EFI_FILE_PROTOCOL* File) {
    UINTN FileInfoSize = 0;
    EFI_FILE_INFO* FileInfo = nullptr;

    if (File->GetInfo(
        File, 
        const_cast<EFI_GUID*>(&EFI_FILE_INFO_GUID),
        &FileInfoSize,
        FileInfo
    ) != EFI_BUFFER_TOO_SMALL) {
        Loader::puts(u"Error retrieveing file info size\n\r");
        EFI::Terminate();
    }

    if (EFI::sys->BootServices->AllocatePool(
        EfiLoaderData,
        FileInfoSize,
        reinterpret_cast<VOID**>(&FileInfo)
    ) != EFI_SUCCESS) {
        Loader::puts(u"Error allocating buffer to read file information data\n\r");
        EFI::Terminate();
    }

    if (File->GetInfo(
        File,
        const_cast<EFI_GUID*>(&EFI_FILE_INFO_GUID),
        &FileInfoSize,
        FileInfo
    ) != EFI_SUCCESS) {
        Loader::puts(u"Error reading file information data\n\r");
        EFI::Terminate();
    }

    return FileInfo;
}
