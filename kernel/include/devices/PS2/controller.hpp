#pragma once

#include <cstdint>

namespace Devices {
	namespace PS2 {
		extern "C" {
			extern uint32_t initialize_ps2_controller(void);
			extern uint16_t identify_ps2_port_1(void);
			extern uint32_t send_byte_ps2_port_1(uint8_t data);
			extern uint32_t recv_byte_ps2_port_1(void);
		}
	}
}