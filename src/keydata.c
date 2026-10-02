// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 David Sugar <tychosoft@gmail.com>

#include <cpr/system.h>
#include <cpr/strchar.h>
#include <cpr/keyfile.h>
#include <defines.h>
#include <getopt.h>

static const char *key_path = NULL;
static const char *key_group = NULL;
static bool opt_prefix = false;

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "hp")) != -1) {
        switch (opt) {
        case 'p':
            opt_prefix = true;
            break;
        default:
        usage:
            cpr_crit(-1, "usage: keydata [options] keyfile [section]");
        }
    }

    if (optind < argc) key_path = argv[optind++];
    if (optind < argc) key_group = argv[optind++];
    if (optind < argc || !key_path) goto usage;

    keysection_t *keys = load_keyfile(key_path);
    if (!keys) cpr_crit(2, "fatal: cannot load keys: %s", key_path);
    if (key_group) keys = find_keysection(keys, key_group);
    if (!keys) cpr_crit(3, "fatalr: cannot find group: %s", key_group);

    keydata_t *list = keys->keys;
    while (list) {
        if (opt_prefix && key_group)
            printf("%s_%s=%s\n", key_group, list->id, list->value);
        else
            printf("%s=%s\n", list->id, list->value);
        list = list->next;
    }
    return 0;
}
