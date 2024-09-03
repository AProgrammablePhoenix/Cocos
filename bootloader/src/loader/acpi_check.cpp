#include <stdint.h>

#include <efi/efi_misc.hpp>
#include <efi/efi.h>
#include <acpi.hpp>

#include <ldstdio.hpp>
#include <ldstdlib.hpp>

#include <loader/acpi_check.hpp>
#include <loader/loader_info.hpp>
#include <loader/paging.hpp>
#include <loader/system_config.hpp>

namespace {
    extern inline int check_table(
        EFI_PHYSICAL_ADDRESS address,
        size_t size,
        const EfiMemoryMap* mmap,
        uint64_t* error_vector
    ) {
        size_t desc_num = mmap->mmap_size / mmap->desc_size;
        for (size_t i = 0; i < desc_num; ++i) {
            const EFI_MEMORY_DESCRIPTOR* descriptor = reinterpret_cast<const EFI_MEMORY_DESCRIPTOR*>(
                reinterpret_cast<const uint8_t*>(mmap->mmap) + i * mmap->desc_size
            );
            if (address >= descriptor->PhysicalStart &&
                address + size < descriptor->PhysicalStart + PAGE_SIZE * descriptor->NumberOfPages
            ) {
                // checks if the table is in a reserved area of memory
                if (descriptor->Type == EfiReservedMemoryType ||
                    descriptor->Type == EfiRuntimeServicesCode ||
                    descriptor->Type == EfiRuntimeServicesData ||
                    descriptor->Type == EfiACPIReclaimMemory ||
                    descriptor->Type == EfiACPIMemoryNVS ||
                    descriptor->Type == EfiMemoryMappedIO ||
                    descriptor->Type == EfiMemoryMappedIOPortSpace ||
                    descriptor->Type == EfiPalCode
                ) {
                    return 0;
                }
                else {
                    *error_vector = descriptor->Type;
                    return -1;
                }
            }
        }

        return -2;
    }

    extern inline void _acpi_mem_check_handler(
        const void* _table,
        const CHAR16* _table_name,
        size_t _table_size,
        const EfiMemoryMap* _mmap
    ) {
        uint64_t error_vector = 0;
        int code = check_table(reinterpret_cast<EFI_PHYSICAL_ADDRESS>(_table), _table_size, _mmap, &error_vector);
        if (code == -1) {
            Loader::puts(u"ACPI Table(");
            Loader::puts(_table_name);
            Loader::puts(u") located in an invalid/unsupported memory region.\n\r");
            Loader::printf(u"Located at 0x%.16llx (memory type 0x%.16llx)\n\r", _table, error_vector);
            Loader::puts(u"Aborting booting process...\n\r");
            EFI::Terminate();
        }
        else if (code == -2) {
            Loader::puts(u"Detected corrupted system memory map while checking ACPI Tables memory regions.\n\r");
            Loader::puts(u"ACPI Table(");
            Loader::puts(_table_name);
            Loader::printf(u") was located at 0x%.16llx and 0x%.16llx bytes long.\n\r", _table, _table_size);
            Loader::puts(u"Aborting booting process...\n\r");
            EFI::Terminate();
        }
    }

    #define _acpi_named_mem_check(TABLE, SIZE, MMAP) do { _acpi_mem_check_handler(TABLE,  u###TABLE, SIZE, MMAP); } while (0)

    extern inline void _acpi_unnamed_mem_check(
        const void* _table,
        const char __table_name[4],
        size_t _table_size,
        const EfiMemoryMap* _mmap
    ) {
        CHAR16 _table_name[4];
        for (size_t i = 0; i < 4; ++i) {
            _table_name[i] = static_cast<CHAR16>(__table_name[i]);
        }
        _acpi_mem_check_handler(_table, _table_name, _table_size, _mmap);
    }

    static constexpr uint8_t FADT_Sig[4] = { 0x46, 0x41, 0x43, 0x50 };
}

