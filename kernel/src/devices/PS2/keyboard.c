#include <stddef.h>
#include <stdint.h>

#include <interrupts/i8042.h>
#include <interrupts/i8259A.h>

// PS/2 KEYBOARD SCAN CODE IDENTIFIERS
#define SCAN_CODE_SET_1 0x01
#define SCAN_CODE_SET_2 0x02
#define SCAN_CODE_SET_3 0x03

// CONSTANTS
#define MAX_RETRY       3
#define FATAL_ERROR     0xDEADBEEF
#define INTERNAL_ERROR  0xBAAAAAAD
#define RESET_PASSED    0xAA

// PS/2 KEYBOARD COMMANDS
#define SET_LEDS                0xED
#define ECHO                    0xEE
#define SCAN_CODE_SET_INTERACT  0xF0
#define ENABLE_SCANNING         0xF4
#define KBD_ACK                 0xFA
#define KBD_RESEND              0xFE
#define KBD_RESET               0xFF

// PS/2 KEYBOARD SUB-COMMANDS
#define GET_SCAN_CODE_SET   0x00
#define SET_SCAN_CODE_SET_1 0x01
#define SET_SCAN_CODE_SET_2 0x02
#define SET_SCAN_CODE_SET_3 0x03

#define SET_SCROLL_LOCK     0x01
#define SET_NUMBER_LOCK     0x02
#define SET_CAPS_LOCK       0x04

static inline unsigned int send_command(uint8_t command) {
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

static inline unsigned int send_command_data(uint8_t command, uint8_t data) {
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

static inline void disable_keyboard() {
    mask_irq(1);
}

static inline unsigned int handle_internal_error() {
    static unsigned int error_count = 0;
    if (++error_count >= MAX_RETRY) {
        disable_keyboard();
        return FATAL_ERROR;
    }
    uint32_t status = send_command(KBD_RESET);
    if (status != 0) {
        handle_internal_error();
    }
    status = recv_byte_ps2_port_1();
    if (status != RESET_PASSED) {
        handle_internal_error();
    }
    return 0;
}

static inline unsigned int get_scan_code_set(unsigned int* scan_code_set) {
    uint32_t status = send_command_data(SCAN_CODE_SET_INTERACT, GET_SCAN_CODE_SET);
    if (status == INTERNAL_ERROR) {
        if (handle_internal_error() != 0) {
            return FATAL_ERROR;
        }
        return get_scan_code_set(scan_code_set);
    }
    else if (status != KBD_ACK) {
        if (handle_internal_error() != 0) {
            return FATAL_ERROR;
        }
        return get_scan_code_set(scan_code_set);
    }
    status = recv_byte_ps2_port_1();
    if (status > 0xFF) {
        if (handle_internal_error() != 0) {
            return FATAL_ERROR;
        }
        return get_scan_code_set(scan_code_set);
    }
    *scan_code_set = status;
    return 0;
}

static inline unsigned int set_scan_code_set(uint8_t scan_code_set) {
    uint32_t status = send_command_data(SCAN_CODE_SET_INTERACT, scan_code_set);
    if (status == INTERNAL_ERROR) {
        if (handle_internal_error() != 0) {
            return FATAL_ERROR;
        }
        return set_scan_code_set(scan_code_set);
    }
    else if (status != KBD_ACK) {
        if (handle_internal_error() != 0) {
            return FATAL_ERROR;
        }
        return set_scan_code_set(scan_code_set);
    }
    return 0;
}

static inline unsigned int reset_leds(void) {
    uint32_t status = send_command_data(SET_LEDS, 0);
    if (status == INTERNAL_ERROR) {
        if (handle_internal_error() != 0) {
            return FATAL_ERROR;
        }
        return reset_leds();
    }
    else if (status != KBD_ACK) {
        if (handle_internal_error() != 0) {
            return FATAL_ERROR;
        }
        return reset_leds();
    }
    return 0;
}

static inline unsigned int echo_check(void) {
    uint32_t status = send_command(ECHO);
    if (status == INTERNAL_ERROR) {
        if (handle_internal_error() != 0) {
            return FATAL_ERROR;
        }
        return echo_check();
    }
    else if (status != ECHO) {
        if (handle_internal_error() != 0) {
            return FATAL_ERROR;
        }
        return echo_check();
    }
    return 0;
}

static inline unsigned int enable_scanning(void) {
    uint32_t status = send_command(ENABLE_SCANNING);
    if (status == INTERNAL_ERROR) {
        if (handle_internal_error() == 0) {
            return FATAL_ERROR;
        }
        return enable_scanning();
    }
    else if (status != KBD_ACK) {
        if (handle_internal_error() != 0) {
            return FATAL_ERROR;
        }
        return FATAL_ERROR;
    }
    return 0;
}

unsigned int initialize_ps2_keyboard(void) {
    // reset LEDs
    if (reset_leds() == FATAL_ERROR) {
        return FATAL_ERROR;
    }

    // tries to set scan scode set 3, otherwise adapts to the current one
    unsigned int scan_code_set = 0;
    if (get_scan_code_set(&scan_code_set) == FATAL_ERROR) {
        return FATAL_ERROR;
    }
    else if (scan_code_set != SCAN_CODE_SET_3) {
        if (set_scan_code_set(SCAN_CODE_SET_3) == FATAL_ERROR) {
            return FATAL_ERROR;
        }
        if (get_scan_code_set(&scan_code_set) == FATAL_ERROR) {
            return FATAL_ERROR;
        }
    }
    
    // Perform ECHO to check if the device is still responsive
    if (echo_check() == FATAL_ERROR) {
        return FATAL_ERROR;
    }

    // Re-enable keyboard scanning
    if (enable_scanning() == FATAL_ERROR) {
        return FATAL_ERROR;
    }

    return 0;
}
