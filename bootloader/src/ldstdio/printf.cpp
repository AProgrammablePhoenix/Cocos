#include <stdarg.h>

#include <efi/efi.h>
#include <efi/efi_datatypes.h>

#include <ldstdio.hpp>
#include <ldstdlib.hpp>

size_t Loader::printf(const CHAR16* restrict format, ...) {
    CHAR16* buffer;
    EFI::sys->BootServices->AllocatePool(
        EfiLoaderData,
        sizeof(CHAR16) * 512,
        reinterpret_cast<VOID**>(&buffer)
    );

    va_list args;
    va_start(args, format);

    size_t n = Loader::vsnprintf(buffer, 512, format, args);

    va_end(args);
    Loader::puts(buffer);

    EFI::sys->BootServices->FreePool(buffer);
    return n;
}
