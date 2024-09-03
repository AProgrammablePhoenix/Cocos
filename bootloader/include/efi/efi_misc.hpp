#pragma once

#ifndef __EFI_STANDALONE__

#include <efi/efi_datatypes.h>

namespace Loader {
    int guidcmp(const EFI_GUID* _guid1, const EFI_GUID* _guid2);
}

namespace EFI {    
    [[noreturn]] void Terminate();
}

#endif
