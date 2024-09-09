#include <cstddef>
#include <cstdint>

#include <mm/PhysicalMemory.hpp>
#include <mm/VirtualMemory.hpp>
#include <mm/VirtualMemoryLayout.hpp>

#include <multitasking/Task.hpp>

#include <screen/Log.hpp>

struct VMemMapBlock {
	uint64_t virtualStart;
	uint64_t availablePages;
};

struct MemoryContext {
	uint64_t availableMemory;
	uint64_t availableBlockMemory;
	uint64_t storedBlocks;
};

enum class AccessPrivilege {
	HIGH,		// every paging structure has the user mode bit cleared
	MEDIUM,		// intermediate paging structures have the user bit set, the PTE has the user bit cleared (should only be used for the legacy DMA zone)
	LOW			// every intermediate paging structure has the user bit set
};

namespace VirtualMemory {
	namespace {
		static MemoryContext kernelContext {
			.availableMemory = VirtualMemoryLayout::KERNEL_HEAP_SIZE,
			.availableBlockMemory = 0,
			.storedBlocks = 0
		};

		static MemoryContext* const userContext = reinterpret_cast<MemoryContext*>(VirtualMemoryLayout::USER_MEMORY_CONTEXT);

		static inline constexpr uint64_t buildVirtualAddress(const VirtualAddress& mapping) {
			return (((mapping.PML4_offset & 0x100) != 0) ? 0xFFFF000000000000 : 0)
				| ((uint64_t)mapping.PML4_offset << 39)
				| ((uint64_t)mapping.PDPT_offset << 30)
				| ((uint64_t)mapping.PD_offset << 21)
				| ((uint64_t)mapping.PT_offset << 12)
				| ((uint64_t)mapping.offset);
		}
		
		template<bool usePrimary = true>
		static inline StatusCode mapPage(uint64_t _physicalAddress, uint64_t _virtualAddress, AccessPrivilege privilege) {
			if (_physicalAddress % PhysicalMemory::FRAME_SIZE != 0 || _virtualAddress % PhysicalMemory::FRAME_SIZE != 0) {
				return StatusCode::INVALID_PARAMETER;
			}

			VirtualAddress mapping = parseVirtualAddress(_virtualAddress);
			PML4E* pml4e = getPML4EAddress<usePrimary>(mapping.PML4_offset);

			if ((pml4e->raw & PML4E_PRESENT) == 0) {
				void* page = PhysicalMemory::Allocate();
				if (page == nullptr) {
					return StatusCode::OUT_OF_MEMORY;
				}
				pml4e->raw = (PhysicalMemory::FilterAddress(page) & PML4E_ADDRESS)
					| (privilege == AccessPrivilege::HIGH ? 0 : PML4E_USERMODE)
					| PML4E_READWRITE
					| PML4E_PRESENT;

				PDPTE* pdpt = getPDPTAddress<usePrimary>(mapping.PML4_offset);
				__asm__ volatile("invlpg (%0)" :: "r"(pdpt));
				zeroPage(pdpt);
			}

			PDPTE* pdpte = getPDPTEAddress<usePrimary>(mapping.PML4_offset, mapping.PDPT_offset);

			if ((pdpte->raw & PDPTE_PRESENT) == 0) {
				void* page = PhysicalMemory::Allocate();
				if (page == nullptr) {
					return StatusCode::OUT_OF_MEMORY;
				}
				pdpte->raw = (PhysicalMemory::FilterAddress(page) & PDPTE_ADDRESS)
					| (privilege == AccessPrivilege::HIGH ? 0 : PDPTE_USERMODE)
					| PDPTE_READWRITE
					| PDPTE_PRESENT;

				PDE* pd = getPDAddress<usePrimary>(mapping.PML4_offset, mapping.PDPT_offset);
				__asm__ volatile("invlpg (%0)" :: "r"(pd));
				zeroPage(pd);
			}

			PDE* pde = getPDEAddress<usePrimary>(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset);

			if ((pde->raw & PDE_PRESENT) == 0) {
				void* page = PhysicalMemory::Allocate();
				if (page == nullptr) {
					return StatusCode::OUT_OF_MEMORY;
				}
				pde->raw = (PhysicalMemory::FilterAddress(page) & PDE_ADDRESS)
					| (privilege == AccessPrivilege::HIGH ? 0 : PDE_USERMODE)
					| PDE_READWRITE
					| PDE_PRESENT;
				
				PTE* pt = getPTAddress<usePrimary>(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset);
				__asm__ volatile("invlpg (%0)" :: "r"(pt));
				zeroPage(pt);
			}

			PTE* pte = getPTEAddress<usePrimary>(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset, mapping.PT_offset);
			pte->raw = (PhysicalMemory::FilterAddress(_physicalAddress) & PTE_ADDRESS)
				| (privilege == AccessPrivilege::LOW ? PTE_USERMODE : 0)
				| PTE_READWRITE
				| PTE_PRESENT;

			__asm__ volatile("invlpg (%0)" :: "r"(_virtualAddress));

			return StatusCode::SUCCESS;
		}

