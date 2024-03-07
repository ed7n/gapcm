/**
 * EDCC: Math
 *
 * Arithmetic functions.
 */
#ifndef _EDCC_MATH_H
#define _EDCC_MATH_H

#include <stdint.h>

/** Returns the larger of the two given 8-bit integers. */
int8_t math_max_s8(const int8_t a, const int8_t b);

/** Returns the larger of the two given unsigned 8-bit integers. */
uint8_t math_max_u8(const uint8_t a, const uint8_t b);

/** Returns the larger of the two given 16-bit integers. */
int16_t math_max_s16(const int16_t a, const int16_t b);

/** Returns the larger of the two given unsinged 16-bit integers. */
uint16_t math_max_u16(const uint16_t a, const uint16_t b);

/** Returns the larger of the two given 32-bit integers. */
int32_t math_max_s32(const int32_t a, const int32_t b);

/** Returns the larger of the two given unsigned 32-bit integers. */
uint32_t math_max_u32(const uint32_t a, const uint32_t b);

/** Returns the larger of the two given 64-bit integers. */
int64_t math_max_s64(const int64_t a, const int64_t b);

/** Returns the larger of the two given unsinged 64-bit integers. */
uint64_t math_max_u64(const uint64_t a, const uint64_t b);

/** Returns the smaller of the two given 8-bit integers. */
int8_t math_min_s8(const int8_t a, const int8_t b);

/** Returns the smaller of the two given unsigned 8-bit integers. */
uint8_t math_min_u8(const uint8_t a, const uint8_t b);

/** Returns the smaller of the two given 16-bit integers. */
int16_t math_min_s16(const int16_t a, const int16_t b);

/** Returns the smaller of the two given unsinged 16-bit integers. */
uint16_t math_min_u16(const uint16_t a, const uint16_t b);

/** Returns the smaller of the two given 32-bit integers. */
int32_t math_min_s32(const int32_t a, const int32_t b);

/** Returns the smaller of the two given unsigned 32-bit integers. */
uint32_t math_min_u32(const uint32_t a, const uint32_t b);

/** Returns the smaller of the two given 64-bit integers. */
int64_t math_min_s64(const int64_t a, const int64_t b);

/** Returns the smaller of the two given unsinged 64-bit integers. */
uint64_t math_min_u64(const uint64_t a, const uint64_t b);

#endif
