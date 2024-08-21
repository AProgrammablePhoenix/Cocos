#include <stdint.h>

#include <efi/efi.h>
#include <efi/efi_misc.h>

#include <stdiok.h>

#include <loader/system_config.h>
#include <loader/pci_setup.h>

#include <acpi.h>

static const uint8_t MCFG_Sig[4] = { 0x4D, 0x43, 0x46, 0x47 }; // "MCFG" in ASCII

EFI_PHYSICAL_ADDRESS locatePCI(const EFI_SYSTEM_CONFIGURATION* sysconfig) {
    if (sysconfig->ACPI_20 != NULL) {
        kprintf(u"ACPI 2.0 FOUND AT: %llx\n\r", sysconfig->ACPI_20);
    }
    else if (sysconfig->ACPI_10 != NULL) {
        kprintf(u"ACPI 1.0 FOUND AT: %llx\n\r", sysconfig->ACPI_10);
    }
    else {
        kputs(u"LOADER PANIC: NO ACPI DATA FOUND\n\r");
        Terminate();
    }

    ACPI_RSDP *RSDP = (ACPI_RSDP*)sysconfig->ACPI_20;
    ACPI_RSDT *RSDT = (ACPI_RSDT*)(uint64_t)RSDP->RsdtAddress;

    size_t entries_count = (RSDT->Length - ((uint8_t*)&RSDT->Entry - (uint8_t*)RSDT)) / sizeof(uint32_t);

    kprintf(u"RSDT AT: %llx (%llu entries)\n\r", RSDT, entries_count);    

    ACPI_MCFG* MCFG = NULL;

    for (size_t i = 0; i < entries_count; ++i) {
        uint32_t* raw_sdth_ptr = &RSDT->Entry + i;
        ACPI_SDTH* SDTH = (ACPI_SDTH*)(uint64_t)*raw_sdth_ptr;
        if (kmemcmp((VOID*)SDTH->Signature, (VOID*)MCFG_Sig, 4)) {
            MCFG = (ACPI_MCFG*)SDTH;
        }
    }

    if (MCFG != NULL) {
        size_t mcfg_entries = (MCFG->Length - ((uint8_t*)&MCFG->Entry - (uint8_t*)MCFG)) / sizeof(PCI_CSBA);
        kprintf(u"  MCFG AT: %llx (%llu entries)\n\r", MCFG, mcfg_entries);

        if (mcfg_entries == 0) {
            kputs(u"LOADER PANIC: CORRUPTED/INVALID MCFG TABLE\n\r");
        }
        else if (mcfg_entries > 1) {
            kputs(u"LOADER WARNING: IGNORING ENTRIES IN MCFG TABLE\n\r");
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
        kputs(u"LOADER PANIC: COULD NOT LOCATE PCI MCFG TABLE");
        Terminate();
    }

    return (EFI_PHYSICAL_ADDRESS)(((PCI_CSBA*)&MCFG->Entry)->BaseAddress);
}