		template<bool usePrimary = true>
		static inline StatusCode mapOnDemand(const void* _address, uint64_t pages, AccessPrivilege privilege) {
			const uint8_t* address = reinterpret_cast<const uint8_t*>(_address);

			for (size_t i = 0; i < pages; ++i) {
				VirtualAddress mapping = parseVirtualAddress(address);

				PML4E* pml4e = getPML4EAddress<usePrimary>(mapping.PML4_offset);

				if ((pml4e->raw & PML4E_PRESENT) == 0) {
					void* page = PhysicalMemory::Allocate();
					if (page == nullptr) {
						return StatusCode::OUT_OF_MEMORY;
					}
					pml4e->raw = (PhysicalMemory::FilterAddress(page) & PML4E_ADDRESS)
						| PML4E_READWRITE
						| PML4E_PRESENT;

					PDPTE* pdpt = getPDPTAddress<usePrimary>(mapping.PML4_offset);
					__asm__ volatile("invlpg (%0)" :: "r"(pdpt));
					zeroPage(pdpt);
				}

				PDPTE* pdpte = getPDPTEAddress<usePrimary>(mapping.PML4_offset, mapping.PDPT_offset);

				if ((pdpte->raw & PDPTE_PRESENT) == 0) {
					void* page = PhysicalMemory::Allocate();
					if (page == nullptr) {
						return StatusCode::OUT_OF_MEMORY;
					}
					pdpte->raw = (PhysicalMemory::FilterAddress(page) & PDPTE_ADDRESS)
						| PDPTE_READWRITE
						| PDPTE_PRESENT;

					PDE* pd = getPDAddress<usePrimary>(mapping.PML4_offset, mapping.PDPT_offset);
					__asm__ volatile("invlpg (%0)" :: "r"(pd));
					zeroPage(pd);
				}

				PDE* pde = getPDEAddress<usePrimary>(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset);

				if ((pde->raw & PDE_PRESENT) == 0) {
					void* page = PhysicalMemory::Allocate();
					if (page == nullptr) {
						return StatusCode::OUT_OF_MEMORY;
					}
					pde->raw = (PhysicalMemory::FilterAddress(page) & PDE_ADDRESS)
						| PDE_READWRITE
						| PDE_PRESENT;

					PTE* pt = getPTAddress<usePrimary>(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset);
					__asm__ volatile("invlpg (%0)" :: "r"(pt));
					zeroPage(pt);
				}

				PTE* pte = getPTEAddress<usePrimary>(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset, mapping.PT_offset);
				pte->raw = NP_ON_DEMAND
					| (privilege == AccessPrivilege::LOW ? NP_USERMODE : 0)
					| NP_READWRITE;

				__asm__ volatile("invlpg (%0)" :: "r"(address));

				address += PhysicalMemory::FRAME_SIZE;
			}

			return StatusCode::SUCCESS;
		}

		static inline StatusCode expandVirtualMemoryMap(uint64_t start, uint64_t blocks) {
			void* page = PhysicalMemory::Allocate();
			if (page == nullptr) {
				return StatusCode::OUT_OF_MEMORY;
			}

			return mapPage(reinterpret_cast<uint64_t>(page), start + blocks * sizeof(VMemMapBlock), AccessPrivilege::HIGH);
		}

		// sorts (to the right) by descending number of available pages
		static inline void sortVirtualMemoryMap(VMemMapBlock* start, size_t n) {
			if (n > 2) {
				VMemMapBlock temp;
				for (size_t i = 0; i < n - 1 && (start + 1)->availablePages > start->availablePages; ++i) {
					temp = *start;
					*start = *(start + 1);
					*++start = temp;
				}
			}
		}
		
		// reverse sorts (to the left) by ascending number of available pages
		static inline void rsortVirtualMemoryMap(VMemMapBlock* start, size_t n) {
			if (n > 2) {
				VMemMapBlock temp;
				for (size_t i = 0; i < n - 1 && (start - 1)->availablePages < start->availablePages; ++i) {
					temp = *start;
					*start = *(start - 1);
					*--start = temp;
				}
			}
		}

		template<AccessPrivilege privilege>
		static inline void* AllocateHintCore(VMemMapBlock* block, uint64_t offset, uint64_t pages) {
			MemoryContext* ctx = privilege == AccessPrivilege::HIGH ? &kernelContext : userContext;

			void* pagesStart = reinterpret_cast<void*>(block->virtualStart);

			if (mapOnDemand(pagesStart, pages, AccessPrivilege::LOW) != StatusCode::SUCCESS) {
				return nullptr;
			}

			block->availablePages -= pages;
			block->virtualStart += pages * PhysicalMemory::FRAME_SIZE;

			sortVirtualMemoryMap(block, ctx->storedBlocks - offset);

			ctx->availableMemory -= pages * PhysicalMemory::FRAME_SIZE;
			return pagesStart;
		}

