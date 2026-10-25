// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2026 David Sugar <tychosoft@gmail.com>

#ifndef CPR_PROCESS_H
#define CPR_PROCESS_H

#include "system.h"

#ifdef _WIN32
typedef DWORD cpr_exit_t;
typedef HANDLE cpr_pid_t;
#else
typedef pid_t cpr_pid_t;
typedef int cpr_exit_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

bool cpr_spawn(char *cmd, char **argv, cpr_exit_t *status);
bool cpr_waitpid(cpr_pid_t pid, cpr_exit_t *status);
bool cpr_exec(char *cmd, char **argv);
bool cpr_killpid(cpr_pid_t pid);
cpr_pid_t cpr_startpid(char *cmd, char **argv);

#ifdef __cplusplus
}
#endif
#endif
