#pragma once

#include <cstdint>

namespace Devices {
	namespace PS2 {
		struct BasicKeyPacket {
			uint8_t scancode;
			uint8_t keypoint;
			uint8_t flags;
		};

		enum class EventResponse {
			IGNORE,
			PACKET_CREATED
		};

		inline constexpr unsigned int KEY_PRESSED = 0x01;

		EventResponse keyboardScanCodeSet1Handler(uint8_t byte, BasicKeyPacket* buffer);
		EventResponse keyboardScanCodeSet2Handler(uint8_t byte, BasicKeyPacket* buffer);
		EventResponse keyboardScanCodeSet3Handler(uint8_t byte, BasicKeyPacket* buffer);

		extern EventResponse (*keyboardEventConverter)(uint8_t byte, BasicKeyPacket* buffer);
	}
}