		template<AccessPrivilege privilege, bool useHint = false>
		static inline void* AllocateCore(uint64_t pages, [[maybe_unused]] void* hintPtr) {
			constexpr uint64_t managementBase =privilege == AccessPrivilege::HIGH
				? VirtualMemoryLayout::KERNEL_HEAP_MANAGEMENT
				: VirtualMemoryLayout::USER_MEMORY_MANAGEMENT;
			
			MemoryContext* ctx = privilege == AccessPrivilege::HIGH ? &kernelContext : userContext;

			VMemMapBlock* vmmb = reinterpret_cast<VMemMapBlock*>(managementBase);
			
			if (ctx->availableMemory < pages * PhysicalMemory::FRAME_SIZE || vmmb->availablePages < pages) {
				return nullptr;
			}

			size_t index = 0;

			if (pages == 0) {
				return nullptr;
			}
			else if (pages == 1) {
				index = ctx->storedBlocks - 1;
				vmmb += index;
			}
			else {
				for (++index, ++vmmb;; ++index, ++vmmb) {
					if (index >= ctx->storedBlocks || vmmb->availablePages < pages) {
						--index;
						--vmmb;
						break;
					}
				}
			}

			if constexpr (useHint) {
				if (hintPtr != nullptr && (reinterpret_cast<uint64_t>(hintPtr) % PhysicalMemory::FRAME_SIZE) == 0) {
					const uint64_t hint = reinterpret_cast<uint64_t>(hintPtr);
					size_t indexOffset = 0;
					unsigned int fitFound = 0;

					for (size_t i = 0; i <= index; ++i) {
						VMemMapBlock* newBlock = vmmb - i;

						if (newBlock->virtualStart <= hint
							&& hint + pages * PhysicalMemory::FRAME_SIZE
							<= newBlock->virtualStart + newBlock->availablePages * PhysicalMemory::FRAME_SIZE
						) {
							indexOffset = i;
							fitFound = 1;
							break;
						}
						else if (newBlock->virtualStart < vmmb->virtualStart && newBlock->virtualStart >= hint) {
							if (hint + pages * PhysicalMemory::FRAME_SIZE
								<= newBlock->virtualStart + newBlock->availablePages * PhysicalMemory::FRAME_SIZE
							) {
								indexOffset = i;
								fitFound = 1;
								continue;
							}
						}
					}

					if (fitFound && vmmb->availablePages > pages) {
						vmmb -= indexOffset;
						index -= indexOffset;

						if (vmmb->virtualStart <= hint) {
							VMemMapBlock prevBlock = {
								.virtualStart = vmmb->virtualStart,
								.availablePages = (hint - vmmb->virtualStart) / PhysicalMemory::FRAME_SIZE
							};
							VMemMapBlock nextBlock = {
								.virtualStart = hint + pages * PhysicalMemory::FRAME_SIZE,
								.availablePages = (
									vmmb->virtualStart
									+ vmmb->availablePages * PhysicalMemory::FRAME_SIZE
									- (hint + pages * PhysicalMemory::FRAME_SIZE)) / PhysicalMemory::FRAME_SIZE
							};

							if (prevBlock.availablePages == 0) {
								return AllocateHintCore<privilege>(vmmb, index, pages);
							}
							else if (nextBlock.availablePages == 0) {
								if (mapOnDemand(hintPtr, pages, privilege) != StatusCode::SUCCESS) {
									return nullptr;
								}

								*vmmb = prevBlock;
								sortVirtualMemoryMap(vmmb, ctx->storedBlocks - index);
								ctx->availableMemory -= pages * PhysicalMemory::FRAME_SIZE;
								
								return hintPtr;
							}
							else {
								if (ctx->availableBlockMemory == 0) {
									if (expandVirtualMemoryMap(managementBase, ctx->storedBlocks) != StatusCode::SUCCESS) {
										return nullptr;
									}
									ctx->availableBlockMemory += PhysicalMemory::FRAME_SIZE;
								}

								if (mapOnDemand(hintPtr, pages, privilege) != StatusCode::SUCCESS) {
									return nullptr;
								}

								VMemMapBlock* splitResidue = reinterpret_cast<VMemMapBlock*>(managementBase) + ctx->storedBlocks++;

								if (prevBlock.availablePages > nextBlock.availablePages) {
									*vmmb = prevBlock;
									*splitResidue = nextBlock;
								}
								else {
									*vmmb = nextBlock;
									*splitResidue = prevBlock;
								}

								sortVirtualMemoryMap(vmmb, ctx->storedBlocks - index);
								rsortVirtualMemoryMap(splitResidue, ctx->storedBlocks);
								ctx->availableMemory -= pages * PhysicalMemory::FRAME_SIZE;
								ctx->availableBlockMemory -= sizeof(VMemMapBlock);

								return hintPtr;
							}
						}
						else {
							return AllocateHintCore<privilege>(vmmb, index, pages);
						}
					}
				}
			}

			void* pagesStart = reinterpret_cast<void*>(vmmb->virtualStart + (vmmb->availablePages -= pages) * PhysicalMemory::FRAME_SIZE);
			int remove = vmmb->availablePages == 0;

			if (mapOnDemand(pagesStart, pages, privilege) != StatusCode::SUCCESS) {
				vmmb->availablePages += pages;
				return nullptr;
			}

			sortVirtualMemoryMap(vmmb, ctx->storedBlocks - index);

			if (remove) {
				--ctx->storedBlocks;
				ctx->availableBlockMemory += sizeof(VMemMapBlock);

				if (ctx->availableBlockMemory % PhysicalMemory::FRAME_SIZE == 0 && ctx->availableBlockMemory > PhysicalMemory::FRAME_SIZE) {
					ctx->availableBlockMemory -= PhysicalMemory::FRAME_SIZE;
					const uint64_t linearAddress = managementBase + ctx->availableBlockMemory;
					VirtualAddress blockMemoryAddress = parseVirtualAddress(linearAddress);
					PTE* blockMemoryPTE = getPTEAddress(
						blockMemoryAddress.PML4_offset,
						blockMemoryAddress.PDPT_offset,
						blockMemoryAddress.PD_offset,
						blockMemoryAddress.PT_offset
					);
					PhysicalMemory::Free(reinterpret_cast<void*>(blockMemoryPTE->raw & PTE_ADDRESS));
					blockMemoryPTE->raw = 0;
					__asm__ volatile("invlpg (%0)" :: "r"(linearAddress));
				}
			}

			ctx->availableMemory -= pages * PhysicalMemory::FRAME_SIZE;
			return pagesStart;
		}

