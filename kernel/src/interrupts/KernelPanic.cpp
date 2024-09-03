#include <cstddef>
#include <cstdint>

#include <efi.h>

#include <interrupts/CoreDump.hpp>
#include <interrupts/KernelPanic.hpp>

#include <screen/Log.hpp>

namespace {
	[[noreturn]] static void kernelPanicShutdownFailed() {
		Log::puts("------ KERNEL PANIC (MAXIMAL SEVERITY) ------\n\r");
		Log::puts("Software shutdown failed, please perform a hard reset manually\n\r");
		Log::puts("Press the power button for an extended period of time.\n\r");
		while (1);
	}

	[[noreturn]] static void kernelPanicShutdownSecondary(const EFI_RUNTIME_SERVICES* rtServices) {
		Log::puts("KERNEL PANIC (HIGH SEVERITY): COULD NOT GET CURRENT TIME\n\r");
		Log::puts("Switching to secondary method, shutting down soon...\n\r");

		// should be enough ticks to show the message
		for (size_t i = 0; i < 80000000; ++i) {
			__asm__ volatile("outb %b0, %w1" :: "a"(0), "Nd"(0x80) : "memory");
		}

		rtServices->ResetSystem(EfiResetShutdown, EFI_ABORTED, 0, nullptr);
		kernelPanicShutdownFailed();
	}

	// this function is undocumented in the C++ headers, and should only be used by assembly code
	extern "C" [[noreturn]] void kpanic(const char* msg, uint64_t errv) {
		Panic::Panic(msg, errv);
	}
}

namespace Panic {
	[[noreturn]] void Panic(const char* msg, uint64_t errv) {
		Log::puts("\n\r------ KERNEL PANIC ------\n\r");

		if (msg != nullptr) {
			Log::puts("\t\t ");
			Log::puts(msg);
			Log::puts("\n\r");
		}

		Panic::dumpCore(errv);

		__asm__ volatile("cli");
		while (1) {
			__asm__ volatile("hlt");
		}
	}

	[[noreturn]] void PanicShutdown(const EFI_RUNTIME_SERVICES* rtServices, const char* msg) {
		Log::puts("------ KERNEL PANIC SHUTDOWN ------\n\r");
		Log::puts("\tREASON: ");
		Log::puts(msg);
		Log::puts("\n\rShutting down in 10 seconds...\n\r");

		EFI_TIME time1, time2;
		EFI_STATUS status = rtServices->GetTime(&time1, nullptr);
		if (status != EFI_SUCCESS) {
			kernelPanicShutdownSecondary(rtServices);
		}

		uint64_t elapsed = 0;

		while (elapsed < 10) {
			if (elapsed % 2 == 0) {
				status = rtServices->GetTime(&time2, nullptr);
				if (status != EFI_SUCCESS) {
					kernelPanicShutdownSecondary(rtServices);
				}
				else if (time1.Second != time2.Second) {
					++elapsed;
				}
			}
			else {
				status = rtServices->GetTime(&time1, nullptr);
				if (status != EFI_SUCCESS) {
					kernelPanicShutdownSecondary(rtServices);
				}
				else if (time1.Second != time2.Second) {
					++elapsed;
				}
			}
		}

		rtServices->ResetSystem(EfiResetShutdown, EFI_ABORTED, 0, nullptr);
		kernelPanicShutdownSecondary(rtServices);
	}
}
