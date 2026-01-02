# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

include(CheckCXXSourceCompiles)
include(CheckIncludeFileCXX)
include(CheckFunctionExists)
include(FindPkgConfig)

if(MWSVC)
        message(FATAL_ERROR "MSVC not supported; use MingW32")
endif()

if(WIN32 OR MSYS)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")

    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s -Wno-builtin-declaration-mismatch")
    set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG} -Wno-builtin-declaration-mismatch")
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE OR CMAKE_TOOLCHAIN_FILE STREQUAL "")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(CPR libcpr>=0.9.0 QUIET)
    pkg_check_modules(MINICRYPT minicrypt>=00.5.0 QUIET)
    pkg_check_modules(SIOT libsiot>=0.1.0 QUIET)
endif()

if(NOT CPR_FOUND)
    set(CPR_LIBRARIES import_cpr)
endif()

if(NOT MINICRYPT_FOUND)
    set(MINICRYPT_LIBRARIES import_minicrypt)
endif()

if(NOT SIOT_FOUND)
    set(SIOT_LIBRARIES import_siot)
endif()

add_compile_options(
    -ffunction-sections
    -fdata-sections
)

add_link_options(
    -Wl,--no-whole-archive
    -Wl,--gc-sections
    -Wl,--gc-sections
    -Wl,--as-needed
    -Wl,--discard-all
    -Wl,--no-undefined
)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(BUILD_DEBUG true)
    add_definitions(-DDEBUG)
else()
    add_definitions(-DNDEBUG)
endif()
