#include <stdint.h>

#include <efi/efi_misc.hpp>
#include <efi/efi.h>

#include <ldstdio.hpp>
#include <ldstdlib.hpp>

#include <loader/system_config.hpp>
#include <loader/pci_setup.hpp>

#include <acpi.hpp>

namespace {
    static constexpr uint8_t MCFG_Sig[4] = { 0x4D, 0x43, 0x46, 0x47 }; // "MCFG" in ASCII
}

EFI_PHYSICAL_ADDRESS Loader::locatePCI(const EFI_SYSTEM_CONFIGURATION* sysconfig) {
    if (sysconfig->ACPI_20 != nullptr) {
        Loader::printf(u"ACPI 2.0 FOUND AT: %llx\n\r", sysconfig->ACPI_20);
    }
    else if (sysconfig->ACPI_10 != nullptr) {
        Loader::printf(u"ACPI 1.0 FOUND AT: %llx\n\r", sysconfig->ACPI_10);
    }
    else {
        Loader::puts(u"LOADER PANIC: NO ACPI DATA FOUND\n\r");
        EFI::Terminate();
    }

    ACPI_RSDP *RSDP = reinterpret_cast<ACPI_RSDP*>(sysconfig->ACPI_20);
    ACPI_RSDT *RSDT = reinterpret_cast<ACPI_RSDT*>(static_cast<uint64_t>(RSDP->RsdtAddress));

    size_t entries_count = (RSDT->Header.Length - 
        (reinterpret_cast<uint8_t*>(&RSDT->Entry) - reinterpret_cast<uint8_t*>(RSDT))
    ) / sizeof(uint32_t);

    Loader::printf(u"RSDT AT: %llx (%llu entries)\n\r", RSDT, entries_count);    

    ACPI_MCFG* MCFG = nullptr;

    for (size_t i = 0; i < entries_count; ++i) {
        uint32_t* raw_sdth_ptr = &RSDT->Entry + i;
        ACPI_SDTH* SDTH = reinterpret_cast<ACPI_SDTH*>(static_cast<uint64_t>(*raw_sdth_ptr));
        if (Loader::memcmp(
            reinterpret_cast<const VOID*>(SDTH->Signature),
            reinterpret_cast<const VOID*>(MCFG_Sig),
            4
        )) {
            MCFG = (ACPI_MCFG*)SDTH;
        }
    }

    if (MCFG != nullptr) {
        size_t mcfg_entries = (MCFG->Header.Length -
            (reinterpret_cast<uint8_t*>(&MCFG->Entry) - reinterpret_cast<uint8_t*>(MCFG))
        ) / sizeof(PCI_CSBA);
        Loader::printf(u"  MCFG AT: %llx (%llu entries)\n\r", MCFG, mcfg_entries);

        if (mcfg_entries == 0) {
            Loader::puts(u"LOADER PANIC: CORRUPTED/INVALID MCFG TABLE\n\r");
            EFI::Terminate();
        }
        else if (mcfg_entries > 1) {
            Loader::puts(u"LOADER WARNING: IGNORING ENTRIES IN MCFG TABLE\n\r");
        }

        // for (size_t i = 0; i < mcfg_entries; ++i) {
        //     PCI_CSBA* csba_ptr = &MCFG->Entry + i;
        //     PCI_CS* ECAM_CS = (PCI_CS*)csba_ptr->BaseAddress;

        //     if (csba_ptr->StartBusNumber != 0) {
        //         kputs(u"LOADER PANIC: CORRUPTED/INVALID MCFG TABLE\n\r");
        //         Terminate();
        //     }

        //     for (size_t bus = csba_ptr->StartBusNumber; bus <= csba_ptr->EndBusNumber; ++bus) {
        //         for (size_t device = 0; device < 32; ++device) {
        //             PCI_CS* device_ecam = (PCI_CS*)((uint8_t*)ECAM_CS + ((bus) << 20 | (device) << 15));
        //             if (device_ecam->VendorID != 0xffff) {
        //                 kprintf(u"Device found (%llx) (class=%u,subclass=%u,bus=%u,device=%u)\n\r",
        //                     device_ecam,
        //                     device_ecam->BaseClassCode,
        //                     device_ecam->SubclassCode,
        //                     bus,
        //                     device
        //                 );
        //                 if ((device_ecam->HeaderType & 0x80) != 0) {
        //                     for (size_t function = 0; function < 8; ++function) {
        //                         PCI_CS* function_ecam = (PCI_CS*)((uint8_t*)ECAM_CS + ((bus) << 20 | (device) << 15 | (function) << 12));
        //                         if (function_ecam->VendorID != 0xffff) {
        //                             kprintf(u"  Function found (%llx) (class=%u,subclass=%u,bus=%u,device=%u,function=%u)\n\r",
        //                                 function_ecam,
        //                                 function_ecam->BaseClassCode,
        //                                 function_ecam->SubclassCode,
        //                                 bus,
        //                                 device,
        //                                 function
        //                             );
        //                         }
        //                     }
        //                 }
        //             }
        //         }
        //     }
        // }
    }
    else {
        Loader::puts(u"LOADER PANIC: COULD NOT LOCATE PCI MCFG TABLE");
        EFI::Terminate();
    }

    return reinterpret_cast<EFI_PHYSICAL_ADDRESS>(reinterpret_cast<PCI_CSBA*>(&MCFG->Entry)->BaseAddress);
}
