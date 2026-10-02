// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 202y David Sugar <tychosoft@gmail.com>

#ifndef LOCAL_VPN_H
#define LOCAL_VPN_H

#include <local/net.h>

bool vpn_detect(const struct sockaddr *sa);
int vpn_connect(const struct sockaddr *addr);
#endif
