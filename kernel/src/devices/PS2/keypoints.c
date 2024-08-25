#include <stdint.h>

#include <devices/PS2/keypoints.h>

#define KEYPOINT(ROW, COLUMN) ((ROW & 0x07) | ((COLUMN & 0x1F) << 3))

// PS/2 Scan Code Set 1

static const uint8_t scan_code_set_1_keypoints[0x100] = {
    KEYPOINT(0, 0),     KEYPOINT(2, 0),     KEYPOINT(3, 1),     KEYPOINT(3, 2),     // 0x00
    KEYPOINT(3, 3),     KEYPOINT(3, 4),     KEYPOINT(3, 5),     KEYPOINT(3, 6),     // 0x04
    KEYPOINT(3, 7),     KEYPOINT(3, 8),     KEYPOINT(3, 9),     KEYPOINT(3, 10),    // 0x08
    KEYPOINT(3, 11),    KEYPOINT(3, 12),    KEYPOINT(3, 13),    KEYPOINT(4, 0),     // 0x0C
    KEYPOINT(4, 1),     KEYPOINT(4, 2),     KEYPOINT(4, 3),     KEYPOINT(4, 4),     // 0x10
    KEYPOINT(4, 5),     KEYPOINT(4, 6),     KEYPOINT(4, 7),     KEYPOINT(4, 8),     // 0x14
    KEYPOINT(4, 9),     KEYPOINT(4, 10),    KEYPOINT(4, 11),    KEYPOINT(4, 12),    // 0x18
    KEYPOINT(5, 13),    KEYPOINT(7, 0),     KEYPOINT(5, 1),     KEYPOINT(5, 2),     // 0x1C
    KEYPOINT(5, 3),     KEYPOINT(5, 4),     KEYPOINT(5, 5),     KEYPOINT(5, 6),     // 0x20
    KEYPOINT(5, 7),     KEYPOINT(5, 8),     KEYPOINT(5, 9),     KEYPOINT(5, 10),    // 0x24
    KEYPOINT(5, 11),    KEYPOINT(3, 0),     KEYPOINT(6, 0),     KEYPOINT(4, 13),    // 0x28
    KEYPOINT(6, 2),     KEYPOINT(6, 3),     KEYPOINT(6, 4),     KEYPOINT(6, 5),     // 0x2C
    KEYPOINT(6, 6),     KEYPOINT(6, 7),     KEYPOINT(6, 8),     KEYPOINT(6, 9),     // 0x30
    KEYPOINT(6, 10),    KEYPOINT(6, 11),    KEYPOINT(6, 12),    KEYPOINT(3, 18),    // 0x34
    KEYPOINT(7, 2),     KEYPOINT(7, 6),     KEYPOINT(5, 0),     KEYPOINT(2, 1),     // 0x38
    KEYPOINT(2, 2),     KEYPOINT(2, 3),     KEYPOINT(2, 4),     KEYPOINT(2, 5),     // 0x3C
    KEYPOINT(2, 6),     KEYPOINT(2, 7),     KEYPOINT(2, 8),     KEYPOINT(2, 9),     // 0x40
    KEYPOINT(2, 10),    KEYPOINT(3, 16),    KEYPOINT(1, 14),    KEYPOINT(4, 16),    // 0x44
    KEYPOINT(4, 17),    KEYPOINT(4, 18),    KEYPOINT(3, 19),    KEYPOINT(5, 16),    // 0x48
    KEYPOINT(5, 17),    KEYPOINT(5, 18),    KEYPOINT(5, 19),    KEYPOINT(6, 16),    // 0x4C
    KEYPOINT(6, 17),    KEYPOINT(6, 18),    KEYPOINT(7, 16),    KEYPOINT(7, 18),    // 0x50
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(2, 11),    // 0x54
    KEYPOINT(2, 12),    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x58
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x5C
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x60
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x64
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x68
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x6C
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x70
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x74
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x78
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x7C
    KEYPOINT(0, 0),     KEYPOINT(2, 0),     KEYPOINT(3, 1),     KEYPOINT(3, 2),     // 0x80
    KEYPOINT(3, 3),     KEYPOINT(3, 4),     KEYPOINT(3, 5),     KEYPOINT(3, 6),     // 0x84
    KEYPOINT(3, 7),     KEYPOINT(3, 8),     KEYPOINT(3, 9),     KEYPOINT(3, 10),    // 0x88
    KEYPOINT(3, 11),    KEYPOINT(3, 12),    KEYPOINT(3, 13),    KEYPOINT(4, 0),     // 0x8C
    KEYPOINT(4, 1),     KEYPOINT(4, 2),     KEYPOINT(4, 3),     KEYPOINT(4, 4),     // 0x90
    KEYPOINT(4, 5),     KEYPOINT(4, 6),     KEYPOINT(4, 7),     KEYPOINT(4, 8),     // 0x94
    KEYPOINT(4, 9),     KEYPOINT(4, 10),    KEYPOINT(4, 11),    KEYPOINT(4, 12),    // 0x98
    KEYPOINT(5, 13),    KEYPOINT(7, 0),     KEYPOINT(5, 1),     KEYPOINT(5, 2),     // 0x9C
    KEYPOINT(5, 3),     KEYPOINT(5, 4),     KEYPOINT(5, 5),     KEYPOINT(5, 6),     // 0xA0
    KEYPOINT(5, 7),     KEYPOINT(5, 8),     KEYPOINT(5, 9),     KEYPOINT(5, 10),    // 0xA4
    KEYPOINT(5, 11),    KEYPOINT(3, 0),     KEYPOINT(6, 0),     KEYPOINT(4, 13),    // 0xA8
    KEYPOINT(6, 2),     KEYPOINT(6, 3),     KEYPOINT(6, 4),     KEYPOINT(6, 5),     // 0xAC
    KEYPOINT(6, 6),     KEYPOINT(6, 7),     KEYPOINT(6, 8),     KEYPOINT(6, 9),     // 0xB0
    KEYPOINT(6, 10),    KEYPOINT(6, 11),    KEYPOINT(6, 12),    KEYPOINT(3, 18),    // 0xB4
    KEYPOINT(7, 2),     KEYPOINT(7, 6),     KEYPOINT(5, 0),     KEYPOINT(2, 1),     // 0xB8
    KEYPOINT(2, 2),     KEYPOINT(2, 3),     KEYPOINT(2, 4),     KEYPOINT(2, 5),     // 0xBC
    KEYPOINT(2, 6),     KEYPOINT(2, 7),     KEYPOINT(2, 8),     KEYPOINT(2, 9),     // 0xC0
    KEYPOINT(2, 10),    KEYPOINT(3, 16),    KEYPOINT(1, 14),    KEYPOINT(4, 16),    // 0xC4
    KEYPOINT(4, 17),    KEYPOINT(4, 18),    KEYPOINT(3, 19),    KEYPOINT(5, 16),    // 0xC8
    KEYPOINT(5, 17),    KEYPOINT(5, 18),    KEYPOINT(5, 19),    KEYPOINT(6, 16),    // 0xCC
    KEYPOINT(6, 17),    KEYPOINT(6, 18),    KEYPOINT(7, 16),    KEYPOINT(7, 18),    // 0xD0
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(2, 11),    // 0xD4
    KEYPOINT(2, 12),    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0xD8
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0xDC
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0xE0
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0xE4
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0xE8
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0xEC
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0xF0
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0xF4
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0xF8
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0)      // 0xFC
};

