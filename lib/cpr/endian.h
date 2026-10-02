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
    return ((uint16_t)(ptr[0]) << 8U) | (uint16_t)(ptr[1]); // NOLINT
}

static inline uint32_t be_get32(const uint8_t *ptr) {
    return ((uint32_t)(ptr[0]) << 24U) | ((uint32_t)(ptr[1]) << 16U) |
           ((uint32_t)(ptr[2]) << 8U) | (uint32_t)(ptr[3]);
}

static inline uint64_t be_get64(const uint8_t *ptr) {
    return ((uint64_t)(ptr[0]) << 56U) | ((uint64_t)(ptr[1]) << 48U) |
           ((uint64_t)(ptr[2]) << 40U) | ((uint64_t)(ptr[3]) << 32U) |
           ((uint64_t)(ptr[4]) << 24U) | ((uint64_t)(ptr[5]) << 16U) |
           ((uint64_t)(ptr[6]) << 8U) | (uint64_t)(ptr[7]);
}

static inline uint8_t le_get8(const uint8_t *ptr) {
    return *ptr;
}

static inline uint16_t le_get16(const uint8_t *ptr) {
    return ((uint16_t)(ptr[1]) << 8U) | (uint16_t)(ptr[0]); // NOLINT
}

static inline uint32_t le_get32(const uint8_t *ptr) {
    return ((uint32_t)(ptr[3]) << 24U) | ((uint32_t)(ptr[2]) << 16U) |
           ((uint32_t)(ptr[1]) << 8U) | (uint32_t)(ptr[0]);
}

static inline uint64_t le_get64(const uint8_t *ptr) {
    return ((uint64_t)(ptr[7]) << 56U) | ((uint64_t)(ptr[6]) << 48U) |
           ((uint64_t)(ptr[5]) << 40U) | ((uint64_t)(ptr[4]) << 32U) |
           ((uint64_t)(ptr[3]) << 24U) | ((uint64_t)(ptr[2]) << 16U) |
           ((uint64_t)(ptr[1]) << 8U) | (uint64_t)(ptr[0]);
}

static inline void be_set8(uint8_t *ptr, uint8_t value) {
    *ptr = value;
}

static inline void be_set16(uint8_t *ptr, uint16_t value) {
    ptr[0] = (uint8_t)((value & 0xff00U) >> 8U);
    ptr[1] = (uint8_t)(value & 0xffU);
}

static inline void be_set32(uint8_t *ptr, uint32_t value) {
    ptr[0] = (uint8_t)((value & 0xff000000UL) >> 24U);
    ptr[1] = (uint8_t)((value & 0x00ff0000UL) >> 16U);
    ptr[2] = (uint8_t)((value & 0x0000ff00UL) >> 8U);
    ptr[3] = (uint8_t)(value & 0xffUL);
}

static inline void be_set64(uint8_t *ptr, uint64_t value) {
    ptr[0] = (uint8_t)((value & 0xff00000000000000ULL) >> 56U);
    ptr[1] = (uint8_t)((value & 0x00ff000000000000ULL) >> 48U);
    ptr[2] = (uint8_t)((value & 0x0000ff0000000000ULL) >> 40U);
    ptr[3] = (uint8_t)((value & 0x000000ff00000000ULL) >> 32U);
    ptr[4] = (uint8_t)((value & 0x00000000ff000000ULL) >> 24U);
    ptr[5] = (uint8_t)((value & 0x0000000000ff0000ULL) >> 16U);
    ptr[6] = (uint8_t)((value & 0x000000000000ff00ULL) >> 8U);
    ptr[7] = (uint8_t)(value & 0xffULL);
}

static inline void le_set8(uint8_t *ptr, uint8_t value) {
    *ptr = value;
}

static inline void le_set16(uint8_t *ptr, uint16_t value) {
    ptr[1] = (uint8_t)((value & 0xff00U) >> 8U);
    ptr[0] = (uint8_t)(value & 0xffU);
}

static inline void le_set32(uint8_t *ptr, uint32_t value) {
    ptr[3] = (uint8_t)((value & 0xff000000UL) >> 24U);
    ptr[2] = (uint8_t)((value & 0x00ff0000UL) >> 16U);
    ptr[1] = (uint8_t)((value & 0x0000ff00UL) >> 8U);
    ptr[0] = (uint8_t)(value & 0xffUL);
}

static inline void le_set64(uint8_t *ptr, uint64_t value) {
    ptr[7] = (uint8_t)((value & 0xff00000000000000ULL) >> 56U);
    ptr[6] = (uint8_t)((value & 0x00ff000000000000ULL) >> 48U);
    ptr[5] = (uint8_t)((value & 0x0000ff0000000000ULL) >> 40U);
    ptr[4] = (uint8_t)((value & 0x000000ff00000000ULL) >> 32U);
    ptr[3] = (uint8_t)((value & 0x00000000ff000000ULL) >> 24U);
    ptr[2] = (uint8_t)((value & 0x0000000000ff0000ULL) >> 16U);
    ptr[1] = (uint8_t)((value & 0x000000000000ff00U) >> 8U);
    ptr[0] = (uint8_t)(value & 0xffULL);
}

#ifdef __cplusplus
}
#endif
#endif
