#include <efi.h>

#include <devices/PS2/keyboard.h>

#include <interrupts/i8042.h>
#include <interrupts/i8254.h>
#include <interrupts/i8259A.h>
#include <interrupts/idt.h>
#include <interrupts/kpanic.h>
#include <interrupts/system_timer.h>

#include <screen/tty.h>

#include <mm/gdt.h>
#include <mm/pmm.h>
#include <mm/vmm.h>

extern void PS2_IRQ1_handler(void);

static EFI_RUNTIME_SERVICES* rtServices;

void kmain() {
    __asm__ volatile("cli");

    kernel_gdt_setup();
    kernel_idt_load();
    kernel_idt_setup();

    uint8_t* linfo = (uint8_t*)0xFFFF8004B8080000;
    rtServices = *(EFI_RUNTIME_SERVICES**)(linfo + 0x250 + *(uint64_t*)linfo);

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

    uint8_t* stack_top = 0;
    uint64_t stack_top_offset = 0;    
    __asm__ volatile("mov %%rsp, %0" : "=r"(stack_top));
    __asm__ volatile("mov %%rbp, %%rax\n\rsub %%rsp, %%rax\n\rmov %%rax, %0" : "=r"(stack_top_offset));

    for (size_t i = 0; i < stack_top_offset; ++i) {
        *(uint8_t*)(KERNEL_STACK_BASE - stack_top_offset + i) = *(stack_top + i);
    }

    __asm__ volatile("mov %0, %%rbp" :: "r"(KERNEL_STACK_BASE));
    __asm__ volatile("mov %0, %%rsp" :: "r"(KERNEL_STACK_BASE - stack_top_offset));

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
        else {
            if ((status = initialize_ps2_keyboard()) != 0) {
                tty_puts("PS/2 Keyboard Initialization failed.\n\r");
                tty_puts("No PS/2 input will be provided unless a USB keyboard is connected.\n\r");
                mask_irq(1);
            }
            else {
                tty_puts("PS/2 Keyboard Initialized.\n\r");
                register_irq(1, &PS2_IRQ1_handler, 0);
            }
        }
    }

    __asm__ volatile("sti");

    void* ecam_0 = map_pci_configuration(*(void**)(linfo + 0x258 + *(uint64_t*)linfo));
    __asm__ volatile("mov %0, %%r15" :: "r"(ecam_0));
    __asm__ volatile("int $3");
    
    __asm__ volatile("jmp .");
}
