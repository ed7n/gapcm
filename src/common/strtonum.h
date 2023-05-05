/**
 * EDCC: String to Number
 *
 * Functions for parsing of strings to numbers.
 *
 * The base of a given string representation is determined between octal (8),
 * decimal (10), and hexadecimal (16) by its format. See the standard library
 * function `strtol` for details. A given non-NULL `error` points to the message
 * of the first encountered error.
 *
 * Support for floats is not needed at this time and are hence excluded.
 */
#ifndef _EDCC_STRTONUM_H
#define _EDCC_STRTONUM_H

#include <stdint.h>

#define STRTONUM_ERROR_BASE "The base is invalid (?)."
#define STRTONUM_ERROR_NAN "The string is not a number."
#define STRTONUM_ERROR_OVER "The number is greater than the maximum."
#define STRTONUM_ERROR_OVERFLOW "The number is too large."
#define STRTONUM_ERROR_RANGE "The number is out of range."
#define STRTONUM_ERROR_UNDER "The number is less than the minimum."
#define STRTONUM_ERROR_UNDERFLOW "The negative number is too large."

/**
 * Parses a long long integer from the given string.
 *
 * Compared to the BSD equivalent, it has different error messages and allows
 * the minimum to be greater than the maximum. See
 * [https://man.openbsd.org/strtonum] for details.
 */
long long strtonum(const char *string, long long minimum, long long maximum,
                   const char **error);

/** Parses an 8-bit integer from the given string. */
int8_t strtoint_s8(const char *string, const char **error);

/** Parses an unsigned 8-bit integer from the given string. */
uint8_t strtoint_u8(const char *string, const char **error);

/** Parses a 16-bit integer from the given string. */
int16_t strtoint_s16(const char *string, const char **error);

/** Parses an unsigned 16-bit integer from the given string. */
uint16_t strtoint_u16(const char *string, const char **error);

/** Parses a 32-bit integer from the given string. */
int32_t strtoint_s32(const char *string, const char **error);

/** Parses an unsigned 32-bit integer from the given string. */
uint32_t strtoint_u32(const char *string, const char **error);

/** Parses a 64-bit integer from the given string. */
int64_t strtoint_s64(const char *string, const char **error);

/** Parses an unsigned 64-bit integer from the given string. */
uint64_t strtoint_u64(const char *string, const char **error);

#endif
