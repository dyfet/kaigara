// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include "service.h"
#include "strchar.h"
#include "thread.h"
#include "signal.h"
#include <sys/stat.h>

#ifdef _WIN32
static HANDLE done = INVALID_HANDLE_VALUE;
#else
static sigset_t sigs;
#endif

static bool logger = false;
static int *excode = NULL;
static cpr_timer_t sigtimer = NULL;
static int interval = 0;

#ifdef _WIN32
static VOID CALLBACK sigalrm(void *arg, DWORD low, DWORD high) {
    if (sigtimer) sigtimer();
}

static void sigint(int signo) {
    if (excode) *excode = SIGINT;
    SetEvent(done);
}

static BOOL stop(DWORD event) {
    switch (event) {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
        fputs("^C\n", stdout);
        fflush(stdout);
        SetEvent(done);
        if (excode) *excode = SIGINT;
        return TRUE;
    default:
        return FALSE;
    }
}

static int wait_timer(int seconds) {
    HANDLE alarm = CreateWaitableTimer(NULL, TRUE, NULL);
    if (!alarm) return -1;
    LARGE_INTEGER due;
    due.QuadPart = -(LONGLONG)seconds * 10000000LL;
    LONG period = seconds * 1000;
    if (!SetWaitableTimer(alarm, &due, period, sigalrm, NULL, FALSE)) {
        CloseHandle(alarm);
        return -1;
    }
    HANDLE handles[1] = {done};
    for (;;) {
        DWORD r = WaitForMultipleObjectsEx(1, handles, FALSE, INFINITE, TRUE);
        if (r == WAIT_OBJECT_0) {
            CancelWaitableTimer(alarm);
            CloseHandle(alarm);
            return SIGINT;
        }
        if (r == WAIT_IO_COMPLETION) continue;
        CancelWaitableTimer(alarm);
        CloseHandle(alarm);
        return -1;
    }
}

void cpr_signals(int *exiting) {
    excode = exiting;
    done = CreateEvent(NULL, TRUE, FALSE, NULL);
    signal(SIGINT, &sigint);
    signal(SIGTERM, &sigint);
    SetConsoleCtrlHandler(&stop, TRUE);
}

int cpr_sigwaitall() {
    if (sigtimer && interval) return wait_timer(interval);
    if (WaitForSingleObject(done, INFINITE) == WAIT_OBJECT_0) return SIGINT;
    return 0;
}

int cpr_sigwait() {
    return cpr_sigwaitall();
}

void cpr_grpmask() {
}

bool cpr_service() {
    bool isService = false;
    DWORD pid = GetCurrentProcessId();
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return false;
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hSnapshot, &pe)) {
        do {
            if (pe.th32ProcessID == pid) {
                if (Process32First(hSnapshot, &pe)) {
                    do {
                        DWORD parentPid = pe.th32ParentProcessID;
                        if (pe.th32ProcessID == parentPid) {
                            isService = eq(pe.szExeFile, "services.exe");
                            break;
                        }
                    } while (Process32Next(hSnapshot, &pe));
                }
                break;
            }
        } while (Process32Next(hSnapshot, &pe));
    }
    CloseHandle(hSnapshot);
    return isService;
}

__attribute__((format(printf, 2, 3))) void cpr_syslog(int priority, const char *fmt, ...) {}
void cpr_closelog() {}
void cpr_openlog(const char *id, int facility, int flags) {}
#else

void cpr_signals(int *exiting) {
    sigemptyset(&sigs);
    sigaddset(&sigs, SIGALRM);
    sigaddset(&sigs, SIGINT);
    sigaddset(&sigs, SIGKILL);
    sigaddset(&sigs, SIGTERM);
    sigaddset(&sigs, SIGHUP);
    sigaddset(&sigs, SIGCHLD);
    sigaddset(&sigs, SIGPIPE); // block for tcp
    pthread_sigmask(SIG_BLOCK, &sigs, NULL);
    excode = exiting;
}

