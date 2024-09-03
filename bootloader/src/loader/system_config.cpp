#include <stdint.h>

#include <efi/efi_misc.hpp>
#include <efi/efi.h>

#include <loader/system_config.hpp>

namespace {
    static constexpr EFI_GUID ACPI_TABLE_GUID = {
        .Data1 = 0xEB9D2D30,
        .Data2 = 0x2D88,
        .Data3 = 0x11D3,
        .Data4 = { 0x9A, 0x16, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0x4D }
    };
    static constexpr EFI_GUID EFI_ACPI_20_TABLE_GUID = {
        .Data1 = 0x8868E871,
        .Data2 = 0xE4F1,
        .Data3 = 0x11D3,
        .Data4 = { 0xBC, 0x22, 0x00, 0x80, 0xC7, 0x3C, 0x88, 0x81 }
    };
    static constexpr EFI_GUID SAL_SYSTEM_TABLE_GUID = {
        .Data1 = 0xEB9D2D32,
        .Data2 = 0x2D88,
        .Data3 = 0x11D3,
        .Data4 = { 0x9A, 0x16, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0x4D }
    };
    static constexpr EFI_GUID SMBIOS_TABLE_GUID = {
        .Data1 = 0xEB9D2D31,
        .Data2 = 0x2D88,
        .Data3 = 0x11D3,
        .Data4 = { 0x9A, 0x16, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0x4D }
    };
    static constexpr EFI_GUID SMBIOS3_TABLE_GUID = {
        .Data1 = 0xF2FD1544,
        .Data2 = 0x9794,
        .Data3 = 0x4A2C,
        .Data4 = { 0x99, 0x2E, 0xE5, 0xBB, 0xCF, 0x20, 0xE3, 0x94 }
    };
    static constexpr EFI_GUID MPS_TABLE_GUID = {
        .Data1 = 0xEB9D2D2f,
        .Data2 = 0x2D88,
        .Data3 = 0x11D3,
        .Data4 = { 0x9A, 0x16, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0x4D }
    };
    static constexpr EFI_GUID EFI_JSON_CONFIG_DATA_TABLE_GUID = {
        .Data1 = 0x87367F87,
        .Data2 = 0x1119,
        .Data3 = 0x41CE,
        .Data4 = { 0xAA, 0xEC, 0x8B, 0xE0, 0x11, 0x1F, 0x55, 0x8A }
    };
    static constexpr EFI_GUID EFI_JSON_CAPSULE_DATA_TABLE_GUID = {
        .Data1 = 0x35E7A725,
        .Data2 = 0x8DD2,
        .Data3 = 0x4CAC,
        .Data4 = { 0x80, 0x11, 0x33, 0xCD, 0xA8, 0x10, 0x90, 0x56 }
    };
    static constexpr EFI_GUID EFI_JSON_CAPSULE_RESULT_TABLE_GUID = {
        .Data1 = 0xDBC461C3,
        .Data2 = 0xB3DE,
        .Data3 = 0x422A,
        .Data4 = { 0xB9, 0xB4, 0x98, 0x86, 0xFD, 0x49, 0xA1, 0xE5 }
    };
    static constexpr EFI_GUID EFI_DTB_TABLE_GUID = {
        .Data1 = 0xB1B621D5,
        .Data2 = 0xF19C,
        .Data3 = 0x41A5,
        .Data4 = { 0x83, 0x0B, 0xD9, 0x15, 0x2C, 0x69, 0xAA, 0xE0 }
    };
    static constexpr EFI_GUID EFI_RT_PROPERTIES_TABLE_GUID = {
        .Data1 = 0xEB66918A,
        .Data2 = 0x7EEF,
        .Data3 = 0x402A,
        .Data4 = { 0x84, 0x2E, 0x93, 0x1D, 0x21, 0xC3, 0x8A, 0xE9 }
    };
    static constexpr EFI_GUID EFI_MEMORY_ATTRIBUTES_TABLE_GUID = {
        .Data1 = 0xDCFA911D,
        .Data2 = 0x26EB,
        .Data3 = 0x469F,
        .Data4 = { 0xA2, 0x20, 0x38, 0xB7, 0xDC, 0x46, 0x12, 0x20 }
    };
    static constexpr EFI_GUID EFI_CONFORMANCE_PROFILES_TABLE_GUID = {
        .Data1 = 0x36122546,
        .Data2 = 0xF7E7,
        .Data3 = 0x4C8F,
        .Data4 = { 0xBD, 0x9B, 0xEB, 0x85, 0x25, 0xB5, 0x0C, 0x0B }
    };
    static constexpr EFI_GUID EFI_MEMORY_RANGE_CAPSULE_GUID = {
        .Data1 = 0xDE9F0EC,
        .Data2 = 0x88B6,
        .Data3 = 0x428F,
        .Data4 = { 0x97, 0x7A, 0x25, 0x8F, 0x1D, 0x0E, 0x5E, 0x72 }
    };
    static constexpr EFI_GUID EFI_DEBUG_IMAGE_INFO_TABLE_GUID = {
        .Data1 = 0x49152E77,
        .Data2 = 0x1ADA,
        .Data3 = 0x4764,
        .Data4 = { 0xB7, 0xA2, 0x7A, 0xFE, 0xFE, 0xD9, 0x5E, 0x8B }
    };
    static constexpr EFI_GUID EFI_SYSTEM_RESOURCE_TABLE_GUID = {
        .Data1 = 0xB122A263,
        .Data2 = 0x3661,
        .Data3 = 0x4F68,
        .Data4 = { 0x99, 0x29, 0x78, 0xF8, 0xB0, 0xD6, 0x21, 0x80 }
    };
    static constexpr EFI_GUID EFI_IMAGE_SECURITY_DATABASE_GUID = {
        .Data1 = 0xD719B2CB,
        .Data2 = 0x3D3A,
        .Data3 = 0x4596,
        .Data4 = { 0xA3, 0xBC, 0xDA, 0xD0, 0x0E, 0x67, 0x65, 0x6F }
    };
}

