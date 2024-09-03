#pragma once

namespace Devices {
	namespace PS2 {
		enum class StatusCode {
			SUCCESS,
			FATAL_ERROR
		};

		StatusCode initializeKeyboard();
		extern "C" void PS2_IRQ1_handler(void);
	}
}
