#!/bin/sh
# SPDX-License-Identifier: BSD-3-Clause
# Copyright (C) 2026 David Sugar <tychosoft@gmail.com>

openssl=$(trypath -q openssl)
if [ -z "$openssl" ] ; then
    echo "fatal: openssl missing"  >&2 ; exit 1 ; fi

host=$(hostname)
genorator="ecparam -genkey -name secp384r1"
digest="-sha256"
days="3560"
name="server"
certs=$(${openssl} version -d | sed 's/.*"\(.*\)"/\1/')
remove=""
force=""
print=""
path=""
while [ $# -gt 0 ]; do
    case "$1" in
        -q)
            genorator="genpkey -algorithm ML-DSA-65"
            digest=""
            ;;
        -r|-remove)
            remove="y"
            ;;
        -f|-force)
            force="y"
            ;;
        -d|-days)
            shift
            days="$1"
            if [ -z "$days" ] ; then
                echo "fatal: days missing"  >&2 ; exit 1 ; fi
            ;;
        -h|-host)
            shift
            host="$1"
            if [ -z "$host" ] ; then
                echo "fatal: host missing"  >&2 ; exit 1 ; fi
            ;;
        -p|-path)
            shift
            path="$1"
            if [ -z "$path" ] ; then
                echo "fatal: path missing"  >&2 ; exit 1 ; fi
            ;;
        -x|-exec)
            echo "$openssl"
            ;;
        -o|-output|-print)
            print="y"
            ;;
        -2|-sha256)
            digest="-sha256"
            ;;
        -5|-sha512)
            digest="-sha512"
            ;;
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

if test ! -z "$*" ; then
    name="$*" ; fi

pem="$certs/certs/$name.pem"
key="$certs/private/$name.key"
if [ ! -z "$path" ] ; then
    pem="$path/$name.pem"
    key="$path/$name.key"
fi

if [ ! -z "$print" ] ; then
    echo "$pem"
    echo "$key"
    exit 0
fi

if [ ! -z "$force$remove" ] ; then
    rm -f "$pem"
    rm -f "$key"
fi

if [ ! -z "$remove" ] ; then
    exit 0 ; fi

if [ -f  "$key" ] ; then
    echo "fatal: exists:$key" >&2 ; exit 1 ; fi

set -e
# shellcheck disable=SC2086
"$openssl" $genorator -out "$key"
"$openssl" req -new -x509 $digest -key "$key" -out "$pem" -days "$days" -subj "/CN=$host"
echo "self-signed $name for $host"

