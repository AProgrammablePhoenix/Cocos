#pragma once

#include <efi/efi_datatypes.h>

#include <loader/paging.hpp>

namespace Loader {
    const void* loadFont(EFI_HANDLE ImageHandle, PML4E* pml4, const PagingInformation* PI);
}
