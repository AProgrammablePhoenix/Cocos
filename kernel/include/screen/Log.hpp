#pragma once

#include <cstdint>

namespace Log {
	void Setup();
	void putcAt(char c, uint32_t x, uint32_t y);
	void putc(char c);
	void puts(const char* s);
}
