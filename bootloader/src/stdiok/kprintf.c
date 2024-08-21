#include <limits.h>
#include <stdarg.h>
#include <wchar.h>

#include <efi/efi.h>
#include <efi/efi_datatypes.h>

#include <stdiok.h>
#include <stdlibk.h>

size_t kprintf(const CHAR16* restrict format, ...) {
    CHAR16 *buffer;
    est->BootServices->AllocatePool(EfiLoaderData, sizeof(CHAR16) * 512, (VOID**)&buffer);

    va_list args;
    va_start(args, format);

    size_t n = kvsnprintf(buffer, 512, format, args);

    va_end(args);
    kputs(buffer);

    est->BootServices->FreePool(buffer);
    return n;
}
