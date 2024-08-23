#include <efi.h>

#include <stddef.h>
#include <stdint.h>

#include <interrupts/core_dump.h>
#include <screen/tty.h>

[[noreturn]] void kpanic(const char* msg, uint64_t errv) {
    tty_puts("\n\r------ KERNEL PANIC ------\n\r");
    
    if (msg != NULL) {
        tty_puts("\t\t ");
        tty_puts(msg);
        tty_puts("\n\r");
    }

    dump_core(errv);

    __asm__ volatile("cli");    
    while (1) {
        __asm__ volatile("hlt");
    }
}

[[noreturn]] void kernel_panic_shutdown_failed() {
    tty_puts("Software shutdown failed, please perform a hard reset manually (press the power button for an extended period of time).\n\r");    
    while (1);
}

[[noreturn]] void kernel_panic_shutdown_secondary(EFI_RUNTIME_SERVICES* rtServices) {
    tty_puts("KERNEL HIGH PANIC: COULD NOT GET CURRENT TIME\n\r");
    tty_puts("Switching to secondary method, shuting down soon...\n\r");

    // should be enough ticks to show the message
    for (size_t i = 0; i < 80000000; ++i) {
        __asm__ volatile("outb %b0, %w1" :: "a"(0), "Nd"(0x80) : "memory");
    }

    rtServices->ResetSystem(EfiResetShutdown, EFI_ABORTED, 0, NULL);

    kernel_panic_shutdown_failed();
}

[[noreturn]] void kernel_panic_shutdown(EFI_RUNTIME_SERVICES* rtServices, const char* message) {
    tty_puts("KERNEL PANIC SHUTDOWN: ");
    tty_puts(message);

    tty_puts("Shuting down in 10 seconds...\n\r");

    EFI_TIME time1, time2;

    EFI_STATUS status = rtServices->GetTime(&time1, NULL);

    if (status != EFI_SUCCESS) {            
        kernel_panic_shutdown_secondary(rtServices);
    }
    
    uint64_t elapsed = 0;

    // this design does not require a memcpy implementation, and is usually faster given the amount of calls made per second
    while (elapsed < 10) {
        if (elapsed % 2 == 0) {
            status = rtServices->GetTime(&time2, NULL);

            if (status != EFI_SUCCESS) {
                kernel_panic_shutdown_secondary(rtServices);
            }               

            if (time1.Second != time2.Second) {
                ++elapsed;
            }
        }
        else {
            status = rtServices->GetTime(&time1, NULL);

            if (status != EFI_SUCCESS) {
                kernel_panic_shutdown_secondary(rtServices);
            }

            if (time1.Second != time2.Second) {
                ++elapsed;
            }
        }
    }

    rtServices->ResetSystem(EfiResetShutdown, EFI_ABORTED, 0, NULL);

    kernel_panic_shutdown_failed();
}
