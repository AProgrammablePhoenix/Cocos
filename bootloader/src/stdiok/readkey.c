#include <efi/efi_datatypes.h>
#include <efi/efi.h>

EFI_INPUT_KEY readkey(void) {
    EFI_INPUT_KEY key;
    while (est->ConIn->ReadKeyStroke(est->ConIn, &key) != EFI_SUCCESS);
    return key;
}
