#pragma once

#include <efi.h>

#include <stdint.h>

[[noreturn]] void kpanic(const char* msg, uint64_t errv);
[[noreturn]] void kernel_panic_shutdown_failed();
[[noreturn]] void kernel_panic_shutdown_secondary(EFI_RUNTIME_SERVICES* rtServices);
[[noreturn]] void kernel_panic_shutdown(EFI_RUNTIME_SERVICES* rtServices, const char* message);
