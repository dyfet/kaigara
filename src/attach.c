// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include <local/shell.h>
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
    bool null = false;
    bool opt_err = false;
    bool opt_wait = false;
    int opt;
    while ((opt = getopt(argc, argv, "ew")) != -1) {
        switch (opt) {
        case 'e':
            opt_err = true;
            break;
        case 'w':
            opt_wait = true;
            break;
        default:
        usage:
            cpr_crit(-1, "usage: detach dev  command [...]");
        }
    }

    if (optind > (argc - 2)) goto usage;
    const char *dev = argv[optind++];
    char *cmd = argv[optind];
    char name[DEV_SIZE];
    if (eq(dev, "-")) null = true;
    if (!dev_name(dev, name, sizeof(name))) cpr_crit(-1, "fatal: invalid dev: %s", dev);
    dev = name;

#ifdef _WIN32
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    char filename[128];
    cpr_memset(&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);

    int count = 0;
    char buf[32768];
    char *cp = buf;
    while (argv[optind]) {
        if (count) *(cp++) = ' ';
        snprintf(cp, 32767 - (cp - buf), "%s", argv[optind++]);
        cp += cpr_strlen(cp, 32767);
        ++count;
    }
    *cp = 0;

    HANDLE handle = CreateFileA(dev, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle == INVALID_HANDLE_VALUE) cpr_crit(-1, "cannot open: %s", dev);
    DWORD type = GetFileType(handle);
    if (!null && type != FILE_TYPE_CHAR) cpr_crit(-1, "not a tty: %s", dev);

    si.hStdInput = handle;
    si.hStdOutput = handle;
    si.hStdError = opt_err ? GetStdHandle(STD_ERROR_HANDLE) : handle;
    si.dwFlags = STARTF_USESTDHANDLES;
    BOOL rtn = CreateProcess(filename, buf, NULL, NULL, TRUE, DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP, NULL, NULL, &si, &pi);
    if (!rtn) cpr_crit(-1, "cannot create process");

    DWORD status = 0;
    if (opt_wait) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        GetExitCodeProcess(pi.hProcess, &status);
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return status;
#else
    int fd = open(dev, O_RDWR);
    if (fd <= 0) cpr_crit(-1, "cannot open: %s", dev);
    if (!null && !isatty(fd)) cpr_crit(-1, "not a tty: %s", dev);
    pid_t pid = fork();
    if (pid < 0) cpr_crit(-1, "cannot fork");

    int status = 0;
    if (opt_wait && pid > 0) waitpid(pid, &status, 0);
    if (pid > 0) return WEXITSTATUS(status);

    dup2(fd, 0);
    dup2(fd, 1);
    if (!opt_err) dup2(fd, 2);
    close(fd);

    if (opt_wait) goto exec;
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
exec:
    if (strchr(cmd, '/'))
        execv(cmd, &argv[optind]); // NOLINT
    else
        execvp(cmd, &argv[optind]); // NOLINT
    exit(-1);
#endif
    return 0;
}
