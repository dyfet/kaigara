// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "ipc.h"

#if defined(__OpenBSD__)
#define random() ((long)arc4random())
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ddeml.h>

static DWORD ipc = 0;
static HSZ service = 0;
static HSZ topic = 0;

bool ipc_client_start(const char *path) {
    UINT r = DdeInitialize(&ipc, NULL, APPCMD_CLIENTONLY, 0);
    if (r != DMLERR_NO_ERROR) return false;

    service = DdeCreateStringHandle(ipc, path, CP_WINANSI);
    topic = DdeCreateStringHandle(ipc, "Request", CP_WINANSI);
    if (!service || !topic) return false;
    return true;
}

void ipc_client_end(void) {
    if (service) {
        DdeFreeStringHandle(ipc, service);
        service = 0;
    }

    if (topic) {
        DdeFreeStringHandle(ipc, topic);
        topic = 0;
    }

    if (ipc) {
        DdeUninitialize(ipc);
        ipc = 0;
    }
}

bool ipc_client_call(const char *req, size_t len, char *res, size_t size, int timeout) {
    HCONV hconv = DdeConnect(ipc, service, topic, NULL);
    if (!hconv) return false;

    HDDEDATA hdata = DdeCreateDataHandle(
    ipc,
    (LPBYTE)req,
    (DWORD)len,
    0,
    0,
    CF_TEXT,
    0);

    if (!hdata) {
        DdeDisconnect(hconv);
        return false;
    }

    DWORD result_len = 0;
    HDDEDATA hret = DdeClientTransaction(
    (LPBYTE)hdata,
    (DWORD)-1,
    hconv,
    0,
    CF_TEXT,
    XTYP_REQUEST,
    timeout,
    NULL);

    if (!hret) {
        DdeDisconnect(hconv);
        return false;
    }

    const char *reply = (const char *)DdeAccessData(hret, &result_len);
    if (!reply) {
        DdeDisconnect(hconv);
        return false;
    }

    size_t n = (result_len < size - 1 ? result_len : size - 1);
    memcpy(res, reply, n);
    res[n] = '\0';

    DdeUnaccessData(hret);
    DdeDisconnect(hconv);
    return true;
}

#else
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <poll.h>

static int ipc = -1;
static struct sockaddr_un server;

bool ipc_client_start(const char *path) {
    ipc = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (ipc < 0) return false;

    struct sockaddr_un client = {0};
    client.sun_family = AF_UNIX;
    client.sun_path[0] = '\0';
    snprintf(client.sun_path + 1, sizeof(client.sun_path) - 1, "cli-%d-%ld", getpid(), random());

    socklen_t client_len = offsetof(struct sockaddr_un, sun_path) + 1 + strlen(client.sun_path + 1);
    if (bind(ipc, (struct sockaddr *)&client, client_len) < 0) {
        close(ipc);
        ipc = -1;
        return false;
    }

    memset(&server, 0, sizeof(server));
    server.sun_family = AF_UNIX;
    strncpy(server.sun_path, path, sizeof(server.sun_path) - 1);
    return true;
}

void ipc_client_end() {
    if (ipc >= 0) {
        close(ipc);
        ipc = -1;
    }
}

bool ipc_client_call(const char *req, size_t len, char *res, size_t size, int timeout) {
    if (ipc < 0) return false;
    ssize_t n = sendto(ipc, req, len, 0, (struct sockaddr *)&server, sizeof(server));
    if (n < 0) return false;

    struct pollfd pfd;
    pfd.fd = ipc;
    pfd.events = POLLIN;
    pfd.revents = 0;

    int r = poll(&pfd, 1, timeout);
    if (r < 0) return false;
    if (pfd.revents & (POLLNVAL | POLLERR | POLLHUP)) return false; // NOLINT
    if (!(pfd.revents & POLLIN)) return false;                      // NOLINT

    n = recv(ipc, res, size - 1, 0);
    if (n < 0) return false;
    res[n] = 0;
    return true;
}

#endif

int ipc_client_exec(char **argv, char **reply, int timeout) {
    if (!argv || !argv[0]) return IPC_BAD_REQUEST;
    if (reply) *reply = NULL;

    char msg[IPC_MAX_MSG_SIZE];
    size_t len = 0;
    int arg = 0;
    int n = snprintf(msg, sizeof(msg), "DO %s\n", *(argv++));
    if (n < 0) return IPC_BAD_REQUEST;
    len = n;
    while (len < (IPC_MAX_MSG_SIZE - 1) && *argv) {
        int max = IPC_MAX_MSG_SIZE - len;
        n = snprintf(msg + len, max, "%s\n", *(argv++));
        if (n < 0 || n > max) return IPC_BAD_REQUEST;
        len += n;
    }
    msg[len] = 0;
    if (!ipc_client_call(msg, len, msg, sizeof(msg), timeout)) return IPC_CALL_FAILED;

    char topic[65];
    int c = IPC_COMMAND_FAILED;
    n = 0;

    if (sscanf(msg, "OK %64s%n", topic, &n) == 1) {
        if (msg[n] == '\n') {
            ++n;
            if (reply) *reply = msg + n;
            return IPC_SUCCESS;
        }
        return IPC_BAD_SESSION;
    }

    if (sscanf(msg, "ERR %d %64s%n", &c, topic, &n) == 2) { // NOLINT
        if (msg[n] == '\n') return c;
        return IPC_BAD_SESSION;
    }
    return IPC_BAD_SESSION;
}
