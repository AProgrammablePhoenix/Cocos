#pragma once

#ifndef __EFI_STANDALONE__

#include <efi/efi_datatypes.h>

int kmemcmp(const VOID* _buf1, const VOID* _buf2, UINTN _size);
int kguidcmp(const EFI_GUID* _guid1, const EFI_GUID* _guid2);

void Terminate(void);

#endif
