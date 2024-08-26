#pragma once

#include <stdint.h>

#define IGNORE         0
#define PACKET_CREATED 1

#define KEY_PRESSED 0x01

typedef struct {
    uint8_t scancode;
    uint8_t keypoint;
    uint8_t flags;
} BasicKeyPacket;

unsigned int ps2_keyboard_scan_code_set_1(uint8_t byte, BasicKeyPacket* buffer);
unsigned int ps2_keyboard_scan_code_set_2(uint8_t byte, BasicKeyPacket* buffer);
unsigned int ps2_keyboard_scan_code_set_3(uint8_t byte, BasicKeyPacket* buffer);
