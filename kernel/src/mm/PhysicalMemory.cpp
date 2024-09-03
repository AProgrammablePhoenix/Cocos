#include <cpuid.h>
#include <cstdint>

#include <efi.h>

#include <mm/PhysicalMemory.hpp>
#include <mm/VirtualMemory.hpp>
#include <mm/VirtualMemoryLayout.hpp>

struct MemMapBlock {
	uint64_t physicalAddress;
	uint64_t availablePages;
};

namespace {
	static uint8_t MAXPHYADDR = 0;

	static uint64_t _setup_filter_address(uint64_t _addr);
	static uint64_t(*_filter_address_handler)(uint64_t _addr) = &_setup_filter_address;	

	static uint64_t _filter_address(uint64_t _addr) {
		return _addr & (((uint64_t)1 << MAXPHYADDR) - 1) & ~0xFFF;
	}
	static uint64_t _setup_filter_address(uint64_t _addr) {
		unsigned int eax = 0, unused = 0;
		__get_cpuid(0x80000008, &eax, &unused, &unused, &unused);
		MAXPHYADDR = eax & 0xFF;
		_filter_address_handler = &_filter_address;
		return PhysicalMemory::FilterAddress(_addr);
	}
}

namespace {
	static uint64_t availableBlockMemory = 0;
	static uint64_t availableMemory = 0;
	static uint64_t storedBlocks = 0;

	static uint8_t* DMA_bitmap = nullptr;

	static constexpr uint64_t DMA_PAGES			= (VirtualMemoryLayout::DMA_ZONE_SIZE / PhysicalMemory::FRAME_SIZE);
	static constexpr uint64_t DMA_BITMAP_SIZE	= (DMA_PAGES / 8);
}

uint64_t PhysicalMemory::FilterAddress(uint64_t address) {
	return _filter_address_handler(address);
}

uint64_t PhysicalMemory::FilterAddress(void* address) {
	return _filter_address_handler(reinterpret_cast<uint64_t>(address));
}

