#include <cstdint>

#include <interrupts/pit.hpp>
#include <multitasking/Task.hpp>
#include <screen/Log.hpp>

// The handler needs C linkage in order to be called from assembly

namespace {
	struct ExecutionContext {
		void* CR3;
		void* RSP;
	};

	extern "C" ExecutionContext system_timer_event_handler(void) {
		Interrupts::PIT::SYSTEM_TIMER_US += Interrupts::PIT::IRQ0_us;
		Multitasking::Task* task = Multitasking::Task::taskSwitch();
		if (task == nullptr) {
			return ExecutionContext {
				.CR3 = nullptr,
				.RSP = nullptr
			};
		}

		return ExecutionContext {
			.CR3 = task->CR3,
			.RSP = task->KernelStackTop
		};
	}
}
