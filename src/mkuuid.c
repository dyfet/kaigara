// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include <cpr/system.h>
#include <cpr/memory.h>
#include <getopt.h>
#include <stdlib.h>

#if defined(_WIN32)
#include <rpc.h>
#elif defined(__linux__)
#include <uuid/uuid.h>
#else
#include <uuid.h>
#endif

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "")) != -1) {
        switch (opt) {
        default:
        usage:
            cpr_crit(-1, "usage: mkuuid");
        }
    }
    if (optind != argc) goto usage;

    char realm[64];
    cpr_memset(&realm, 0, sizeof(realm));
#if defined(_WIN32)
    UUID uuid;
    UuidCreate(&uuid);
    RPC_CSTR tmp = NULL;
    UuidToStringA(&uuid, &tmp);
    cpr_memcpy(realm, sizeof(realm), tmp, 37);
    RpcStringFreeA(&tmp);
#elif defined(__linux__)
    uuid_t uuid;
    uuid_generate(uuid);
    uuid_unparse_lower(uuid, realm);
#else
    uuid_t u;
    uint32_t status;
    char *tmp = NULL;

    uuid_create(&u, &status);
    uuid_to_string(&u, &tmp, &status);

    for (int i = 0; tmp[i]; i++)
        realm[i] = tolower((unsigned char)tmp[i]);
    realm[36] = '\0';
    free(tmp);
#endif
    printf("%s\n", realm);
    return 1;
}
