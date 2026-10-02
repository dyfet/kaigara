# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 David Sugar <tychosoft@gmail.com>

configure_file(new.sh new)
file(CHMOD ${CMAKE_CURRENT_BINARY_DIR}/new
     PERMISSIONS OWNER_READ OWNER_EXECUTE
                 GROUP_READ GROUP_EXECUTE
                 WORLD_READ WORLD_EXECUTE)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/new
        DESTINATION ${CMAKE_INSTALL_BINDIR})

configure_file(edit.sh edit)
file(CHMOD ${CMAKE_CURRENT_BINARY_DIR}/edit
     PERMISSIONS OWNER_READ OWNER_EXECUTE
                 GROUP_READ GROUP_EXECUTE
                 WORLD_READ WORLD_EXECUTE)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/edit
        DESTINATION ${CMAKE_INSTALL_BINDIR})


configure_file(mci.sh mci)
file(CHMOD ${CMAKE_CURRENT_BINARY_DIR}/mci
     PERMISSIONS OWNER_READ OWNER_EXECUTE
                 GROUP_READ GROUP_EXECUTE
                 WORLD_READ WORLD_EXECUTE)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/mci
        DESTINATION ${CMAKE_INSTALL_BINDIR})



configure_file(git-changes.sh git-changes)
file(CHMOD ${CMAKE_CURRENT_BINARY_DIR}/git-changes
     PERMISSIONS OWNER_READ OWNER_EXECUTE
                 GROUP_READ GROUP_EXECUTE
                 WORLD_READ WORLD_EXECUTE)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/git-changes
        DESTINATION ${CMAKE_INSTALL_BINDIR})

configure_file(git-origin.sh git-origin)
file(CHMOD ${CMAKE_CURRENT_BINARY_DIR}/git-origin
     PERMISSIONS OWNER_READ OWNER_EXECUTE
                 GROUP_READ GROUP_EXECUTE
                 WORLD_READ WORLD_EXECUTE)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/git-origin
        DESTINATION ${CMAKE_INSTALL_BINDIR})

configure_file(web-wami.sh web-wami)
file(CHMOD ${CMAKE_CURRENT_BINARY_DIR}/web-wami
     PERMISSIONS OWNER_READ OWNER_EXECUTE
                 GROUP_READ GROUP_EXECUTE
                 WORLD_READ WORLD_EXECUTE)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/web-wami
        DESTINATION ${CMAKE_INSTALL_BINDIR})


configure_file(bye.sh bye)
file(CHMOD ${CMAKE_CURRENT_BINARY_DIR}/bye
     PERMISSIONS OWNER_READ OWNER_EXECUTE
                 GROUP_READ GROUP_EXECUTE
                 WORLD_READ WORLD_EXECUTE)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/bye
        DESTINATION ${CMAKE_INSTALL_BINDIR})

configure_file(local-install.sh local-install)
file(CHMOD ${CMAKE_CURRENT_BINARY_DIR}/local-install
     PERMISSIONS OWNER_READ OWNER_EXECUTE
                 GROUP_READ GROUP_EXECUTE
                 WORLD_READ WORLD_EXECUTE)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/local-install
        DESTINATION ${CMAKE_INSTALL_BINDIR})

configure_file(docker-prune.sh docker-prune)
file(CHMOD ${CMAKE_CURRENT_BINARY_DIR}/docker-prune
     PERMISSIONS OWNER_READ OWNER_EXECUTE
                 GROUP_READ GROUP_EXECUTE
                 WORLD_READ WORLD_EXECUTE)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/docker-prune
        DESTINATION ${CMAKE_INSTALL_BINDIR})


