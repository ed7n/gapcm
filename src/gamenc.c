/**
 * GAPCM Encoder
 *
 * Entry point to the encoder application. It consists of the main function from
 * which the application initializes into an instance.
 */

#include "apphelp.h"
#include "appinfo.h"
#include "common/application.h"
#include "common/constants.h"
#include "gam.h"
#include "gapcm/gapcm.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

/** Usage syntax. */
#define GAMENC_APPHELP_USAGE "Usage: -o <path> [<field>|<option>]... <file>"
/** Explanation to syntax. */
#define GAMENC_APPHELP_EXPLANATION                                             \
  "\
Where:\n\
  -o,  --output <path>      Path to output game PCM file. `-` for pipe.\n\
\n\
Header Fields:\n\
  -c,  --channels {1|2}     1: mono (default), 2: stereo.\n\
  -ea, --echo-pans <levels[6]>\n\
                            Echo pans for channels 3 to 8. Low nibble: left,\n\
                            high nibble: right.\n\
  -ed, --echo-delay <ticks> Echo delay.\n\
  -el, --echo-levels <levels[3]>\n\
                            Echo levels for channel pairs 3 and 4 to 7 and 8.\n\
  -ep, --echo-pregap <ticks>\n\
                            First echo delay.\n\
  -m,  --mark <blocks>      Loop start position. Default is `0`.\n\
  -n,  --length <frames>    Length between stream start and loop end. `-1` for\n\
                            maximum. Default is automatic.\n\
  -p,  --pregap <blocks>    Artificial silence length. Default is `0`.\n\
\n\
Options:\n\
  -t, --trail               Include samples after the loop end.\n\
\n\
The input file must be headerless, unsigned 8-bit PCM. Echo header fields\n\
default to zero which disables the feature. See the prober for details on\n\
units.\n" APPHELP_EXPLANATION
/** Application name. */
#define GAMENC_APPINFO_NAME APPINFO_NAME "enc"
/** Application description. */
#define GAMENC_APPINFO_DESCRIPTION APPINFO_DESCRIPTION "encoder."

int gamenc_error_header(void) {
  application_print_message(GAMENC_APPINFO_NAME, "Encode header failed.");
  return EXIT_FAILURE;
}

int gamenc_act(struct GamInstance *i) {
  int out = EXIT_SUCCESS;
  uint8_t *sector = malloc(GAPCM_SECTOR_SIZE);
  while (true) {
    if (i->source == stdin) {
      application_print_message(GAMENC_APPINFO_NAME, GAM_INFO_LISTEN);
    }
    if (gapcm_encode_header(i->header, sector) != GAPCM_SECTOR_SIZE) {
      out = gamenc_error_header();
      break;
    }
    i->write_count += fwrite(sector, 1, GAPCM_SECTOR_SIZE, i->output);
    if (i->write_count != GAPCM_SECTOR_SIZE) {
      break;
    }
    unsigned long long count =
        i->options->trail
            ? gapcm_encode_stream_for(i->header, i->source, i->output,
                                      UINT32_MAX)
            : gapcm_encode_stream(i->header, i->source, i->output);
    uint16_t channel_count = gapcm_to_channelcount(i->header->format);
    i->write_count += count;
    if (i->options->has_length) {
      unsigned long long comparand =
          i->header->length * channel_count * GAPCM_SECTOR_BLOCKS;
      comparand = comparand - comparand % GAPCM_SECTOR_SIZE +
                  GAPCM_SECTOR_SIZE *
                      (comparand % GAPCM_SECTOR_SIZE > 0 ? channel_count : 0);
      if (i->options->trail ? count < comparand : count != comparand) {
        out = EXIT_FAILURE;
        application_print_message(i->options->output, GAM_ERROR_OUTPUT);
      }
    } else {
      errno = 0;
      if (fseek(i->output, 0, SEEK_SET) == SUCCESS) {
        i->header->length = count / GAPCM_SECTOR_BLOCKS / channel_count;
        if (gapcm_encode_header(i->header, sector) != GAPCM_SECTOR_SIZE) {
          out = gamenc_error_header();
          break;
        }
        fwrite(sector, 1, GAPCM_SECTOR_SIZE, i->output);
      } else {
        out = EXIT_FAILURE;
        application_print_message(i->options->output, strerror(errno));
      }
    }
    if ((i->options->trail || !i->options->has_length) && feof(i->source) &&
        !ferror(i->source)) {
      clearerr(i->source);
    }
    break;
  }
  free(sector);
  if (out == EXIT_SUCCESS) {
    gam_check_files(i, &out);
  }
  return out;
}

