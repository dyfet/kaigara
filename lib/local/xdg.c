// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 202y David Sugar <tychosoft@gmail.com>

#ifdef _WIN32
#define MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS 0
#include <windows.h>

#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif

#include <shlobj.h>
#include <knownfolders.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "xdg.h"

#ifdef _WIN32
static char *xdg_get_temp() {
    char buf[MAX_PATH];
    DWORD len = GetTempPathA(sizeof(buf), buf);
    if (len == 0 || len >= sizeof(buf)) return strdup("C:\\Windows\\Temp");

    size_t n = strlen(buf);
    if (n > 0 && (buf[n - 1] == '\\' || buf[n - 1] == '/')) buf[n - 1] = '\0';
    return strdup(buf);
}

static char *xdg_get_config() {
    PWSTR wpath = NULL;
    if (SUCCEEDED(SHGetKnownFolderPath(&FOLDERID_RoamingAppData, 0, NULL, &wpath))) {
        int len = WideCharToMultiByte(CP_UTF8, 0, wpath, -1, NULL, 0, NULL, NULL);
        if (len > 0) {
            char *out = malloc(len);
            if (out) {
                WideCharToMultiByte(CP_UTF8, 0, wpath, -1, out, len, NULL, NULL);
                CoTaskMemFree(wpath);
                return out;
            }
        }
        CoTaskMemFree(wpath);
    }
    return strdup("C:\\Users\\Public");
}

static char *xdg_get_cache() {
    PWSTR wpath = NULL;
    if (SUCCEEDED(SHGetKnownFolderPath(&FOLDERID_LocalAppData, 0, NULL, &wpath))) {
        int len = WideCharToMultiByte(CP_UTF8, 0, wpath, -1, NULL, 0, NULL, NULL);
        if (len > 0) {
            char *out = malloc(len);
            if (out) {
                WideCharToMultiByte(CP_UTF8, 0, wpath, -1, out, len, NULL, NULL);
                CoTaskMemFree(wpath);
                return out;
            }
        }
        CoTaskMemFree(wpath);
    }
    return strdup("C:\\Windows\\Temp");
}

static void xdg_parse_user_dirs(xdg_t *xdg) {
    xdg->temp = xdg_get_temp();
    xdg->config = xdg_get_config();
    xdg->cache = xdg->data = xdg->state = xdg_get_cache();

    HKEY key;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders", 0, KEY_READ, &key) != ERROR_SUCCESS) return;

    char buf[MAX_PATH];
    DWORD len = sizeof(buf);

    // Desktop
    len = sizeof(buf);
    if (!xdg->desktop && RegGetValueA(key, NULL, "Desktop", RRF_RT_ANY, NULL, buf, &len) == ERROR_SUCCESS) xdg->desktop = strdup(buf);

    // Documents
    len = sizeof(buf);
    if (!xdg->documents && RegGetValueA(key, NULL, "Personal", RRF_RT_ANY, NULL, buf, &len) == ERROR_SUCCESS) xdg->documents = strdup(buf);

    // Downloads
    len = sizeof(buf);
    if (!xdg->downloads && RegGetValueA(key, NULL, "{374DE290-123F-4565-9164-39C4925E467B}", RRF_RT_ANY, NULL, buf, &len) == ERROR_SUCCESS) xdg->downloads = strdup(buf);

    // Pictures
    len = sizeof(buf);
    if (!xdg->pictures && RegGetValueA(key, NULL, "My Pictures", RRF_RT_ANY, NULL, buf, &len) == ERROR_SUCCESS) xdg->pictures = strdup(buf);

    // Music
    len = sizeof(buf);
    if (!xdg->music && RegGetValueA(key, NULL, "My Music", RRF_RT_ANY, NULL, buf, &len) == ERROR_SUCCESS) xdg->music = strdup(buf);

    // Videos
    len = sizeof(buf);
    if (!xdg->videos && RegGetValueA(key, NULL, "My Video", RRF_RT_ANY, NULL, buf, &len) == ERROR_SUCCESS) xdg->videos = _strdup(buf);

    RegCloseKey(key);
}

#else

#include <unistd.h>

static char *xdg_expand_home(const char *val) {
    const char *home = xdg_userdir();
    if (!home) home = "";
    if (strncmp(val, "$HOME/", 6) == 0) {
        size_t len = strlen(home) + strlen(val + 5) + 1;
        char *out = malloc(len);
        snprintf(out, len, "%s/%s", home, val + 6);
        return out;
    }
    return strdup(val);
}

static void xdg_set_field(xdg_t *xdg, const char *key, const char *val) {
    if (strcmp(key, "XDG_DESKTOP_DIR") == 0)
        xdg->desktop = xdg_expand_home(val);
    else if (strcmp(key, "XDG_DOCUMENTS_DIR") == 0)
        xdg->documents = xdg_expand_home(val);
    else if (strcmp(key, "XDG_DOWNLOAD_DIR") == 0)
        xdg->downloads = xdg_expand_home(val);
    else if (strcmp(key, "XDG_PICTURES_DIR") == 0)
        xdg->pictures = xdg_expand_home(val);
    else if (strcmp(key, "XDG_VIDEOS_DIR") == 0)
        xdg->videos = xdg_expand_home(val);
    else if (strcmp(key, "XDG_MUSIC_DIR") == 0)
        xdg->music = xdg_expand_home(val);
    else if (strcmp(key, "XDG_PUBLIC_DIR") == 0)
        xdg->public = xdg_expand_home(val);
}

