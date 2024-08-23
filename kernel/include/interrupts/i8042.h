#pragma once

#include <stdint.h>

uint32_t initialize_ps2_controller(void);
uint16_t identify_ps2_port_1(void);
uint32_t send_byte_ps2_port_1(uint8_t data);
uint32_t recv_byte_ps2_port_1();
