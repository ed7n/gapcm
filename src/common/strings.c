#include "strings.h"
#include "constants.h"
#include <stdarg.h>
#include <string.h>

/** `string_equals_allany` mode: all. */
#define STRINGS_EQUALS_ALL true
/** `string_equals_allany` mode: any. */
#define STRINGS_EQUALS_ANY false

/**
 * Returns whether the given string is equal to all (true) or any (false) of the
 * given comparands. Parenthesized values are for `preset`.
 */
static bool string_equals_allany(const char *string, const bool preset,
                                 int count, va_list comparands) {
  while (count-- > 0) {
    if ((strcmp(string, va_arg(comparands, char *)) == SUCCESS) != preset) {
      return !preset;
    }
  }
  return preset;
}

bool string_equals(const char *restrict string,
                   const char *restrict comparand) {
  return strcmp(string, comparand) == SUCCESS;
}

bool string_equals_all(const char *string, int count, ...) {
  va_list comparands;
  va_start(comparands, count);
  bool out =
      string_equals_allany(string, STRINGS_EQUALS_ALL, count, comparands);
  va_end(comparands);
  return out;
}

bool string_equals_any(const char *string, int count, ...) {
  va_list comparands;
  va_start(comparands, count);
  bool out =
      string_equals_allany(string, STRINGS_EQUALS_ANY, count, comparands);
  va_end(comparands);
  return out;
}