PhysicalMemory::StatusCode PhysicalMemory::Setup() {
	const uint64_t mmapSize = *reinterpret_cast<uint64_t*>(VirtualMemoryLayout::OS_BOOT_DATA + VirtualMemoryLayout::BOOT_MEMORY_MAP_SIZE_OFFSET);
	const uint64_t descriptorSize = *reinterpret_cast<uint64_t*>(VirtualMemoryLayout::OS_BOOT_DATA + VirtualMemoryLayout::BOOT_MEMORY_MAP_DESCRIPTOR_SIZE_OFFSET);
	uint8_t* const mmap = reinterpret_cast<uint8_t*>(VirtualMemoryLayout::OS_BOOT_DATA + VirtualMemoryLayout::BOOT_FLAT_MEMORY_MAP_OFFSET);
	const size_t descriptorCount = mmapSize / descriptorSize;

	DMA_bitmap = reinterpret_cast<uint8_t*>(VirtualMemoryLayout::OS_BOOT_DATA + mmapSize + VirtualMemoryLayout::BOOT_DMA_ZONE_BITMAP_OFFSET);

	MemMapBlock* currentBlockPtr = reinterpret_cast<MemMapBlock*>(VirtualMemoryLayout::PHYSICAL_MEMORY_MAP);

	for (size_t i = 0; i < descriptorCount; ++i) {
		EFI_MEMORY_DESCRIPTOR* descriptor = reinterpret_cast<EFI_MEMORY_DESCRIPTOR*>(mmap + i * descriptorSize);

		if (descriptor->Type == EfiConventionalMemory || descriptor->Type == EfiLoaderCode || descriptor->Type == EfiLoaderData
			|| descriptor->Type == EfiBootServicesCode || descriptor->Type == EfiBootServicesData || descriptor->Type == LoaderTemporaryMemory
		) {
			if (descriptor->PhysicalStart < VirtualMemoryLayout::DMA_ZONE_SIZE) {
				int64_t endDMAOffset = descriptor->PhysicalStart + FRAME_SIZE * descriptor->NumberOfPages - VirtualMemoryLayout::DMA_ZONE_SIZE;

				if (endDMAOffset <= 0) {
					continue;
				}

				descriptor->NumberOfPages = endDMAOffset / FRAME_SIZE;
				descriptor->PhysicalStart = VirtualMemoryLayout::DMA_ZONE_SIZE;
			}

			if (descriptor->NumberOfPages == 0) {
				continue;
			}
			else if (availableBlockMemory == 0) {
				uint64_t pageAddress = descriptor->PhysicalStart + FRAME_SIZE * (--descriptor->NumberOfPages);

				VirtualMemory::VirtualAddress mapping = VirtualMemory::parseVirtualAddress(reinterpret_cast<uint64_t>(currentBlockPtr));

				VirtualMemory::PML4E* pml4e = VirtualMemory::getPML4EAddress(mapping.PML4_offset);

				if ((pml4e->raw & VirtualMemory::PML4E_PRESENT) == 0) {
					// use the last page and try again
					pml4e->raw = (FilterAddress(pageAddress) & VirtualMemory::PML4E_ADDRESS)
						| VirtualMemory::PML4E_READWRITE
						| VirtualMemory::PML4E_PRESENT;

					VirtualMemory::PDPTE* pdpt = VirtualMemory::getPDPTEAddress(mapping.PML4_offset, 0);
					VirtualMemory::zeroPage(pdpt);

					if (descriptor->NumberOfPages == 0) {
						continue;
					}
					pageAddress = descriptor->PhysicalStart + FRAME_SIZE * (--descriptor->NumberOfPages);
				}

				VirtualMemory::PDPTE* pdpte = VirtualMemory::getPDPTEAddress(mapping.PML4_offset, mapping.PDPT_offset);

				if ((pdpte->raw & VirtualMemory::PDPTE_PRESENT) == 0) {
					pdpte->raw = (FilterAddress(pageAddress) & VirtualMemory::PDPTE_ADDRESS)
						| VirtualMemory::PDPTE_READWRITE
						| VirtualMemory::PDPTE_PRESENT;

					VirtualMemory::PDE* pd = VirtualMemory::getPDEAddress(mapping.PML4_offset, mapping.PDPT_offset, 0);
					VirtualMemory::zeroPage(pd);

					if (descriptor->NumberOfPages == 0) {
						continue;
					}
					pageAddress = descriptor->PhysicalStart + FRAME_SIZE * (--descriptor->NumberOfPages);
				}

				VirtualMemory::PDE* pde = VirtualMemory::getPDEAddress(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset);

				if ((pde->raw & VirtualMemory::PDE_PRESENT) == 0) {
					pde->raw = (FilterAddress(pageAddress) & VirtualMemory::PDE_ADDRESS)
						| VirtualMemory::PDE_READWRITE
						| VirtualMemory::PDE_PRESENT;
					
					VirtualMemory::PTE* pt = VirtualMemory::getPTEAddress(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset, 0);
					VirtualMemory::zeroPage(pt);

					if (descriptor->NumberOfPages == 0) {
						continue;
					}
					pageAddress = descriptor->PhysicalStart + FRAME_SIZE * (--descriptor->NumberOfPages);
				}

				VirtualMemory::PTE* pte = VirtualMemory::getPTEAddress(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset, mapping.PT_offset);
				pte->raw = VirtualMemory::PTE_XD
					| (FilterAddress(pageAddress) & VirtualMemory::PTE_ADDRESS)
					| VirtualMemory::PTE_READWRITE
					| VirtualMemory::PTE_PRESENT;

				if (descriptor->NumberOfPages == 0) {
					continue;
				}

				availableBlockMemory += FRAME_SIZE;
			}

			currentBlockPtr->physicalAddress = descriptor->PhysicalStart;
			currentBlockPtr->availablePages = descriptor->NumberOfPages;
			++currentBlockPtr;
			++storedBlocks;
			availableMemory += FRAME_SIZE * descriptor->NumberOfPages;
			availableBlockMemory -= sizeof(MemMapBlock);
		}
	}

	if (currentBlockPtr == reinterpret_cast<MemMapBlock*>(VirtualMemoryLayout::PHYSICAL_MEMORY_MAP)) {
		// not enough memory to setup the Physical Memory Manager (PMM)
		return StatusCode::OUT_OF_MEMORY;
	}

	DMA_bitmap[0] |= 1; // reserve the first DMA page to make NULL pointers invalid.

	return StatusCode::SUCCESS;
}

