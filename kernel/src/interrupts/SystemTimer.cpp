#include <cstdint>

#include <interrupts/pit.hpp>
#include <screen/Log.hpp>

// The handler needs C linkage in order to be called from assembly

namespace {
	extern "C" void system_timer_event_handler(void) {
		Interrupts::PIT::SYSTEM_TIMER_US += Interrupts::PIT::IRQ0_us;
	}
}