int cpr_sigwaitall() {
    int sig = 0;
    sigdelset(&sigs, SIGPIPE);
retry:
    if (interval) alarm(interval);
    if (sigwait(&sigs, &sig)) return 0;
    if (sig == SIGALRM) {
        if (sigtimer) sigtimer();
        goto retry;
    }
    if (sig != SIGTERM) *excode = sig;
    return sig;
}

int cpr_sigwait() {
    sigdelset(&sigs, SIGHUP);
    sigdelset(&sigs, SIGCHLD);
    return cpr_sigwaitall();
}

void cpr_grpmask() {
    umask(007);
}

bool cpr_service() {
    return getuid() == 0 || getpid() == 1 || getppid() == 1;
}

__attribute__((format(printf, 2, 3))) void cpr_syslog(int priority, const char *fmt, ...) {
    if (!logger) return;
    va_list args;
    va_start(args, fmt);
    vsyslog(priority, fmt, args);
    va_end(args);
}

void cpr_closelog() {
    logger = false;
    closelog();
}

void cpr_openlog(const char *id, int facility, int flags) {
    if (!id) return;
    int level = LOG_INFO;
    switch (cpr_verbose) {
    case 0:
        level = LOG_ERR;
        break;
    case 1:
        level = LOG_WARNING;
        break;
    case 2:
        level = LOG_NOTICE;
        break;
    case 3:
        level = LOG_INFO;
        break;
    default:
        level = LOG_DEBUG;
        break;
    }
    logger = true;
    openlog(id, flags, facility);
    setlogmask(LOG_UPTO(level));
}
#endif

void cpr_sigtimer(int seconds, cpr_timer_t proc) {
    interval = seconds;
    sigtimer = proc;
}

__attribute__((noreturn)) __attribute__((format(printf, 2, 3))) void cpr_sysexit(int exit_code, const char *fmt, ...) {
#ifndef _WIN32
    if (logger) {
        va_list args;
        va_start(args, fmt);
        vsyslog(LOG_CRIT, fmt, args);
        va_end(args);
    }
#endif
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fputc('\n', stderr);
    fflush(stderr);
    va_end(args);
    quick_exit(exit_code);
}

__attribute__((format(printf, 3, 4))) void cpr_logger(FILE *out, int level, const char *fmt, ...) {
    static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
    const char *type = "info";
#ifndef _WIN32
    int priority = -1;
#endif
    switch (level) {
    case 0:
        type = "fatal";
#ifndef _WIN32
        priority = LOG_ERR;
#endif
        break;
    case 1:
        type = "warning";
#ifndef _WIN32
        priority = LOG_WARNING;
#endif
        break;
    case 2:
        type = "notice";
#ifndef _WIN32
        priority = LOG_NOTICE;
#endif
        break;
    case 3:
#ifndef _WIN32
        priority = LOG_INFO;
#endif
        break;
    default:
#if !defined(NDEBUG) && !defined(_WIN32)
        priority = LOG_DEBUG;
#endif
        break;
    }
#ifndef _WIN32
    if (priority != -1 && logger) {
        va_list args;
        va_start(args, fmt);
        vsyslog(priority, fmt, args);
        va_end(args);
    }
#endif
    if (level <= cpr_verbose) {
        pthread_mutex_lock(&mtx);
        time_t now;
        struct tm ts = {0};
        char buf[80];
        time(&now);
        cpr_localtime(&now, &ts);
        strftime(buf, sizeof(buf), LOG_DATETIME_FORMAT, &ts);
        fprintf(out, "%s %s: ", buf, type);
        if (logger) {
            va_list args;
            va_start(args, fmt);
            vfprintf(out, fmt, args);
            va_end(args);
        } else if (out != stderr && out != stdout) {
            va_list args;
            va_start(args, fmt);
            vfprintf(stderr, fmt, args);
            fprintf(stderr, "\n");
            va_end(args);
        }
        va_list args;
        va_start(args, fmt);
        vfprintf(out, fmt, args);
        fputc('\n', out);
        fflush(out);
        pthread_mutex_unlock(&mtx);
        va_end(args);
    }
}
