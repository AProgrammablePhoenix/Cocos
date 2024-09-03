#include <efi/efi_datatypes.h>
#include <efi/efi.h>

#include <ldstdio.hpp>

EFI_INPUT_KEY EFI::readkey(void) {
    EFI_INPUT_KEY key;
    while (EFI::sys->ConIn->ReadKeyStroke(EFI::sys->ConIn, &key) != EFI_SUCCESS);
    return key;
}
