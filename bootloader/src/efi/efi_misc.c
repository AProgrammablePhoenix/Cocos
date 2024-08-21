#include <efi/efi_datatypes.h>
#include <efi/efi.h>

#include <stdiok.h>

// returns any non-zero value if the two values are equal, 0 otherwise
int kmemcmp(const VOID* _buf1, const VOID* _buf2, UINTN _size) {
    const uint8_t* _raw1 = (uint8_t*)_buf1;
    const uint8_t* _raw2 = (uint8_t*)_buf2;

    for (UINTN i = 0; i < _size; ++i) {
        if (_raw1[i] != _raw2[i]) {
            return 0;
        }
    }

    return 1;
}

int kguidcmp(const EFI_GUID* _guid1, const EFI_GUID* _guid2) {
    if (sizeof(EFI_GUID) % sizeof(uint64_t) == 0) {
        for (size_t i = 0; i < sizeof(EFI_GUID) / sizeof(uint64_t); ++i) {
            if (*((uint64_t*)_guid1 + i) != *((uint64_t*)_guid2 + i)) {
                return 0;
            }
        }

        return 1;
    }

    return kmemcmp((VOID*)_guid1, (VOID*)_guid2, sizeof(EFI_GUID));
}

void Terminate(void) {
    kputs(u"\n\rPress a key to terminate.\n\r");
    readkey();
    est->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
}
