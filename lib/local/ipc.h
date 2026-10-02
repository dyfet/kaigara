// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 David Sugar <tychosoft@gmail.com>

#ifndef LOCAL_IPC_H
#define LOCAL_IPC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

#define IPC_MAX_MSG_SIZE 32768

#define IPC_SUCCESS 0
#define IPC_COMMAND_FAILED 10
#define IPC_UNKNOWN_COMMAND 80
#define IPC_TOO_MANY_ARGUMENTS 81
#define IPC_INVALID_ARGUMENTS 82
#define IPC_TIMEOUT 90
#define IPC_BAD_SESSION 96
#define IPC_BAD_REQUEST 97
#define IPC_BAD_RESPONSE 98
#define IPC_CALL_FAILED 99

bool ipc_client_call(const char *req, size_t len, char *res, size_t size, int timeout);
bool ipc_client_start(const char *path);
void ipc_client_end();
int ipc_client_exec(char **argv, char **reply, int timeout);

#ifdef __cplusplus
}
#endif
#endif
