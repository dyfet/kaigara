# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

file(GLOB import_cpr cpr/*.h cpr/*.c)
file(GLOB import_minicrypt minicrypt/*.h minicrypt/*.c)
file(GLOB local_lib local/*.h local/*.c)

add_library(import_minicrypt STATIC ${import_minicrypt})
if(WIN32 OR MSYS)
    target_link_libraries(import_minicrypt PUBLIC advapi32)
endif()

add_library(import_cpr STATIC ${import_cpr})
target_link_libraries(import_cpr PUBLIC Threads::Threads)
if(WIN32 OR MSYS)
    target_link_libraries(import_cpr PUBLIC ws2_32 iphlpapi)
endif()

add_library(local STATIC ${local_lib})
target_link_libraries(local PUBLIC import_cpr)


