#include <cstddef>
#include <cstdint>

#include <mm/VirtualMemoryLayout.hpp>
#include <screen/Log.hpp>

namespace {
	static constexpr size_t GLYPH_WIDTH			= 8;
	static constexpr size_t GLYPH_HEIGHT		= 16;
	static constexpr size_t GLYPH_COLUMN_MASK	= 0x80;

	static struct {
		uint32_t width;
		uint32_t height;
		uint32_t ppsl;
		uint32_t* framebuffer;
	} screenInfo;

	static struct {
		uint32_t x;
		uint32_t y;

		uint32_t actualRows;        // number of characters per column given the font
		uint32_t actualColumns;     // number of characters per row given the font

		uint32_t currentRow;
		uint32_t currentColumn;

		uint32_t foreground;
		uint32_t background;
	} screenContext;

	struct PSF1_HEADER {
		uint16_t magic;
		uint8_t fontMode;
		uint8_t glyphSize;
	};
	static const PSF1_HEADER* const FONT_HEADER = reinterpret_cast<PSF1_HEADER*>(VirtualMemoryLayout::OS_LOADER_PSF_FONT);

	static inline constexpr uint32_t* getPixelAddress(uint32_t x, uint32_t y) {
		return &screenInfo.framebuffer[y * screenInfo.ppsl + x];
	}

	static inline constexpr uint32_t getFontPixel(unsigned int codepoint, uint32_t row, uint32_t column) {
		const uint8_t rowByte = *(reinterpret_cast<const uint8_t*>(FONT_HEADER + 1) + GLYPH_HEIGHT * codepoint + row);
		return rowByte & (GLYPH_COLUMN_MASK >> column);
	}

	static inline void scrollLine(uint32_t row) {
		for (size_t x = 0; x < screenInfo.width; ++x) {
			*getPixelAddress(x, row - GLYPH_HEIGHT) = *getPixelAddress(x, row);
		}
	}

	static inline void scroll() {
		for (size_t y = GLYPH_HEIGHT; y < screenInfo.height; ++y) {
			scrollLine(y);
		}
		for (size_t y = screenInfo.height - GLYPH_HEIGHT; y < screenInfo.height; ++y) {
			for (size_t x = 0; x < screenInfo.width; ++x) {
				*getPixelAddress(x, y) = screenContext.background;
			}
		}
	}

	static inline void carriageReturn() {
		screenContext.currentColumn = 0;
		screenContext.x = 0;
	}

	static inline void lineFeed() {
		if (screenContext.currentRow < screenContext.actualRows - 1) {
			++screenContext.currentRow;
			screenContext.y += GLYPH_HEIGHT;
		}
		else {
			scroll();
		}
	}
}

namespace Log {
	void Setup() {
		const uint64_t mmap_size = *reinterpret_cast<uint64_t*>(VirtualMemoryLayout::OS_BOOT_DATA + VirtualMemoryLayout::BOOT_MEMORY_MAP_SIZE_OFFSET);

		screenInfo.width = *reinterpret_cast<uint32_t*>(VirtualMemoryLayout::OS_BOOT_DATA + mmap_size + VirtualMemoryLayout::BOOT_FRAMEBUFFER_XRES_OFFSET);
		screenInfo.height = *reinterpret_cast<uint32_t*>(VirtualMemoryLayout::OS_BOOT_DATA + mmap_size + VirtualMemoryLayout::BOOT_FRAMEBUFFER_YRES_OFFSET);
		screenInfo.ppsl = *reinterpret_cast<uint32_t*>(VirtualMemoryLayout::OS_BOOT_DATA + mmap_size + VirtualMemoryLayout::BOOT_FRAMEBUFFER_PPSL_OFFSET);
		screenInfo.framebuffer = *reinterpret_cast<uint32_t**>(VirtualMemoryLayout::OS_BOOT_DATA + mmap_size + VirtualMemoryLayout::BOOT_FRAMEBUFFER_ADDRESS_OFFSET);

		screenContext.x = 0;
		screenContext.y = 0;
		screenContext.actualColumns = screenInfo.width / GLYPH_WIDTH;
		screenContext.actualRows = screenInfo.height / GLYPH_HEIGHT;
		screenContext.currentColumn = 0;
		screenContext.currentRow = 0;
		screenContext.foreground = 0x00FFFFFF;
		screenContext.background = 0x00000000;
	}

	void putcAt(char c, uint32_t x, uint32_t y) {
		unsigned int codepoint = static_cast<unsigned char>(c);
		if (codepoint >= 0x80 || codepoint < 0x20) {
			codepoint = 0x20;
		}

		for (size_t r = 0; r < GLYPH_HEIGHT; ++r) {
			for (size_t c = 0; c < GLYPH_WIDTH; ++c) {
				*getPixelAddress(x + c, y + r) = getFontPixel(codepoint, r, c) ? screenContext.foreground : screenContext.background;
			}
		}
	}

	void putc(char c) {
		if (c == '\0' || c == '\a' || c == '\f') {
			return;
		}
		else if (c == '\b') {
			if (screenContext.currentRow == 0 && screenContext.currentColumn == 0) {
				return;
			}
			else if (screenContext.currentColumn == 0) {
				--screenContext.currentRow;
				screenContext.currentColumn = screenContext.actualColumns - 1;

				uint32_t temp_y = screenContext.y -= GLYPH_HEIGHT;
				uint32_t temp_x = screenContext.x = screenInfo.width - GLYPH_HEIGHT;

				putcAt('\b', temp_x, temp_y);
			}
			else {
				--screenContext.currentColumn;
				
				uint32_t temp_x = screenContext.x -= GLYPH_WIDTH;

				putcAt('\b', temp_x, screenContext.y);
			}
		}
		else if (c == '\t') {
			const uint32_t shift = 4 - (screenContext.currentColumn % 4);
			screenContext.currentColumn += shift;

			if (screenContext.currentColumn >= screenContext.actualColumns) {
				carriageReturn();
				lineFeed();
			}
			else {
				screenContext.x += shift * GLYPH_WIDTH;
			}
		}
		else if (c == '\n' || c == '\v') {
			lineFeed();
		}
		else if (c == '\r') {
			carriageReturn();
		}
		else {
			putcAt(c, screenContext.x, screenContext.y);
			screenContext.x += GLYPH_WIDTH;

			if (++screenContext.currentColumn >= screenContext.actualColumns) {
				carriageReturn();
				lineFeed();
			}
		}
	}

	void puts(const char* s) {
		while (*s != '\0') {
			putc(*s++);
		}
	}
}
