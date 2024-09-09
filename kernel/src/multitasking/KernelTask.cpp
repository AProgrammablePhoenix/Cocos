#include <cstddef>
#include <cstdint>

#include <interrupts/KernelPanic.hpp>
#include <mm/Heap.hpp>
#include <mm/VirtualMemory.hpp>
#include <mm/VirtualMemoryLayout.hpp>
#include <multitasking/KernelTask.hpp>
#include <multitasking/Task.hpp>

namespace {
	extern "C" uint8_t userembedded_start[];
	extern "C" uint8_t userembedded_end[];

	const uint8_t* taskImageStartPtr = reinterpret_cast<uint8_t*>(userembedded_start);
	const uint8_t* taskImageEndPtr = reinterpret_cast<uint8_t*>(userembedded_end);
	const size_t taskImageSize = taskImageEndPtr - taskImageStartPtr;

	constexpr VirtualMemory::VirtualAddress secondaryMapping = VirtualMemory::parseVirtualAddress(VirtualMemoryLayout::SECONDARY_RECURSIVE_PML4);
	VirtualMemory::PML4E* secondaryPML4 = VirtualMemory::getPML4EAddress(secondaryMapping.PML4_offset);

	__attribute__((noinline)) static inline void* setupTaskPages() {
		void* CR3 = PhysicalMemory::Allocate();
		if (CR3 == nullptr) {
			return nullptr;
		}

		if (VirtualMemory::SetupTask(CR3) != VirtualMemory::StatusCode::SUCCESS) {
			return nullptr;
		}
		// secondary mapping is now set to CR3, all high memory kernel structures are now set up.

		// DMA shared region

		void* page = PhysicalMemory::Allocate();
		if (page == nullptr) {
			return nullptr;
		}

		VirtualMemory::PML4E* DMA_PML4E = VirtualMemory::getPML4EAddress<false>(0);
		DMA_PML4E->raw = (PhysicalMemory::FilterAddress(page) & VirtualMemory::PML4E_ADDRESS)
			| (VirtualMemory::PML4E_USERMODE)
			| (VirtualMemory::PML4E_READWRITE)
			| (VirtualMemory::PML4E_PRESENT);

		page = PhysicalMemory::Allocate();
		if (page == nullptr) {
			return nullptr;
		}

		VirtualMemory::PDPTE* DMA_PDPTE = VirtualMemory::getPDPTEAddress<false>(0, 0);
		__asm__ volatile("invlpg (%0)" :: "r"(DMA_PDPTE));
		DMA_PDPTE->raw = (PhysicalMemory::FilterAddress(page) & VirtualMemory::PDPTE_ADDRESS)
			| (VirtualMemory::PDPTE_USERMODE)
			| (VirtualMemory::PDPTE_READWRITE)
			| (VirtualMemory::PDPTE_PRESENT);

		page = PhysicalMemory::Allocate();
		if (page == nullptr) {
			return nullptr;
		}

		/// TODO: allocate every single DMA Page Table (even if nothing is in it) at kernel start

		VirtualMemory::PDE* DMA_PDE = VirtualMemory::getPDEAddress<false>(0, 0, 0);
		__asm__ volatile("invlpg (%0)" :: "r"(DMA_PDE));
		DMA_PDE->raw = (PhysicalMemory::FilterAddress(page) & VirtualMemory::PDE_ADDRESS)
			| (VirtualMemory::PDE_USERMODE)
			| (VirtualMemory::PDE_READWRITE)
			| (VirtualMemory::PDE_PRESENT);

		constexpr uint64_t DMA_PTs = VirtualMemoryLayout::DMA_ZONE_SIZE / (VirtualMemory::PT_ENTRIES * VirtualMemory::PTE_COVERAGE);
		static_assert(DMA_PTs < VirtualMemory::PD_ENTRIES, "Legacy DMA Zone is too large (the current size is larger than 1 GB)");

		for (size_t i = 0; i < VirtualMemoryLayout::DMA_ZONE_SIZE / (VirtualMemory::PT_ENTRIES * VirtualMemory::PTE_COVERAGE); ++i) {
			VirtualMemory::PTE* current = VirtualMemory::getPTEAddress(0, 0, 0, i);
			VirtualMemory::PTE* shared = VirtualMemory::getPTEAddress<false>(0, 0, 0, i);
			shared->raw = current->raw;
		}

		// share the current kernel tasks space with the new one

		uint64_t remaining = taskImageSize;
		uint64_t address = reinterpret_cast<uint64_t>(taskImageStartPtr);

		if (address % VirtualMemory::PML4E_COVERAGE != 0) {
			Panic::Panic("Kernel tasks code/data must be aligned on a 512 GB boundary");
		}
		else if (taskImageSize % VirtualMemory::PTE_COVERAGE != 0) {
			Panic::Panic("Kernel tasks code/data size must be aligned on a KB boundary");
		}

		while (remaining >= VirtualMemory::PML4E_COVERAGE) {
			VirtualMemory::VirtualAddress mapping = VirtualMemory::parseVirtualAddress(address);
			VirtualMemory::PML4E* current = VirtualMemory::getPML4EAddress(mapping.PML4_offset);
			VirtualMemory::PML4E* shared = VirtualMemory::getPML4EAddress<false>(mapping.PML4_offset);

			shared->raw = current->raw |= VirtualMemory::PML4E_USERMODE;
			address += VirtualMemory::PML4E_COVERAGE;
			remaining -= VirtualMemory::PML4E_COVERAGE;
		}
		if (remaining > 0) {
			void* _pml4e_page = PhysicalMemory::Allocate();
			if (_pml4e_page == nullptr) {
				return nullptr;
			}
			VirtualMemory::VirtualAddress mapping = VirtualMemory::parseVirtualAddress(address);
			VirtualMemory::PML4E* _pml4e = VirtualMemory::getPML4EAddress<false>(mapping.PML4_offset);
			_pml4e->raw = (PhysicalMemory::FilterAddress(_pml4e_page) & VirtualMemory::PML4E_ADDRESS)
				| VirtualMemory::PML4E_USERMODE
				| VirtualMemory::PML4E_READWRITE
				| VirtualMemory::PML4E_PRESENT;
			
			__asm__ volatile("invlpg (%0)" :: "r"(VirtualMemory::getPDPTAddress<false>(mapping.PML4_offset)));

			while (remaining >= VirtualMemory::PDPTE_COVERAGE) {
				mapping = VirtualMemory::parseVirtualAddress(address);
				VirtualMemory::PDPTE* current = VirtualMemory::getPDPTEAddress(mapping.PML4_offset, mapping.PDPT_offset);
				VirtualMemory::PDPTE* shared = VirtualMemory::getPDPTEAddress<false>(mapping.PML4_offset, mapping.PDPT_offset);

				shared->raw = current->raw |= VirtualMemory::PDPTE_USERMODE;
				address += VirtualMemory::PDPTE_COVERAGE;
				remaining -= VirtualMemory::PDPTE_COVERAGE;
			}
			if (remaining > 0) {
				void* _pdpte_page = PhysicalMemory::Allocate();
				if (_pdpte_page == nullptr) {
					return nullptr;
				}
				mapping = VirtualMemory::parseVirtualAddress(address);
				VirtualMemory::PDPTE* _pdpte = VirtualMemory::getPDPTEAddress<false>(mapping.PML4_offset, mapping.PDPT_offset);
				_pdpte->raw = (PhysicalMemory::FilterAddress(_pdpte_page) & VirtualMemory::PDPTE_ADDRESS)
					| VirtualMemory::PDPTE_USERMODE
					| VirtualMemory::PDPTE_READWRITE
					| VirtualMemory::PDPTE_PRESENT;

				__asm__ volatile("invlpg (%0)" :: "r"(VirtualMemory::getPDAddress<false>(mapping.PML4_offset, mapping.PDPT_offset)));

				while (remaining >= VirtualMemory::PDE_COVERAGE) {
					mapping = VirtualMemory::parseVirtualAddress(address);
					VirtualMemory::PDE* current = VirtualMemory::getPDEAddress(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset);
					VirtualMemory::PDE* shared = VirtualMemory::getPDEAddress<false>(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset);

					shared->raw = current->raw |= VirtualMemory::PDE_USERMODE;
					address += VirtualMemory::PDE_COVERAGE;
					remaining -= VirtualMemory::PDE_COVERAGE;
				}
				if (remaining > 0) {
					void* _pde_page = PhysicalMemory::Allocate();
					if (_pde_page == nullptr) {
						return nullptr;
					}
					mapping = VirtualMemory::parseVirtualAddress(address);
					VirtualMemory::PDE* _pde = VirtualMemory::getPDEAddress<false>(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset);
					_pde->raw = (PhysicalMemory::FilterAddress(_pde_page) & VirtualMemory::PDE_ADDRESS)
						| VirtualMemory::PDE_USERMODE
						| VirtualMemory::PDE_READWRITE
						| VirtualMemory::PTE_PRESENT;

					__asm__ volatile("invlpg (%0)" :: "r"(VirtualMemory::getPTAddress<false>(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset)));

					while (remaining >= VirtualMemory::PTE_COVERAGE) {
						mapping = VirtualMemory::parseVirtualAddress(address);
						VirtualMemory::PTE* current = VirtualMemory::getPTEAddress(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset, mapping.PT_offset);
						VirtualMemory::PTE* shared = VirtualMemory::getPTEAddress<false>(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset, mapping.PT_offset);

						shared->raw = current->raw |= VirtualMemory::PTE_USERMODE;
						address += VirtualMemory::PTE_COVERAGE;
						remaining -= VirtualMemory::PTE_COVERAGE;
					}
				}
			}
		}

		return CR3;
	}

