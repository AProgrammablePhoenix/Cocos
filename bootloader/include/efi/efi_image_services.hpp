#pragma once

#ifndef __EFI_STANDALONE__

#include <efi/efi_datatypes.h>
#include <efi/efi.h>

namespace EFI {
    EFI_LOADED_IMAGE_PROTOCOL* getLoadedImageProtocol(EFI_HANDLE ImageHandle);
};

#endif