static const uint8_t ext_1_scan_code_set_1_keypoints[0x100] = {
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x00
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x04
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x08
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x0C
    KEYPOINT(2, 17),    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x10
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x14
    KEYPOINT(0, 0),     KEYPOINT(2, 19),    KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x18
    KEYPOINT(7, 19),    KEYPOINT(7, 11),    KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x1C
    KEYPOINT(1, 16),    KEYPOINT(1, 19),    KEYPOINT(2, 18),    KEYPOINT(0, 0),     // 0x20
    KEYPOINT(2, 16),    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x24
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x28
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(1, 17),    KEYPOINT(0, 0),     // 0x2C
    KEYPOINT(1, 18),    KEYPOINT(0, 0),     KEYPOINT(0, 10),    KEYPOINT(0, 0),     // 0x30
    KEYPOINT(0, 0),     KEYPOINT(3, 17),    KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x34
    KEYPOINT(7, 8),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x38
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x3C
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x40
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(2, 14),    // 0x44
    KEYPOINT(6, 13),    KEYPOINT(2, 15),    KEYPOINT(0, 0),     KEYPOINT(7, 12),    // 0x48
    KEYPOINT(0, 0),     KEYPOINT(7, 14),    KEYPOINT(0, 0),     KEYPOINT(3, 15),    // 0x4C
    KEYPOINT(7, 13),    KEYPOINT(4, 14),    KEYPOINT(2, 13),    KEYPOINT(3, 14),    // 0x50
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x54
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(7, 1),     // 0x58
    KEYPOINT(7, 9),     KEYPOINT(7, 10),    KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x5C
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x60
    KEYPOINT(0, 0),     KEYPOINT(0, 11),    KEYPOINT(0, 12),    KEYPOINT(0, 13),    // 0x64
    KEYPOINT(0, 14),    KEYPOINT(0, 15),    KEYPOINT(0, 16),    KEYPOINT(0, 17),    // 0x68
    KEYPOINT(0, 18),    KEYPOINT(0, 19),    KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x6C
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x70
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x74
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x78
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x7C
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x80
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x84
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x88
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x8C
    KEYPOINT(2, 17),    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x90
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x94
    KEYPOINT(0, 0),     KEYPOINT(2, 19),    KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x98
    KEYPOINT(7, 19),    KEYPOINT(7, 11),    KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x9C
    KEYPOINT(1, 16),    KEYPOINT(1, 19),    KEYPOINT(2, 18),    KEYPOINT(0, 0),     // 0xA0
    KEYPOINT(2, 16),    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0xA4
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0xA8
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(1, 17),    KEYPOINT(0, 0),     // 0xAC
    KEYPOINT(1, 18),    KEYPOINT(0, 0),     KEYPOINT(0, 10),    KEYPOINT(0, 0),     // 0xB0
    KEYPOINT(0, 0),     KEYPOINT(3, 17),    KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0xB4
    KEYPOINT(7, 8),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0xB8
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0xBC
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0xC0
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(2, 14),    // 0xC4
    KEYPOINT(6, 13),    KEYPOINT(2, 15),    KEYPOINT(0, 0),     KEYPOINT(7, 12),    // 0xC8
    KEYPOINT(0, 0),     KEYPOINT(7, 14),    KEYPOINT(0, 0),     KEYPOINT(3, 15),    // 0xCC
    KEYPOINT(7, 13),    KEYPOINT(4, 14),    KEYPOINT(2, 13),    KEYPOINT(3, 14),    // 0xD0
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0xD4
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(7, 1),     // 0xD8
    KEYPOINT(7, 9),     KEYPOINT(7, 10),    KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0xDC
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0xE0
    KEYPOINT(0, 0),     KEYPOINT(0, 11),    KEYPOINT(0, 12),    KEYPOINT(0, 13),    // 0xE4
    KEYPOINT(0, 14),    KEYPOINT(0, 15),    KEYPOINT(0, 16),    KEYPOINT(0, 17),    // 0xE8
    KEYPOINT(0, 18),    KEYPOINT(0, 19),    KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0xEC
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0xF0
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0xF4
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0xF8
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0)      // 0xFC
};

