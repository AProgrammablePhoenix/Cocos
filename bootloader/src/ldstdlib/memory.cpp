#include <stdint.h>

#include <ldstdlib.hpp>

// basic and non-optimized code (no need for the loader to use more optimized versions)

int Loader::memcmp(const VOID* _buf1, const VOID* _buf2, UINTN size) {
    for (size_t i = 0; i < size; ++i) {
        if (*(reinterpret_cast<const uint8_t*>(_buf1) + i) != *(reinterpret_cast<const uint8_t*>(_buf2) + i)) {
            return 0;
        }
    }

    return 1;
}

void* Loader::memcpy(void* restrict dest, const void* restrict src, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        *(reinterpret_cast<uint8_t*>(dest) + i) = *(reinterpret_cast<const uint8_t*>(src) + i);
    }

    return dest;
}

void* Loader::memset(void* dest, int ch, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        *(reinterpret_cast<uint8_t*>(dest) + i) = static_cast<uint8_t>(ch);
    }

    return dest;
}