		template<AccessPrivilege privilege> static inline StatusCode FreeCore(void* ptr, uint64_t pages) {
			constexpr uint64_t managementBase = privilege == AccessPrivilege::HIGH
				? VirtualMemoryLayout::KERNEL_HEAP_MANAGEMENT
				: VirtualMemoryLayout::USER_MEMORY_MANAGEMENT;

			MemoryContext* ctx = privilege == AccessPrivilege::HIGH ? &kernelContext : userContext;

			if (pages == 0) {
				return StatusCode::SUCCESS;
			}
			
			uint64_t address = reinterpret_cast<uint64_t>(ptr);

			if constexpr (privilege == AccessPrivilege::LOW) {
				if (address > VirtualMemoryLayout::USER_MEMORY + VirtualMemoryLayout::USER_MEMORY_SIZE
					|| address < VirtualMemoryLayout::USER_MEMORY
					|| address + pages * PhysicalMemory::FRAME_SIZE < address
				) {
					return StatusCode::INVALID_PARAMETER;
				}
			}

			for (size_t i = 0; i < pages; ++i, address += PhysicalMemory::FRAME_SIZE) {
				VirtualAddress mapping = parseVirtualAddress(address);
				PML4E* pml4e = getPML4EAddress(mapping.PML4_offset);
				PDPTE* pdpte = getPDPTEAddress(mapping.PML4_offset, mapping.PDPT_offset);
				PDE* pde = getPDEAddress(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset);
				PTE* pte = getPTEAddress(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset, mapping.PT_offset);

				if constexpr (privilege == AccessPrivilege::LOW) {
					if ((pml4e->raw & PML4E_PRESENT) == 0 || (pdpte->raw & PDPTE_PRESENT) == 0 || (pde->raw & PDE_PRESENT) == 0 || (pte->raw == 0)) {
						return StatusCode::INVALID_PARAMETER;
					}
				}

				if ((pte->raw & PTE_PRESENT) != 0) {
					void* pageAddress = reinterpret_cast<void*>(pte->raw & PTE_ADDRESS);
					if (PhysicalMemory::Free(pageAddress) != PhysicalMemory::StatusCode::SUCCESS) {
						return StatusCode::OUT_OF_MEMORY;
					}
					__asm__ volatile("invlpg (%0)" :: "r"(address));
				}
				else if ((pte->raw & NP_ON_DEMAND) != 0) {
					// do stuff with the swap file
				}
				pte->raw = 0;
			}

			VMemMapBlock vmmb = {
				.virtualStart = reinterpret_cast<uint64_t>(ptr),
				.availablePages = pages
			};

			if (ctx->availableBlockMemory == 0) {
				auto status = expandVirtualMemoryMap(managementBase, ctx->storedBlocks);
				if (status != StatusCode::SUCCESS) {
					return status;
				}
				ctx->availableBlockMemory += PhysicalMemory::FRAME_SIZE;
			}

			VMemMapBlock* blockPtr = reinterpret_cast<VMemMapBlock*>(managementBase) + ctx->storedBlocks++;
			*blockPtr = vmmb;
			ctx->availableBlockMemory -= sizeof(VMemMapBlock);
			ctx->availableMemory += pages * PhysicalMemory::FRAME_SIZE;
			rsortVirtualMemoryMap(blockPtr, ctx->storedBlocks);

			return StatusCode::SUCCESS;
		}
	}

