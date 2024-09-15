#include <cstddef>
#include <cstdint>

#include <mm/VirtualMemoryLayout.hpp>
#include <screen/Framebuffer.hpp>

namespace {
    static Framebuffer::Info info;
}

namespace Framebuffer {
    Info Setup() {
        const uint64_t mmap_size = *reinterpret_cast<uint64_t*>(
            VirtualMemoryLayout::OS_BOOT_DATA + VirtualMemoryLayout::BOOT_MEMORY_MAP_SIZE_OFFSET
        );

        info.Size = *reinterpret_cast<uint64_t*>(
            VirtualMemoryLayout::OS_BOOT_DATA + mmap_size + VirtualMemoryLayout::BOOT_FRAMEBUFFER_SIZE_OFFSET
        );
        info.Address = *reinterpret_cast<uint32_t**>(
            VirtualMemoryLayout::OS_BOOT_DATA + mmap_size + VirtualMemoryLayout::BOOT_FRAMEBUFFER_ADDRESS_OFFSET
        );

		info.XResolution = *reinterpret_cast<uint32_t*>(
            VirtualMemoryLayout::OS_BOOT_DATA + mmap_size + VirtualMemoryLayout::BOOT_FRAMEBUFFER_XRES_OFFSET
        );
		info.YResolution = *reinterpret_cast<uint32_t*>(
            VirtualMemoryLayout::OS_BOOT_DATA + mmap_size + VirtualMemoryLayout::BOOT_FRAMEBUFFER_YRES_OFFSET
        );
		info.PixelsPerScanLine = *reinterpret_cast<uint32_t*>(
            VirtualMemoryLayout::OS_BOOT_DATA + mmap_size + VirtualMemoryLayout::BOOT_FRAMEBUFFER_PPSL_OFFSET
        );

        return info;
    }

    Info RequestInfo() {
        return info;
    }
}
