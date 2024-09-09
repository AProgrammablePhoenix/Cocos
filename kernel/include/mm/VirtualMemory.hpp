#pragma once

#include <cstdint>
#include <type_traits>

#include <mm/PhysicalMemory.hpp>
#include <mm/VirtualMemoryLayout.hpp>

namespace VirtualMemory {
	// primary paging loops

	inline constexpr uint64_t PAGING_LOOP_MASK	= (VirtualMemoryLayout::RECURSIVE_MEMORY_MAPPING >> 39) & 0x1FF;
	inline constexpr uint64_t PAGING_LOOP_1		= VirtualMemoryLayout::RECURSIVE_MEMORY_MAPPING;
	inline constexpr uint64_t PAGING_LOOP_2		= PAGING_LOOP_1 | (PAGING_LOOP_MASK << 30);
	inline constexpr uint64_t PAGING_LOOP_3		= PAGING_LOOP_2 | (PAGING_LOOP_MASK << 21);
	inline constexpr uint64_t PAGING_LOOP_4		= PAGING_LOOP_3 | (PAGING_LOOP_MASK << 12);

	// secondary paging loops

	namespace Secondary {
		inline constexpr uint64_t PAGING_LOOP_MASK  = (VirtualMemoryLayout::SECONDARY_RECURSIVE_PML4 >> 39) & 0x1FF;
		inline constexpr uint64_t PAGING_LOOP_1		= VirtualMemoryLayout::SECONDARY_RECURSIVE_PML4;
		inline constexpr uint64_t PAGING_LOOP_2 = PAGING_LOOP_1 | (PAGING_LOOP_MASK << 30);
		inline constexpr uint64_t PAGING_LOOP_3 = PAGING_LOOP_2 | (PAGING_LOOP_MASK << 21);
		inline constexpr uint64_t PAGING_LOOP_4 = PAGING_LOOP_3 | (PAGING_LOOP_MASK << 12);
	}


	// Standard values

	inline constexpr uint64_t PML4_ENTRIES		= 512;
	inline constexpr uint64_t PDPT_ENTRIES		= 512;
	inline constexpr uint64_t PD_ENTRIES		= 512;
	inline constexpr uint64_t PT_ENTRIES		= 512;

	inline constexpr uint64_t PML4E_COVERAGE	= 0x0000008000000000;
	inline constexpr uint64_t PDPTE_COVERAGE	= 0x0000000040000000;
	inline constexpr uint64_t PDE_COVERAGE		= 0x0000000000200000;
	inline constexpr uint64_t PTE_COVERAGE		= 0x0000000000001000;

	inline constexpr uint64_t PML4E_PRESENT		= 0x0000000000000001;
	inline constexpr uint64_t PML4E_READWRITE	= 0x0000000000000002;
	inline constexpr uint64_t PML4E_USERMODE	= 0x0000000000000004;
	inline constexpr uint64_t PML4E_PWT			= 0x0000000000000008;
	inline constexpr uint64_t PML4E_PCD			= 0x0000000000000010;
	inline constexpr uint64_t PML4E_ACCESSED	= 0x0000000000000020;
	inline constexpr uint64_t PML4E_ADDRESS		= 0x000FFFFFFFFFF000;
	inline constexpr uint64_t PML4E_XD			= 0x8000000000000000;

	inline constexpr uint64_t PDPTE_PRESENT		= 0x0000000000000001;
	inline constexpr uint64_t PDPTE_READWRITE	= 0x0000000000000002;
	inline constexpr uint64_t PDPTE_USERMODE	= 0x0000000000000004;
	inline constexpr uint64_t PDPTE_PWT			= 0x0000000000000008;
	inline constexpr uint64_t PDPTE_PCD			= 0x0000000000000010;
	inline constexpr uint64_t PDPTE_ACCESSED	= 0x0000000000000020;
	inline constexpr uint64_t PDPTE_DIRTY		= 0x0000000000000040;
	inline constexpr uint64_t PDPTE_PAGE_SIZE	= 0x0000000000000080;
	inline constexpr uint64_t PDPTE_GLOBAL		= 0x0000000000000100;
	inline constexpr uint64_t PDPTE_ADDRESS		= 0x000FFFFFFFFFF000;
	inline constexpr uint64_t PDPTE_XD			= 0x8000000000000000;

