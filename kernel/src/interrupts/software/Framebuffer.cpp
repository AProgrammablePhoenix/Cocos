#include <cstddef>
#include <cstdint>

#include <interrupts/Software.hpp>
#include <screen/Framebuffer.hpp>

namespace {
    extern "C" int32_t swFramebufferRequest(Interrupts::Software::FramebufferInfo* infoPtr, uint64_t CS) {
        // Perform SECPM check
        // Perform arguments check

        Framebuffer::Info info = Framebuffer::RequestInfo();

        infoPtr->XResolution = info.XResolution;
        infoPtr->YResolution = info.YResolution;
        infoPtr->PixelsPerScanLine = info.PixelsPerScanLine;

        return Interrupts::Software::Status::OK;
    }

    extern "C" int32_t swFramebufferWrite(void* fbPtr, uint64_t CS) {
        // Perform SECPM check
        // Perform arguments check

        Framebuffer::Info info = Framebuffer::RequestInfo();

        for (size_t i = 0; i < info.Size / sizeof(uint64_t); ++i) {
            *(reinterpret_cast<uint64_t*>(info.Address) + i) = *(reinterpret_cast<uint64_t*>(fbPtr) + i);
        }

        return Interrupts::Software::Status::OK;
    }
}
