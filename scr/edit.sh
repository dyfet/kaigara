#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 David Sugar <tychosoft@gmail.com>

mode=""
line=""
while [ $# -gt 0 ]; do
    case "$1" in
		+*)
            if [ ! -z "$line" ]; then
                echo "fatal: line already set" >&2 ; fi
			line="$1"
			;;
        -n|-e|-x)
            if [ ! -z "$mode" ]; then
                echo "fatal: mode already set" >&2 ; fi
            mode="$1"
            ;;
        --)
            shift
            break
            ;;
		-)
			break
			;;
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

if [ -z "$EDITOR" ]; then
    EDITOR="vim" ; fi

if [ $# -ne 1 ]; then
    echo "fatal: incorrect arguments"  >&2 ; exit 1 ; fi

if [ ! -z "$bide" ]; then
    mode="-" ; fi

export EDITOR

if [ -d "$1" ]; then
    echo "fatal: directory: $1" >&2 ; fi

case "$mode" in
-x)
    if [ ! -x "$1" ]; then
        echo "fatal: not found or exec: $1" >&2 ; fi
    ;;
-n)
    if [ ! -f "$1" ]; then
         echo "fatal: exists: $1" >&2 ; fi
    ;;
-e)
    if [ -f "$1" ]; then
         echo "fatal: not found: $1" >&2 ; fi
    ;;
esac

if [ ! -f "$1" ]; then
    new  "$1" >/dev/null 2>&1 ; fi

# shellcheck disable=SC2086
"$EDITOR" $line "$1" 

if [ -f "$1" ]; then
    git add  "$1" >/dev/null 2>&1 ; fi



