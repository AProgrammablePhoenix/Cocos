#include <stdint.h>

#include <devices/PS2/keypoints.h>

#include <interrupts/i8042.h>

#include <screen/tty.h>

unsigned int (*ps2_keyboard_event_converter)(uint8_t byte, BasicKeyPacket* buffer);

static inline void _utoax(uint64_t x, char buffer[17]) {
    char tmp[16];
    char *tp = tmp;

    uint64_t i;
    uint64_t v = x;

    while (v || tp == tmp) {
        i = v % 16;
        v /= 16;

        if (i < 10) {
            *tp++ = i + u'0';
        } else {
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

void ps2_keyboard_event_handler(void) {
    uint32_t byte = recv_byte_ps2_port_1();
    if (byte > 0xFF) {
        return;
    }

    char buffer[17] = { 0 };

    BasicKeyPacket packet;
    if (ps2_keyboard_event_converter(byte, &packet) == PACKET_CREATED) {
        tty_puts("Keyboard event: ");
        _utoax(packet.scancode, buffer);
        tty_puts(buffer);
        tty_putc(',');
        _utoax(packet.keypoint, buffer);
        tty_puts(buffer);
        tty_putc(',');
        tty_puts(packet.flags != 0 ? "PRESSED" : "RELEASED");
        tty_puts("\n\r");
    }
}
