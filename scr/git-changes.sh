#!/bin/sh
# SPDX-License-Identifier: BSD-3-Clause
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

if [ "$#" -gt 2 ]; then
    echo "usage: git-changes [tag] [end]" >&2
    exit 1
fi

while [ ! -d .git ]; do
    if [ ! -d ../usr ] ; then
        cd ..
    else
        echo "fatal: No git archive" >&2
        exit 2
    fi
done

if [ -z "$1" ]; then
    tag=$(git describe --abbrev=0 --tags 2>/dev/null)
else
    tag="$1"
fi

end="$2"
if [ ! -z "$end" ]; then
    echo "From $tag To $end"
elif [ ! -z "$tag" ]; then
    echo "Since $tag"
else
    echo "Since start"
fi

if [ -z "$tag" ]; then
    git log --no-merges --pretty=format:"* %s" | sed -e 's/^*/-/'
else
    git log --no-merges --pretty=format:"* %s" "$tag..$end" | sed -e 's/^*/-/'
fi
echo

