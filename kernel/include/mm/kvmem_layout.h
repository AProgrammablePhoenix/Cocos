#pragma once

#define DMA_ZONE                    0x0000000000000000

#define KERNEL_IMAGE                0xFFFF800000000000
#define KERNEL_HEAP                 0xFFFF800002000000
#define KERNEL_STACK                0xFFFF800021EFF000

#define EFI_RT_SVC                  0xFFFF800022000000
#define ACPI_DATA                   0xFFFF800026000000
#define EFI_MMIO                    0xFFFF800036000000

#define PHYSICAL_MEM_MAP            0xFFFF800436000000

#define EFI_GOP_FRAMEBUFFER         0xFFFF8004B6000000

#define OS_LOADER_INFO              0xFFFF8004B8000000
#define TTY_FONT                    0xFFFF8004B8000000
#define BOOT_DATA                   0xFFFF8004B8080000

#define PCI_CONFIG_SPACE_MAP        0xFFFF8004BA000000

#define MEMORY_HOLE_0               0xFFFF8004BC000000

#define VIRTUAL_MEM_MAP             0xFFFF808000000000
#define KERNEL_VMEM_MAP             0xFFFF808000000000
#define USER_VMEM_MAP               0xFFFF808000100000

#define MEMORY_HOLE_1               0xFFFF810000000000

#define RECURSIVE_MAPPING           0xFFFFFF0000000000

#define RT_PROCSS_DATA              0xFFFFFF8000000000


#define DMA_ZONE_SIZE               0x0000000001000000

#define KERNEL_IMAGE_SIZE           (KERNEL_HEAP - KERNEL_IMAGE)
#define KERNEL_HEAP_SIZE            (KERNEL_STACK - KERNEL_HEAP)
#define KERNEL_STACK_SIZE           (EFI_RT_SVC - KERNEL_STACK)

#define EFI_RT_SVC_SIZE             (ACPI_DATA - EFI_RT_SVC)
#define ACPI_DATA_SIZE              (EFI_MMIO - ACPI_DATA)
#define EFI_MMIO_SIZE               (PHYSICAL_MEM_MAP - EFI_MMIO)

#define PHYSICAL_MEM_MAP_SIZE       (EFI_GOP_FRAMEBUFFER - PHYSICAL_MEM_MAP)

#define EFI_GOP_FRAMEBUFFER_SIZE    (OS_LOADER_INFO - EFI_GOP_FRAMEBUFFER)

#define OS_LOADER_INFO_SIZE         (MEMORY_HOLE_0 - OS_LOADER_INFO)
#define TTY_FONT_SIZE               (BOOT_DATA - TTY_FONT)
#define BOOT_DATA_SIZE              (PCI_CONFIG_SPACE_MAP - BOOT_DATA)

#define PCI_CONFIG_SPACE_MAP_SIZE   (MEMORY_HOLE_0 - PCI_CONFIG_SPACE_MAP)

#define VIRTUAL_MEM_MAP_SIZE        (MEMORY_HOLE_1 - VIRTUAL_MEM_MAP)
#define KERNEL_VMEM_MAP_SIZE        (USER_VMEM_MAP - KERNEL_VMEM_MAP)
#define USER_VMEM_MAP_SIZE          (MEMORY_HOLE_1 - USER_VMEM_MAP)