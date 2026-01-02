# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

add_executable(plist plist.c)
target_link_libraries(plist PRIVATE local)

add_executable(ifaddr ifaddr.c)
target_link_libraries(ifaddr PRIVATE local)

add_executable(rand rand.c)
target_link_libraries(rand PRIVATE local ${MINICRYPT_LIBRARIES})

add_executable(cls cls.c)
target_link_libraries(cls PRIVATE local ${SIOT_LIBRARIES})

install(TARGETS plist rand ifaddr cls DESTINATION ${CMAKE_INSTALL_BINDIR})

