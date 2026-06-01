#include "scutils/memory/scmalloc.h"

SC_BEGIN_NAMESPACE

SC_BEGIN_DETAIL_NAMESPACE

namespace
{
	struct DoUsingJEMallocInitializer
	{
		bool operator()() const noexcept
		{
			if (mallocx == nullptr || rallocx == nullptr || xallocx == nullptr ||
				sallocx == nullptr || dallocx == nullptr || sdallocx == nullptr ||
				nallocx == nullptr || mallctl == nullptr ||
				mallctlnametomib == nullptr || mallctlbymib == nullptr) {
				return false;
			}

			volatile uint64_t* counter;
			size_t counterLen = sizeof(uint64_t*);
			if (mallctl("thread.allocatedp", static_cast<void*>(&counter), &counterLen, nullptr, 0) != 0)
				return false;

			if (counterLen != sizeof(uint64_t*))
				return false;

			uint64_t origAllocated = *counter;
			static void* volatile ptr = malloc(1);
			if (!ptr)
				return false;
			free(ptr);
			return (origAllocated != *counter);
		}
	};
}

bool ScUsingJEMallocInitializer::operator()() const noexcept {
	using Initializer = DoUsingJEMallocInitializer;
	return detail::ScFastStaticBool<Initializer>::get(std::memory_order_relaxed);
}

inline bool getTCMallocNumericProperty(const char* name, size_t* out) noexcept {
	return MallocExtension_Internal_GetNumericProperty(name, strlen(name), out);
}

namespace
{
	struct DoUsingTCMallocInitializer
	{
		bool operator()() const noexcept
		{
			if (MallocExtension_Internal_GetNumericProperty == nullptr || sdallocx == nullptr || nallocx == nullptr)
				return false;

			static const char kAllocBytes[] = "generic.current_allocated_bytes";
			size_t before_bytes = 0;
			getTCMallocNumericProperty(kAllocBytes, &before_bytes);

			static void* volatile ptr = malloc(1);
			if (!ptr)
				return false;

			size_t after_bytes = 0;
			getTCMallocNumericProperty(kAllocBytes, &after_bytes);
			free(ptr);

			return (before_bytes != after_bytes);
		}
	};
}

bool ScUsingTCMallocInitializer::operator()() const noexcept {
	using Initializer = DoUsingTCMallocInitializer;
	return detail::ScFastStaticBool<Initializer>::get(std::memory_order_relaxed);
}

SC_END_DETAIL_NAMESPACE

SC_END_NAMESPACE