	void UpdateSecondaryRecursiveMapping(void* newAddress) {
		constexpr VirtualAddress secondaryMapping = parseVirtualAddress(VirtualMemoryLayout::SECONDARY_RECURSIVE_PML4);
		PML4E* secondaryPML4 = getPML4EAddress(secondaryMapping.PML4_offset);

		secondaryPML4->raw = PML4E_XD
			| (PhysicalMemory::FilterAddress(newAddress) & PML4E_ADDRESS)
			| PML4E_READWRITE
			| PML4E_PRESENT;

		__asm__ volatile("invlpg (%0)" :: "r"(getPML4Address<false>()));
	}

	StatusCode Setup() {
		// set up identity paging for the DMA zone
		for (size_t i = VirtualMemoryLayout::DMA_ZONE;
			i < VirtualMemoryLayout::DMA_ZONE + VirtualMemoryLayout::DMA_ZONE_SIZE;
			i += PhysicalMemory::FRAME_SIZE
		) {
			if (PhysicalMemory::QueryDMAAddress(i) == PhysicalMemory::StatusCode::ALLOCATED) {
				mapPage(i, i, AccessPrivilege::MEDIUM);
			}
		}

		// make the NULL memory page reserved and unusable
		getPTEAddress(0, 0, 0, 0)->raw = 0;
		__asm__ volatile("invlpg (%0)" :: "r"((uint64_t)0));

		// set up kernel heap
		void* basePage = PhysicalMemory::Allocate();
		if (basePage == nullptr) {
			return StatusCode::OUT_OF_MEMORY;
		}

		auto status = mapPage(reinterpret_cast<uint64_t>(basePage), VirtualMemoryLayout::KERNEL_HEAP_MANAGEMENT, AccessPrivilege::HIGH);
		if (status != StatusCode::SUCCESS) {
			return status;
		}

		kernelContext.availableBlockMemory = PhysicalMemory::FRAME_SIZE - sizeof(VMemMapBlock);
		kernelContext.storedBlocks = 1;

		VMemMapBlock* kernelHeapBlockPtr = reinterpret_cast<VMemMapBlock*>(VirtualMemoryLayout::KERNEL_HEAP_MANAGEMENT);
		kernelHeapBlockPtr->virtualStart = VirtualMemoryLayout::KERNEL_HEAP;
		kernelHeapBlockPtr->availablePages = kernelContext.availableMemory / PhysicalMemory::FRAME_SIZE;

		// setup a temporary main core dump in case a setup failure happens
		basePage = PhysicalMemory::Allocate();
		if (basePage == nullptr) {
			return StatusCode::OUT_OF_MEMORY;
		}
		
		status = mapPage(reinterpret_cast<uint64_t>(basePage), VirtualMemoryLayout::MAIN_CORE_DUMP, AccessPrivilege::HIGH);
		return status;
	}

