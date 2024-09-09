#pragma once

#include <cstddef>
#include <cstdint>

namespace Multitasking {
	enum class StatusCode {
		SUCCESS,
		INVALID_PARAMETER,
		FAILED
	};

	struct Task {
		Task* prev = nullptr;
		Task* next = nullptr;
		void* CR3 = nullptr;
		void* InstructionPointer = nullptr;
		void* KernelStackTop = nullptr;
		uint64_t TaskID = 0;

		static uint64_t taskCount();
		static StatusCode addTask(Task* task);
		static Task* taskSwitch();
	};
}
