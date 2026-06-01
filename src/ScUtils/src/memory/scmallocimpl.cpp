#include "scutils/memory/scmallocimpl.h"

extern "C" {

#ifdef SC_CC_MSVC
	const char* mallocxWeak = nullptr;
	const char* rallocxWeak = nullptr;
	const char* xallocxWeak = nullptr;
	const char* sallocxWeak = nullptr;
	const char* dallocxWeak = nullptr;
	const char* sdallocxWeak = nullptr;
	const char* nallocxWeak = nullptr;
	const char* mallctlWeak = nullptr;
	const char* mallctlnametomibWeak = nullptr;
	const char* mallctlbymibWeak = nullptr;
	const char* MallocExtension_Internal_GetNumericPropertyWeak = nullptr;
#elif !SC_HAS_WEAK_SYMBOLS
#	if (!defined(USE_JEMALLOC) && !defined(SC_USE_JEMALLOC))
	void* (*mallocx)(size_t, int) = nullptr;
	void* (*rallocx)(void*, size_t, int) = nullptr;
	size_t(*xallocx)(void*, size_t, size_t, int) = nullptr;
	size_t(*sallocx)(const void*, int) = nullptr;
	void (*dallocx)(void*, int) = nullptr;
	void (*sdallocx)(void*, size_t, int) = nullptr;
	size_t(*nallocx)(size_t, int) = nullptr;
	int (*mallctl)(const char*, void*, size_t*, void*, size_t) = nullptr;
	int (*mallctlnametomib)(const char*, size_t*, size_t*) = nullptr;
	int (*mallctlbymib)(const size_t*, size_t, void*, size_t*, void*, size_t) =
		nullptr;
#	endif
	bool (*MallocExtension_Internal_GetNumericProperty)(const char*, size_t, size_t*) = nullptr;
#endif
}