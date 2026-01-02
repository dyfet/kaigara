// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include "keyfile.h"

keysection_t *append_keysections(keysection_t *root, keysection_t *into) {
    if (!into) return root;
    keysection_t *prior = into;
    while (prior && prior->next) {
        prior = prior->next;
    }
    if (prior)
        prior->next = root;
    return into;
}

const char *get_keyvalue(keysection_t *section, const char *id) {
    keydata_t *key = section->keys;
    while (key) {
        if (eq(key->id, id))
            return key->value;
        key = key->next;
    }
    return NULL;
}

keydata_t *make_keydata(keysection_t *group, const char *id, const char *value) {
    keydata_t *key = malloc(sizeof(keydata_t));
    if (!key) return NULL;

    key->id = cpr_lower(id, 32);
    key->value = strdup(value);
    ;
    key->next = group->keys;
    group->keys = key;
    return key;
}

keysection_t *make_keysection(const char *id, keysection_t *prior) {
    keysection_t *section = malloc(sizeof(keysection_t));
    if (prior)
        prior->next = section;
    if (!section)
        return NULL;

    section->next = NULL;
    section->keys = NULL;
    section->id = cpr_upper(id, 32);
    return section;
}

keysection_t *find_keysection(keysection_t *root, const char *id) {
    while (root && !match(id, root->id)) {
        root = root->next;
    }
    return root;
}

keysection_t *load_keyfile(const char *path) {
    // FlawFinder: ignore
    FILE *fp = fopen(path, "r");
    char *lp;
    char linebuf[96];

    if (!fp)
        return NULL;

    keysection_t *root = make_keysection("*", NULL);
    keysection_t *current = root;
    while (!feof(fp) && NULL != (lp = fgets(linebuf, sizeof(linebuf), fp))) {
        size_t len = cpr_strlen(lp, sizeof(linebuf));
        if (!len || lp[len - 1] != '\n') continue;
        while (len > 0 && isspace(lp[--len])) // NOLINT(bugprone-inc-dec-in-conditions)
            lp[len] = 0;

        while (*lp && isspace(*lp))
            ++lp;

        if (!*lp) continue;
        if (*lp == '[' && lp[len] == ']') {
            while (len > 1 && isspace(lp[--len])) // NOLINT(bugprone-inc-dec-in-conditions)
                ;

            if (len < 2) continue;
            while (isspace(*(++lp)))
                --len;

            lp[len] = 0;
            current = make_keysection(lp, current);
            continue;
        } else if (!isalnum(*lp)) {
            continue;
        } else {
            char *kw = lp;
            while (*kw && *kw != '=')
                ++kw;
            if (kw == lp || !*kw) continue;
            char *body = kw + 1;
            --kw;
            while (kw > lp && isspace(*kw))
                --kw;
            if (kw == lp) continue;
            *(++kw) = 0;
            if (*body && isspace(*body))
                ++body;
            kw = lp;
            char *ep = body + cpr_strlen(body, sizeof(linebuf)) - 1;
            if ((*body == '\'' || *body == '\"') && (*ep == *body)) {
                *ep = 0;
                if (*body)
                    ++body;
            }
            make_keydata(current, kw, body);
        }
    }

    fclose(fp); // NOLINT
    return root;
}

bool save_keyfile(const char *path, keysection_t *root) {
    if (!path || !root) return false;
    remove(path);
    FILE *fp = fopen(path, "w"); // FlawFinder: ignore
    if (!fp) return false;
    for (keysection_t *section = root; section != NULL; section = section->next) {
        fprintf(fp, "[%s]\n", section->id);
        for (keydata_t *key = section->keys; key != NULL; key = key->next) {
            if (strchr(key->value, '\'') != NULL)
                fprintf(fp, "%s=\"%s\"\n", key->id, key->value);
            else
                fprintf(fp, "%s='%s'\n", key->id, key->value);
        }
    }
    fclose(fp);
    return true;
}

void free_keyfile(keysection_t *root) {
    while (root != NULL) {
        keysection_t *next_sect = root->next;
        keydata_t *key = root->keys;
        while (key != NULL) {
            keydata_t *next_key = key->next;
            free(key->id);
            free(key->value);
            free(key);
            key = next_key;
        }
        free(root->id);
        root = next_sect;
    }
}
