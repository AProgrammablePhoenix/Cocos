#pragma once

#include <cstdint>

#include <efi.h>

namespace Panic {
	[[noreturn]] void Panic(const char* msg);
	[[noreturn]] void Panic(const char* msg, uint64_t errv);
	[[noreturn]] void PanicShutdown(const EFI_RUNTIME_SERVICES* rtServices, const char* msg);
}
