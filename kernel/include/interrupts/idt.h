#pragma once

void kernel_idt_load(void);
void kernel_idt_setup(void);
void register_irq(unsigned int irq_line, void(*handler)(void), unsigned is_trap);
