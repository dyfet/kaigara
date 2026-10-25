// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#ifndef _WIN32
#define _XOPEN_SOURCE 700
#include <locale.h>
#include <langinfo.h>
#include <stdio.h>
#include <locale.h>
#endif

#include <local/con.h>
#include <cpr/system.h>
#include <getopt.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "")) != -1) {
        switch (opt) {
        default:
        usage:
            cpr_crit(-1, "usage: cls [-r]");
        }
    }
    if (optind != argc) goto usage;
    if (!con_istty()) cpr_crit(-1, "fatal: not tty");

#ifdef _WIN32
    UINT cp = GetConsoleOutputCP();
    if (cp == 65001)
        printf("UTF-8\n");
    else
        printf("%u\n", cp);
#else
    setlocale(LC_CTYPE, "");
    printf("%s\n", nl_langinfo(CODESET));
#endif
    return 0;
}
