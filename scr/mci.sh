#!/bin/sh
# SPDX-License-Identifier: BSD-3-Clause
# Copyright (C) 2026 David Sugar <tychosoft@gmail.com>

if [ "$#" -gt 1 ]; then
    echo "usage: mci [target]" >&2
    exit 1
fi

set +e
INSTALL=/usr/local
config=""
if [ ! -w /usr/local ]; then
    INSTALL="$HOME/.local"
    config="$cmake -D CMAKE_INSTALL_PREFIX=$HOME/.local"
fi

# bootstrap support
if [ -d .git ]; then
    ROOT="."
else
    ROOT=$(execdir -rq)
fi

# non-git nci orihect
if [ -z "$ROOT" ]; then
    ROOT=$(execdir -p CMakeLists.txt -rq) ; fi

if [ -z "$ROOT" ]; then
    echo "fatal: no root project" >&2 ; exit 1 ; fi

case $(uname) in
    MINGW64_NT-*)
        INSTALL="$HOME/.local"
        config="$cmake -D CMAKE_INSTALL_PREFIX=$HOME/.local"
        cmake=$(which cmake)  >/dev/null 2>&1
        ;;
    Darwin)
        cmake=/usr/local/bin/cmake
        ;;
    *BSD)
        cmake=/usr/local/bin/cmake
        if [ -x /usr/pkg/bin/cmake ]; then
            cmake=/usr/pkg/bin/cmake ; fi
        ;;
    *)
        if [ -x /usr/bin/cmake3 ]; then
            cmake=/usr/bin/cmake3
        elif [ -x /usr/local/bin/cmake ]; then
            cmake=/usr/local/bin/cmake
        else
            cmake=$(which cmake)
        fi
        ;;
    
esac

ninja=$(which ninja 2>/dev/null)
if test ! -z "$ninja" ; then
    config="$config -GNinja" ; fi

if test -z "$make" ; then
    make="/usr/bin/make" ; fi

if [ -f "$HOME/.mcirc" ]; then
    # shellcheck disable=SC1091
    . "$HOME/.mcirc" ; fi

if [ -f "$ROOT/../.mcirc" ]; then
    # shellcheck disable=SC1091
    . "$ROOT/../.mcirc" ; fi

if [ -f "$ROOT/.mcirc" ]; then
    # shellcheck disable=SC1091
    . "$ROOT/.mcirc" ; fi

if [ ! -f "$ROOT/CMakeLists.txt" ]; then
    echo "fatal: nonly supports cmake projects" >&2 ; fi

set -e
cd "$ROOT"
ROOT="."
target="$1"
if [ -z "$target" ]; then  target="all" ; fi
case "$target" in
    install|in|-i)
        if [ ! -d "$INSTALL" ]; then
            echo "fatal: no install" >&2 ; exit 1 ; fi
        target="install"
        ;;
    uninstall|-u)
        if [ ! -d "$INSTALL" ]; then
            echo "fatal: no install" >&2 ; exit 1 ; fi
        if [ ! -f install_manifest.txt ]; then
            echo "fatal: not installed" >&2 ; fi
        target="uninstall"
        ;;
    cpp|cppcheck|-lc)
        target="cppcheck"
        ;;
    shell|sh|-s|-ls)
        target="lint_shell"
        ;;
    lint|-l)
        target="lint"
        ;;
    build|-b)
        rm -f CMakeCache.txt
        target="all"
        ;;
    test|-t)
        target="test"
        ;;
    dist|-d)
        target="dist"
        ;;
    all|-a)
        target="all"
        ;;
    format|-f)
        target="format"
        ;;
    refresh|-cr|-rc)
        if [ -f CMakeCache.txt ]; then
            rm -f CMakeCache.txt
            rm -f Makefile
        fi
        target="all"
        ;;
    purge|distclean|-dc)
        echo "Purge"
        if [ -f CMakeCache.txt ]; then
            "$cmake" --build . --target "clean"
            rm -f CMakeCache.txt
            rm -f Makefile
        fi
        exit 0
        ;;   
    config|-c|clean)
        echo "Config"
        if [ -f CMakeCache.txt ]; then
            "$cmake" --build . --target "clean"
            rm -f CMakeCache.txt
            rm -f Makefile
        fi
        # shellcheck disable=SC2086
        exec "$cmake" $config -DCMAKE_BUILD_TYPE=Debug .
        target="clean"
        ;;
    debug|-bd)
        if [ -f CMakeCache.txt ]; then
            "$cmake" --build . --target "clean" ; fi
        rm -f Makefile
        rm -f  CMakeCache.txt 
        # shellcheck disable=SC2086
        "$cmake" $config -DCMAKE_BUILD_TYPE=Debug .
        target="all"
        ;;
    release|-br|-r)
        if [ -f CMakeCache.txt ]; then
            "$cmake" --build . --target "clean" ; fi
        rm -f Makefile
        rm -f  CMakeCache.txt 
        # shellcheck disable=SC2086
        "$cmake" $config -DCMAKE_BUILD_TYPE=RelMinSize .
        target="all"
        ;;
    local|-ri)
        if [ ! -d "$INSTALL" ]; then
            echo "fatal: no install" >&2 ; exit 1 ; fi
        if [ -f CMakeCache.txt ]; then
            "$cmake" --build . --target "clean" ; fi
        rm -f Makefile
        rm -f CMakeCache.txt 
        # shellcheck disable=SC2086
        exec "$cmake" $config -DCMAKE_BUILD_TYPE=RelMinSize .
        ;;
    profile|-bp|-p)
        if [ -f CMakeCache.txt ]; then
            "$cmake" --build . --target "clean" ; fi
        rm -f Makefile
        rm -f CMakeCache.txt 
        # shellcheck disable=SC2086
        "$cmake" $config -DCMAKE_BUILD_TYPE=Profile .
        target="all"
        ;;

    *)
        echo "fatal: unknown target: $target" >&2
        exit 1
        ;;
esac

echo "Building $target"
if [ ! -f CMakeCache.txt ]; then
    rm -f Makefile
    # shellcheck disable=SC2086
    "$cmake" $config -DCMAKE_BUILD_TYPE=Debug .
fi
exec "$cmake" --build . --target "$target"


