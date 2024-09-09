#include <cstddef>
#include <cstdint>

#include <mm/VirtualMemory.hpp>
#include <multitasking/Task.hpp>

namespace Multitasking {
	namespace {
		static Task* currentTask = nullptr;
		static uint64_t _taskCount = 0;
	}

	uint64_t Task::taskCount() {
		return _taskCount;
	}

	StatusCode Task::addTask(Task* task) {
		if (task == nullptr
			|| task->TaskID != taskCount()
			|| task->prev != nullptr
			|| task->next != nullptr
			|| task->CR3 == nullptr
			|| task->InstructionPointer == nullptr
			|| task->KernelStackTop == nullptr
		) {
			return StatusCode::INVALID_PARAMETER;
		}

		if (currentTask == nullptr) {
			currentTask = task;
		}
		else {
			task->prev = currentTask->prev;

			if (task->prev == nullptr) {
				task->prev = currentTask;
			}

			task->prev->next = task;

			task->next = currentTask;

			if (task->next == nullptr) {
				task->next = currentTask;
			}
			
			task->next->prev = task;
		}

		++_taskCount;

		return StatusCode::SUCCESS;
	}

	Task* Task::taskSwitch() {
		if (currentTask != nullptr && currentTask->next != nullptr) {
			currentTask = currentTask->next;
		}

		return currentTask;
	}
}
