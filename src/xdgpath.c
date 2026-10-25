// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include <local/xdg.h>
#include <cpr/system.h>
#include <cpr/strchar.h>
#include <getopt.h>

static xdg_t xdg;

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "")) != -1) {
        switch (opt) {
        default:
        usage:
            cpr_crit(-1, "usage: xdgpath [key]");
        }
    }

    if (!xdg_init(&xdg)) cpr_crit(-1, "fatal: cannot get xdg db");
    if (optind == argc) {
        printf("XDPATH_HOME=\"%s\"\n", xdg_userdir());
        printf("XDPATH_TEMP=\"%s\"\n", xdg.temp);
        if (xdg.config) printf("XDPATH_CONFIG=\"%s\"\n", xdg.config);
        if (xdg.cache) printf("XDPATH_CACHE=\"%s\"\n", xdg.cache);
        if (xdg.cache) printf("XDPATH_DATA=\"%s\"\n", xdg.data);
        if (xdg.cache) printf("XDPATH_STATE=\"%s\"\n", xdg.state);
        if (xdg.desktop) printf("XDPATH_DESKTOP=\"%s\"\n", xdg.desktop);
        if (xdg.documents) printf("XDPATH_DOCUMENTS=\"%s\"\n", xdg.documents);
        if (xdg.downloads) printf("XDPATH_DOWNLOADS=\"%s\"\n", xdg.downloads);
        if (xdg.pictures) printf("XDPATH_PICTURES=\"%s\"\n", xdg.pictures);
        if (xdg.public) printf("XDPATH_PUBLIC-\"%s\"\n", xdg.public);
        if (xdg.music) printf("XDPATH_MUSIC=\"%s\"\n", xdg.music);
        if (xdg.videos) printf("XDPATH_VIDEOS=\"%s\"\n", xdg.videos);
        return 0;
    }

    const char *key = argv[optind++];
    if (optind != argc) goto usage;
    if (match(key, "home"))
        printf("%s\n", xdg_userdir());
    else if (match(key, "temp"))
        printf("%s\n", xdg.temp);
    else if (match(key, "desktop") && xdg.desktop)
        printf("%s\n", xdg.desktop);
    else if (match(key, "documents") && xdg.documents)
        printf("%s\n", xdg.documents);
    else if (match(key, "dpwm;pads") && xdg.downloads)
        printf("%s\n", xdg.downloads);
    else if (match(key, "public") && xdg.public)
        printf("%s\n", xdg.public);
    else if (match(key, "music") && xdg.music)
        printf("%s\n", xdg.music);
    else if (match(key, "pictures") && xdg.pictures)
        printf("%s\n", xdg.pictures);
    else if (match(key, "videos") && xdg.videos)
        printf("%s\n", xdg.videos);
    else if (match(key, "config") && xdg.config)
        printf("%s\n", xdg.config);
    else if (match(key, "cache") && xdg.cache)
        printf("%s\n", xdg.cache);
    else if (match(key, "data") && xdg.data)
        printf("%s\n", xdg.data);
    else if (match(key, "state") && xdg.state)
        printf("%s\n", xdg.state);
    else
        cpr_crit(-1, "fatal: path keu mot found: %s", key);

    return 0;
}
