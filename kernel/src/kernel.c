#include <efi.h>

#include <screen/tty.h>

#include <mm/pmm.h>
#include <mm/vmm.h>

extern void kernel_gdt_setup(void);
extern void kernel_idt_load(void);
extern void kernel_idt_setup(void);
extern void register_irq(unsigned int irq_line, void(*handler)(void), unsigned is_trap);

[[noreturn]] static void kernel_panic_shutdown_failed() {
    tty_puts("Software shutdown failed, please perform a hard reset manually (press the power button for an extended period of time).\n\r");    
    while (1);
}

[[noreturn]] static void kernel_panic_shutdown_secondary(EFI_RUNTIME_SERVICES* rtServices) {
    tty_puts("KERNEL HIGH PANIC: COULD NOT GET CURRENT TIME\n\r");
    tty_puts("Switching to secondary method, shuting down soon...\n\r");

    // should be enough ticks to show the message
    for (size_t i = 0; i < 80000000; ++i) {
        __asm__ volatile("outb %b0, %w1" :: "a"(0), "Nd"(0x80) : "memory");
    }

    rtServices->ResetSystem(EfiResetShutdown, EFI_ABORTED, 0, NULL);

    kernel_panic_shutdown_failed();
}

[[noreturn]] static void kernel_panic_shutdown(EFI_RUNTIME_SERVICES* rtServices, const char* message) {
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

extern void initialize_pic(void);
extern void mask_irq(uint64_t irq_line);
extern void PIT_IRQ0_handler(void);
extern void initialize_pit(void);
extern int32_t initialize_ps2_controller(void);
extern int32_t identify_ps2_port_1(void);

void kmain() {
    __asm__ volatile("cli");

    kernel_gdt_setup();
    kernel_idt_load();
    kernel_idt_setup();

    uint8_t* linfo = (uint8_t*)0xFFFF8004B8080000;
    EFI_RUNTIME_SERVICES* rtServices = *(EFI_RUNTIME_SERVICES**)(linfo + 0x250 + *(uint64_t*)linfo);

    tty_setup();
    tty_puts("TTY Enabled\n\r");

    int32_t status = 0; 

    if ((status = pmm_setup()) != 0) {
        if (status == -1) {
            kernel_panic_shutdown(rtServices, "PMM INITIALIZATION FAILED (OUT OF MEMORY)\n\r");
        }
        else {
            kernel_panic_shutdown(rtServices, "PMM INITIALIZATION FAILED (UNKNOWN REASON)\n\r");
        }
    }

    tty_puts("PMM Initialized\n\r");

    if ((status = vmm_setup()) != 0) {
        kernel_panic_shutdown(rtServices, "VMM INITIALIZATION FAILED\n\r");
    }

    __asm__ volatile("mov %rsp, %r15");

    tty_puts("VMM Initialized\n\r");

    register_irq(0, &PIT_IRQ0_handler, 0);
    initialize_pic();
    initialize_pit();

    if ((status = initialize_ps2_controller()) != 0) {
        tty_puts("PS/2 Controller initialization failed.\n\r");
        mask_irq(1);
    }
    else {
        tty_puts("PS/2 Controller Initialized.\n\r");

        if ((uint32_t)(status = identify_ps2_port_1()) > 0xFFFF) {
            tty_puts("PS/2 Identify failed for device on port 1.\n\r");
            mask_irq(1);
        }
    }

    __asm__ volatile("sti");

    //void* ecam_0 = map_pci_configuration(*(void**)(linfo + 0x258 + *(uint64_t*)linfo));
    
    __asm__ volatile("jmp .");
}
