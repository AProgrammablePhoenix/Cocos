#pragma once

#include <cstdint>

namespace Interrupts {
	namespace PIC {
		extern "C" {
			extern void initialize_pic(void);
			extern void disable_pic(void);
			extern void mask_irq(uint8_t irqLine);
			extern void enable_irq(uint8_t irqLine);
		}
	}
}