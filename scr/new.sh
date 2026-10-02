#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 David Sugar <tychosoft@gmail.com>

template() {
    YEAR=$(date +%Y)
    if [ -z "$AUTHOR" ]; then
        AUTHOR=$(getent passwd "$USER" | cut -d: -f5 | cut -d, -f1) ; fi
    exec sed \
        -e "s/!!AUTHOR!!/$AUTHOR/g" \
        -e "s/!!EMAIL!!/$EMAIL/g" \
        -e "s/!!USER!!/$USER/g" \
        -e "s/!!YEAR!!/$YEAR/g"
}

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
    shift
done

if [ "$#" -ne 1 ]; then
    echo "usage: new target" >&2
    exit 1
fi

target="$1"
file=$(basename "$target")
if [ -f "$target" ] ; then echo "fatal: exists: ${1}" ; exit 1 ; fi
case "$file" in
    *.c)
        source="code.c"
        ;;
    *.h)
        source="code.h"
        ;;
    *.cpp|*.c++)
        source="code.cpp"
        ;;
    *.hpp|*.h++)
        source="code.hpp"
        ;;
    *.m)
        source="code.m"
        ;;
    *.go)
        source="code.go"
        ;;
    *.py)
        source="code.py"
        ;;
    *.pl)
        source="code.pl"
        ;;
    *.rs)
        source="code.rs"
        ;;
    *.cs)
        source="code.cs"
        ;;
    *.cmake|CMakeLists.txt)
        source="code.cmake"
        ;;
    *.mk|*.am|*.ac|Makefile)
        source="code.mk"
        ;;
    *.md)
        source="doc.md"
        ;;
    *.txt)
        source="doc.txt"
        ;;
    *.sh)
        source="code.sh"
        ;;
    *.1|*.2|*.3|*.4|*.5|*.6|*.7|*.8|*.9)
        source="doc.man"
        ;;
    *.gz|*.z|*.bin|*.tar|*.ar|*.o|*.obj|*.zip)
        echo "fatal: binary: %target" 2>&1
        exit 1
        ;;
    .*)
        source=$(echo "$file" | sed -e 's/^[.]//')
        ;;
    *)
        source="%file"
        ;;
esac

GIT=$(execdir -q)
if [ ! -z "$GIT" ]; then
    AUTHOR=$(git config --global user.name)
    EMAIL=$(git config --global user.email)
    if [ -f "$GIT/.new/$source" ]; then
        template <"$GIT/.new/$source" >"$target" ; fi
fi

if [ -f ~/.new/"$source" ] ; then
    template <~/.new/"$source" >"$target" ; fi 

echo "fatal: no template: $target"
exit 1

