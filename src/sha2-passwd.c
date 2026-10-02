// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 David Sugar <tychosoft@gmail.com>

#include <local/con.h>
#include <cpr/system.h>
#include <cpr/memory.h>
#include <local/con.h>
#include <minicrypt/sha256.h>
#include <minicrypt/random.h>
#include <getopt.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    char *opt_prompt = "password:";
    bool opt_salt = false;
    int opt;
    while ((opt = getopt(argc, argv, "p:s")) != -1) {
        switch (opt) {
        case 'p':
            opt_prompt = optarg;
            break;
        case 's':
            opt_salt = true;
            break;
        default:
        usage:
            cpr_crit(-1, "usage: cls [-s] [pass]");
        }
    }

    char *pass = NULL;
    uint8_t salt[8];
    if (optind < argc) pass = argv[optind++];
    if (optind != argc) goto usage;
    if (opt_salt && mc_make_random(salt, sizeof(salt)) != sizeof(salt))
        cpr_crit(-1, "fatal: cannot make salt");

    char line[256];
    if (!pass) {
        if (!con_passwd(line, sizeof(line), opt_prompt)) exit(1);
        pass = line;
    }

    uint8_t digest[MC_SHA256_DIGEST_SIZE];
    mc_sha256_ctx ctx;
    mc_sha256_init(&ctx);
    if (opt_salt) mc_sha256_update(&ctx, salt, sizeof(salt));
    mc_sha256_update(&ctx, (uint8_t *)pass, strlen(pass));
    mc_sha256_final(&ctx, digest);
    pass = cpr_hexdup(digest, sizeof(digest));

    if (opt_salt) printf("%s ", cpr_hexdup(salt, sizeof(salt)));
    printf("%s\n", pass);
    return 0;
}