static void xdg_parse_user_dirs(xdg_t *xdg) {
    const char *home = xdg_userdir();
    if (!home) return;

    char path[512];
    snprintf(path, sizeof(path), "%s/.config/user-dirs.dirs", home);

    FILE *f = fopen(path, "r");
    if (!f) return;

    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || !strstr(line, "_DIR=")) continue;
        char *eq = strchr(line, '=');
        if (!eq) continue;

        *eq = 0;
        char *key = line;
        char *val = eq + 1;

        char *start = strchr(val, '"');
        char *end = strrchr(val, '"');
        if (!start || !end || end <= start) continue;

        *end = 0;
        start++;
        xdg_set_field(xdg, key, start);
    }

    fclose(f);
}

#endif

static char *xdg_path(const char *subdir) {
    char buf[256];
    if (subdir)
        snprintf(buf, sizeof(buf), "%s/%s", xdg_userdir(), subdir);
    else
        snprintf(buf, sizeof(buf), "%s", xdg_userdir());
    return strdup(buf);
}

char *xdg_desktop() {
#ifdef _WIN32
    return "WINDOWS";
#else
    char *cp = getenv("XDG_CURRENT_DESKTOP");
    if (!cp || !*cp) return "NONE";
    return cp;
#endif
}

char *xdg_userdir() {
#ifdef _WIN32
    char *cp = getenv("USERPROFILE");
#else
    char *cp = getenv("HOME");
#endif
    if (!cp || !*cp) return ".";
    return cp;
}

bool xdg_init(xdg_t *xdg) {
    if (!xdg) return false;
    memset(xdg, 0, sizeof(xdg_t));
    xdg_parse_user_dirs(xdg);

#ifndef _WIN32
    const char *temp = getenv("TMP");
    if (temp && *temp && !xdg->temp) xdg->temp = strdup(temp);

    const char *config = getenv("XDG_CONFIG_HOME");
    if (config && *config && !xdg->config) xdg->config = strdup(config);

    const char *cache = getenv("XDG_CACHE_HOME");
    if (cache && *cache && !xdg->cache) xdg->cache = strdup(cache);

    const char *data = getenv("XDG_DATA_HOME");
    if (data && *data && !xdg->data) xdg->data = strdup(data);

    const char *state = getenv("XDG_STATE_HOME");
    if (state && *state && !xdg->state) xdg->state = strdup(state);

    char buf[512];
    snprintf(buf, sizeof(buf), "%s/tmp", xdg_userdir());
    if (!access(buf, X_OK | W_OK) && !xdg->downloads) xdg->downloads = strdup(buf);
    if (!access(buf, X_OK | W_OK) && !xdg->temp) xdg->temp = strdup(buf);
    if (!xdg->temp) xdg->temp = strdup("/tmp");
    if (!xdg->config) {
        snprintf(buf, sizeof(buf), "%s/.config", xdg_userdir());
        xdg->config = strdup(buf);
    }

    if (!xdg->cache) {
        snprintf(buf, sizeof(buf), "%s/.cache", xdg_userdir());
        xdg->cache = strdup(buf);
    }

    if (!xdg->data) {
        snprintf(buf, sizeof(buf), "%s/.local/share", xdg_userdir());
        xdg->data = strdup(buf);
    }

    if (!xdg->state) {
        snprintf(buf, sizeof(buf), "%s/.local/state", xdg_userdir());
        xdg->state = strdup(buf);
    }
#endif

    if (!xdg->desktop) xdg->desktop = xdg_path(NULL);
    if (!xdg->documents) xdg->documents = strdup(xdg->desktop);
    if (!xdg->downloads) xdg->downloads = strdup(xdg->documents);
    if (!xdg->pictures) xdg->pictures = strdup(xdg->documents);
    if (!xdg->videos) xdg->videos = strdup(xdg->documents);
    if (!xdg->music) xdg->music = strdup(xdg->documents);
    if (!xdg->videos) xdg->videos = strdup(xdg->documents);
    return true;
}

void xdg_free(xdg_t *xdg) {
    if (!xdg) return;
    if (xdg->desktop) free(xdg->desktop);
    if (xdg->documents) free(xdg->documents);
    if (xdg->downloads) free(xdg->downloads);
    if (xdg->pictures) free(xdg->pictures);
    if (xdg->videos) free(xdg->videos);
    if (xdg->music) free(xdg->music);
    if (xdg->public) free(xdg->public);
    if (xdg->temp) free(xdg->temp);
    if (xdg->config) free(xdg->config);
    if (xdg->cache) free(xdg->cache);
    if (xdg->data) free(xdg->data);
    if (xdg->state) free(xdg->state);
    memset(xdg, 0, sizeof(xdg_t));
}
