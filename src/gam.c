#include "gam.h"
#include "common/application.h"
#include "common/constants.h"
#include "common/strings.h"
#include "gapcm/gapcm.h"
#include <stdlib.h>
#include <string.h>

static int gam_parse_bool(struct ApplicationParseContext *c, bool *present) {
  // Wunused-parameter
  if (c != NULL) {
    *present = true;
  }
  return EXIT_SUCCESS;
}

static int gam_parse_u8(struct ApplicationParseContext *c, uint8_t *integer,
                        bool *present) {
  long long number;
  int out = application_parse_integer(c, &number, 0, UINT8_MAX, "[0, 255]");
  if (out == EXIT_SUCCESS) {
    *integer = number;
    *present = true;
  }
  return out;
}

static int gam_parse_u8s(struct ApplicationParseContext *c, uint8_t *integers,
                         const size_t count, bool *present) {
  int out;
  for (size_t index = 0; index < count; index++) {
    long long number;
    out = application_parse_integer(c, &number, 0, UINT8_MAX, "[0, 255]");
    if (out != EXIT_SUCCESS) {
      break;
    }
    integers[index] = number;
  }
  if (out == EXIT_SUCCESS) {
    *present = true;
  }
  return out;
}

bool gam_check_files(struct GamInstance *i, int *success) {
  *success = EXIT_SUCCESS;
  if (ferror(i->source) != SUCCESS) {
    *success = EXIT_FAILURE;
    application_print_message(i->options->source, GAM_ERROR_READ);
  }
  if (feof(i->source) != SUCCESS) {
    application_print_message(i->options->source, GAM_ERROR_EOF);
  }
  if (ferror(i->output) != SUCCESS) {
    *success = EXIT_FAILURE;
    application_print_message(i->options->output, GAM_ERROR_WRITE);
  }
  if (feof(i->output) != SUCCESS) {
    *success = EXIT_FAILURE;
    application_print_message(i->options->output, GAM_ERROR_EOF);
  }
  return *success == EXIT_SUCCESS;
}

int gam_error_header(const char *restrict head) {
  application_print_message(head, GAM_ERROR_HEADER);
  return EXIT_FAILURE;
}

struct GamInstance *gam_instance_free(struct GamInstance *i) {
  i->header = gapcm_header_free(i->header);
  i->options = gam_options_free(i->options);
  i->parse = application_parsecontext_free(i->parse);
  free(i);
  return NULL;
}

struct GamInstance *gam_instance_make(char *arguments[], int count) {
  struct GamInstance *out = malloc(sizeof(*out));
  out->header = gapcm_header_make();
  out->options = gam_options_make();
  out->output = NULL;
  out->parse = application_parsecontext_make(arguments, count);
  out->read_count = 0;
  out->source = NULL;
  out->write_count = 0;
  return out;
}

bool gam_open_output(const char *restrict name, FILE **restrict file,
                     int *success) {
  *success = application_file_open(name, "wb", file, " output", stdout);
  return *success == EXIT_SUCCESS;
}

bool gam_open_source(const char *restrict name, FILE **restrict file,
                     int *success) {
  *success = application_file_open(name, "rb", file, " input", stdin);
  return *success == EXIT_SUCCESS;
}

struct GamOption *gam_option_free(struct GamOption *o) {
  free(o);
  return NULL;
}

struct GamOption *gam_option_make(
    const char *key, const char *name,
    int (*handler)(struct ApplicationParseContext *, struct GamOptions *)) {
  struct GamOption *out = malloc(sizeof(*out));
  out->HANDLER = handler;
  out->NAME = key;
  out->NAME_LONG = name;
  return out;
}

struct GamOptions *gam_options_free(struct GamOptions *o) {
  free(o->output);
  free(o->source);
  free(o);
  return NULL;
}

struct GamOptions *gam_options_make(void) {
  struct GamOptions *out = calloc(1, sizeof(*out));
  out->output = NULL;
  out->source = NULL;
  out->has_channels = false;
  out->has_echo_delay = false;
  out->has_echo_levels = false;
  out->has_echo_pans = false;
  out->has_echo_pregap = false;
  out->has_length = false;
  out->has_loop = false;
  out->has_mark = false;
  out->has_pregap = false;
  out->info = false;
  out->trail = false;
  return out;
}

