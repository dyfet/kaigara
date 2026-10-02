# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

include(CheckCXXSourceCompiles)
include(CheckIncludeFileCXX)
include(CheckFunctionExists)
include(FindPkgConfig)

if(MSVC)
    message(FATAL_ERROR "MSVC not supported; use MingW32")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    include(CheckIncludeFile)
    check_include_file("uuid/uuid.h" HAVE_UUID_UUID_H)
    if(NOT HAVE_UUID_UUID_H)
        message(FATAL_ERROR  "Missing uuid/uuid.h")
    endif()
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
