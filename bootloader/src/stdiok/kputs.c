#include <efi/efi.h>

INTN kputs(CHAR16* s) {
    return est->ConOut->OutputString(est->ConOut, s);
}
