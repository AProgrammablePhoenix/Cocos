#pragma once

#include <efi/efi_datatypes.h>

struct EFI_SYSTEM_CONFIGURATION {
    VOID    *ACPI_10;
    VOID    *ACPI_20;
    VOID    *SAL_SYSTEM;
    VOID    *SMBIOS;
    VOID    *SMBIOS3;
    VOID    *MPS;
    VOID    *JSON_CONFIG_DATA;
    VOID    *JSON_CAPSULE_DATA;
    VOID    *JSON_CAPSULE_RESULT;
    VOID    *DTB;
    VOID    *RT_PROPERTIES;
    VOID    *MEMORY_ATTRIBUTES;
    VOID    *CONFORMANCE_PROFILES;
    VOID    *MEMORY_RANGE_CAPSULE;
    VOID    *DEBUG_IMAGE_INFO;
    VOID    *SYSTEM_RESOURCE;
    VOID    *IMAGE_SECURITY_DATABASE;
};

namespace Loader {
    void detectSystemConfiguration(EFI_SYSTEM_CONFIGURATION* sysconfig);
}