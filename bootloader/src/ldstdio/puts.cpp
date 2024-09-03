#include <efi/efi.h>

#include <ldstdio.hpp>

INTN Loader::puts(const CHAR16* s) {
    return EFI::sys->ConOut->OutputString(EFI::sys->ConOut, const_cast<CHAR16*>(s));
}
