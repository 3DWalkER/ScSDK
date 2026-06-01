if(@PROJECT_NAME@_FOUND)
	return()
endif()

if(NOT TARGET @PROJECT_NAME@::@PROJECT_NAME@)
	include("${CMAKE_CURRENT_LIST_DIR}/@PROJECT_NAME@-targets.cmake")
endif()

# Compute the installation prefix relative to this file.
get_filename_component(SC_SDK_DEFAULT_ROOT_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(SC_SDK_DEFAULT_ROOT_DIR "${SC_SDK_DEFAULT_ROOT_DIR}" PATH)
get_filename_component(SC_SDK_DEFAULT_ROOT_DIR "${SC_SDK_DEFAULT_ROOT_DIR}" PATH)
get_filename_component(SC_SDK_DEFAULT_ROOT_DIR "${SC_SDK_DEFAULT_ROOT_DIR}" PATH)
if(SC_SDK_DEFAULT_ROOT_DIR STREQUAL "/")
	seet(SC_SDK_DEFAULT_ROOT_DIR "")
endif()

set(@PROJECT_NAME@_FOUND TRUE)
set(@PROJECT_NAME@_INCLUDE_DIRS "${SC_SDK_DEFAULT_ROOT_DIR}/include")

set(@PROJECT_NAME@_TARGETS @PROJECT_NAME@::@PROJECT_NAME@)
