#pragma once

#include <stdint.h>

#include <efi/efi.h>

typedef struct {
    uint32_t ResX;                      // horizontal resolution
    uint32_t ResY;                      // vertical resolution
    uint32_t PPSL;                      // pixels per scan line
    EFI_GRAPHICS_PIXEL_FORMAT PXFMT;    // pixel format
    uint32_t* FBADDR;                   // frame buffer address
    uint64_t FBSIZE;                    // frame buffer size
} BasicGraphics;

BasicGraphics loadGraphics();
