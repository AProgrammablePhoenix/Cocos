#pragma once

#include <cstdint>

namespace Interrupts {
    namespace Software {
        namespace Status {
            static constexpr int32_t OK                  = 0;
            static constexpr int32_t SECPM_ERROR         = -1;
            static constexpr int32_t INVALID_ARGUMENT    = -2;
        }

        struct FramebufferInfo {
            uint64_t XResolution;
            uint64_t YResolution;
            uint64_t PixelsPerScanLine;
            uint64_t Reserved;
        };

        extern "C" void swFramebufferManager(void);
    }
}
