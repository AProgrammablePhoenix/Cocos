#pragma once

#include <cstddef>
#include <cstdint>

namespace Framebuffer {
    struct Info {
        uint64_t Size;
        uint32_t* Address;
        uint32_t XResolution;
        uint32_t YResolution;
        uint32_t PixelsPerScanLine;        
    };

    Info Setup();
    Info RequestInfo();
}