uint64_t PhysicalMemory::QueryMemoryUsage() {
	return availableMemory;
}

PhysicalMemory::StatusCode PhysicalMemory::QueryDMAAddress(uint64_t address) {
	uint64_t page = address / FRAME_SIZE;
	if (address >= VirtualMemoryLayout::DMA_ZONE_SIZE) {
		return StatusCode::INVALID_PARAMETER;
	}

	return ((DMA_bitmap[page / 8] & (1 << (page % 8))) == 0) ? StatusCode::FREE : StatusCode::ALLOCATED;
}

void* PhysicalMemory::AllocateDMA(uint64_t pages) {
	if (pages == 0) {
		return nullptr;
	}

	uint64_t startPage = 0;
	uint64_t pagesFound = 0;

	for (size_t i = 0; i < DMA_BITMAP_SIZE; ++i) {
		uint8_t byte = DMA_bitmap[i];

		for (size_t j = 0; j < 8; ++j) {
			if ((byte & (1 << j)) != 0) {
				pagesFound = 0;
			}
			else {
				if (pagesFound++ == 0) {
					startPage = static_cast<uint64_t>(8) * i + j;
				}
			}

			if (pagesFound >= pages) {
				for (size_t p = startPage; p < startPage + pages; ++p) {
					size_t x = p / 8;
					size_t y = p % 8;

					DMA_bitmap[x] |= 1 << y;
				}
				return reinterpret_cast<void*>(startPage * FRAME_SIZE);
			}
		}
	}

	return nullptr;
}

void* PhysicalMemory::Allocate() {
	if (storedBlocks == 0 || availableMemory < FRAME_SIZE) {
		return nullptr;
	}

	MemMapBlock* mmb = reinterpret_cast<MemMapBlock*>(VirtualMemoryLayout::PHYSICAL_MEMORY_MAP) + storedBlocks - 1;

	void* page = reinterpret_cast<void*>(mmb->physicalAddress + FRAME_SIZE * (--mmb->availablePages));

	if (mmb->availablePages == 0) {
		--storedBlocks;
		availableBlockMemory += sizeof(MemMapBlock);
	}
	availableMemory -= FRAME_SIZE;

	return page;
}

PhysicalMemory::StatusCode PhysicalMemory::FreeDMA(void* ptr, uint64_t pages) {
	const uint64_t address = reinterpret_cast<uint64_t>(ptr);

	if (address % FRAME_SIZE != 0 || address >= VirtualMemoryLayout::DMA_ZONE_SIZE
		|| address + FRAME_SIZE * pages > VirtualMemoryLayout::DMA_ZONE_SIZE || pages > DMA_PAGES
	) {
		return StatusCode::INVALID_PARAMETER;
	}

	const uint64_t firstPage = address / FRAME_SIZE;

	for (size_t p = firstPage; p < firstPage + pages; ++p) {
		size_t x = p / 8;
		size_t y = p % 8;

		DMA_bitmap[x] &= ~(1 << y);
	}

	return StatusCode::SUCCESS;
}

