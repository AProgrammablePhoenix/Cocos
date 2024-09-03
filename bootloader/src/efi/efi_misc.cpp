#include <efi/efi_datatypes.h>
#include <efi/efi_misc.hpp>
#include <efi/efi.h>

#include <ldstdio.hpp>
#include <ldstdlib.hpp>

int Loader::guidcmp(const EFI_GUID* _guid1, const EFI_GUID* _guid2) {
    if constexpr (sizeof(EFI_GUID) % sizeof(uint64_t) == 0) {
        for (size_t i = 0; i < sizeof(EFI_GUID) / sizeof(uint64_t); ++i) {
            if (*(reinterpret_cast<const uint64_t*>(_guid1) + i) !=
                *(reinterpret_cast<const uint64_t*>(_guid2) + i)
            ) {
                return 0;
            }
        }

        return 1;
    }
    else {
        return Loader::memcmp(
            reinterpret_cast<const VOID*>(_guid1),
            reinterpret_cast<const VOID*>(_guid2),
            sizeof(EFI_GUID)
        );
    }
}

[[noreturn]] void EFI::Terminate(void) {
    Loader::puts(u"\n\rPress a key to terminate.\n\r");
    readkey();
    EFI::sys->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, nullptr);
    Loader::puts(u"System shutdown failed, press the power button for an extended period of time.\n\r");
    while (1);
}