	inline constexpr uint64_t PDE_PRESENT		= 0x0000000000000001;
	inline constexpr uint64_t PDE_READWRITE		= 0x0000000000000002;
	inline constexpr uint64_t PDE_USERMODE		= 0x0000000000000004;
	inline constexpr uint64_t PDE_PWT			= 0x0000000000000008;
	inline constexpr uint64_t PDE_PCD			= 0x0000000000000010;
	inline constexpr uint64_t PDE_ACCESSED		= 0x0000000000000020;
	inline constexpr uint64_t PDE_DIRTY			= 0x0000000000000040;
	inline constexpr uint64_t PDE_PAGE_SIZE		= 0x0000000000000080;
	inline constexpr uint64_t PDE_GLOBAL		= 0x0000000000000100;
	inline constexpr uint64_t PDE_ADDRESS		= 0x000FFFFFFFFFF000;
	inline constexpr uint64_t PDE_PK			= 0x7800000000000000;
	inline constexpr uint64_t PDE_XD			= 0x8000000000000000;

	inline constexpr uint64_t PTE_PRESENT		= 0x0000000000000001;
	inline constexpr uint64_t PTE_READWRITE		= 0x0000000000000002;
	inline constexpr uint64_t PTE_USERMODE		= 0x0000000000000004;
	inline constexpr uint64_t PTE_PWT			= 0x0000000000000008;
	inline constexpr uint64_t PTE_PCD			= 0x0000000000000010;
	inline constexpr uint64_t PTE_ACCESSED		= 0x0000000000000020;
	inline constexpr uint64_t PTE_DIRTY			= 0x0000000000000040;
	inline constexpr uint64_t PTE_PAT			= 0x0000000000000080;
	inline constexpr uint64_t PTE_GLOBAL		= 0x0000000000000100;
	inline constexpr uint64_t PTE_ADDRESS		= 0x000FFFFFFFFFF000;
	inline constexpr uint64_t PTE_PK			= 0x7800000000000000;
	inline constexpr uint64_t PTE_XD			= 0x8000000000000000;

	// Custom values (when the page is present and valid)

	inline constexpr uint64_t PTE_LOCK			= 0x0000000000000200;

	// Masks used for non-present entries (if the page entry is 0, it is invalid)

	// reserved, must be 0
	inline constexpr uint64_t NP_PRESENT	= 0x0000000000000001;
	inline constexpr uint64_t NP_READWRITE	= 0x0000000000000002;
	inline constexpr uint64_t NP_USERMODE	= 0x0000000000000004;
	inline constexpr uint64_t NP_PWT		= 0x0000000000000008;
	inline constexpr uint64_t NP_PCD		= 0x0000000000000010;
	inline constexpr uint64_t NP_PAT		= 0x0000000000000020;
	inline constexpr uint64_t NP_GLOBAL		= 0x0000000000000040;
	inline constexpr uint64_t NP_PK			= 0x0000000000000780;
	// Cleared if the page entry is invalid, or in the swap file,
	// Set if the page is reserved for on-demand mapping
	inline constexpr uint64_t NP_ON_DEMAND	= 0x0000000000000800;
	// Index of the page in the swap file, this field is ignored if NP_ON_DEMAND is set
	inline constexpr uint64_t NP_INDEX		= 0xFFFFFFFFFFFFE000;

	struct PTE {
		uint64_t raw;
	};

	struct PDE {
		uint64_t raw;
	};

	struct PDPTE {
		uint64_t raw;
	};

	struct PML4E {
		uint64_t raw;
	};

	struct VirtualAddress {
		uint16_t PML4_offset;
		uint16_t PDPT_offset;
		uint16_t PD_offset;
		uint16_t PT_offset;
		uint16_t offset;
	};

	enum class StatusCode {
		SUCCESS,
		OUT_OF_MEMORY,
		INVALID_PARAMETER
	};

	template<typename T> concept AddressType = std::is_same_v<T, uint64_t> || std::is_pointer<T>::value;
	template<AddressType T> constexpr VirtualAddress parseVirtualAddress(T address) {
		if constexpr (std::is_same_v<T, uint64_t>) {
			return VirtualAddress{
				.PML4_offset = static_cast<uint16_t>((address >> 39) & 0x1FF),
				.PDPT_offset = static_cast<uint16_t>((address >> 30) & 0x1FF),
				.PD_offset = static_cast<uint16_t>((address >> 21) & 0x1FF),
				.PT_offset = static_cast<uint16_t>((address >> 12) & 0x1FF),
				.offset = static_cast<uint16_t>(address & 0xFFF)
			};
		}
		else {
			return parseVirtualAddress(reinterpret_cast<uint64_t>(address));
		}
	};

