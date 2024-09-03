#pragma once

#include <loader/loader_info.hpp>
#include <loader/system_config.hpp>

namespace Loader {
    void check_acpi(const EFI_SYSTEM_CONFIGURATION* sysconfig, const EfiMemoryMap* mmap);
}
