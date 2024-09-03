#pragma once

#ifndef __EFI_STANDALONE__

#include <efi/efi_datatypes.h>
#include <efi/efi.h>

namespace EFI {
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* getDeviceSFSP(EFI_HANDLE ImageHandle, EFI_HANDLE DeviceHandle);
    EFI_FILE_PROTOCOL* openDeviceVolume(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* SFSP);
    EFI_FILE_PROTOCOL* openReadOnlyFile(EFI_FILE_PROTOCOL* Volume, CHAR16* FilePath);
    EFI_FILE_INFO* getFileInfo(EFI_FILE_PROTOCOL* File);
};

#endif
