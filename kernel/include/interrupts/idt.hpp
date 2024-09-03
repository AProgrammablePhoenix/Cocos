#pragma once

namespace Interrupts {
	extern "C" void kernel_idt_load(void);
	extern "C" void kernel_idt_setup(void);
	extern "C" void register_irq(unsigned int irqLine, void(*handler)(void), unsigned int isTrap);
}
