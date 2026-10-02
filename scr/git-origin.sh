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
    shift
done

if [ "$#" -eq 0 ] ; then
    git remote -v | grep ^origin | grep "[(]fetch[)]" | cut -f2 | cut -d ' ' -f1
    exit 0
fi

if [ "$#" -ne 1 ] || [ -z "$1" ]; then
    echo "usage: git-origin [remote]" >&2
    exit 1
fi

origin=$(git remote -v | grep ^origin | grep "[(]fetch[)]" | cut -f2 | cut -d ' ' -f1)
current=$(git branch -a | grep "^[*] " | cut -d ' ' -f2)
target="$1"
if [ ! -z "$origin" ]; then
    git remote rm origin ; fi

if git remote add origin "$target" ; then
    git update origin "$current" 2>/dev/null
    git push --set-upstream --tags origin "$current"
    exit 0
fi

