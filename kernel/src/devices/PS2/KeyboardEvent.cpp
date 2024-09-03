#include <cstdint>

#include <devices/PS2/controller.hpp>
#include <devices/PS2/Keypoints.hpp>

#include <screen/Log.hpp>

namespace Devices::PS2 {
    EventResponse(*keyboardEventConverter)(uint8_t byte, BasicKeyPacket* buffer);

    namespace {
        static inline void _utoax(uint64_t x, char buffer[17]) {
            char tmp[16] = { 0 };
            char* tp = tmp;

            uint64_t i;
            uint64_t v = x;

            while (v || tp == tmp) {
                i = v % 16;
                v /= 16;

                if (i < 10) {
                    *tp++ = i + u'0';
                }
                else {
                    *tp++ = i + u'a' - 10;
                }
            }

            uint16_t len = tp - tmp;
            while (len < 2) {
                *tp++ = '0';
                ++len;
            }

            while (tp > tmp) {
                *buffer++ = *--tp;
            }
            *buffer++ = '\0';
        }

        // The handler needs C linkage in order to be called from assembly

        extern "C" void ps2_keyboard_event_handler(void) {
            uint32_t byte = recv_byte_ps2_port_1();
            if (byte > 0xFF) {
                return;
            }

            char buffer[17] = { 0 };

            BasicKeyPacket packet;
            if (keyboardEventConverter(byte, &packet) == EventResponse::PACKET_CREATED) {
                Log::puts("Keyboard event: ");
                _utoax(packet.scancode, buffer);
                Log::puts(buffer);
                Log::putc(',');
                _utoax(packet.keypoint, buffer);
                Log::puts(buffer);
                Log::putc(',');
                Log::puts(packet.flags != 0 ? "PRESSED" : "RELEASED");
                Log::puts("\n\r");
            }
        }
    }
}