	StatusCode SetupTask(void* CR3) {
		// setting up recursive paging in the PML4
		PML4E* vroot = reinterpret_cast<PML4E*>(MapGeneralPage(CR3));
		if (vroot == nullptr) {
			return StatusCode::OUT_OF_MEMORY;
		}

		(vroot + 509)->raw = (vroot + 510)->raw = PML4E_XD
			| (PhysicalMemory::FilterAddress(CR3) & PML4E_ADDRESS)
			| PML4E_READWRITE
			| PML4E_PRESENT;

		auto status = UnmapGeneralPage(vroot);
		if (status != StatusCode::SUCCESS) {
			return status;
		}

		UpdateSecondaryRecursiveMapping(CR3);

		// copying shared kernel memory
		for (size_t i = 256; i < 509; ++i) {
			volatile PML4E* current = getPML4EAddress(i);
			PML4E* shared = getPML4EAddress<false>(i);

			shared->raw = current->raw;
		}
		// clearing entries that are not shared
		for (size_t i = 0; i < 256; ++i) {
			getPML4EAddress<false>(i)->raw = 0;
		}
		getPML4EAddress<false>(511)->raw = 0;

		// set up the kernel stack and the stack guard
		status = mapOnDemand<false>(
			reinterpret_cast<void*>(VirtualMemoryLayout::KERNEL_STACK_USABLE),
			(VirtualMemoryLayout::KERNEL_STACK_USABLE_SIZE - PhysicalMemory::FRAME_SIZE) / PhysicalMemory::FRAME_SIZE,
			AccessPrivilege::HIGH
		);

		VirtualAddress stackGuardMapping = parseVirtualAddress(VirtualMemoryLayout::KERNEL_STACK_GUARD);
		PTE* stackGuardPTE = getPTEAddress<false>(
			stackGuardMapping.PML4_offset,
			stackGuardMapping.PDPT_offset,
			stackGuardMapping.PD_offset,
			stackGuardMapping.PT_offset
		);
		stackGuardPTE->raw = 0;
		__asm__ volatile("invlpg (%0)" :: "r"(VirtualMemoryLayout::KERNEL_STACK_GUARD));

		void* stackTop = PhysicalMemory::Allocate();
		void* stackReserve = PhysicalMemory::Allocate();

		if (stackTop == nullptr || stackReserve == nullptr) {
			return StatusCode::OUT_OF_MEMORY;
		}

		status = mapPage<false>(
			reinterpret_cast<uint64_t>(stackTop),
			VirtualMemoryLayout::KERNEL_STACK_RESERVE - PhysicalMemory::FRAME_SIZE,
			AccessPrivilege::HIGH
		);
		if (status != StatusCode::SUCCESS) {
			return status;
		}

		status = mapPage<false>(
			reinterpret_cast<uint64_t>(stackReserve),
			VirtualMemoryLayout::KERNEL_STACK_RESERVE,
			AccessPrivilege::HIGH
		);
		if (status != StatusCode::SUCCESS) {
			return status;
		}

		// set up the process context zone
		status = mapOnDemand<false>(
			reinterpret_cast<void*>(VirtualMemoryLayout::PROCESS_CONTEXT),
			VirtualMemoryLayout::PROCESS_CONTEXT_SIZE / PhysicalMemory::FRAME_SIZE,
			AccessPrivilege::HIGH
		);
		if (status != StatusCode::SUCCESS) {
			return status;
		}

		// allocate two pages for the main and secondary core dump zones
		void* mainDumpPage = PhysicalMemory::Allocate();
		if (mainDumpPage == nullptr) {
			return StatusCode::OUT_OF_MEMORY;
		}

		status = mapPage<false>(
			reinterpret_cast<uint64_t>(mainDumpPage),
			VirtualMemoryLayout::MAIN_CORE_DUMP,
			AccessPrivilege::HIGH
		);
		if (status != StatusCode::SUCCESS) {
			return status;
		}

		void* mappedDumpPage = MapGeneralPage(mainDumpPage);
		if (mappedDumpPage == nullptr) {
			return StatusCode::OUT_OF_MEMORY;
		}
		zeroPage(mappedDumpPage);

		status = UnmapGeneralPage(mappedDumpPage);
		if (status != StatusCode::SUCCESS) {
			return status;
		}

		void* secondaryDumpPage = PhysicalMemory::Allocate();
		if (secondaryDumpPage == nullptr) {
			return StatusCode::OUT_OF_MEMORY;
		}

		status = mapPage<false>(
			reinterpret_cast<uint64_t>(secondaryDumpPage),
			VirtualMemoryLayout::SECONDARY_CORE_DUMP,
			AccessPrivilege::HIGH
		);
		if (status != StatusCode::SUCCESS) {
			return status;
		}

		mappedDumpPage = MapGeneralPage(secondaryDumpPage);
		if (mappedDumpPage == nullptr) {
			return StatusCode::OUT_OF_MEMORY;
		}
		zeroPage(mappedDumpPage);

		status = UnmapGeneralPage(mappedDumpPage);
		if (status != StatusCode::SUCCESS) {
			return status;
		}

		// Map user stack, leaving one extra guard page, usable stack size: 0x1FF000 bytes, or 2 MB - 4 KB = 2,093,056 bytes
		status = mapOnDemand<false>(
			reinterpret_cast<void*>(VirtualMemoryLayout::USER_STACK + PhysicalMemory::FRAME_SIZE),
			(VirtualMemoryLayout::USER_STACK_SIZE - PhysicalMemory::FRAME_SIZE) / PhysicalMemory::FRAME_SIZE,
			AccessPrivilege::LOW
		);

		if (status != StatusCode::SUCCESS) {
			return status;
		}

		// set up the user memory 
		void* basePage = PhysicalMemory::Allocate();
		if (basePage == nullptr) {
			return StatusCode::OUT_OF_MEMORY;
		}

		status = mapPage<false>(reinterpret_cast<uint64_t>(basePage), VirtualMemoryLayout::USER_MEMORY_CONTEXT, AccessPrivilege::HIGH);
		if (status != StatusCode::SUCCESS) {
			return status;
		}

		void* vbasePage = MapGeneralPage(basePage);
		if (vbasePage == nullptr) {
			return StatusCode::OUT_OF_MEMORY;
		}

		MemoryContext* newUserContext = reinterpret_cast<MemoryContext*>(vbasePage);

		newUserContext->availableMemory = VirtualMemoryLayout::USER_MEMORY_SIZE - VirtualMemoryLayout::USER_STACK_SIZE;
		newUserContext->availableBlockMemory = PhysicalMemory::FRAME_SIZE 
			- sizeof(VMemMapBlock) 
			- (VirtualMemoryLayout::USER_MEMORY_MANAGEMENT - VirtualMemoryLayout::USER_MEMORY_CONTEXT);
		newUserContext->storedBlocks = 1;

		VMemMapBlock* userMemoryBlockPtr = reinterpret_cast<VMemMapBlock*>(
			reinterpret_cast<uint8_t*>(vbasePage) + (VirtualMemoryLayout::USER_MEMORY_MANAGEMENT - VirtualMemoryLayout::USER_MEMORY_CONTEXT)
		);
		userMemoryBlockPtr->virtualStart = VirtualMemoryLayout::USER_MEMORY;
		userMemoryBlockPtr->availablePages = newUserContext->availableMemory / PhysicalMemory::FRAME_SIZE;

		UnmapGeneralPage(vbasePage);

		return StatusCode::SUCCESS;
	}

