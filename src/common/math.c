#include "math.h"

#define MATH_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MATH_MIN(a, b) (((a) < (b)) ? (a) : (b))

int8_t math_max_s8(const int8_t a, const int8_t b) { return MATH_MAX(a, b); }

uint8_t math_max_u8(const uint8_t a, const uint8_t b) { return MATH_MAX(a, b); }

int16_t math_max_s16(const int16_t a, const int16_t b) {
  return MATH_MAX(a, b);
}

uint16_t math_max_u16(const uint16_t a, const uint16_t b) {
  return MATH_MAX(a, b);
}

int32_t math_max_s32(const int32_t a, const int32_t b) {
  return MATH_MAX(a, b);
}

uint32_t math_max_u32(const uint32_t a, const uint32_t b) {
  return MATH_MAX(a, b);
}

int64_t math_max_s64(const int64_t a, const int64_t b) {
  return MATH_MAX(a, b);
}

uint64_t math_max_u64(const uint64_t a, const uint64_t b) {
  return MATH_MAX(a, b);
}

int8_t math_min_s8(const int8_t a, const int8_t b) { return MATH_MIN(a, b); }

uint8_t math_min_u8(const uint8_t a, const uint8_t b) { return MATH_MIN(a, b); }

int16_t math_min_s16(const int16_t a, const int16_t b) {
  return MATH_MIN(a, b);
}

uint16_t math_min_u16(const uint16_t a, const uint16_t b) {
  return MATH_MIN(a, b);
}

int32_t math_min_s32(const int32_t a, const int32_t b) {
  return MATH_MIN(a, b);
}

uint32_t math_min_u32(const uint32_t a, const uint32_t b) {
  return MATH_MIN(a, b);
}

int64_t math_min_s64(const int64_t a, const int64_t b) {
  return MATH_MIN(a, b);
}

uint64_t math_min_u64(const uint64_t a, const uint64_t b) {
  return MATH_MIN(a, b);
}

#undef MATH_MAX
#undef MATH_MIN