PhysicalMemory::StatusCode PhysicalMemory::Free(void* ptr) {
	const uint64_t address = reinterpret_cast<uint64_t>(ptr);

	MemMapBlock* blockPtr = reinterpret_cast<MemMapBlock*>(VirtualMemoryLayout::PHYSICAL_MEMORY_MAP) + storedBlocks;

	if (availableBlockMemory == 0) {
		if (storedBlocks * sizeof(MemMapBlock) >= VirtualMemoryLayout::PHYSICAL_MEMORY_MAP_SIZE) {
			return StatusCode::OUT_OF_MEMORY;
		}

		VirtualMemory::VirtualAddress mapping = VirtualMemory::parseVirtualAddress(blockPtr);

		VirtualMemory::PML4E* pml4e = VirtualMemory::getPML4EAddress(mapping.PML4_offset);

		if ((pml4e->raw & VirtualMemory::PML4E_PRESENT) == 0) {
			void* _page = Allocate();

			if (_page == nullptr) {
				return StatusCode::OUT_OF_MEMORY;
			}

			pml4e->raw = (FilterAddress(_page) & VirtualMemory::PML4E_ADDRESS)
				| VirtualMemory::PML4E_READWRITE
				| VirtualMemory::PML4E_PRESENT;

			VirtualMemory::PDPTE* pdpt = VirtualMemory::getPDPTEAddress(mapping.PML4_offset, 0);
			VirtualMemory::zeroPage(pdpt);
		}

		VirtualMemory::PDPTE* pdpte = VirtualMemory::getPDPTEAddress(mapping.PML4_offset, mapping.PDPT_offset);

		if ((pdpte->raw & VirtualMemory::PDPTE_PRESENT) == 0) {
			void* _page = Allocate();

			if (_page == nullptr) {
				return StatusCode::OUT_OF_MEMORY;
			}

			pdpte->raw = (FilterAddress(_page) & VirtualMemory::PDPTE_ADDRESS)
				| VirtualMemory::PDPTE_READWRITE
				| VirtualMemory::PDPTE_PRESENT;

			VirtualMemory::PDE* pd = VirtualMemory::getPDEAddress(mapping.PML4_offset, mapping.PDPT_offset, 0);
			VirtualMemory::zeroPage(pd);
		}

		VirtualMemory::PDE* pde = VirtualMemory::getPDEAddress(mapping.PML4_offset, mapping.PDPT_offset, mapping.PDPT_offset);

		if ((pde->raw & VirtualMemory::PDE_PRESENT) == 0) {
			void* _page = Allocate();

			if (_page == nullptr) {
				return StatusCode::OUT_OF_MEMORY;
			}

			pde->raw = (FilterAddress(_page) & VirtualMemory::PDE_ADDRESS)
				| VirtualMemory::PDE_READWRITE
				| VirtualMemory::PDE_PRESENT;

			VirtualMemory::PTE* pt = VirtualMemory::getPTEAddress(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset, 0);
			VirtualMemory::zeroPage(pt);
		}

		VirtualMemory::PTE* pte = VirtualMemory::getPTEAddress(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset, mapping.PT_offset);
		void* _page = Allocate();
		availableBlockMemory += FRAME_SIZE;

		if (_page == nullptr) {
			pte->raw = VirtualMemory::PTE_XD
				| (FilterAddress(ptr) & VirtualMemory::PTE_ADDRESS)
				| VirtualMemory::PTE_READWRITE
				| VirtualMemory::PTE_PRESENT;
			return StatusCode::SUCCESS;
		}

		pte->raw = VirtualMemory::PTE_XD
			| (FilterAddress(_page) & VirtualMemory::PTE_ADDRESS)
			| VirtualMemory::PTE_READWRITE
			| VirtualMemory::PTE_PRESENT;
	}

	blockPtr->availablePages = 1;
	blockPtr->physicalAddress = address;
	++storedBlocks;
	availableBlockMemory -= sizeof(MemMapBlock);
	availableMemory += FRAME_SIZE;
	return StatusCode::SUCCESS;
}
