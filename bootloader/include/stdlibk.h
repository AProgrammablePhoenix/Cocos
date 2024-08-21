#pragma once

#include <efi/efi_datatypes.h>

INTN kitoa(INTN x, CHAR16 *buffer, INT32 radix);
INTN kutoa(UINTN x, CHAR16* buffer, INT32 radix);

void* memcpy(void *restrict dest, const void *restrict src, size_t count);
void* memset(void* dest, int ch, size_t count);
