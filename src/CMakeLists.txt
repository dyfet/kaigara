# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

add_executable(plist plist.c)
target_link_libraries(plist PRIVATE local)

add_executable(mkpath mkpath.c)
target_link_libraries(mkpath PRIVATE local)

add_executable(fnlist fnlist.c)
target_link_libraries(fnlist PRIVATE local)

add_executable(ifaddr ifaddr.c)
target_link_libraries(ifaddr PRIVATE local)

add_executable(hostaddr hostaddr.c)
target_link_libraries(hostaddr PRIVATE local)

add_executable(execdir execdir.c)
target_link_libraries(execdir PRIVATE local)

add_executable(freespc freespc.c)
target_link_libraries(freespc PRIVATE local import_minicrypt)

add_executable(md5-digest md5-digest.c)
target_link_libraries(md5-digest PRIVATE local import_minicrypt)

add_executable(sha1-digest sha1-digest.c)
target_link_libraries(sha1-digest PRIVATE local import_minicrypt)

add_executable(sha2-digest sha2-digest.c)
target_link_libraries(sha2-digest PRIVATE local import_minicrypt)

add_executable(sha2-passwd sha2-passwd.c)
target_link_libraries(sha2-passwd PRIVATE local import_minicrypt)

add_executable(inkey inkey.c)
target_link_libraries(inkey PRIVATE local)

add_executable(choose choose.c)
target_link_libraries(choose PRIVATE local)

add_executable(echoat echoat.c)
target_link_libraries(echoat PRIVATE local)

add_executable(detach detach.c)
target_link_libraries(detach PRIVATE local)

add_executable(attach attach.c)
target_link_libraries(attach PRIVATE local)

add_executable(keydata keydata.c)
target_link_libraries(keydata PRIVATE local)

add_executable(rand rand.c)
target_link_libraries(rand PRIVATE local import_minicrypt)

add_executable(cls cls.c)
target_link_libraries(cls PRIVATE local)

add_executable(xdgtype xdgtype.c)
target_link_libraries(xdgtype PRIVATE local)

add_executable(xdgpath xdgpath.c)
target_link_libraries(xdgpath PRIVATE local)

add_executable(istty istty.c)
target_link_libraries(istty PRIVATE local)

add_executable(mkuuid mkuuid.c)
target_link_libraries(mkuuid PRIVATE local)
if(WIN32 OR MSYS)
    target_link_libraries(mkuuid PUBLIC rpcrt4)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(mkuuid PUBLIC uuid)
endif()

add_executable(codepage codepage.c)
target_link_libraries(codepage PRIVATE local)

add_executable(gopher gopher.c)
target_link_libraries(gopher PRIVATE local)

add_executable(finger finger.c)
target_link_libraries(finger PRIVATE local)

add_executable(hexver hexver.c)
target_link_libraries(hexver PRIVATE local)

add_executable(ipcshell ipcshell.c)
target_link_libraries(ipcshell PRIVATE local)

add_executable(ipcfifo ipcfifo.c)
target_link_libraries(ipcfifo PRIVATE local)

add_executable(rmfifo rmfifo.c)
target_link_libraries(rmfifo PRIVATE local)

add_executable(isfifo isfifo.c)
target_link_libraries(isfifo PRIVATE local)

if(NOT WIN32 AND NOT MSYS) 
    add_executable(ttyprint ttyprint.c)
    target_link_libraries(ttyprint PRIVATE local)

    add_executable(ttyinput ttyinput.c)
    target_link_libraries(ttyinput PRIVATE local)

    install(TARGETS ttyprint ttyinput DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

install(TARGETS mkpath ipcshell ipcfifo rmfifo isfifo codepage mkuuid istty execdir freespc inkey choose echoat keydata hostaddr detach attach finger plist fnlist md5-digest sha1-digest sha2-digest sha2-passwd rand ifaddr cls gopher hexver DESTINATION ${CMAKE_INSTALL_BINDIR})


