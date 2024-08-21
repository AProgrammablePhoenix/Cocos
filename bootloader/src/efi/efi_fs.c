#include <efi/efi_datatypes.h>
#include <efi/efi_misc.h>
#include <efi/efi.h>

#include <stdiok.h>

static const EFI_GUID EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID = {
    .Data1 = 0x964E5B22,
    .Data2 = 0x6459,
    .Data3 = 0x11D2,
    .Data4 = { 0x8E, 0x39, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B }
};

static EFI_GUID EFI_FILE_INFO_GUID = {
    .Data1 = 0x09576E92,
    .Data2 = 0x6D3F,
    .Data3 = 0x11D2,
    .Data4 = { 0x8E, 0x39, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B }
};

EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* getDeviceSFSP(EFI_HANDLE ImageHandle, EFI_HANDLE DeviceHandle) {
    EFI_GUID** BootDeviceProtocols;
    UINTN DeviceProtocolsCount;
    
    if (est->BootServices->ProtocolsPerHandle(DeviceHandle, &BootDeviceProtocols, &DeviceProtocolsCount) != EFI_SUCCESS) {
        kputs(u"Error retrieving device protocols\n\r");
        Terminate();
    }

    for (UINTN k = 0; k < DeviceProtocolsCount; ++k) {
        if (kguidcmp(BootDeviceProtocols[k], &EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID)) {
            EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* efi_sfsp;

            if (est->BootServices->OpenProtocol(DeviceHandle, BootDeviceProtocols[k], (VOID**)&efi_sfsp, ImageHandle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL) != EFI_SUCCESS) {
                kputs(u"Error opening device file system protocol\n\r");
                Terminate();
            }

            est->BootServices->FreePool(BootDeviceProtocols);
            return efi_sfsp;
        }
    }

    est->BootServices->FreePool(BootDeviceProtocols);
    return NULL;
}

EFI_FILE_PROTOCOL* openDeviceVolume(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* SFSP) {
    EFI_FILE_PROTOCOL* efi_root_fsp = NULL;

    if (SFSP != NULL && SFSP->OpenVolume(SFSP, &efi_root_fsp) != EFI_SUCCESS) {
        kputs(u"Error opening root volume on device\n\r");
        Terminate();
    }

    return efi_root_fsp;
}

EFI_FILE_PROTOCOL* openReadOnlyFile(EFI_FILE_PROTOCOL* Volume, CHAR16* FilePath) {
    EFI_FILE_PROTOCOL* efi_file_fsp = NULL;
    EFI_STATUS Status;

    if (Volume != NULL && (Status = Volume->Open(Volume, &efi_file_fsp, FilePath, 0x0000000000000001, 0)) != EFI_SUCCESS) {
        kprintf(u"Error opening file (error %llx)\n\r", Status);
        Terminate();
    }

    return efi_file_fsp;
}

EFI_FILE_INFO* getFileInfo(EFI_FILE_PROTOCOL* File) {
    UINTN FileInfoSize = 0;
    EFI_FILE_INFO* FileInfo = NULL;

    if (File->GetInfo(File, &EFI_FILE_INFO_GUID, &FileInfoSize, FileInfo) != EFI_BUFFER_TOO_SMALL) {
        kputs(u"Error retrieving file info size\n\r");
        Terminate();
    }

    if (est->BootServices->AllocatePool(EfiLoaderData, FileInfoSize, (VOID**)&FileInfoSize) != EFI_SUCCESS) {
        kputs(u"Error allocating buffer to read file information data\n\r");
        Terminate();
    }

    if (File->GetInfo(File, &EFI_FILE_INFO_GUID, &FileInfoSize, FileInfo) != EFI_SUCCESS) {
        kputs(u"Error reading file information data\n\r");
        Terminate();
    }

    return FileInfo;
}
