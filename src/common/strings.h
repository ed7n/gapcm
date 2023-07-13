/**
 * EDCC: Strings
 *
 * Functions for operating on strings.
 */
#ifndef _EDCC_STRINGS_H
#define _EDCC_STRINGS_H

#include <stdbool.h>

/** Safely performs the given assignment. */
void string_assign(const char **assignee, const char *assigner);

/** Returns whether the given string is equal to the given comparand. */
bool string_equals(const char *string, const char *comparand);

/** Returns whether the given string is equal to all of the given comparands. */
bool string_equals_all(const char *string, int count, ...);

/** Returns whether the given string is equal to any of the given comparands. */
bool string_equals_any(const char *string, int count, ...);

#endif