int gamenc_done(struct GamInstance *i) {
  int out = application_file_close(i->output, i->options->output);
  int out_source = application_file_close(i->source, i->options->source);
  if (out == EXIT_SUCCESS) {
    out = out_source;
  }
  return out;
}

void gamenc_print_header(void) {
  application_print_strings(
      1, GAMENC_APPINFO_NAME SPACE APPINFO_VER SPACE
      "by Brendon" SPACE APPINFO_DATE "." EOL
      "——" GAMENC_APPINFO_DESCRIPTION SPACE APPINFO_URL EOL EOL);
}

int gamenc_help(void) {
  gamenc_print_header();
  application_print_strings(
      1, GAMENC_APPHELP_USAGE EOL GAMENC_APPHELP_EXPLANATION EOL);
  return GAM_EXIT_QUIT;
}

int gamenc_read(struct GamInstance *i) {
  struct GaPcmHeader *h = i->header;
  struct GamOptions *o = i->options;
  const char *error = NULL;
  int out = EXIT_SUCCESS;
  while (true) {
    h->echo_delay = o->has_echo_delay ? o->echo_delay : 0;
    if (o->has_echo_levels) {
      for (size_t index = 0; index < 3; index++) {
        h->echo_levels[index] = o->echo_levels[index];
      }
    }
    if (o->has_echo_pans) {
      for (size_t index = 0; index < 6; index++) {
        h->echo_pans[index] = o->echo_pans[index];
      }
    }
    h->echo_pregap = o->has_echo_pregap ? o->echo_pregap : 0;
    h->format =
        o->has_channels ? gapcm_to_format(o->channels) : GAPCM_FORMAT_MONO;
    h->length = o->has_length ? o->length : UINT32_MAX;
    h->mark = o->has_mark ? o->mark : 0;
    h->pregap = o->has_pregap ? o->pregap : 0;
    if (!gapcm_header_check(h, &error)) {
      out = EXIT_FAILURE;
      application_print_message(o->source, error);
      break;
    }
    if (!gam_open_source(o->source, &i->source, &out) ||
        !gam_open_output(o->output, &i->output, &out)) {
      break;
    }
    if (!o->has_length && i->output == stdout) {
      application_print_message(
          GAMENC_APPINFO_NAME,
          "Automatic length may be unavailable with standard output.");
    }
    break;
  }
  return out;
}

#define GAMENC_OPTION_COUNT 10
/** The main method is the entry point to this application. */
int main(int argument_count, char *arguments[]) {
  if (argument_count < 2) {
    gamenc_print_header();
    application_print_strings(1,
                              GAMENC_APPHELP_USAGE EOL APPHELP_INVITATION EOL);
    return EXIT_SUCCESS;
  }
  struct GamInstance *instance = gam_instance_make(arguments, argument_count);
  struct GamOption **options = malloc(sizeof(options) * GAMENC_OPTION_COUNT);
  options[0] = gam_option_make("-c", "--channels", gam_parse_channels);
  options[1] = gam_option_make("-ea", "--echo-pans", gam_parse_echo_pans);
  options[2] = gam_option_make("-ed", "--echo-delay", gam_parse_echo_delay);
  options[3] = gam_option_make("-el", "--echo-levels", gam_parse_echo_levels);
  options[4] = gam_option_make("-ep", "--echo-pregap", gam_parse_echo_pregap);
  options[5] = gam_option_make("-m", "--mark", gam_parse_mark);
  options[6] = gam_option_make("-n", "--length", gam_parse_length);
  options[7] = gam_option_make("-o", "--output", gam_parse_output);
  options[8] = gam_option_make("-p", "--pregap", gam_parse_pregap);
  options[9] = gam_option_make("-t", "--trail", gam_parse_trail);
  int out = gam_run(instance, options, GAMENC_OPTION_COUNT, gamenc_help,
                    gamenc_read, gamenc_act, gamenc_done);
  instance = gam_instance_free(instance);
  for (size_t index = 0; index < GAMENC_OPTION_COUNT; index++) {
    options[index] = gam_option_free(options[index]);
  }
  free(options);
  return out;
}
#undef GAMENC_OPTION_COUNT