static const uint8_t ext_2_scan_code_set_1_scancodes_pressed[4] = { 0xE0, 0x2A, 0xE0, 0x37 };
static const uint8_t ext_2_scan_code_set_1_scancodes_released[4] = { 0xE0, 0xB7, 0xE0, 0xAA };
static const uint8_t ext_2_scan_code_set_1_keypoint = KEYPOINT(1, 13);

static const uint8_t ext_3_scan_code_set_1_scancodes[6] = { 0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5 };
static const uint8_t ext_3_scan_code_set_1_keypoint = KEYPOINT(1, 15);

// PS/2 Scan Code Set 2

static const uint8_t scan_code_set_2_keypoints[0x84] = {
    KEYPOINT(0, 0),     KEYPOINT(2, 9),     KEYPOINT(0, 0),     KEYPOINT(2, 5),     // 0x00
    KEYPOINT(2, 3),     KEYPOINT(2, 1),     KEYPOINT(2, 2),     KEYPOINT(2, 12),    // 0x04
    KEYPOINT(0, 0),     KEYPOINT(2, 10),    KEYPOINT(2, 8),     KEYPOINT(2, 6),     // 0x08
    KEYPOINT(2, 4),     KEYPOINT(4, 0),     KEYPOINT(3, 0),     KEYPOINT(0, 0),     // 0x0C
    KEYPOINT(0, 0),     KEYPOINT(7, 2),     KEYPOINT(6, 0),     KEYPOINT(0, 0),     // 0x10
    KEYPOINT(7, 0),     KEYPOINT(4, 1),     KEYPOINT(3, 1),     KEYPOINT(0, 0),     // 0x14
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(6, 2),     KEYPOINT(5, 2),     // 0x18
    KEYPOINT(5, 1),     KEYPOINT(4, 2),     KEYPOINT(3, 2),     KEYPOINT(0, 0),     // 0x1C
    KEYPOINT(0, 0),     KEYPOINT(6, 4),     KEYPOINT(6, 3),     KEYPOINT(5, 3),     // 0x20
    KEYPOINT(4, 3),     KEYPOINT(3, 4),     KEYPOINT(3, 3),     KEYPOINT(0, 0),     // 0x24
    KEYPOINT(0, 0),     KEYPOINT(7, 6),     KEYPOINT(6, 5),     KEYPOINT(5, 4),     // 0x28
    KEYPOINT(4, 5),     KEYPOINT(4, 4),     KEYPOINT(3, 5),     KEYPOINT(0, 0),     // 0x2C
    KEYPOINT(0, 0),     KEYPOINT(6, 7),     KEYPOINT(6, 6),     KEYPOINT(5, 6),     // 0x30
    KEYPOINT(5, 5),     KEYPOINT(4, 6),     KEYPOINT(3, 6),     KEYPOINT(0, 0),     // 0x34
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(6, 8),     KEYPOINT(5, 7),     // 0x38
    KEYPOINT(4, 7),     KEYPOINT(3, 7),     KEYPOINT(3, 8),     KEYPOINT(0, 0),     // 0x3C
    KEYPOINT(0, 0),     KEYPOINT(6, 9),     KEYPOINT(5, 8),     KEYPOINT(4, 8),     // 0x40
    KEYPOINT(4, 9),     KEYPOINT(3, 10),    KEYPOINT(3, 9),     KEYPOINT(0, 0),     // 0x44
    KEYPOINT(0, 0),     KEYPOINT(6, 10),    KEYPOINT(6, 11),    KEYPOINT(5, 9),     // 0x48
    KEYPOINT(5, 10),    KEYPOINT(4, 10),    KEYPOINT(3, 11),    KEYPOINT(0, 0),     // 0x4C
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(5, 11),    KEYPOINT(0, 0),     // 0x50
    KEYPOINT(4, 11),    KEYPOINT(3, 12),    KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x54
    KEYPOINT(5, 0),     KEYPOINT(6, 12),    KEYPOINT(5, 13),    KEYPOINT(4, 12),    // 0x58
    KEYPOINT(4, 13),    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x5C
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x60
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(3, 13),    KEYPOINT(0, 0),     // 0x64
    KEYPOINT(0, 0),     KEYPOINT(6, 16),    KEYPOINT(0, 0),     KEYPOINT(5, 16),    // 0x68
    KEYPOINT(4, 16),    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x6C
    KEYPOINT(7, 16),    KEYPOINT(7, 18),    KEYPOINT(6, 17),    KEYPOINT(5, 17),    // 0x70
    KEYPOINT(5, 18),    KEYPOINT(4, 17),    KEYPOINT(2, 0),     KEYPOINT(3, 16),    // 0x74
    KEYPOINT(2, 11),    KEYPOINT(5, 19),    KEYPOINT(6, 18),    KEYPOINT(3, 19),    // 0x78
    KEYPOINT(3, 18),    KEYPOINT(4, 18),    KEYPOINT(1, 14),    KEYPOINT(0, 0),     // 0x7C
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(2, 7)      // 0x80
};

