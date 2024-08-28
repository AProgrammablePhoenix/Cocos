#include <stdint.h>

#include <devices/PCI/pci.h>

#include <screen/tty.h>

void pci_tree_walk(void* _ecam_0_addr) {
    for (size_t bus = 0; bus < 256; ++bus) {
        for (size_t device = 0; device < 32; ++device) {
            PCI_CS* device_ecam = (PCI_CS*)((uint8_t*)_ecam_0_addr + ((bus) << 20 | (device) << 15));
            if (device_ecam->VendorID != 0xffff) {

            }
        }
    }
}
