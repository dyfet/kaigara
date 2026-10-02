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
    echo "usage: bye" >&2
    exit 1
fi

case $(tty) in
/dev/tty00|/dev/constty|/dev/ttyu0)
    exec sudo halt -p
    ;;
esac
