#include <cstdint>
#include <cstddef>

#include <devices/PS2/controller.hpp>
#include <devices/PS2/Keyboard.hpp>
#include <devices/PS2/Keypoints.hpp>

#include <interrupts/pic.hpp>

namespace Devices::PS2 {
	namespace {
		// PS/2 KEYBOARD SCAN CODE IDENTIFIERS
		static inline constexpr uint8_t SCAN_CODE_SET_1 = 0x01;
		static inline constexpr uint8_t SCAN_CODE_SET_2 = 0x02;
		static inline constexpr uint8_t SCAN_CODE_SET_3 = 0x03;

		// CONSTANTS
		static inline constexpr uint32_t MAX_RETRY		= 3;
		static inline constexpr uint32_t FATAL_ERROR	= 0xDEADBEEF;
		static inline constexpr uint32_t INTERNAL_ERROR = 0xBAAAAAAD;
		static inline constexpr uint8_t RESET_PASSED	= 0xAA;

		// PS/2 KEYBOARD COMMANDS
		static inline constexpr uint8_t SET_LEDS				= 0xED;
		static inline constexpr uint8_t ECHO					= 0xEE;
		static inline constexpr uint8_t SCAN_CODE_SET_INTERACT	= 0xF0;
		static inline constexpr uint8_t ENABLE_SCANNING			= 0xF4;
		static inline constexpr uint8_t KBD_ACK					= 0xFA;
		static inline constexpr uint8_t KBD_RESEND				= 0xFE;
		static inline constexpr uint8_t KBD_RESET				= 0xFF;

		// PS/2 KEYBOARD SUB-COMMANDS
		static inline constexpr uint8_t GET_SCAN_CODE_SET	= 0x00;
		static inline constexpr uint8_t SET_SCAN_CODE_SET_1 = 0x01;
		static inline constexpr uint8_t SET_SCAN_CODE_SET_2 = 0x02;
		static inline constexpr uint8_t SET_SCAN_CODE_SET_3 = 0x03;

		static inline constexpr uint8_t SET_SCROLL_LOCK = 0x01;
		static inline constexpr uint8_t SET_NUMBER_LOCK = 0x02;
		static inline constexpr uint8_t SET_CAPS_LOCK	= 0x04;

		static inline uint32_t sendCommand(uint8_t command) {
			for (size_t t = 0; t < MAX_RETRY; ++t) {
				uint32_t status = send_byte_ps2_port_1(command);
				if (status != 0) {
					continue;
				}
				status = recv_byte_ps2_port_1();
				if (status != KBD_RESEND) {
					return status;
				}
			}

			return INTERNAL_ERROR;
		}

		static inline unsigned int sendCommandData(uint8_t command, uint8_t data) {
			for (size_t t = 0; t < MAX_RETRY; ++t) {
				uint32_t status = send_byte_ps2_port_1(command);
				if (status != 0) {
					continue;
				}
				status = send_byte_ps2_port_1(data);
				if (status != 0) {
					continue;
				}
				status = recv_byte_ps2_port_1();
				if (status != KBD_RESEND) {
					return status;
				}
			}

			return INTERNAL_ERROR;
		}

		static inline void disableKeyboard() {
			Interrupts::PIC::mask_irq(1);
		}

		static inline unsigned int handleInternalError() {
			static unsigned int errorCount = 0;
			if (++errorCount >= MAX_RETRY) {
				disableKeyboard();
				return FATAL_ERROR;
			}
			uint32_t status = sendCommand(KBD_RESET);
			if (status != 0) {
				handleInternalError();
			}
			status = recv_byte_ps2_port_1();
			if (status != RESET_PASSED) {
				handleInternalError();
			}
			return 0;
		}

