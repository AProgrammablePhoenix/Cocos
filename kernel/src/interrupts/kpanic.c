#include <stddef.h>
#include <stdint.h>

#include <screen/tty.h>

extern void dump_core(uint64_t errv);

void kpanic(const char* msg, uint64_t errv) {
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
