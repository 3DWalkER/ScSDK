include(CheckCXXSourceCompiles)
include(CheckCXXSourceRuns)
include(CheckFunctionExists)
include(CheckIncludeFileCXX)
include(CheckSymbolExists)
include(CheckTypeSize)
include(CheckCXXCompilerFlag)

if (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
	CHECK_INCLUDE_FILE_CXX(malloc_np.h SC_USE_JEMALLOC)
else()
	CHECK_INCLUDE_FILE_CXX(jemalloc/jemalloc.h SC_USE_JEMALLOC)
endif()

check_cxx_source_runs("
	extern \"C\" int sc_example_undefined_weak_symbol() __attribute__((weak));
	int main(int argc, char** argv) {
		auto f = sc_example_undefined_weak_symbol; // null pointer
		return f ? f() : 0; // must compile, link, and run with null pointer
	}"
	SC_HAS_WEAK_SYMBOLS
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/scconfig.h.cmake
	${CMAKE_CURRENT_SOURCE_DIR}/include/scutils/utils/scconfig.h
)