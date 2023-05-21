/**
 * Undo setting of `errno` to `EBADF` on `fflush`. Flushing input streams is
 * undefined on older POSIX systems where `EBADF` may be set. This is defined by
 * default for compatibility and should be undefined for debugging.
 */
#define APPLICATION_FFLUSH_EBADF

#include "application.h"
#include "constants.h"
#include "strings.h"
#include "strtonum.h"
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/** Stream to which messages are to be printed. */
#define APPLICATION_FILE_PRINT stderr

int application_error_argument_bad(const char *restrict option,
                                   const char *restrict argument,
                                   const char *restrict body) {
  application_print_strings(7, "Invalid `", option, "` argument `", argument,
                            "`: ", body, EOL);
  return EXIT_FAILURE;
}

int application_error_argument_nul(const char *restrict option) {
  application_print_strings(4, "Insufficient arguments for `", option, "`.",
                            EOL);
  return EXIT_FAILURE;
}

int application_error_option_bad(const char *restrict option) {
  application_print_message("Invalid option", option);
  return EXIT_FAILURE;
}

int application_file_close(FILE *file, const char *restrict name) {
  if (file == NULL) {
    return EXIT_SUCCESS;
  }
  errno = SUCCESS;
  if (file == stdin || file == stdout || file == stderr) {
    if (fflush(file) == SUCCESS) {
      return EXIT_SUCCESS;
    }
#ifdef APPLICATION_FFLUSH_EBADF
    if (file == stdin && errno == EBADF) {
      errno = SUCCESS;
      return EXIT_SUCCESS;
    }
#endif
  } else if (fclose(file) == SUCCESS) {
    return EXIT_SUCCESS;
  }
  application_print_message(name, strerror(errno));
  return EXIT_FAILURE;
}

int application_file_open(const char *restrict name, const char *restrict modes,
                          FILE **restrict file,
                          const char *restrict description,
                          FILE *restrict standard) {
  if (name == NULL || string_equals(name, NUL_STRING)) {
    application_print_strings(4, "No", description, " file.", EOL);
    return EXIT_FAILURE;
  }
  if (string_equals(name, APPLICATION_FILE_NAME_STANDARD)) {
    if (ferror(standard) != SUCCESS) {
      application_print_strings(5, name,
                                ": An error had occurred on the standard",
                                description, " stream.", EOL);
      return EXIT_FAILURE;
    }
    *file = standard;
  } else {
    errno = SUCCESS;
    *file = fopen(name, modes);
    if (*file == NULL || errno) {
      application_print_message(name, strerror(errno));
      return EXIT_FAILURE;
    }
  }
  return EXIT_SUCCESS;
}

int application_parse_integer(struct ApplicationParseContext *c,
                              long long *integer, const long long minimum,
                              const long long maximum,
                              const char *restrict range) {
  if (c->index >= c->COUNT) {
    return application_error_argument_nul(c->option);
  }
  const char *error = NULL;
  c->argument = c->arguments[c->index++];
  long long result = strtonum(c->argument, minimum, maximum, &error);
  if (error != NULL) {
    application_error_argument_bad(c->option, c->argument, error);
    if (range != NULL && errno == ERANGE) {
      application_print_message("Valid range", range);
    }
    return EXIT_FAILURE;
  } else if (errno) {
    return application_error_argument_bad(c->option, c->argument,
                                          strerror(errno));
  }
  *integer = result;
  return EXIT_SUCCESS;
}

int application_parse_string(struct ApplicationParseContext *c,
                             char **restrict string) {
  if (c->index >= c->COUNT) {
    return application_error_argument_nul(c->option);
  }
  c->argument = c->arguments[c->index++];
  *string = malloc(sizeof(**string) * (strlen(c->argument) + 1));
  strcpy(*string, c->argument);
  return EXIT_SUCCESS;
}

bool application_print_message(const char *restrict head,
                               const char *restrict body) {
  return application_print_strings(4, head, ": ", body, EOL);
}

bool application_print_strings(int count, const char *restrict string, ...) {
  if (fputs(string, APPLICATION_FILE_PRINT) == EOF) {
    return false;
  }
  count--;
  va_list strings;
  va_start(strings, string);
  bool out = true;
  while (count-- > 0) {
    if (fputs(va_arg(strings, char *), APPLICATION_FILE_PRINT) == EOF) {
      out = false;
      break;
    }
  }
  va_end(strings);
  return out;
}

struct ApplicationParseContext *
application_parsecontext_free(struct ApplicationParseContext *c) {
  for (int index = 0; index < c->COUNT; index++) {
    free(c->arguments[index]);
  }
  free(c->arguments);
  free(c);
  return NULL;
}

struct ApplicationParseContext *application_parsecontext_make(char *arguments[],
                                                              int count) {
  struct ApplicationParseContext *out = malloc(sizeof(*out));
  out->arguments = malloc(sizeof(*arguments) * count);
  out->COUNT = count;
  while (--count >= 0) {
    out->arguments[count] =
        malloc(sizeof(**arguments) * (strlen(arguments[count]) + 1));
    strcpy(out->arguments[count], arguments[count]);
  }
  out->argument = NULL;
  out->index = 1;
  out->option = NULL;
  out->out = EXIT_SUCCESS;
  out->parse_options = true;
  return out;
}