int gam_parse(struct ApplicationParseContext *c, struct GamOption **cases,
              const size_t count, struct GamOptions *options,
              int (*help)(void)) {
  while (c->index < c->COUNT && c->out == EXIT_SUCCESS) {
    c->option = c->arguments[c->index++];
    if (c->parse_options) {
      if (gam_parse_option(c, cases, count, options)) {
      } else if (string_equals_any(c->option, 2, "-h", "--help")) {
        c->out = help();
      } else if (string_equals(c->option, "--")) {
        c->parse_options = false;
      } else if (c->index < c->COUNT) {
        c->out = application_error_option_bad(c->option);
      } else {
        break;
      }
      c->option = NULL;
      continue;
    }
    break;
  }
  if (c->option != NULL && c->out == EXIT_SUCCESS) {
    options->source =
        malloc(sizeof(*options->source) * (strlen(c->option) + 1));
    strcpy(options->source, c->option);
  }
  return c->out;
}

int gam_parse_channels(struct ApplicationParseContext *c,
                       struct GamOptions *options) {
  long long number;
  int out = application_parse_integer(c, &number, 1, 2, "[1, 2]");
  if (out == EXIT_SUCCESS) {
    options->channels = number;
    options->has_channels = true;
  }
  return out;
}

int gam_parse_echo_delay(struct ApplicationParseContext *c,
                         struct GamOptions *options) {
  return gam_parse_u8(c, &options->echo_delay, &options->has_echo_delay);
}

int gam_parse_echo_levels(struct ApplicationParseContext *c,
                          struct GamOptions *options) {
  return gam_parse_u8s(c, options->echo_levels, 3, &options->has_echo_levels);
}

int gam_parse_echo_pans(struct ApplicationParseContext *c,
                        struct GamOptions *options) {
  return gam_parse_u8s(c, options->echo_pans, 6, &options->has_echo_pans);
}

int gam_parse_echo_pregap(struct ApplicationParseContext *c,
                          struct GamOptions *options) {
  return gam_parse_u8(c, &options->echo_pregap, &options->has_echo_pregap);
}

int gam_parse_info(struct ApplicationParseContext *c,
                   struct GamOptions *options) {
  return gam_parse_bool(c, &options->info);
}

int gam_parse_length(struct ApplicationParseContext *c,
                     struct GamOptions *options) {
  long long number;
  int out = application_parse_integer(c, &number, -1, UINT32_MAX, "32-bit");
  if (out == EXIT_SUCCESS) {
    options->length = number;
    options->has_length = true;
  }
  return out;
}

int gam_parse_mark(struct ApplicationParseContext *c,
                   struct GamOptions *options) {
  long long number;
  int out = application_parse_integer(c, &number, 0, UINT32_MAX, "32-bit");
  if (out == EXIT_SUCCESS) {
    options->mark = number;
    options->has_mark = true;
  }
  return out;
}

bool gam_parse_option(struct ApplicationParseContext *c,
                      struct GamOption **cases, const size_t count,
                      struct GamOptions *options) {
  for (size_t index = 0; index < count; index++) {
    if (string_equals_any(c->option, 2, cases[index]->NAME,
                          cases[index]->NAME_LONG)) {
      c->out = cases[index]->HANDLER(c, options);
      return true;
    }
  }
  return false;
}

int gam_parse_output(struct ApplicationParseContext *c,
                     struct GamOptions *options) {
  return application_parse_string(c, &options->output);
}

int gam_parse_pregap(struct ApplicationParseContext *c,
                     struct GamOptions *options) {
  return gam_parse_u8(c, &options->pregap, &options->has_pregap);
}

int gam_parse_trail(struct ApplicationParseContext *c,
                    struct GamOptions *options) {
  return gam_parse_bool(c, &options->trail);
}

int gam_run(struct GamInstance *i, struct GamOption **o, const size_t count,
            int (*help)(void), int (*read)(struct GamInstance *),
            int (*act)(struct GamInstance *),
            int (*done)(struct GamInstance *)) {
  enum GamMode mode = PARSE;
  int out = EXIT_FAILURE;
  do {
    switch (mode) {
    case PARSE:
      out = gam_parse(i->parse, o, count, i->options, help);
      mode = READ;
      break;
    case READ:
      out = read(i);
      mode = ACT;
      break;
    case ACT:
      out = act(i);
      mode = DONE;
      break;
    case DONE:
      out = done(i);
      return out == GAM_EXIT_QUIT ? EXIT_SUCCESS : out;
    }
    if (out != EXIT_SUCCESS) {
      done(i);
      return out == GAM_EXIT_QUIT ? EXIT_SUCCESS : out;
    }
  } while (true);
}