	template<bool usePrimary = true>
	constexpr PTE* getPTAddress(uint64_t pml4_offset, uint64_t pdpt_offset, uint64_t pd_offset) {
		if constexpr (usePrimary) {
			return reinterpret_cast<PTE*>(PAGING_LOOP_1 | (pml4_offset << 30) | (pdpt_offset << 21) | (pd_offset << 12));
		}
		else {
			return reinterpret_cast<PTE*>(Secondary::PAGING_LOOP_1 | (pml4_offset << 30) | (pdpt_offset << 21) | (pd_offset << 12));
		}
	};
	template<bool usePrimary = true>
	constexpr PDE* getPDAddress(uint64_t pml4_offset, uint64_t pdpt_offset) {
		if constexpr (usePrimary) {
			return reinterpret_cast<PDE*>(PAGING_LOOP_2 | (pml4_offset << 21) | (pdpt_offset << 12));
		}
		else {
			return reinterpret_cast<PDE*>(Secondary::PAGING_LOOP_2 | (pml4_offset << 21) | (pdpt_offset << 12));
		}
	};
	template<bool usePrimary = true>
	constexpr PDPTE* getPDPTAddress(uint64_t pml4_offset) {
		if constexpr (usePrimary) {
			return reinterpret_cast<PDPTE*>(PAGING_LOOP_3 | (pml4_offset << 12));
		}
		else {
			return reinterpret_cast<PDPTE*>(Secondary::PAGING_LOOP_3 | (pml4_offset << 12));
		}
	};
	template<bool usePrimary = true>
	constexpr PML4E* getPML4Address() {
		if constexpr (usePrimary) {
			return reinterpret_cast<PML4E*>(PAGING_LOOP_4);
		}
		else {
			return reinterpret_cast<PML4E*>(Secondary::PAGING_LOOP_4);
		}
	};
	
	template<bool usePrimary = true>
	constexpr PTE* getPTEAddress(uint64_t pml4_offset, uint64_t pdpt_offset, uint64_t pd_offset, uint64_t pt_offset) {
		return getPTAddress<usePrimary>(pml4_offset, pdpt_offset, pd_offset) + pt_offset;
	};
	template<bool usePrimary = true>
	constexpr PDE* getPDEAddress(uint64_t pml4_offset, uint64_t pdpt_offset, uint64_t pd_offset) {
		return getPDAddress<usePrimary>(pml4_offset, pdpt_offset) + pd_offset;
	};
	template<bool usePrimary = true>
	constexpr PDPTE* getPDPTEAddress(uint64_t pml4_offset, uint64_t pdpt_offset) {
		return getPDPTAddress<usePrimary>(pml4_offset) + pdpt_offset;
	};
	template<bool usePrimary = true>
	constexpr PML4E* getPML4EAddress(uint64_t pml4_offset) {
		return getPML4Address<usePrimary>() + pml4_offset;
	};

	template<AddressType T> void zeroPage(T address) {
		if constexpr (std::is_same_v<T, uint64_t>) {
			uint64_t* pagePtr = reinterpret_cast<uint64_t*>(address);
			for (size_t qword = 0; qword < PhysicalMemory::FRAME_SIZE / sizeof(uint64_t); ++qword) {
				*(pagePtr + qword) = 0;
			}
		}
		else {
			zeroPage(reinterpret_cast<uint64_t>(address));
		}
	};

	void UpdateSecondaryRecursiveMapping(void* newAddress);

	StatusCode Setup();
	StatusCode SetupTask(void* CR3);

	void* AllocateDMA(uint64_t pages);
	void* AllocateKernelHeap(uint64_t pages);
	void* AllocateUserPages(uint64_t pages);
	void* AllocateUserPagesAt(uint64_t pages, void* ptr);

	StatusCode FreeDMA(void* ptr, uint64_t pages);
	StatusCode FreeKernelHeap(void* ptr, uint64_t pages);
	StatusCode FreeUserPages(void* ptr, uint64_t pages);

	void* MapGeneralPage(void* page);
	StatusCode UnmapGeneralPage(void* vpage);

	void* MapPCIConfiguration(void* configurationAddress);
	StatusCode UnmapPCIConfiguration(void* vconfigurationAddress);
}