	__attribute__((noinline)) static inline void* setupTaskContext(void* IP) {
		void* const absoluteContextPtr = reinterpret_cast<void*>(VirtualMemoryLayout::KERNEL_STACK_RESERVE);
		const VirtualMemory::VirtualAddress mapping = VirtualMemory::parseVirtualAddress(absoluteContextPtr);
		VirtualMemory::PTE* CtxPTE = VirtualMemory::getPTEAddress<false>(mapping.PML4_offset, mapping.PDPT_offset, mapping.PD_offset, mapping.PT_offset);

		void* physicalFrame = reinterpret_cast<void*>(CtxPTE->raw & VirtualMemory::PTE_ADDRESS);
		void* mappedFrame = VirtualMemory::MapGeneralPage(physicalFrame);
		if (mappedFrame == nullptr) {
			return nullptr;
		}

		struct _TaskCtx {
			void* InstructionPointer;
			uint64_t CodeSelector;
			uint64_t RFlags;
			void* RSP;
			uint64_t StackSelector;
		} *const TaskContext = static_cast<_TaskCtx*>(mappedFrame);

		TaskContext->InstructionPointer = IP;
		TaskContext->CodeSelector = 0x1B;
		TaskContext->RFlags = 0x200; // interrupts enabled
		TaskContext->RSP = reinterpret_cast<void*>(VirtualMemoryLayout::USER_STACK + VirtualMemoryLayout::USER_STACK_SIZE);
		TaskContext->StackSelector = 0x23;

		VirtualMemory::UnmapGeneralPage(mappedFrame);

		return absoluteContextPtr;
	}
}

namespace Multitasking {
	Task* setupKernelTask(void* entryPointPtr) {
		void* TaskCR3 = setupTaskPages();
		if (TaskCR3 == nullptr) {
			return nullptr;
		}

		void* KernelStackPointer = setupTaskContext(entryPointPtr);
		if (KernelStackPointer == nullptr) {
			return nullptr;
		}

		Task* task = reinterpret_cast<Task*>(Heap::Allocate(sizeof(Task)));
		if (task == nullptr) {
			return nullptr;
		}

		task->prev = nullptr;
		task->next = nullptr;
		task->CR3 = TaskCR3;
		task->InstructionPointer = entryPointPtr;
		task->KernelStackTop = KernelStackPointer;
		task->TaskID = Task::taskCount();

		return task;
	}

	StatusCode loadKernelTask(void* entryPointPtr) {
		Task* task = setupKernelTask(entryPointPtr);
		return Task::addTask(task);
	}
}