static const uint8_t ext_1_scan_code_set_2_keypoints[0x80] = {
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x00
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x04
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x08
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x0C
    KEYPOINT(0, 11),    KEYPOINT(7, 8),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x10
    KEYPOINT(7, 11),    KEYPOINT(2, 17),    KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x14
    KEYPOINT(0, 12),    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x18
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(7, 1),     // 0x1C
    KEYPOINT(0, 13),    KEYPOINT(1, 17),    KEYPOINT(0, 0),     KEYPOINT(1, 16),    // 0x20
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(7, 9),     // 0x24
    KEYPOINT(0, 14),    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(1, 19),    // 0x28
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(7, 10),    // 0x2C
    KEYPOINT(0, 15),    KEYPOINT(0, 0),     KEYPOINT(1, 18),    KEYPOINT(0, 0),     // 0x30
    KEYPOINT(2, 18),    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x34
    KEYPOINT(0, 16),    KEYPOINT(0, 0),     KEYPOINT(0, 10),    KEYPOINT(2, 16),    // 0x38
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x3C
    KEYPOINT(0, 17),    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x40
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x44
    KEYPOINT(0, 18),    KEYPOINT(0, 0),     KEYPOINT(3, 17),    KEYPOINT(0, 0),     // 0x48
    KEYPOINT(0, 0),     KEYPOINT(2, 19),    KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x4C
    KEYPOINT(0, 19),    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x50
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x54
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(7, 19),    KEYPOINT(0, 0),     // 0x58
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x5C
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x60
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x64
    KEYPOINT(0, 0),     KEYPOINT(3, 15),    KEYPOINT(0, 0),     KEYPOINT(7, 12),    // 0x68
    KEYPOINT(2, 14),    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x6C
    KEYPOINT(2, 13),    KEYPOINT(3, 14),    KEYPOINT(7, 13),    KEYPOINT(0, 0),     // 0x70
    KEYPOINT(7, 14),    KEYPOINT(6, 13),    KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x74
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(4, 14),    KEYPOINT(0, 0),     // 0x78
    KEYPOINT(0, 0),     KEYPOINT(2, 15),    KEYPOINT(0, 0),     KEYPOINT(0, 0)      // 0x7C
};

