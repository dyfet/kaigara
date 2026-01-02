# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

cmake_minimum_required(VERSION 3.16.0)
Project(kaigara VERSION 0.0.1 LANGUAGES C)
set(CMAKE_C_STANDARD 17)
set(C_STANDARD_REQUIRED 17)

file(
    GLOB LINT_SOURCES
    RELATIVE ${PROJECT_SOURCE_DIR}
    src/*.h src/*.c lib/local/*.h lib/local/*.c)

include(cmake/custom.cmake OPTIONAL)
include(cmake/project.cmake)
include(cmake/features.cmake)
include(cmake/coverage.cmake)
include(cmake/clint.cmake)
include(cmake/tune.cmake)
include(cmake/deploy.cmake OPTIONAL)

file(GLOB optional .git[a-z]* *.json *.in *.yml .clang* cmake/*)
file(GLOB markdown *.md)

include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/lib)

add_subdirectory(lib)

add_compile_definitions(
    PROJECT_VERSION="${PROJECT_VERSION}"
    TUNE_FAMILY_INET=${TUNE_FAMILY_INET}
)

add_subdirectory(src)

if(WIN32)
    include(InstallRequiredSystemLibraries)
    set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md")
    set(CPACK_PACKAGE_VERSION_MAJOR "${CMAKE_PROJECT_VERSION_MAJOR}")
    set(CPACK_PACKAGE_VERSION_MINOR "${CMAKE_PROJECT_VERSION_MINOR}")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "Kaigama")
#    set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/Kaigama.ico")
    set(CPACK_GENERATOR "INNOSETUP")
    include(CPack)
endif()
