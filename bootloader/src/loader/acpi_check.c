#include <stdint.h>

#include <efi/efi.h>
#include <efi/efi_misc.h>
#include <acpi.h>

#include <stdiok.h>

#include <loader/loader_info.h>
#include <loader/paging_loader.h>
#include <loader/system_config.h>

extern inline int check_table(EFI_PHYSICAL_ADDRESS address, size_t size, const EfiMemoryMap* mmap, uint64_t* error_vector) {
    size_t desc_num = mmap->mmap_size / mmap->desc_size;
    for (size_t i = 0; i < desc_num; ++i) {
        const EFI_MEMORY_DESCRIPTOR* descriptor = (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)mmap->mmap + i * mmap->desc_size);
        if (address >= descriptor->PhysicalStart && address + size < descriptor->PhysicalStart + PAGE_SIZE * descriptor->NumberOfPages) {
            if (descriptor->Type == EfiACPIReclaimMemory) {
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

extern inline void _acpi_mem_check_handler(const void* _table, const CHAR16* _table_name, size_t _table_size, const EfiMemoryMap* _mmap) {
    uint64_t error_vector = 0;
    int code = check_table((EFI_PHYSICAL_ADDRESS)_table, _table_size, _mmap, &error_vector);
    if (code == -1) {
        kputs(u"ACPI Table(");
        kputs((CHAR16*)_table_name);
        kputs(u") located in an invalid/unsupported memory region.\n\r");
        kprintf(u"Located at 0x%#.16llx (memory type 0x%.16llx)\n\r", _table, error_vector);
        kputs(u"Aborting booting process...\n\r");
        Terminate();
    }
    else if (code == -2) {
        kputs(u"Detected corrupted system memory map while checking ACPI Tables memory regions.\n\r");
        kputs(u"Table ");
        kputs((CHAR16*)_table_name);
        kprintf(u" was located at 0x%.16llx and 0x%.16llx bytes long.\n\r", _table, _table_size);
        kputs(u"Aborting booting process...\n\r");
        Terminate();
    }
}

#define _acpi_named_mem_check(TABLE, SIZE, MMAP) do { _acpi_mem_check_handler(TABLE,  u###TABLE, SIZE, MMAP); } while (0)

extern inline void _acpi_unnamed_mem_check(const void* _table, const char __table_name[4], size_t _table_size, const EfiMemoryMap* _mmap) {
    CHAR16 _table_name[4];
    for (size_t i = 0; i < 4; ++i) {
        _table_name[i] = (CHAR16)__table_name[i];
    }
    _acpi_mem_check_handler(_table, _table_name, _table_size, _mmap);
}

static const uint8_t FADT_Sig[4] = { 0x46, 0x41, 0x43, 0x50 };

// makes sure ACPI tables are in ACPI Reclaim memory
// TODO: extend to make sure they are not in system memory (i.e., they are in reserved areas of memory)
void check_acpi(const EFI_SYSTEM_CONFIGURATION* sysconfig, const EfiMemoryMap* mmap) {
    if (sysconfig->ACPI_20 != 0) {
        kputs(u"Testing ACPI 2.0+ memory regions...\n\r");

        ACPI_RSDP* RSDP = (ACPI_RSDP*)sysconfig->ACPI_20;
        _acpi_named_mem_check(RSDP, RSDP->Length, mmap);

        ACPI_XSDT* XSDT = (ACPI_XSDT*)RSDP->XsdtAddress;
        _acpi_named_mem_check(XSDT, XSDT->Length, mmap);

        size_t entries_count = (XSDT->Length - ((uint8_t*)&XSDT->Entry - (uint8_t*)XSDT)) / sizeof(uint64_t);

        for (size_t i = 0; i < entries_count; ++i) {
            uint64_t* raw_sdth_ptr = &XSDT->Entry + i;
            ACPI_SDTH* SDTH = (ACPI_SDTH*)*raw_sdth_ptr;
            _acpi_unnamed_mem_check(SDTH, (const char*)SDTH->Signature, SDTH->Length, mmap);

            if (kmemcmp((VOID*)SDTH->Signature, (VOID*)FADT_Sig, 4)) {
                ACPI_FADT* FADT = (ACPI_FADT*)SDTH;
                if (FADT->X_DSDT != 0) {
                    ACPI_SDTH* DSDT = (ACPI_SDTH*)FADT->X_DSDT;
                    _acpi_unnamed_mem_check(DSDT, (const char*)DSDT->Signature, DSDT->Length, mmap);
                }
                else {
                    ACPI_SDTH* DSDT = (ACPI_SDTH*)(uint64_t)FADT->DSDT;
                    _acpi_unnamed_mem_check(DSDT, (const char*)DSDT->Signature, DSDT->Length, mmap);
                }
            }
        }

        kputs(u"    Passed.\n\r");
    }
    else {
        kputs(u"Testing ACPI 1.0 memory regions...\n\r");

        ACPI_RSDP* RSDP = (ACPI_RSDP*)sysconfig->ACPI_10;
        _acpi_named_mem_check(RSDP, LEGACY_RSDP_SIZE, mmap);

        ACPI_RSDT* RSDT = (ACPI_RSDT*)(uint64_t)RSDP->RsdtAddress;
        _acpi_named_mem_check(RSDT, RSDT->Length, mmap);

        size_t entries_count = (RSDT->Length - ((uint8_t*)&RSDT->Entry - (uint8_t*)RSDT)) / sizeof(uint32_t);

        for (size_t i = 0; i < entries_count; ++i) {
            uint32_t* raw_sdth_ptr = &RSDT->Entry + i;
            ACPI_SDTH* SDTH = (ACPI_SDTH*)(uint64_t)*raw_sdth_ptr;
            _acpi_unnamed_mem_check(SDTH, (const char*)SDTH->Signature, SDTH->Length, mmap);

            if (kmemcmp((VOID*)SDTH->Signature, (VOID*)FADT_Sig, 4)) {
                ACPI_FADT* FADT = (ACPI_FADT*)SDTH;
                if (FADT->X_DSDT != 0) {
                    ACPI_SDTH* DSDT = (ACPI_SDTH*)FADT->X_DSDT;
                    _acpi_unnamed_mem_check(DSDT, (const char*)DSDT->Signature, DSDT->Length, mmap);
                }
                else {
                    ACPI_SDTH* DSDT = (ACPI_SDTH*)(uint64_t)FADT->DSDT;
                    _acpi_unnamed_mem_check(DSDT, (const char*)DSDT->Signature, DSDT->Length, mmap);
                }
            }
        }

        kputs(u"Passed.\n\r");
    }
}
