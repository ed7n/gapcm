/**
 * EDCC: Application
 *
 * Application utilities.
 *
 * Unless trivial or stated otherwise, operator functions return their success
 * with `EXIT_SUCCESS` or `true`, and `EXIT_FAILURE` or `false`.
 */
#ifndef _EDCC_APPLICATION_H
#define _EDCC_APPLICATION_H

#include <stdbool.h>
#include <stdio.h>

/** Standard stream name. */
#define APPLICATION_FILE_NAME_STANDARD "-"

/** Represents a parse context. */
struct ApplicationParseContext {
  /** Arguments to parse. */
  char **arguments;
  /** Current argument. */
  char *argument;
  /** Current option. */
  char *option;
  /** Count of arguments to parse. */
  int COUNT;
  /** Current index. */
  int index;
  /** Exit code. */
  int out;
  /** Parse options? */
  bool parse_options;
};

int application_error_argument_bad(const char *option, const char *argument,
                                   const char *body);

int application_error_argument_nul(const char *option);

int application_error_option_bad(const char *option);

/** Closes the given file. Closing NULL returns success. */
int application_file_close(FILE *file, const char *name);

/**
 * Opens the given file by its name to the given location. The given description
 * should be preceded by a word separator.
 */
int application_file_open(const char *name, const char *modes, FILE **file,
                          const char *description, FILE *standard);

/** Parses an integer from the given context to the given location. */
int application_parse_integer(struct ApplicationParseContext *c,
                              long long *integer, long long minimum,
                              long long maximum, const char *range);

/** Parses a string from the given context to the given location. */
int application_parse_string(struct ApplicationParseContext *context,
                             char **string);

/** Frees the given parse context. */
struct ApplicationParseContext *
application_parsecontext_free(struct ApplicationParseContext *context);

/** Makes a parse context with the given. */
struct ApplicationParseContext *application_parsecontext_make(char *arguments[],
                                                              int count);

/** Prints the given message. */
bool application_print_message(const char *head, const char *body);

/** Prints the given strings. */
bool application_print_strings(int count, const char *string, ...);

#endif
