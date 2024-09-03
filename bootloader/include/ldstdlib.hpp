#pragma once

#include <efi/efi_datatypes.h>
#include <efi/efi.h>

namespace Loader {
    INTN itoa(INTN x, CHAR16* buffer, INT32 radix);
    INTN utoa(UINTN x, CHAR16* buffer, INT32 radix);

    int memcmp(const VOID* _buf1, const VOID* _buf2, UINTN size);
    void* memcpy(void* restrict dest, const void* restrict src, size_t count);
    void* memset(void* dest, int ch, size_t count);
}
