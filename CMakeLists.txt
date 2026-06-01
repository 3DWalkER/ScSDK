# Set the minimum version requirement
cmake_minimum_required(VERSION 3.12...3.29)

# CMake called variable
option(SC_BUILD_SHARED_LIBS "If enabled, all sdk libraries will be build as shared objects; otherwise all libraries will be built as static objects" ON)
option(SC_SIMPLE_INSTALL "If enabled, removes all the additional indirection (platform/cpu/config) in the bin and lib directories on the install step" ON)
option(SC_ENABLE_TESTING "Flag to enable/disable building unit and integration tests" ON)

set(SC_BUILD_ONLY "" CACHE STRING "A semi-colon delimited list of the projects to build")

get_filename_component(SC_SDK_NATIVE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)

# Load the built-in dependency module of CMake
include(CMakePackageConfigHelpers)

# Setup user specified installation directory if any, regardless previous platform default settings
if(NOT CMAKE_INSTALL_PREFIX OR CMAKE_INSTALL_PREFIX STREQUAL "")
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Installation prefix" FORCE)
endif()

if(CMAKE_INSTALL_BINDIR)
    set(SC_BINARY_DIRECTORY "${CMAKE_INSTALL_BINDIR}")
else()
    set(SC_BINARY_DIRECTORY "bin")
endif()

if(NOT SC_LIBRARY_DIRECTORY)
    if(CMAKE_INSTALL_LIBDIR)
        set(SC_LIBRARY_DIRECTORY "${CMAKE_INSTALL_LIBDIR}")
    else()
        set(SC_LIBRARY_DIRECTORY "lib")
    endif()
endif()

set(SC_ARCHIVE_DIRECTORY "${SC_LIBRARY_DIRECTORY}")

if(NOT SC_INCLUDE_DIRECTORY)
    if(CMAKE_INSTALL_INCLUDEDIR)
        set(SC_INCLUDE_DIRECTORY "${CMAKE_INSTALL_INCLUDEDIR}")
    else()
        set(SC_INCLUDE_DIRECTORY "include")
    endif()
endif()

# Set project name
project(ScSDK LANGUAGES CXX)

# Set the name of the predefined target folder
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "Built-In")

# Set up C++
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the path of the CMake module
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Load project configuration and validation scripts
include(utilities)
include(initialize_project_version)
include(sdks)
include(ide)

# add subdirectory
sc_add_sdk()