#include <stdint.h>

// basic and non-optimized code (no need for the loader to use more optimized versions)

void* memcpy(void *restrict dest, const void *restrict src, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        *((uint8_t*)dest + i) = *((uint8_t*)src + i);
    }

    return dest;
}

void* memset(void* dest, int ch, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        *((uint8_t*)dest + i) = (uint8_t)ch;
    }

    return dest;
}