static const uint8_t ext_2_scan_code_set_2_scancodes_pressed[4] = { 0xE0, 0x12, 0xE0, 0x7C };
static const uint8_t ext_2_scan_code_set_2_scancodes_released[6] = { 0xE0, 0xF0, 0x7C, 0xE0, 0xF0, 0x12 };
static const uint8_t ext_2_scan_code_set_2_keypoint = KEYPOINT(1, 13);

static const uint8_t ext_3_scan_code_set_3_scancodes[8] = { 0xE1, 0x14, 0x77, 0xE1, 0xF0, 0x14, 0xF0, 0x77 };
static const uint8_t ext_3_scan_code_set_3_keypoint = KEYPOINT(1, 15);

static const uint8_t scan_code_set_3_keypoints[0x90] = {
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x00
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(2, 1),     // 0x04
    KEYPOINT(2, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x08
    KEYPOINT(0, 0),     KEYPOINT(4, 0),     KEYPOINT(3, 0),     KEYPOINT(2, 2),     // 0x0C
    KEYPOINT(0, 0),     KEYPOINT(7, 0),     KEYPOINT(6, 0),     KEYPOINT(0, 0),     // 0x10
    KEYPOINT(5, 0),     KEYPOINT(4, 1),     KEYPOINT(3, 1),     KEYPOINT(2, 3),     // 0x14
    KEYPOINT(0, 0),     KEYPOINT(7, 2),     KEYPOINT(6, 2),     KEYPOINT(5, 2),     // 0x18
    KEYPOINT(5, 1),     KEYPOINT(4, 2),     KEYPOINT(3, 2),     KEYPOINT(2, 4),     // 0x1C
    KEYPOINT(0, 0),     KEYPOINT(6, 4),     KEYPOINT(6, 3),     KEYPOINT(5, 3),     // 0x20
    KEYPOINT(4, 3),     KEYPOINT(3, 4),     KEYPOINT(3, 3),     KEYPOINT(2, 5),     // 0x24
    KEYPOINT(0, 0),     KEYPOINT(7, 6),     KEYPOINT(6, 5),     KEYPOINT(5, 4),     // 0x28
    KEYPOINT(4, 5),     KEYPOINT(4, 4),     KEYPOINT(0, 0),     KEYPOINT(2, 6),     // 0x2C
    KEYPOINT(0, 0),     KEYPOINT(6, 7),     KEYPOINT(6, 6),     KEYPOINT(5, 6),     // 0x30
    KEYPOINT(5, 5),     KEYPOINT(4, 6),     KEYPOINT(3, 6),     KEYPOINT(2, 7),     // 0x34
    KEYPOINT(0, 0),     KEYPOINT(7, 8),     KEYPOINT(6, 8),     KEYPOINT(5, 7),     // 0x38
    KEYPOINT(4, 7),     KEYPOINT(3, 7),     KEYPOINT(3, 8),     KEYPOINT(2, 8),     // 0x3C
    KEYPOINT(0, 0),     KEYPOINT(6, 9),     KEYPOINT(5, 8),     KEYPOINT(4, 8),     // 0x40
    KEYPOINT(4, 9),     KEYPOINT(3, 10),    KEYPOINT(3, 9),     KEYPOINT(2, 9),     // 0x44
    KEYPOINT(0, 0),     KEYPOINT(6, 10),    KEYPOINT(3, 17),    KEYPOINT(5, 9),     // 0x48
    KEYPOINT(5, 10),    KEYPOINT(4, 10),    KEYPOINT(3, 11),    KEYPOINT(2, 10),    // 0x4C
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(5, 11),    KEYPOINT(0, 0),     // 0x50
    KEYPOINT(4, 11),    KEYPOINT(3, 12),    KEYPOINT(2, 11),    KEYPOINT(1, 13),    // 0x54
    KEYPOINT(7, 11),    KEYPOINT(6, 12),    KEYPOINT(5, 13),    KEYPOINT(4, 12),    // 0x58
    KEYPOINT(4, 13),    KEYPOINT(0, 0),     KEYPOINT(2, 12),    KEYPOINT(1, 14),    // 0x5C
    KEYPOINT(7, 13),    KEYPOINT(7, 12),    KEYPOINT(1, 15),    KEYPOINT(6, 13),    // 0x60
    KEYPOINT(3, 14),    KEYPOINT(3, 15),    KEYPOINT(3, 13),    KEYPOINT(2, 13),    // 0x64
    KEYPOINT(0, 0),     KEYPOINT(6, 16),    KEYPOINT(7, 14),    KEYPOINT(5, 16),    // 0x68
    KEYPOINT(4, 16),    KEYPOINT(4, 14),    KEYPOINT(2, 14),    KEYPOINT(2, 15),    // 0x6C
    KEYPOINT(7, 16),    KEYPOINT(7, 18),    KEYPOINT(6, 17),    KEYPOINT(5, 17),    // 0x70
    KEYPOINT(5, 18),    KEYPOINT(4, 17),    KEYPOINT(3, 16),    KEYPOINT(0, 0),     // 0x74
    KEYPOINT(0, 0),     KEYPOINT(7, 19),    KEYPOINT(6, 18),    KEYPOINT(0, 0),     // 0x78
    KEYPOINT(5, 19),    KEYPOINT(4, 18),    KEYPOINT(3, 18),    KEYPOINT(0, 0),     // 0x7C
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x80
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x84
    KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(0, 0),     KEYPOINT(7, 1),     // 0x88
    KEYPOINT(7, 9),     KEYPOINT(7, 10),    KEYPOINT(0, 0),     KEYPOINT(0, 0),     // 0x8C
};

#define SCAN_CODE_SET_1_EXT_1_2 0xE0
#define SCAN_CODE_SET_1_EXT_2   0x2A
#define SCAN_CODE_SET_1_EXT_3   0xE1

unsigned int ps2_keyboard_scan_code_set_1(uint8_t byte, BasicKeyPacket* buffer) {
    static enum {
        DEFAULT,
        EXTENDED1,
        EXTENDED2_PRESS,
        EXTENDED2_RELEASE,
        EXTENDED3
    } state;

    static unsigned int bytes_read = 0;
    static unsigned int on_error = 0;

    switch (state) {
        case DEFAULT:
            switch (byte) {
                case SCAN_CODE_SET_1_EXT_1_2:
                    ++bytes_read;
                    state = EXTENDED1;
                    return IGNORE;
                case SCAN_CODE_SET_1_EXT_3:
                    ++bytes_read;
                    state = EXTENDED3;
                    return IGNORE;
                default:
                    BasicKeyPacket packet = {
                        .scancode = byte,
                        .keypoint = scan_code_set_1_keypoints[byte],
                        .flags = (byte & 0x80 ? 0 : KEY_PRESSED)
                    };
                    *buffer = packet;
                    return PACKET_CREATED;
            };
        case EXTENDED1:
            if (byte == ext_2_scan_code_set_1_scancodes_pressed[1]) {
                ++bytes_read;
                state = EXTENDED2_PRESS;
                return IGNORE;
            }
            else if (byte == ext_2_scan_code_set_1_scancodes_released[1]) {
                ++bytes_read;
                state = EXTENDED2_RELEASE;
                return IGNORE;
            }
            else {
                bytes_read = 0;
                state = DEFAULT;
                BasicKeyPacket packet = {
                    .scancode = byte,
                    .keypoint = ext_1_scan_code_set_1_keypoints[byte],
                    .flags = (byte & 0x80 ? 0 : KEY_PRESSED)
                };
                *buffer = packet;
                return PACKET_CREATED;
            }
        case EXTENDED2_PRESS:
            if (byte == ext_2_scan_code_set_1_scancodes_pressed[bytes_read] && !on_error) {
                if (++bytes_read == 4) {
                    bytes_read = 0;
                    state = DEFAULT;
                    BasicKeyPacket packet = {
                        .scancode = byte,
                        .keypoint = ext_2_scan_code_set_1_keypoint,
                        .flags = KEY_PRESSED
                    };
                    *buffer = packet;
                    return PACKET_CREATED;
                }
                return IGNORE;
            }
            else {
                on_error = 1;
                if (++bytes_read == 4) {
                    bytes_read = 0;
                    state = DEFAULT;
                    on_error = 0;
                }
                return IGNORE;
            }
        case EXTENDED2_RELEASE:
            if (byte == ext_2_scan_code_set_1_scancodes_released[bytes_read] && !on_error) {
                if (++bytes_read == 4) {
                    bytes_read = 0;
                    state = DEFAULT;
                    BasicKeyPacket packet = {
                        .scancode = byte,
                        .keypoint = ext_2_scan_code_set_1_keypoint,
                        .flags = 0
                    };
                    *buffer = packet;
                    return PACKET_CREATED;
                }
                return IGNORE;
            }
            else {
                on_error = 1;
                if (++bytes_read == 4) {
                    bytes_read = 0;
                    state = DEFAULT;
                    on_error = 0;
                }
                return IGNORE;
            }
        case EXTENDED3:
            if (byte == ext_3_scan_code_set_1_scancodes[bytes_read] && !on_error) {
                if (++bytes_read == 6) {
                    bytes_read = 0;
                    state = DEFAULT;
                    BasicKeyPacket packet = {
                        .scancode = byte,
                        .keypoint = ext_3_scan_code_set_1_keypoint,
                        .flags = KEY_PRESSED
                    };
                    *buffer = packet;
                    return PACKET_CREATED;
                }
                return IGNORE;
            }
            else {
                on_error = 1;
                if (++bytes_read == 6) {
                    bytes_read = 0;
                    state = DEFAULT;
                    on_error = 0;
                }
                return IGNORE;
            }
        default:
            return IGNORE;
    }
}

unsigned int ps2_keyboard_scan_code_set_2(uint8_t byte, BasicKeyPacket* buffer) {
    static enum {
        DEFAULT,
        EXTENDED1,
        EXTENDED2_PRESSED,
        EXTENDED2_RELEASED,
        EXTENDED3
    } state;


}

unsigned int ps2_keyboard_scan_code_set_3(void) {
    // do stuff
}
