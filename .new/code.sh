#!/bin/sh
# SPDX-License-Identifier: BSD-3-Clause
# Copyright (C) !!YEAR!! !!AUTHOR!! <!!EMAIL!!>

while [ $# -gt 0 ]; do
    case "$1" in
        -*)
            echo "fatal: unrecognized argument: ${1}.."  >&2
            exit 1
            ;;
        *)
            break
            ;;
    esac
    shift
done

