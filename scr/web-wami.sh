#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 David Sugar <tychosoft@gmail.com>

mode="ip"
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
    echo "usage: web-wami [options]" >&2
    exit 1
fi

curl -s https://ipinfo.io/$mode ; echo
