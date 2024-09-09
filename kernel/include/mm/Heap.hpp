#pragma once

#include <cstddef>

namespace Heap {
	void* Allocate(size_t size);
	void Free(void* ptr);
}
