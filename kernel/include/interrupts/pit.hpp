#pragma once

#include <cstdint>

namespace Interrupts {
	namespace PIT {
		extern "C" {
			extern volatile uint64_t SYSTEM_TIMER_US;
			extern volatile const uint32_t IRQ0_frequency;
			extern volatile const uint16_t IRQ0_us;
			extern volatile const uint16_t PIT_reload_value;
			extern void initialize_pit(void);
			extern void disable_pit(void);
		}
	}
}
