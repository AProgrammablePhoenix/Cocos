#include <stddef.h>
#include <stdint.h>

#include <mm/kvmem_layout.h>
#include <screen/tty.h>

#define GLYPH_WIDTH         8
#define GLYPH_HEIGHT        16
#define GLYPH_COLUMN_MASK   0x80

#define PSF1_MODE512        0x01
#define PSF1_MODEHASTAB     0x02
#define PSF1_MODESEQ        0x04

#define FRAMEBUFFER         ((uint32_t*)EFI_GOP_FRAMEBUFFER)

const struct {
    uint16_t magic;
    uint8_t font_mode;
    uint8_t glyph_size;
} *const PSF1_HEADER = (void*)TTY_FONT;

static tty_info_t info;
static struct {
    uint32_t x;
    uint32_t y;

    uint32_t actualRows;        // number of characters per column given the font
    uint32_t actualColumns;     // number of characters per row given the font

    uint32_t currentRow;
    uint32_t currentColumn;

    uint32_t foreground;
    uint32_t background;
} tty_context;

void tty_setup(void) {    
    const uint8_t* linfo = (uint8_t*)BOOT_DATA;
    const size_t mmap_size = *(uint64_t*)linfo;
    linfo += 2 * sizeof(uint64_t)
        + mmap_size
        + sizeof(uint8_t*)
        + 3 * sizeof(uint64_t)
        + 512 * sizeof(uint8_t);
    
    info.width = *(uint32_t*)linfo;
    linfo += sizeof(uint32_t);
    info.height = *(uint32_t*)linfo;
    linfo += sizeof(uint32_t);
    info.ppsl = *(uint32_t*)linfo;
    linfo += 2 * sizeof(uint32_t);
    info.framebuffer = *(unsigned char**)linfo;

    tty_context.x = 0;
    tty_context.y = 0;
    tty_context.actualColumns = info.width / GLYPH_WIDTH;
    tty_context.actualRows = info.height / GLYPH_HEIGHT;
    tty_context.currentColumn = 0;
    tty_context.currentRow = 0;
    tty_context.foreground = 0x00FFFFFF;
    tty_context.background = 0x00000000;
}

static inline uint32_t* get_fb_addr(uint32_t x, uint32_t y) {
    return &FRAMEBUFFER[y * info.ppsl + x];
}

static inline uint32_t get_font_pixel(unsigned int codepoint, uint32_t row, uint32_t column) {
    const uint8_t row_byte = *((uint8_t*)(PSF1_HEADER + 1) + GLYPH_HEIGHT * codepoint + row);
    return row_byte & (GLYPH_COLUMN_MASK >> column);
}

void tty_putc_at(char c, uint32_t x, uint32_t y) {
    unsigned int codepoint = (unsigned char)c;
    if (codepoint >= 0x80 || codepoint < 0x20) {
        codepoint = 0x20;
    }

    for (size_t i = 0; i < GLYPH_HEIGHT; ++i) {
        for (size_t j = 0; j < GLYPH_WIDTH; ++j) {
            *get_fb_addr(x + j, y + i) = get_font_pixel(codepoint, i, j) ? tty_context.foreground : tty_context.background;
        }
    }
}

static inline void scroll_tty_line(uint32_t row) {
    for (size_t x = 0; x < info.width; ++x) {
        *get_fb_addr(x, row - GLYPH_HEIGHT) = *get_fb_addr(x, row);
    }
}

static void tty_scroll(void) {
    for (size_t y = GLYPH_HEIGHT; y < info.height; ++y) {
        scroll_tty_line(y);
    }
    for (size_t y = info.height - GLYPH_HEIGHT; y < info.height; ++y) {
        for (size_t x = 0; x < info.width; ++x) {
            *get_fb_addr(x, y) = tty_context.background;
        }
    }
}

static inline void tty_carriagereturn() {
    tty_context.currentColumn = 0;
    tty_context.x = 0;
}

static inline void tty_linefeed() {
    if (tty_context.currentRow < tty_context.actualRows - 1) {
        ++tty_context.currentRow;
        tty_context.y += GLYPH_HEIGHT;
    }
    else {
        tty_scroll();
    }
}

void tty_putc(char c) {
    if (c == '\0' || c== '\a' || c== '\f') {
        return;
    }
    else if (c == '\b') {
        if (tty_context.currentRow == 0 && tty_context.currentColumn == 0) {
            return;
        }
        else if (tty_context.currentColumn == 0) {
            --tty_context.currentRow;
            tty_context.currentColumn = tty_context.actualColumns - 1;

            uint32_t temp_y = tty_context.y -= GLYPH_HEIGHT;
            uint32_t temp_x = tty_context.x = info.width - GLYPH_WIDTH;

            tty_putc_at(' ', temp_x, temp_y);
        }
        else {
            --tty_context.currentColumn;

            uint32_t temp_x = tty_context.x -= GLYPH_WIDTH;

            tty_putc_at('\b', temp_x, tty_context.y);
        }
    }
    else if (c == '\t') {
        const uint32_t shift = tty_context.currentColumn % 4;
        tty_context.currentColumn += 4 - shift;

        if (tty_context.currentColumn >= tty_context.actualColumns) {
            tty_context.currentColumn = 0;
            tty_context.x = 0;
            tty_linefeed();
        }
        else {
            tty_context.x += shift * GLYPH_WIDTH;
        }
    }
    else if (c == '\n' || c== '\v') {
        tty_linefeed();
    }
    else if (c == '\r') {
        tty_carriagereturn();
    }
    else {
        tty_putc_at(c, tty_context.x, tty_context.y);
        tty_context.x += GLYPH_WIDTH;

        if (++tty_context.currentColumn >= tty_context.actualColumns) {
            tty_carriagereturn();
            tty_linefeed();
        }
    }
}

void tty_puts(const char* s) {
    while (*s != '\0') {
        tty_putc(*s++);
    }
}
