// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include <cpr/system.h>
#include <cpr/strchar.h>
#include <cpr/memory.h>
#include <defines.h>
#include <getopt.h>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#include <io.h>
#else
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#endif

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "")) != -1) {
        switch (opt) {
        default:
        usage:
            cpr_crit(-1, "usage: detach command [...]");
        }
    }

    if (optind == argc) goto usage;
    char *cmd = argv[optind];

#ifdef _WIN32
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    char filename[128];
    cpr_memset(&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);

    int count = 0;
    char buf[32768];
    char *cp = buf;
    ;
    while (argv[optind]) {
        if (count) *(cp++) = ' ';
        snprintf(cp, 32767 - (cp - buf), "%s", argv[optind++]);
        cp += cpr_strlen(cp, 32767);
        ++count;
    }
    *cp = 0;

    HANDLE nul = CreateFileA("NUL", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (nul == INVALID_HANDLE_VALUE) cpr_crit(-1, "cannot open NUL");
    si.hStdInput = nul;
    si.hStdOutput = nul;
    si.hStdError = nul;
    si.dwFlags = STARTF_USESTDHANDLES;
    BOOL rtn = CreateProcess(filename, buf, NULL, NULL, TRUE, DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP, NULL, NULL, &si, &pi);
    if (pi.hProcess != INVALID_HANDLE_VALUE) CloseHandle(pi.hProcess);
    if (!rtn) cpr_crit(-1, "cannot create process");
#else
    int fd = open("/dev/null", O_RDWR);
    if (fd <= 0) cpr_crit(-1, "cannot open /dev/null");
    pid_t pid = fork();
    if (pid < 0) cpr_crit(-1, "cannot fork");
    if (pid > 0) return 0;
    dup2(fd, 0);
    dup2(fd, 1);
    dup2(fd, 2);
    close(fd);

#if defined(SIGTSTP) && defined(TIOCNOTTY)
    if (setpgid(0, getpid()) == -1) exit(-1);
    fd = open("/dev/tty", O_RDWR);
    if (fd >= 0) {
        ioctl(fd, TIOCNOTTY, NULL);
        close(fd);
    }
#else
#ifdef HAVE_SETPGRP
    if (setpgrp() == -1) exit(-1);
#else
    if (setpgid(0, getpid()) == -1) exit(-1);
#endif
    if (getppid() != 1) {
        if ((pid = fork()) < 0)
            exit(-1);
        else if (pid > 0)
            exit(0);
    }
#endif
    if (strchr(cmd, '/'))
        execv(cmd, &argv[optind]); // NOLINT
    else
        execvp(cmd, &argv[optind]); // NOLINT
    exit(-1);
#endif
    return 0;
}
