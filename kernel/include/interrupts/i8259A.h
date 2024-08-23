#pragma once

#include <stdint.h>

void initialize_pic(void);
void disable_pic(void);
void mask_irq(uint8_t irq_line);
void enable_irq(uint8_t irq_line);
