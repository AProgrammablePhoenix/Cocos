#include <stddef.h>
#include <stdint.h>

#include <interrupts/i8254.h>
#include <screen/tty.h>

void system_timer_event_handler(void) {
    SYSTEM_TIMER_US += IRQ0_us;
}
