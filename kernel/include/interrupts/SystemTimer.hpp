#pragma once

namespace Interrupts {
	namespace SystemTimer {
		extern "C" {
			extern void PIT_IRQ0_handler(void);
		}
	}
}