	void* AllocateDMA(uint64_t pages) {
		void* const allocated = PhysicalMemory::AllocateDMA(pages);
		if (allocated == nullptr) {
			return nullptr;
		}
		
		uint64_t address = reinterpret_cast<uint64_t>(allocated);

		for (size_t i = 0; i < pages; ++i, address += PhysicalMemory::FRAME_SIZE) {
			if (mapPage(address, address, AccessPrivilege::MEDIUM) != StatusCode::SUCCESS) {
				return nullptr;
			}
		}

		return allocated;
	}

	void* AllocateKernelHeap(uint64_t pages) {
		return AllocateCore<AccessPrivilege::HIGH>(pages, nullptr);
	}

	void* AllocateUserPages(uint64_t pages) {
		return AllocateCore<AccessPrivilege::LOW>(pages, nullptr);
	}

	void* AllocateUserPagesAt(uint64_t pages, void* ptr) {
		return AllocateCore<AccessPrivilege::LOW, true>(pages, ptr);
	}

	StatusCode FreeDMA(void* ptr, uint64_t pages) {
		auto status = PhysicalMemory::FreeDMA(ptr, pages);
		if (status != PhysicalMemory::StatusCode::SUCCESS) {
			return StatusCode::INVALID_PARAMETER;
		}

		uint64_t address = reinterpret_cast<uint64_t>(ptr);

		for (size_t i = 0; i < pages; ++i, address += PhysicalMemory::FRAME_SIZE) {
			VirtualAddress mapping = parseVirtualAddress(address);
			PTE* pte = getPTEAddress(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset, mapping.PT_offset);
			pte->raw = 0;
		}

		return StatusCode::SUCCESS;
	}

	StatusCode FreeKernelHeap(void* ptr, uint64_t pages) {
		return FreeCore<AccessPrivilege::HIGH>(ptr, pages);
	}

	StatusCode FreeUserPages(void* ptr, uint64_t pages) {
		return FreeCore<AccessPrivilege::LOW>(ptr, pages);
	}

	void* MapGeneralPage(void* pageAddress) {
		constexpr uint64_t GP_PAGES = VirtualMemoryLayout::GENERAL_PURPOSE_MAPPINGS_SIZE / PhysicalMemory::FRAME_SIZE;

		uint64_t address = VirtualMemoryLayout::GENERAL_PURPOSE_MAPPINGS;

		for (size_t i = 0; i < GP_PAGES; ++i, address += PhysicalMemory::FRAME_SIZE) {
			VirtualAddress mapping = parseVirtualAddress(address);

			PDPTE* pdpte = getPDPTEAddress(mapping.PML4_offset, mapping.PDPT_offset);

			if ((pdpte->raw & PDPTE_PRESENT) == 0) {
				void* page = PhysicalMemory::Allocate();
				if (page == nullptr) {
					return nullptr;
				}
				pdpte->raw =
					(PhysicalMemory::FilterAddress(page) & PDPTE_ADDRESS)
					| PDPTE_READWRITE
					| PDPTE_PRESENT;

				PDE* pd = getPDAddress(mapping.PML4_offset, mapping.PDPT_offset);
				zeroPage(pd);
			}

			PDE* pde = getPDEAddress(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset);

			if ((pde->raw & PDE_PRESENT) == 0) {
				void* page = PhysicalMemory::Allocate();
				if (page == nullptr) {
					return nullptr;
				}
				pde->raw =
					(PhysicalMemory::FilterAddress(page) & PDE_ADDRESS)
					| PDE_READWRITE
					| PDE_PRESENT;

				PTE* pt = getPTAddress(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset);
				zeroPage(pt);
			}

			PTE* pte = getPTEAddress(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset, mapping.PT_offset);

			if ((pte->raw & PTE_PRESENT) == 0) {
				pte->raw = (PhysicalMemory::FilterAddress(pageAddress) & PTE_ADDRESS)
					| PTE_READWRITE
					| PTE_PRESENT;

				__asm__ volatile("invlpg (%0)" :: "r"(address));
				return reinterpret_cast<void*>(address);
			}
		}

		return nullptr;
	}

