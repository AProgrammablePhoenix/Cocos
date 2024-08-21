#pragma once

#include <stdint.h>

#pragma pack(push)
#pragma pack(1)

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t ppsl;
    unsigned char* framebuffer;
} tty_info_t;

#pragma pack(pop)

void tty_putc_at(char c, uint32_t x, uint32_t y);
void tty_setup(void);
void tty_putc(char c);
void tty_puts(const char* s);
