// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#ifndef DEFINES_H
#define DEFINES_H

// Tuning
#ifndef TUNE_FAMILY_INET
#define TUNE_FAMILY_INET    AF_INET
#endif

// Linting defines
#ifndef PROJECT_VERSION
#define PROJECT_VERSION "unknown"
#endif

#if (TUNE_FAMILY_INET != AF_INET) && (TUNE_FAMILY_INET != AF_INET6)
#error "Server onlu supports ipv4 or ipv6 default"
#endif

#endif
