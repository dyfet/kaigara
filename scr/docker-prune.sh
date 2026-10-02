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
    echo "usage: docker-prune" >&2
    exit 1
fi

case $(uname) in
Darwin)
    if test -d /Applications/Docker.app ; then
        appexe="Docker.app" ; fi
    ;;
*)
    appexe=$(which docker)
    ;;
esac

if test ! -x "$appexe" ; then echo "fatal: no docker" >&2 ; exit 1 ; fi
if pgrep -f "$appexe" >/dev/null 2>&1 ; then
    :  # process exists
else
    exit 0
fi

ids=$(docker ps -a -q --filter status=exited 2>/dev/null)
if test ! -z "$ids" ; then
    docker rm "$ids"
    ids=$(docker images --filter dangling=true -q 2>/dev/null)
    if test ! -z "$ids" ; then docker rmi "$ids" ; fi
fi