	StatusCode UnmapGeneralPage(void* vpage) {
		VirtualAddress mapping = parseVirtualAddress(vpage);

		PML4E* pml4e = getPML4EAddress(mapping.PML4_offset);
		if ((pml4e->raw & PML4E_PRESENT) == 0) {
			return StatusCode::INVALID_PARAMETER;
		}

		PDPTE* pdpte = getPDPTEAddress(mapping.PML4_offset, mapping.PDPT_offset);
		if ((pdpte->raw & PDPTE_PRESENT) == 0) {
			return StatusCode::INVALID_PARAMETER;
		}

		PDE* pde = getPDEAddress(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset);
		if ((pde->raw & PDE_PRESENT) == 0) {
			return StatusCode::INVALID_PARAMETER;
		}

		PTE* pte = getPTEAddress(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset, mapping.PT_offset);
		pte->raw = 0;

		__asm__ volatile("invlpg (%0)" :: "r"(vpage));
		return StatusCode::SUCCESS;
	}

	void* MapPCIConfiguration(void* configurationAddress) {
		constexpr uint64_t PCI_SPACE_PAGES = VirtualMemoryLayout::PCI_CONFIGURATION_SPACE_SIZE / PhysicalMemory::FRAME_SIZE;

		uint64_t address = VirtualMemoryLayout::PCI_CONFIGURATION_SPACE;

		for (size_t i = 0; i < PCI_SPACE_PAGES; ++i, address += PhysicalMemory::FRAME_SIZE) {
			VirtualAddress mapping = parseVirtualAddress(address);

			PDPTE* pdpte = getPDPTEAddress(mapping.PML4_offset, mapping.PDPT_offset);

			if ((pdpte->raw & PDPTE_PRESENT) == 0) {
				void* page = PhysicalMemory::Allocate();
				if (page == nullptr) {
					return nullptr;
				}
				pdpte->raw =
					(PhysicalMemory::FilterAddress(page) & PDPTE_ADDRESS)
					| PDPTE_READWRITE
					| PDPTE_PRESENT;

				PDE* pd = getPDAddress(mapping.PML4_offset, mapping.PDPT_offset);
				zeroPage(pd);
			}

			PDE* pde = getPDEAddress(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset);

			if ((pde->raw & PDE_PRESENT) == 0) {
				void* page = PhysicalMemory::Allocate();
				if (page == nullptr) {
					return nullptr;
				}
				pde->raw =
					(PhysicalMemory::FilterAddress(page) & PDE_PRESENT)
					| PDE_READWRITE
					| PDE_PRESENT;

				PTE* pt = getPTAddress(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset);
				zeroPage(pt);
			}

			PTE* pte = getPTEAddress(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset, mapping.PT_offset);

			if ((pte->raw & PTE_PRESENT) == 0) {
				pte->raw = (PhysicalMemory::FilterAddress(configurationAddress) & PTE_ADDRESS)
					| PTE_PCD
					| PTE_PWT
					| PTE_READWRITE
					| PTE_PRESENT;

				__asm__ volatile("invlpg (%0)" :: "r"(address));
				return reinterpret_cast<void*>(address);
			}
		}

		return nullptr;
	}

	StatusCode UnmapPCIConfiguration(void* vconfigurationAddress) {
		VirtualAddress PCIMapping = parseVirtualAddress(vconfigurationAddress);

		PML4E* pml4e = getPML4EAddress(PCIMapping.PML4_offset);
		if ((pml4e->raw & PML4E_PRESENT) == 0) {
			return StatusCode::INVALID_PARAMETER;
		}

		PDPTE* pdpte = getPDPTEAddress(PCIMapping.PML4_offset, PCIMapping.PDPT_offset);
		if ((pdpte->raw & PDPTE_PRESENT) == 0) {
			return StatusCode::INVALID_PARAMETER;
		}

		PDE* pde = getPDEAddress(PCIMapping.PML4_offset, PCIMapping.PDPT_offset, PCIMapping.PD_offset);
		if ((pde->raw & PDE_PRESENT) == 0) {
			return StatusCode::INVALID_PARAMETER;
		}

		PTE* pte = getPTEAddress(PCIMapping.PML4_offset, PCIMapping.PDPT_offset, PCIMapping.PD_offset, PCIMapping.PT_offset);
		pte->raw = 0;

		__asm__ volatile("invlpg (%0)" :: "r"(vconfigurationAddress));
		return StatusCode::SUCCESS;
	}
}
