/**
 * GAPCM Info
 *
 * Entry point to the prober application. It consists of the main function from
 * which the application initializes into an instance.
 */

#include "apphelp.h"
#include "appinfo.h"
#include "common/application.h"
#include "common/constants.h"
#include "common/strings.h"
#include "gam.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

/** Usage syntax. */
#define GAMINFO_APPHELP_USAGE "Usage: [<field>] <file>"
/** Explanation to syntax. */
#define GAMINFO_APPHELP_EXPLANATION                                            \
  "\
Header Fields:\n\
  -c,  --channels     1: mono, 2: stereo.\n\
  -ea, --echo-pans    Echo pans for channels 3 to 8. Low nibble: left, high\n\
                      nibble: right.\n\
  -ed, --echo-delay   Echo delay in ticks.\n\
  -el, --echo-levels  Echo levels for channel pairs 3 and 4 to 7 and 8.\n\
  -ep, --echo-pregap  First echo delay in ticks.\n\
  -m,  --mark       Loop start position in blocks.\n\
  -n,  --length       Length between stream start and loop end in frames.\n\
  -p,  --pregap       Artificial silence length in blocks.\n\
\n\
If none is given, then it prints the header in a friendly format. A block spans\n\
1024 samples, a frame spans one sample for mono, two for stereo.\n" APPHELP_EXPLANATION
/** Application name. */
#define GAMINFO_APPINFO_NAME APPINFO_NAME "info"
/** Application description. */
#define GAMINFO_APPINFO_DESCRIPTION APPINFO_DESCRIPTION "header prober."

int gaminfo_error_options(void) {
  const char *message;
  int number = rand();
  if (number < RAND_MAX / 8) {
    message = "Hey chief, we can't take too many options.";
  } else if (number < RAND_MAX / 4) {
    message = "Pooh! That's too many options!";
  } else {
    message = "Too many options.";
  }
  application_print_message(GAMINFO_APPINFO_NAME, message);
  return EXIT_FAILURE;
}

int gaminfo_act(struct GamInstance *i) {
  struct GaPcmHeader *h = i->header;
  char *o = i->options->output;
  int out = -1;
  if (o == NULL) {
    char string[GAPCM_HEADER_STRING_CAPACITY];
    gapcm_header_stringify(h, string);
    out = puts(string);
  } else if (string_equals_any(o, 2, "-c", "--channels")) {
    out = printf("%u%s", gapcm_to_channelcount(h->format), EOL);
  } else if (string_equals_any(o, 2, "-ea", "--echo-pans")) {
    out = printf("%02x %02x %02x %02x %02x %02x%s", h->echo_pans[0],
                 h->echo_pans[1], h->echo_pans[2], h->echo_pans[3],
                 h->echo_pans[4], h->echo_pans[5], EOL);
  } else if (string_equals_any(o, 2, "-ed", "--echo-delay")) {
    out = printf("%u%s", h->echo_delay, EOL);
  } else if (string_equals_any(o, 2, "-el", "--echo-levels")) {
    out = printf("%u %u %u%s", h->echo_levels[0], h->echo_levels[1],
                 h->echo_levels[2], EOL);
  } else if (string_equals_any(o, 2, "-ep", "--echo-pregap")) {
    out = printf("%u%s", h->echo_pregap, EOL);
  } else if (string_equals_any(o, 2, "-m", "--mark")) {
    out = printf("%u%s", h->mark, EOL);
  } else if (string_equals_any(o, 2, "-n", "--length")) {
    out = printf("%u%s", h->length, EOL);
  } else if (string_equals_any(o, 2, "-p", "--pregap")) {
    out = printf("%u%s", h->pregap, EOL);
  }
  return out > 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

int gaminfo_done(struct GamInstance *i) {
  return application_file_close(i->source, i->options->source);
}

int gaminfo_parse_option(struct ApplicationParseContext *c,
                         struct GamOptions *options) {
  options->length = options->output != NULL ? options->length + 1 : 0;
  options->output = realloc(options->output, strlen(c->option) + 1);
  strcpy(options->output, c->option);
  return EXIT_SUCCESS;
}

void gaminfo_print_header(void) {
  application_print_strings(
      1, GAMINFO_APPINFO_NAME SPACE APPINFO_VER SPACE
      "by Brendon" SPACE APPINFO_DATE "." EOL
      "——" GAMINFO_APPINFO_DESCRIPTION SPACE APPINFO_URL EOL EOL);
}

int gaminfo_help(void) {
  gaminfo_print_header();
  application_print_strings(
      1, GAMINFO_APPHELP_USAGE EOL GAMINFO_APPHELP_EXPLANATION EOL);
  return GAM_EXIT_QUIT;
}

int gaminfo_read(struct GamInstance *i) {
  int out;
  if (!gam_open_source(i->options->source, &i->source, &out)) {
    return out;
  }
  uint8_t *sector = malloc(GAPCM_SECTOR_BYTES);
  while (true) {
    const char *error = NULL;
    if (i->source == stdin) {
      application_print_message(GAMINFO_APPINFO_NAME, GAM_INFO_LISTEN);
    }
    i->read_count += fread(sector, 1, GAPCM_SECTOR_BYTES, i->source);
    if (i->read_count != GAPCM_SECTOR_BYTES ||
        gapcm_decode_header(sector, i->header) != GAPCM_SECTOR_BYTES) {
      out = gam_error_header(i->options->source);
      break;
    }
    if (gapcm_header_check(i->header, &error)) {
      if (i->options->length > 0) {
        out = gaminfo_error_options();
      }
    } else {
      application_print_message(i->options->source, error);
    }
    break;
  }
  free(sector);
  return out;
}

#define GAMINFO_OPTION_COUNT 8
/** The main method is the entry point to this application. */
int main(int argument_count, char *arguments[]) {
  if (argument_count < 2) {
    gaminfo_print_header();
    application_print_strings(1,
                              GAMINFO_APPHELP_USAGE EOL APPHELP_INVITATION EOL);
    return EXIT_SUCCESS;
  }
  srand(time(NULL));
  struct GamInstance *instance = gam_instance_make(arguments, argument_count);
  struct GamOption **options = malloc(sizeof(options) * GAMINFO_OPTION_COUNT);
  options[0] = gam_option_make("-c", "--channels", gaminfo_parse_option);
  options[1] = gam_option_make("-ea", "--echo-pans", gaminfo_parse_option);
  options[2] = gam_option_make("-ed", "--echo-delay", gaminfo_parse_option);
  options[3] = gam_option_make("-el", "--echo-levels", gaminfo_parse_option);
  options[4] = gam_option_make("-ep", "--echo-pregap", gaminfo_parse_option);
  options[5] = gam_option_make("-m", "--mark", gaminfo_parse_option);
  options[6] = gam_option_make("-n", "--length", gaminfo_parse_option);
  options[7] = gam_option_make("-p", "--pregap", gaminfo_parse_option);
  int out = gam_run(instance, options, GAMINFO_OPTION_COUNT, gaminfo_help,
                    gaminfo_read, gaminfo_act, gaminfo_done);
  instance = gam_instance_free(instance);
  for (size_t index = 0; index < GAMINFO_OPTION_COUNT; index++) {
    options[index] = gam_option_free(options[index]);
  }
  free(options);
  return out;
}
#undef GAMINFO_OPTION_COUNT
