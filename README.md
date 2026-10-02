# Kaigara - 介殻

A small collection of miscellaneous portable command line system utilities and
a testing ground for standard system practices. This includes newer versions of
classic utilities that can use a better network security model that emphasizes
zero-trust networking with wireguard / tailscale rather than adding crappy user
mode SSL stacks everywhere. Even used locally even WIFI becomes secure from
external spying. 

This package only focuses on system utilities and library code I use for this
security model. Ideas of how such secure private network systems can safely
interact with the public interact are to be explored in other packages such as
Yakuza and secure proxy servers like Troll. Wireguard / Tailscale also means I
can directly use motive socket calls even for transport security. Using SSL would become redundant and computationally wasteful.

In offering standardized cross-platform tooling this builds with Windows using
MSYS to produce stand-alone Windows binaries as well as on posix platforms. it
also offers some shell scripting I use to standardize my own development
practices while working blind and some utilities are meant to support
development for some of my other packages. I principally develop on Alpine
Linux, though I do test on BSD, I also use the MinGW cross-compiler on Alpine
to test producing Windows compatible binaries.

## Configuration

This project uses cmake for configuration. It can be used directly with cmake
or with a common IDE such as VSC, qtcreator, and CLion. This project supports
in-tree, out-off-tree, and in-source builds  For in-source builds, the mci
utility can be used to manage all builds and makes after bootstrap assuming
you have write access to the /usr/local prefix you are using. The ~/.local
prefix prefix may alternately be used.  Bootstrapping from a new checkout is
then like this:

```
git clone karageira repo
sh scr/mci.sh -i
```

You can then use **mci** to manage all builds from the shell rather than cmake
and make / ninja. This is how I manage my project builds locally.

## Dependencies

Kaigara vendors its primary dependencies; libcpr and minicrypt. Kaigara assumes
a C17 (or later) C compiler with threading support is being used. Kaigara can
build and run on any POSIX compliant platform, including MingW32 and MSYS. Ninja
build is recommended to optimize parallel builds.

## Distribution

Distributions of this package are provided as detached source tarballs made
from a tagged release from our internal source repository. These stand-alone
detached tarballs can be used to make packages for many GNU/Linux systems, and
for BSD ports. They may also be used to build and install the software directly
on a target platform.

## Licensing

Kaigara Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

This project collectively is licensed using the GNU GPL 3.0 or later. There may
be some constituent parts under other licenses such as Apache 3.0, BSD. MIT, or
a similar license. These are all GPU 3.0 compatible licenses and In a
collective work these are promoted to and treated as GNU GPL 3.0.

This program is free software: you can redistribute it and/or modify it
effectively as a collective work under the terms of the GNU General Public
License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

## Testing

There are testing programs for testing various library functions. These run
simple tests that will be expanded to improve code coverage over time. The test
programs and the utilise may also be used as examples of how a given API
works. There is a **lint** target that can be used to verify code changes.

## Vendoring

A number of external libraries are included and linked statically. I do this
for libraries that have been well vetted and also publicly available as
stand-alone builds for years but have never been packaged. This package vendors
libcpr and minicrypt.