		static inline unsigned int getScanCodeSet(unsigned int* scanCodeSet) {
			uint32_t status = sendCommandData(SCAN_CODE_SET_INTERACT, GET_SCAN_CODE_SET);
			if (status == INTERNAL_ERROR) {
				if (handleInternalError() != 0) {
					return FATAL_ERROR;
				}
				return getScanCodeSet(scanCodeSet);
			}
			else if (status != KBD_ACK) {
				if (handleInternalError() != 0) {
					return FATAL_ERROR;
				}
				return getScanCodeSet(scanCodeSet);
			}
			status = recv_byte_ps2_port_1();
			if (status > 0xFF) {
				if (handleInternalError() != 0) {
					return FATAL_ERROR;
				}
				return getScanCodeSet(scanCodeSet);
			}
			*scanCodeSet = status;
			return 0;
		}

		static inline unsigned int setScanCodeSet(uint8_t scanCodeSet) {
			uint32_t status = sendCommandData(SCAN_CODE_SET_INTERACT, scanCodeSet);
			if (status == INTERNAL_ERROR) {
				if (handleInternalError() != 0) {
					return FATAL_ERROR;
				}
				return setScanCodeSet(scanCodeSet);
			}
			else if (status != KBD_ACK) {
				if (handleInternalError() != 0) {
					return FATAL_ERROR;
				}
				return setScanCodeSet(scanCodeSet);
			}
			return 0;
		}

		static inline unsigned int resetLEDS(void) {
			uint32_t status = sendCommandData(SET_LEDS, 0);
			if (status == INTERNAL_ERROR) {
				if (handleInternalError() != 0) {
					return FATAL_ERROR;
				}
				return resetLEDS();
			}
			else if (status != KBD_ACK) {
				if (handleInternalError() != 0) {
					return FATAL_ERROR;
				}
				return resetLEDS();
			}
			return 0;
		}

		static inline unsigned int echoCheck(void) {
			uint32_t status = sendCommand(ECHO);
			if (status == INTERNAL_ERROR) {
				if (handleInternalError() != 0) {
					return FATAL_ERROR;
				}
				return echoCheck();
			}
			else if (status != ECHO) {
				if (handleInternalError() != 0) {
					return FATAL_ERROR;
				}
				return echoCheck();
			}
			return 0;
		}

		static inline unsigned int enableScanning(void) {
			uint32_t status = sendCommand(ENABLE_SCANNING);
			if (status == INTERNAL_ERROR) {
				if (handleInternalError() == 0) {
					return FATAL_ERROR;
				}
				return enableScanning();
			}
			else if (status != KBD_ACK) {
				if (handleInternalError() != 0) {
					return FATAL_ERROR;
				}
				return FATAL_ERROR;
			}
			return 0;
		}
	}

	StatusCode initializeKeyboard(void) {
		// resets LEDs
		if (resetLEDS() == FATAL_ERROR) {
			return StatusCode::FATAL_ERROR;
		}

		// tries to set scan code set 1, otherwise adapts to the current one
		unsigned int scanCodeSet = 0;
		if (getScanCodeSet(&scanCodeSet) == FATAL_ERROR) {
			return StatusCode::FATAL_ERROR;
		}
		else if (scanCodeSet != SCAN_CODE_SET_1) {
			if (setScanCodeSet(SCAN_CODE_SET_1) == FATAL_ERROR) {
				return StatusCode::FATAL_ERROR;
			}
			if (getScanCodeSet(&scanCodeSet) == FATAL_ERROR) {
				return StatusCode::FATAL_ERROR;
			}
		}

		// Performs ECHO to check if the device is still responsive
		if (echoCheck() == FATAL_ERROR) {
			return StatusCode::FATAL_ERROR;
		}

		// selects the correct scan code converter
		if (scanCodeSet == SCAN_CODE_SET_1) {
			keyboardEventConverter = &keyboardScanCodeSet1Handler;
		}
		else if (scanCodeSet == SCAN_CODE_SET_2) {
			keyboardEventConverter = &keyboardScanCodeSet2Handler;
		}
		else if (scanCodeSet == SCAN_CODE_SET_3) {
			keyboardEventConverter = &keyboardScanCodeSet3Handler;
		}
		else {
			disableKeyboard();
			return StatusCode::FATAL_ERROR;
		}

		// Re-enables keyboard scanning
		if (enableScanning() == FATAL_ERROR) {
			disableKeyboard();
			return StatusCode::FATAL_ERROR;
		}

		return StatusCode::SUCCESS;
	}
}