// makes sure ACPI tables are in ACPI Reclaim memory
void Loader::check_acpi(const EFI_SYSTEM_CONFIGURATION* sysconfig, const EfiMemoryMap* mmap) {
    if (sysconfig->ACPI_20 != 0) {
        Loader::puts(u"Testing ACPI 2.0+ memory regions...\n\r");

        ACPI_RSDP* RSDP = reinterpret_cast<ACPI_RSDP*>(sysconfig->ACPI_20);
        _acpi_named_mem_check(RSDP, RSDP->Length, mmap);

        ACPI_XSDT* XSDT = reinterpret_cast<ACPI_XSDT*>(RSDP->XsdtAddress);
        _acpi_named_mem_check(XSDT, XSDT->Header.Length, mmap);

        size_t entries_count = (XSDT->Header.Length - 
            (reinterpret_cast<uint8_t*>(&XSDT->Entry) - reinterpret_cast<uint8_t*>(XSDT))
        ) / sizeof(uint64_t);

        for (size_t i = 0; i < entries_count; ++i) {
            uint64_t* raw_sdth_ptr = &XSDT->Entry + i;
            ACPI_SDTH* SDTH = reinterpret_cast<ACPI_SDTH*>(*raw_sdth_ptr);
            _acpi_unnamed_mem_check(SDTH, reinterpret_cast<const char*>(SDTH->Signature), SDTH->Length, mmap);

            if (Loader::memcmp(
                reinterpret_cast<const VOID*>(SDTH->Signature),
                reinterpret_cast<const VOID*>(FADT_Sig),
                4)
            ) {
                ACPI_FADT* FADT = reinterpret_cast<ACPI_FADT*>(SDTH);
                if (FADT->X_DSDT != 0) {
                    ACPI_SDTH* DSDT = reinterpret_cast<ACPI_SDTH*>(FADT->X_DSDT);
                    _acpi_unnamed_mem_check(
                        DSDT,
                        reinterpret_cast<const char*>(DSDT->Signature),
                        DSDT->Length,
                        mmap
                    );
                }
                else {
                    ACPI_SDTH* DSDT = reinterpret_cast<ACPI_SDTH*>(static_cast<uint64_t>(FADT->DSDT));
                    _acpi_unnamed_mem_check(
                        DSDT,
                        reinterpret_cast<const char*>(DSDT->Signature),
                        DSDT->Length,
                        mmap
                    );
                }
            }
        }

        Loader::puts(u"    Passed.\n\r");
    }
    else {
        Loader::puts(u"Testing ACPI 1.0 memory regions...\n\r");

        ACPI_RSDP* RSDP = reinterpret_cast<ACPI_RSDP*>(sysconfig->ACPI_10);
        _acpi_named_mem_check(RSDP, LEGACY_RSDP_SIZE, mmap);

        ACPI_RSDT* RSDT = reinterpret_cast<ACPI_RSDT*>(static_cast<uint64_t>(RSDP->RsdtAddress));
        _acpi_named_mem_check(RSDT, RSDT->Header.Length, mmap);

        size_t entries_count = (RSDT->Header.Length - 
            (reinterpret_cast<uint8_t*>(&RSDT->Entry) - reinterpret_cast<uint8_t*>(RSDT))
        ) / sizeof(uint32_t);

        for (size_t i = 0; i < entries_count; ++i) {
            uint32_t* raw_sdth_ptr = &RSDT->Entry + i;
            ACPI_SDTH* SDTH = reinterpret_cast<ACPI_SDTH*>(static_cast<uint64_t>(*raw_sdth_ptr));
            _acpi_unnamed_mem_check(
                SDTH,
                reinterpret_cast<const char*>(SDTH->Signature),
                SDTH->Length,
                mmap
            );

            if (Loader::memcmp(
                reinterpret_cast<const VOID*>(SDTH->Signature),
                reinterpret_cast<const VOID*>(FADT_Sig),
                4)
            ) {
                ACPI_FADT* FADT = reinterpret_cast<ACPI_FADT*>(SDTH);
                if (FADT->X_DSDT != 0) {
                    ACPI_SDTH* DSDT = (ACPI_SDTH*)FADT->X_DSDT;
                    _acpi_unnamed_mem_check(
                        DSDT,
                        reinterpret_cast<const char*>(DSDT->Signature),
                        DSDT->Length,
                        mmap
                    );
                }
                else {
                    ACPI_SDTH* DSDT = reinterpret_cast<ACPI_SDTH*>(static_cast<uint64_t>(FADT->DSDT));
                    _acpi_unnamed_mem_check(
                        DSDT,
                        reinterpret_cast<const char*>(DSDT->Signature),
                        DSDT->Length,
                        mmap
                    );
                }
            }
        }

        Loader::puts(u"Passed.\n\r");
    }
}
