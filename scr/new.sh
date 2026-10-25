#!/bin/sh
# SPDX-License-Identifier: BSD-3-Clause
# Copyright (C) 2026 David Sugar <tychosoft@gmail.com>

template() {
    YEAR=$(date +%Y)
    if [ -z "$AUTHOR" ]; then
        AUTHOR=$(getent passwd "$USER" | cut -d: -f5 | cut -d, -f1) ; fi
    exec sed \
        -e "s/!!AUTHOR!!/$AUTHOR/g" \
        -e "s/!!EMAIL!!/$EMAIL/g" \
        -e "s/!!USER!!/$USER/g" \
        -e "s/!!YEAR!!/$YEAR/g" \
        -e "s/!!NAME!!/$NAME/g" \
        -e "s/!!HEADER!!/$HEADER/g" 
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
hdrext=""
file=$(basename "$target")
path=$(dirname "$target")
path=$(realpath "$path")
header="$file"
if [ -f "$target" ] ; then echo "fatal: exists: ${1}" ; exit 1 ; fi
case "$file" in
    *.c)
        source="code.c"
        header=$(basename "$file" .c)
        hdrext=".h"
        case "$path" in 
        */test|*/tests)
            source="test.c"
            ;;
        esac
        ;;
    *.h)
        source="code.h"
        header=$(basename "$file" .h)
        hdrext=".h"
        case "$path" in
        */test|*/tests)
            source="test.h"
            ;;
        esac
        ;;
    *.cpp|*.c++)
        source="code.cpp"
        header=$(basename "$file" .cpp)
        hdrext=".hpp"
        case "$path" in
        */test|*/tests)
            source="test.cpp"
            ;;
        esac
        ;;
    *.hpp|*.h++)
        source="code.hpp"
        header=$(basename "$file" .hpp)
        hdrext=".hpp"
        case "$path" in
        */test|*/tests)
            source="test.hpp"
            ;;
        esac
        ;;
    *.m)
        source="code.m"
        ;;
    *.go)
        source="code.go"
        case "$file" in
        *_test.go)
            source="test.go"
            ;;
        esac
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

NAME=$(printf "%s" "$header" | tr '[:lower:]' '[:upper:]')
HEADER="$header$hdrext"
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

