#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 David Sugar <tychosoft@gmail.com>

while [ $# -gt 0 ]; do
    case "$1" in
        -*) # unknown long option
            echo "fatal: unrecognized argument: ${1}.."  >&2
            exit 1
            ;;
        *)
            break   # first non-option → stop parsing
            ;;
    esac
done

if [ "$#" -gt 0 ]; then
    echo "usage: local-install" >&2
    exit 1
fi

prefix=$(execdir -p .git) ; if test -z "$prefix" ; then exit 1 ; fi
cd "$prefix" || (echo "fatal: cannot access: $prefix" 2>&1 ; exit 1)
if test ! -f CMakeCache.txt ; then
    echo "fatal: not in tree project" >&2 ; exit 1 ; fi

install=$(grep '^CMAKE_INSTALL_PREFIX:' CMakeCache.txt | head -1 | cut -d= -f2)
if test -w "$install" ; then
    exec cmake --build . --target install ; exit 1 ; fi
exec sudo cmake --build . --target install
exit 1




