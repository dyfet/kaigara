// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#ifndef SIOT_CRC_H
#define SIOT_CRC_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

uint16_t sio_crc16(const uint8_t *data, size_t size);
uint32_t sio_crc32(const uint8_t *data, size_t size);

#ifdef __cplusplus
}
#endif
#endif
