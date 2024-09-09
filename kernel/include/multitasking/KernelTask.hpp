#pragma once

#include <cstdint>

#include <multitasking/Task.hpp>

namespace Multitasking {
	Task* setupKernelTask(void* entryPointPtr);
	StatusCode loadKernelTask(void* entryPointPtr);
}
