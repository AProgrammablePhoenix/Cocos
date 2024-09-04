#pragma once

namespace VirtualMemoryLayout {
	/// Locations of the different zones

	inline constexpr uint64_t DMA_ZONE					= 0x0000000000000000;

	inline constexpr uint64_t USER_MEMORY				= 0x0000000001000000;

	inline constexpr uint64_t KERNEL_IMAGE				= 0xFFFF800000000000;
	inline constexpr uint64_t KERNEL_HEAP				= 0xFFFF800020000000;

	inline constexpr uint64_t EFI_RUNTIME_SERVICES		= 0xFFFF800100000000;
	inline constexpr uint64_t EFI_GOP_FRAMEBUFFER		= 0xFFFF800104000000;
	inline constexpr uint64_t EFI_ACPI_NVS_DATA			= 0xFFFF800108000000;
	inline constexpr uint64_t PCI_CONFIGURATION_SPACE	= 0xFFFF800118000000;

	inline constexpr uint64_t OS_LOADER_INFO			= 0xFFFF800138000000;
	inline constexpr uint64_t OS_LOADER_PSF_FONT		= 0xFFFF800138000000;
	inline constexpr uint64_t OS_BOOT_DATA				= 0xFFFF800138080000;

	inline constexpr uint64_t PHYSICAL_MEMORY_MAP		= 0xFFFF80013A000000;
	inline constexpr uint64_t KERNEL_HEAP_MANAGEMENT	= 0xFFFF8001BA000000;

	inline constexpr uint64_t RECURSIVE_MEMORY_MAPPING	= 0xFFFFFF0000000000;

	inline constexpr uint64_t RUNTIME_PROCESS_DATA		= 0xFFFFFF8000000000;
	inline constexpr uint64_t KERNEL_STACK				= 0xFFFFFF8000000000;
	inline constexpr uint64_t KERNEL_STACK_GUARD		= 0xFFFFFF8000000000;
	inline constexpr uint64_t KERNEL_STACK_USABLE		= 0xFFFFFF8000001000;
	inline constexpr uint64_t KERNEL_STACK_RESERVE		= 0xFFFFFF80000FF000;
	inline constexpr uint64_t PROCESS_CONTEXT			= 0xFFFFFF8001000000;
	inline constexpr uint64_t MAIN_CORE_DUMP 			= 0xFFFFFF8001000000;
	inline constexpr uint64_t SECONDARY_CORE_DUMP 		= 0xFFFFFF8001008000;
	inline constexpr uint64_t USER_MEMORY_CONTEXT		= 0xFFFFFF8001100000;
	inline constexpr uint64_t USER_MEMORY_MANAGEMENT	= 0xFFFFFF8001100020;

	/// Sizes of the different zones

	inline constexpr uint64_t DMA_ZONE_SIZE					= 0x0000000001000000;

	inline constexpr uint64_t USER_MEMORY_SIZE				= 0x00007FFFFF000000;

	inline constexpr uint64_t KERNEL_IMAGE_SIZE				= 0x0000000020000000;
	inline constexpr uint64_t KERNEL_HEAP_SIZE				= 0x00000000E0000000;

	inline constexpr uint64_t EFI_RUNTIME_SERVICES_SIZE		= 0x0000000004000000;
	inline constexpr uint64_t EFI_GOP_FRAMEBUFFER_SIZE		= 0x0000000004000000;
	inline constexpr uint64_t EFI_ACPI_NVS_DATA_SIZE		= 0x0000000010000000;
	inline constexpr uint64_t PCI_CONFIGURATION_SPACE_SIZE	= 0x0000000020000000;

	inline constexpr uint64_t OS_LOADER_INFO_SIZE			= 0x0000000002000000;
	inline constexpr uint64_t OS_LOADER_PSF_FONT_SIZE		= 0x0000000000080000;
	inline constexpr uint64_t OS_BOOT_DATA_SIZE				= 0x0000000001F80000;

	inline constexpr uint64_t PHYSICAL_MEMORY_MAP_SIZE		= 0x0000000080000000;
	inline constexpr uint64_t KERNEL_HEAP_MANAGEMENT_SIZE	= 0x0000000000800000;

	inline constexpr uint64_t RECURSIVE_MEMORY_MAPPING_SIZE	= 0x0000008000000000;

	inline constexpr uint64_t RUNTIME_PROCESS_DATA_SIZE		= 0x0000008000000000;
	inline constexpr uint64_t KERNEL_STACK_SIZE				= 0x0000000000100000;
	inline constexpr uint64_t KERNEL_STACK_GUARD_SIZE		= 0x0000000000001000;
	inline constexpr uint64_t KERNEL_STACK_USABLE_SIZE		= 0x00000000000FE000;
	inline constexpr uint64_t KERNEL_STACK_RESERVE_SIZE		= 0x0000000000001000;
	inline constexpr uint64_t PROCESS_CONTEXT_SIZE			= 0x0000000000100000;
	inline constexpr uint64_t CORE_DUMP_SIZE 				= 0x0000000000008000;
	inline constexpr uint64_t USER_MEMORY_MANAGEMENT_SIZE	= 0x0000007FFEF00000;

	/// offsets in OS_BOOT_DATA

	// relative to OS_BOOT_DATA

	inline constexpr uint64_t BOOT_MEMORY_MAP_SIZE_OFFSET				= 0x000;
	inline constexpr uint64_t BOOT_MEMORY_MAP_DESCRIPTOR_SIZE_OFFSET	= 0x008;
	inline constexpr uint64_t BOOT_FLAT_MEMORY_MAP_OFFSET				= 0x010;

	// relative to OS_BOOT_DATA + [OS_BOOT_DATA + BOOT_MEMORY_MAP_SIZE_OFFSET]

	inline constexpr uint64_t BOOT_KERNEL_ENTRY_POINT_OFFSET			= 0x018;
	inline constexpr uint64_t BOOT_DMA_ZONE_BITMAP_OFFSET				= 0x020;
	inline constexpr uint64_t BOOT_FRAMEBUFFER_XRES_OFFSET				= 0x220;
	inline constexpr uint64_t BOOT_FRAMEBUFFER_YRES_OFFSET				= 0x224;
	inline constexpr uint64_t BOOT_FRAMEBUFFER_PPSL_OFFSET				= 0x228;
	inline constexpr uint64_t BOOT_FRAMEBUFFER_PXFMT_OFFSET				= 0x22C;
	inline constexpr uint64_t BOOT_FRAMEBUFFER_ADDRESS_OFFSET			= 0x230;
	inline constexpr uint64_t BOOT_FRAMEBUFFER_SIZE_OFFSET				= 0x238;
	inline constexpr uint64_t BOOT_RUNTIME_SERVICES_ADDRESS				= 0x240;
	inline constexpr uint64_t BOOT_PCIE_ECAM0_ADDRESS					= 0x248;
	inline constexpr uint64_t BOOT_ACPI_REVISION						= 0x250;
	inline constexpr uint64_t BOOT_ACPI_RSDP							= 0x258;
}
