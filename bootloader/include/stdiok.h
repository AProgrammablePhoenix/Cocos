#pragma once

#include <efi/efi_datatypes.h>

#include <stdarg.h>
#include <wchar.h>

#include <efi/efi.h>

INTN kputs(CHAR16* s);

size_t kvsnprintf(CHAR16* restrict buffer, size_t bufsz, const CHAR16* restrict format, va_list vlist);
size_t kprintf(const CHAR16* restrict format, ...);

EFI_INPUT_KEY readkey(void);
