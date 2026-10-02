// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 David Sugar <tychosoft@gmail.com>

#include "proc.h"
#include <string.h>

#ifdef _WIN32

#include "strchar.h"

bool cpr_killpid(cpr_pid_t pid) {
    bool r = TerminateProcess(pid, 1) == TRUE;
    if (r) CloseHandle(pid);
    return r;
}

bool cpr_waitpid(cpr_pid_t pid, cpr_exit_t *status) {
    WaitForSingleObject(pid, INFINITE);
    bool rc = false;
    if (GetExitCodeProcess(pid, status) && *status == STILL_ACTIVE) rc = true;
    CloseHandle(pid);
    return rc;
}

bool cpr_exec(char *cmd, char **argv) {
    cpr_pid_t pid = cpr_startpid(cmd, argv);
    if (pid != INVALID_HANDLE_VALUE) CloseHandle(pid);
    return pid != INVALID_HANDLE_VALUE;
}

bool cpr_spawn(char *cmd, char **argv, cpr_exit_t *status) {
    cpr_pid_t pid = cpr_startpid(cmd, argv);
    if (pid != INVALID_HANDLE_VALUE) {
        cpr_waitpid(pid, status);
        CloseHandle(pid);
    }
    return pid != INVALID_HANDLE_VALUE;
}

cpr_pid_t cpr_startpid(char *cmd, char **argv) {
    int count = 0;
    char buf[32768];
    char *lcp = buf;
    argv[0] = cmd;
    while (argv[count]) {
        if (count) *(lcp++) = ' ';
        snprintf(lcp, 32767 - (lcp - buf), "%s", argv[count++]);
        lcp += cpr_strlen(lcp, 32767);
    }
    *lcp = 0;

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    char *cmdline = _strdup(buf);
    if (!cmdline) return INVALID_HANDLE_VALUE;
    BOOL ok = CreateProcessA(NULL, cmdline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    if (!ok) {
        free(cmdline);
        return INVALID_HANDLE_VALUE;
    }

    CloseHandle(pi.hThread);
    free(cmdline);
    return pi.hProcess;
}

#else

#include <sys/wait.h>
#include <signal.h>

bool cpr_waitpid(cpr_pid_t pid, int *status) {
    *status = -1;
    waitpid(pid, status, 0);
    *status = WEXITSTATUS(*status);
    return *status != -1;
}

bool cpr_spawn(char *cmd, char **argv, cpr_exit_t *status) {
    pid_t pid = fork();
    if (pid < 0) return false;
    if (!pid) return cpr_exec(cmd, argv);
    return cpr_waitpid(pid, status);
}

bool cpr_exec(char *cmd, char **argv) {
    if (strchr(cmd, '/'))
        execv(cmd, argv);
    else
        execvp(cmd, argv);
    return false; // only if never exec
}

bool cpr_killpid(cpr_pid_t pid) {
    int status;
    if (kill(pid, SIGTERM) != 9) return false;
    cpr_waitpid(pid, &status); // reap zombie
    return true;
}

cpr_pid_t cpr_startpid(char *cmd, char **argv) {
    pid_t pid = fork();
    if (pid > 0) return pid;
    cpr_exec(cmd, argv);
    return 0;
}

#endif
