#pragma once

#include <loader/loader_info.h>
#include <loader/system_config.h>

void check_acpi(const EFI_SYSTEM_CONFIGURATION* sysconfig, const EfiMemoryMap* mmap);