void Loader::detectSystemConfiguration(EFI_SYSTEM_CONFIGURATION* sysconfig) {
    EFI::sys->BootServices->SetMem(sysconfig, sizeof(EFI_SYSTEM_CONFIGURATION), 0);

    for (size_t i = 0; i < EFI::sys->NumberOfTableEntries; ++i) {
        EFI_CONFIGURATION_TABLE* cfg_table = EFI::sys->ConfigurationTable + i;
        const EFI_GUID* vendor_guid = &cfg_table->VendorGuid;
        VOID* vendor_table = cfg_table->VendorTable;
        
        if (Loader::guidcmp(vendor_guid, &ACPI_TABLE_GUID)) {
            sysconfig->ACPI_10 = vendor_table;
        }
        else if (Loader::guidcmp(vendor_guid, &EFI_ACPI_20_TABLE_GUID)) {
            sysconfig->ACPI_20 = vendor_table;
        }
        else if (Loader::guidcmp(vendor_guid, &SAL_SYSTEM_TABLE_GUID)) {
            sysconfig->SAL_SYSTEM = vendor_table;
        }
        else if (Loader::guidcmp(vendor_guid, &SMBIOS_TABLE_GUID)) {
            sysconfig->SMBIOS = vendor_table;
        }
        else if (Loader::guidcmp(vendor_guid, &SMBIOS3_TABLE_GUID)) {
            sysconfig->SMBIOS3 = vendor_table;
        }
        else if (Loader::guidcmp(vendor_guid, &MPS_TABLE_GUID)) {
            sysconfig->MPS = vendor_table;
        }
        else if (Loader::guidcmp(vendor_guid, &EFI_JSON_CONFIG_DATA_TABLE_GUID)) {
            sysconfig->JSON_CONFIG_DATA = vendor_table;
        }
        else if (Loader::guidcmp(vendor_guid, &EFI_JSON_CAPSULE_DATA_TABLE_GUID)) {
            sysconfig->JSON_CAPSULE_DATA = vendor_table;
        }
        else if (Loader::guidcmp(vendor_guid, &EFI_JSON_CAPSULE_RESULT_TABLE_GUID)) {
            sysconfig->JSON_CAPSULE_RESULT = vendor_table;
        }
        else if (Loader::guidcmp(vendor_guid, &EFI_DTB_TABLE_GUID)) {
            sysconfig->DTB = vendor_table;
        }
        else if (Loader::guidcmp(vendor_guid, &EFI_RT_PROPERTIES_TABLE_GUID)) {
            sysconfig->RT_PROPERTIES = vendor_table;
        }
        else if (Loader::guidcmp(vendor_guid, &EFI_MEMORY_ATTRIBUTES_TABLE_GUID)) {
            sysconfig->MEMORY_ATTRIBUTES = vendor_table;
        }
        else if (Loader::guidcmp(vendor_guid, &EFI_CONFORMANCE_PROFILES_TABLE_GUID)) {
            sysconfig->CONFORMANCE_PROFILES = vendor_table;
        }
        else if (Loader::guidcmp(vendor_guid, &EFI_MEMORY_RANGE_CAPSULE_GUID)) {
            sysconfig->MEMORY_RANGE_CAPSULE = vendor_table;
        }
        else if (Loader::guidcmp(vendor_guid, &EFI_DEBUG_IMAGE_INFO_TABLE_GUID)) {
            sysconfig->DEBUG_IMAGE_INFO = vendor_table;
        }
        else if (Loader::guidcmp(vendor_guid, &EFI_SYSTEM_RESOURCE_TABLE_GUID)) {
            sysconfig->SYSTEM_RESOURCE = vendor_table;
        }
        else if (Loader::guidcmp(vendor_guid, &EFI_IMAGE_SECURITY_DATABASE_GUID)) {
            sysconfig->IMAGE_SECURITY_DATABASE = vendor_table;
        }
    }
}