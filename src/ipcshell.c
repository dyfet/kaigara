// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include <local/scan.h>
#include <local/ipc.h>
#include <cpr/system.h>
#include <cpr/strchar.h>
#include <cpr/memory.h>
#include <defines.h>
#include <getopt.h>

static bool opt_silence = false;
static int opt_timeout = 15000;

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "st:")) != -1) {
        switch (opt) {
        case 's':
            opt_silence = true;
            break;
        case 't':
            opt_timeout = scan_value(optarg, 1, 60000);
            if (!opt_timeout) {
                fprintf(stderr, "%s: invalid timeout: %s\n", argv[0], optarg);
                goto usage;
            }
            break;
        default:
        usage:
            cpr_crit(-1, "usage: ipcshell ipc-path ipc-command [...]");
        }
    }

    if (optind > (argc - 2)) goto usage;
    char *path = argv[optind++];
    char *name = strrchr(path, '.');
    if (!eq(name, ".ipc")) cpr_crit(-1, "fatal: not ipc path: %s", path);
    name = strchr(path, '/');

#ifdef _WIN32
    if (name) path = ++name;
#else
    if (name) {
        *(name++) = 0;
        if (access(path, R_OK) && !chdir(path)) cpr_crit(-1, "fatal: cannot access service: %s", name);
        path = name;
    }
    if (access(path, W_OK)) cpr_crit(-1, "fatal: not active: %s", path);
#endif

    const char *cmd = argv[optind];
    if (!ipc_client_start(path)) cpr_crit(-1, "fatal: cannot open: %s", path);
    char *result = NULL;
    int code = ipc_client_exec(&argv[optind], &result, opt_timeout);
    switch (code) {
    case IPC_SUCCESS:
        if (result && *result) fputs(result, stdout);
        break;
    case IPC_CALL_FAILED:
        if (!opt_silence) fprintf(stderr, "error: call failed: %s\n", path);
        break;
    case IPC_TIMEOUT:
        if (!opt_silence) fprintf(stderr, "error: timeout\n");
        break;
    case IPC_BAD_SESSION:
        if (!opt_silence) fprintf(stderr, "error: bad session\n");
        break;
    case IPC_BAD_REQUEST:
        if (!opt_silence) fprintf(stderr, "error: bad request\n");
        break;
    case IPC_BAD_RESPONSE:
        if (!opt_silence) fprintf(stderr, "error: bad response\n");
        break;
    case IPC_UNKNOWN_COMMAND:
        if (!opt_silence) fprintf(stderr, "error: unknown command: %s\n", cmd);
        break;
    case IPC_INVALID_ARGUMENTS:
    case IPC_TOO_MANY_ARGUMENTS:
        if (!opt_silence) fprintf(stderr, "error: bad arguments: %s\n", cmd);
        break;
    default:
        if (!opt_silence) fprintf(stderr, "error: command %s: %d\n", cmd, code);
        break;
    }

    ipc_client_end();
    return code;
}
