// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#ifndef CPR_SERVICE_H
#define CPR_SERVICE_H

#include "system.h"

#ifndef _WIN32
#include <syslog.h>
#else
#define LOG_AUTH 0
#define LOG_AUTHPRIV 0
#define LOG_DAEMON 0
#define LOG_EMERG 0
#define LOG_CRIT 0
#define LOG_INFO 1
#define LOG_WARNING 2
#define LOG_NOTICE 3
#define LOG_ERR 0
#define LOG_DEBUG 4
#define LOG_NDELAY 0
#define LOG_NOWAIT 0
#define LOG_PERROR 0
#define LOG_PID 0
#endif

bool cpr_service();
__attribute__((noreturn)) __attribute__((format(printf, 2, 3))) void cpr_sysexit(int exit_code, const char *fmt, ...);
__attribute__((format(printf, 3, 4))) void cpr_logger(FILE *out, int level, const char *fmt, ...);
__attribute__((format(printf, 2, 3))) void cpr_syslog(int priority, const char *fmt, ...);
void cpr_openlog(const char *id, int facility, int flags);
void cpr_closelog();
#endif
