#include <stddef.h>
#include <stdint.h>

#include <screen/tty.h>

extern volatile uint64_t SYSTEM_TIMER_US;
extern const uint64_t IRQ0_us;

void system_timer_event_handler(void) {
    SYSTEM_TIMER_US += IRQ0_us;
}
