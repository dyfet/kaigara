// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include "service.h"
#include "strchar.h"
#include "thread.h"

#ifdef _WIN32
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
                DWORD parentPid = pe.th32ParentProcessID;
                if (Process32First(hSnapshot, &pe)) {
                    do {
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
static bool logger = false;

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
        level = LOG_INFO;
        break;
    case 2:
        level = LOG_NOTICE;
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

__attribute__((noreturn)) __attribute__((format(printf, 2, 3))) void cpr_sysexit(int exit_code, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

#ifndef _WIN32
    if (logger)
        vsyslog(LOG_CRIT, fmt, args);
#endif
    vfprintf(stderr, fmt, args); // FlawFinder: ignore
    fputc('\n', stderr);
    fflush(stderr);
    va_end(args);
    quick_exit(exit_code);
}

__attribute__((format(printf, 3, 4))) void cpr_logger(FILE *out, int level, const char *fmt, ...) {
    static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
    va_list args;
    va_start(args, fmt);
    const char *type = "note";
#ifndef _WIN32
    int priority = -1;
#endif
    switch (level) {
    case 0:
        type = "fail";
#ifndef _WIN32
        priority = LOG_ERR;
#endif
        break;
    case 1:
        type = "warn";
#ifndef _WIN32
        priority = LOG_WARNING;
#endif
        break;
    case 2:
        type = "info";
#ifndef _WIN32
        priority = LOG_INFO;
#endif
        break;
    case 3:
#ifndef _WIN32
        priority = LOG_NOTICE;
#endif
        cpr_syslog(LOG_NOTICE, fmt, args);
        break;
    default:
#if !defined(NDEBUG) && !defined(_WIN32)
        priority = LOG_DEBUG;
#endif
        break;
    }
#ifndef _WIN32
    if (priority != -1 && logger) vsyslog(priority, fmt, args);
#endif
    if (cpr_verbose <= level) {
        pthread_mutex_lock(&mtx);
        time_t now;
        struct tm ts = {0};
        char buf[80];
        time(&now);
#ifdef _WIN32
        localtime_s(&ts, &now);
#else
        localtime_r(&now, &ts);
#endif
        strftime(buf, sizeof(buf), LOG_DATETIME_FORMAT, &ts);
        fprintf(out, "%s %s: ", buf, type);
        vfprintf(out, fmt, args); // FlawFinder: ignore
        fputc('\n', out);
        fflush(out);
        pthread_mutex_unlock(&mtx);
    }
    va_end(args);
}
