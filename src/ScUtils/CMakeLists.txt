# Load project configuration and validation scripts
include(cmake/scutils_config_checks.cmake)

# Load project-specific external dependencies
sc_load_external_deps()

# Define main project utility library with dependencies
sc_add_project(${SC_SDK_UTILS} "scutils" "utility library for the Sc C++ SDK"
	DEPENDS cjson spdlog
)

# Add bundled fmtlib source file from spdlog
set(SC_BUNDLED_FMT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/deps/spdlog/src/bundled_fmtlib_format.cpp")
target_sources(${PROJECT_NAME} PRIVATE "${SC_BUNDLED_FMT_SRC}")
source_group("Source Files\\io" FILES "${SC_BUNDLED_FMT_SRC}")

# Preprocessor definitions for library export and fmtlib symbol visibility
target_compile_definitions(${PROJECT_NAME} PRIVATE SC_API_EXPORTS FMT_LIB_EXPORT)