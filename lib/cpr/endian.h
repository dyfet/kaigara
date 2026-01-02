// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#ifndef CPR_ENDIAN_H
#define CPR_ENDIAN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline uint8_t be_get8(const uint8_t *ptr) {
    return *ptr;
}

static inline uint16_t be_get16(const uint8_t *ptr) {
    return ((uint16_t)(ptr[0]) << 8) | (uint16_t)(ptr[1]);
}

static inline uint32_t be_get32(const uint8_t *ptr) {
    return ((uint32_t)(ptr[0]) << 24) | ((uint32_t)(ptr[1]) << 16) |
           ((uint32_t)(ptr[2]) << 8) | (uint32_t)(ptr[3]);
}

static inline uint64_t be_get64(const uint8_t *ptr) {
    return ((uint64_t)(ptr[0]) << 56) | ((uint64_t)(ptr[1]) << 48) |
           ((uint64_t)(ptr[2]) << 40) | ((uint64_t)(ptr[3]) << 32) |
           ((uint64_t)(ptr[4]) << 24) | ((uint64_t)(ptr[5]) << 16) |
           ((uint64_t)(ptr[6]) << 8) | (uint64_t)(ptr[7]);
}

static inline uint8_t le_get8(const uint8_t *ptr) {
    return *ptr;
}

static inline uint16_t le_get16(const uint8_t *ptr) {
    return ((uint16_t)(ptr[1]) << 8) | (uint16_t)(ptr[0]);
}

static inline uint32_t le_get32(const uint8_t *ptr) {
    return ((uint32_t)(ptr[3]) << 24) | ((uint32_t)(ptr[2]) << 16) |
           ((uint32_t)(ptr[1]) << 8) | (uint32_t)(ptr[0]);
}

static inline uint64_t le_get64(const uint8_t *ptr) {
    return ((uint64_t)(ptr[7]) << 56) | ((uint64_t)(ptr[6]) << 48) |
           ((uint64_t)(ptr[5]) << 40) | ((uint64_t)(ptr[4]) << 32) |
           ((uint64_t)(ptr[3]) << 24) | ((uint64_t)(ptr[2]) << 16) |
           ((uint64_t)(ptr[1]) << 8) | (uint64_t)(ptr[0]);
}

static inline void be_set8(uint8_t *ptr, uint8_t value) {
    *ptr = value;
}

static inline void be_set16(uint8_t *ptr, uint16_t value) {
    ptr[0] = (uint8_t)((value & 0xff00) >> 8);
    ptr[1] = (uint8_t)(value & 0xff);
}

static inline void be_set32(uint8_t *ptr, uint32_t value) {
    ptr[0] = (uint8_t)((value & 0xff000000) >> 24);
    ptr[1] = (uint8_t)((value & 0x00ff0000) >> 16);
    ptr[2] = (uint8_t)((value & 0x0000ff00) >> 8);
    ptr[3] = (uint8_t)(value & 0xff);
}

static inline void be_set64(uint8_t *ptr, uint64_t value) {
    ptr[0] = (uint8_t)((value & 0xff00000000000000) >> 56);
    ptr[1] = (uint8_t)((value & 0x00ff000000000000) >> 48);
    ptr[2] = (uint8_t)((value & 0x0000ff0000000000) >> 40);
    ptr[3] = (uint8_t)((value & 0x000000ff00000000) >> 32);
    ptr[4] = (uint8_t)((value & 0x00000000ff000000) >> 24);
    ptr[5] = (uint8_t)((value & 0x0000000000ff0000) >> 16);
    ptr[6] = (uint8_t)((value & 0x000000000000ff00) >> 8);
    ptr[7] = (uint8_t)(value & 0xff);
}

static inline void le_set8(uint8_t *ptr, uint8_t value) {
    *ptr = value;
}

static inline void le_set16(uint8_t *ptr, uint16_t value) {
    ptr[1] = (uint8_t)((value & 0xff00) >> 8);
    ptr[0] = (uint8_t)(value & 0xff);
}

static inline void le_set32(uint8_t *ptr, uint32_t value) {
    ptr[3] = (uint8_t)((value & 0xff000000) >> 24);
    ptr[2] = (uint8_t)((value & 0x00ff0000) >> 16);
    ptr[1] = (uint8_t)((value & 0x0000ff00) >> 8);
    ptr[0] = (uint8_t)(value & 0xff);
}

static inline void le_set64(uint8_t *ptr, uint64_t value) {
    ptr[7] = (uint8_t)((value & 0xff00000000000000) >> 56);
    ptr[6] = (uint8_t)((value & 0x00ff000000000000) >> 48);
    ptr[5] = (uint8_t)((value & 0x0000ff0000000000) >> 40);
    ptr[4] = (uint8_t)((value & 0x000000ff00000000) >> 32);
    ptr[3] = (uint8_t)((value & 0x00000000ff000000) >> 24);
    ptr[2] = (uint8_t)((value & 0x0000000000ff0000) >> 16);
    ptr[1] = (uint8_t)((value & 0x000000000000ff00) >> 8);
    ptr[0] = (uint8_t)(value & 0xff);
}

#ifdef __cplusplus
}
#endif
#endif
