#include "strtonum.h"
#include "constants.h"
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

/**
 * Checks the given integral parse for errors and sets `errno` and the given
 * message accordingly.
 */
static long long strtoll_check(const long long number, char **restrict position,
                               const long long minimum, const long long maximum,
                               const char **restrict error) {
  switch (errno) {
  case SUCCESS:
    if (**position != '\0') {
      errno = EINVAL;
      if (error != NULL) {
        *error = STRTONUM_ERROR_NAN;
      }
    } else if (number > maximum) {
      errno = ERANGE;
      if (error != NULL) {
        *error = STRTONUM_ERROR_OVER;
      }
    } else if (number < minimum) {
      errno = ERANGE;
      if (error != NULL) {
        *error = STRTONUM_ERROR_UNDER;
      }
    } else if (error != NULL) {
      *error = NULL;
    }
    break;
  case EINVAL:
    if (error != NULL) {
      *error = STRTONUM_ERROR_BASE;
    }
    break;
  case ERANGE:
    if (error != NULL) {
      *error = number == LLONG_MAX   ? STRTONUM_ERROR_OVERFLOW
               : number == LLONG_MIN ? STRTONUM_ERROR_UNDERFLOW
                                     : STRTONUM_ERROR_RANGE;
    }
  }
  return number;
}

/** Parses a long long integer from the given string. */
static long long strtoint_parse(const char *restrict string,
                                char **restrict position) {
  errno = SUCCESS;
  return strtoll(string, position, 0);
}

/**
 * Checks the given unsigned integral parse for errors and sets `errno` and the
 * given message accordingly.
 */
static unsigned long long strtoull_check(const unsigned long long number,
                                         char **restrict position,
                                         const unsigned long long minimum,
                                         const unsigned long long maximum,
                                         const char **restrict error) {
  switch (errno) {
  case SUCCESS:
    if (**position != '\0') {
      errno = EINVAL;
      if (error != NULL) {
        *error = STRTONUM_ERROR_NAN;
      }
    } else if (number > maximum) {
      errno = ERANGE;
      if (error != NULL) {
        *error = STRTONUM_ERROR_OVER;
      }
    } else if (number < minimum) {
      errno = ERANGE;
      if (error != NULL) {
        *error = STRTONUM_ERROR_UNDER;
      }
    } else if (error != NULL) {
      *error = NULL;
    }
    break;
  case EINVAL:
    if (error != NULL) {
      *error = STRTONUM_ERROR_BASE;
    }
    break;
  case ERANGE:
    if (error != NULL) {
      *error =
          number == ULLONG_MAX ? STRTONUM_ERROR_OVERFLOW : STRTONUM_ERROR_RANGE;
    }
  }
  return number;
}

/** Parses an unsigned long long integer from the given string. */
static unsigned long long strtouint_parse(const char *restrict string,
                                          char **restrict position) {
  errno = SUCCESS;
  return strtoull(string, position, 0);
}

long long strtonum(const char *restrict string, const long long minimum,
                   const long long maximum, const char **restrict error) {
  char *position = NULL;
  return strtoll_check(strtoint_parse(string, &position), &position, minimum,
                       maximum, error);
}

int8_t strtoint_s8(const char *restrict string, const char **restrict error) {
  char *position = NULL;
  return strtoll_check(strtoint_parse(string, &position), &position, INT8_MIN,
                       INT8_MAX, error);
}

uint8_t strtoint_u8(const char *restrict string, const char **restrict error) {
  char *position = NULL;
  return strtoull_check(strtouint_parse(string, &position), &position, 0,
                        UINT8_MAX, error);
}

int16_t strtoint_s16(const char *restrict string, const char **restrict error) {
  char *position = NULL;
  return strtoll_check(strtoint_parse(string, &position), &position, INT16_MIN,
                       INT16_MAX, error);
}

uint16_t strtoint_u16(const char *restrict string,
                      const char **restrict error) {
  char *position = NULL;
  return strtoull_check(strtouint_parse(string, &position), &position, 0,
                        UINT16_MAX, error);
}

int32_t strtoint_s32(const char *restrict string, const char **restrict error) {
  char *position = NULL;
  return strtoll_check(strtoint_parse(string, &position), &position, INT32_MIN,
                       INT32_MAX, error);
}

uint32_t strtoint_u32(const char *restrict string,
                      const char **restrict error) {
  char *position = NULL;
  return strtoull_check(strtouint_parse(string, &position), &position, 0,
                        UINT32_MAX, error);
}

int64_t strtoint_s64(const char *restrict string, const char **restrict error) {
  char *position = NULL;
  return strtoll_check(strtoint_parse(string, &position), &position, INT64_MIN,
                       INT64_MAX, error);
}

uint64_t strtoint_u64(const char *restrict string,
                      const char **restrict error) {
  char *position = NULL;
  return strtoull_check(strtouint_parse(string, &position), &position, 0,
                        UINT64_MAX, error);
}
