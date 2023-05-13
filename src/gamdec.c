/**
 * GAPCM Decoder
 *
 * Entry point to the decoder application. It consists of the main function from
 * which the application initializes into an instance.
 */

#include "apphelp.h"
#include "appinfo.h"
#include "common/application.h"
#include "common/constants.h"
#include "gam.h"
#include "gapcm/gapcm.h"
#include <stdlib.h>

/** Application name. */
#define GAMDEC_APPINFO_NAME APPINFO_NAME "dec"

/** Application description. */
#define GAMDEC_APPINFO_DESCRIPTION APPINFO_DESCRIPTION "decoder."

/** Explanation to syntax. */
#define GAMDEC_APPHELP_EXPLANATION                                             \
  "\
Where:\n\
  -o, --output <path>     Path to output headerless, unsigned 8-bit PCM file.\n\
                          `-` for pipe.\n\
Header Overrides:\n\
  -c, --channels {1|2}    1: mono, 2: stereo.\n\
  -m, --mark <blocks>     Loop start position.\n\
  -n, --length <frames>   Length between stream start and loop end. `-1` for\n\
                          maximum.\n\
  -p, --pregap <blocks>   Artificial silence length.\n\
\n\
Options:\n\
  -i, --info              Prints the header in a friendly format.\n\
  -l, --loop <count>      Write the given count of loops. `0` to stop at the\n\
                          mark; no loop. `-1` for 65535. Default is `2`.\n\
  -t, --trail             Include samples after the loop end.\n\
\n\
Echo, fade, and gain features are not supported; get their parameters with the\n\
prober and apply them elsewhere. Also look there for details on units.\n" APPHELP_EXPLANATION

/** Usage syntax. */
#define GAMDEC_APPHELP_USAGE "Usage: -o <path> [<override>|<option>]... <file>"

bool gamdec_act_check(struct GamInstance *i, int *success,
                      const unsigned long long count,
                      const unsigned long long comparand) {
  if (count != comparand) {
    *success = EXIT_FAILURE;
    application_print_message(i->options->output, GAM_ERROR_OUTPUT);
  } else if (feof(i->source) && !ferror(i->source)) {
    clearerr(i->source);
  }
  return *success == EXIT_SUCCESS;
}

int gamdec_act(struct GamInstance *i) {
  int out = EXIT_SUCCESS;
  i->write_count += gapcm_decode_pregap(i->header->pregap, i->output);
  while (i->write_count == GAPCM_BLOCK_SIZE * i->header->pregap) {
    unsigned long long mark = GAPCM_BLOCK_SIZE * i->header->mark;
    unsigned long long length =
        i->header->length * gapcm_to_channelcount(i->header->format);
    unsigned long long length_loop = length - mark;
    if (i->options->loop > 1) {
      unsigned long long count = gapcm_decode_stream(
          i->header, i->source, i->output, i->options->loop - 1);
      if (!gamdec_act_check(i, &out, count,
                            length + length_loop * (i->options->loop - 2))) {
        if (count == length) {
          application_print_message(i->options->source,
                                    "Seek may have failed.");
        }
        break;
      }
      if (!gam_check_files(i, &out)) {
        break;
      }
      if (gapcm_decode_seek(i->source, i->header->mark) != SUCCESS) {
        out = EXIT_FAILURE;
        application_print_message(i->options->source, "Seek failed.");
        break;
      }
    }
    if (i->options->trail) {
      i->write_count +=
          gapcm_decode_stream_for(i->header, i->source, i->output, UINT32_MAX);
      if (feof(i->source) && !ferror(i->source)) {
        clearerr(i->source);
      }
    } else {
      unsigned long long count;
      if (i->options->loop > 1) {
        count = gapcm_decode_loop(i->header, i->source, i->output, 1);
        gamdec_act_check(i, &out, count, length_loop);
      } else {
        count = gapcm_decode_stream(i->header, i->source, i->output,
                                    i->options->loop);
        gamdec_act_check(i, &out, count, mark + length_loop * i->options->loop);
      }
      i->write_count += count;
    }
    break;
  }
  if (out == EXIT_SUCCESS) {
    gam_check_files(i, &out);
  }
  return out;
}

int gamdec_done(struct GamInstance *i) {
  int out = application_file_close(i->output, i->options->output);
  int out_source = application_file_close(i->source, i->options->source);
  if (out == EXIT_SUCCESS) {
    out = out_source;
  }
  return out;
}

void gamdec_print_header(void) {
  application_print_strings(
      1, GAMDEC_APPINFO_NAME SPACE APPINFO_VER SPACE
      "by Brendon" SPACE APPINFO_DATE "." EOL
      "——" GAMDEC_APPINFO_DESCRIPTION SPACE APPINFO_URL EOL EOL);
}

int gamdec_help(void) {
  gamdec_print_header();
  application_print_strings(
      1, GAMDEC_APPHELP_USAGE EOL GAMDEC_APPHELP_EXPLANATION EOL);
  return GAM_EXIT_QUIT;
}

int gamdec_read(struct GamInstance *i) {
  int out;
  if (!gam_open_source(i->options->source, &i->source, &out)) {
    return out;
  }
  struct GaPcmHeader *h = i->header;
  struct GamOptions *o = i->options;
  uint8_t *sector = malloc(GAPCM_SECTOR_SIZE);
  while (true) {
    const char *error = NULL;
    if (i->source == stdin) {
      if (o->has_loop) {
        application_print_message(
            GAMDEC_APPINFO_NAME,
            "Looping may be unavailable with standard input.");
      }
      application_print_message(GAMDEC_APPINFO_NAME, GAM_INFO_LISTEN);
    }
    i->read_count += fread(sector, 1, GAPCM_SECTOR_SIZE, i->source);
    if (i->read_count != GAPCM_SECTOR_SIZE ||
        gapcm_decode_header(sector, h) != GAPCM_SECTOR_SIZE) {
      out = gam_error_header(o->source);
      break;
    }
    if (o->has_channels) {
      h->format = gapcm_to_format(o->channels);
    }
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
    if (o->has_length) {
      h->length = o->length;
    }
    if (!o->has_loop) {
      o->loop = 2;
    }
    if (o->has_mark) {
      h->mark = o->mark;
    }
    if (o->has_pregap) {
      h->pregap = o->pregap;
    }
    if (o->info) {
      char string[GAPCM_HEADER_STRING_CAPACITY];
      gapcm_header_stringify(i->header, string);
      application_print_strings(2, string, EOL);
    }
    if (!gapcm_header_check(h, &error)) {
      out = EXIT_FAILURE;
      application_print_message(o->source, error);
      break;
    }
    if (!o->has_channels && gapcm_to_channelcount(h->format) == 2) {
      application_print_message(GAMDEC_APPINFO_NAME, GAM_ALERT_STEREO);
    }
    gam_open_output(o->output, &i->output, &out);
    break;
  }
  free(sector);
  return out;
}

#define GAMDEC_OPTION_COUNT 8
/** The main method is the entry point to this application. */
int main(int argument_count, char *arguments[]) {
  if (argument_count < 2) {
    gamdec_print_header();
    application_print_strings(1,
                              GAMDEC_APPHELP_USAGE EOL APPHELP_INVITATION EOL);
    return EXIT_SUCCESS;
  }
  struct GamInstance *instance = gam_instance_make(arguments, argument_count);
  struct GamOption **options = malloc(sizeof(options) * GAMDEC_OPTION_COUNT);
  options[0] = gam_option_make("-c", "--channels", gam_parse_channels);
  options[1] = gam_option_make("-i", "--info", gam_parse_info);
  options[2] = gam_option_make("-l", "--loop", gam_parse_loop);
  options[3] = gam_option_make("-m", "--mark", gam_parse_mark);
  options[4] = gam_option_make("-n", "--length", gam_parse_length);
  options[5] = gam_option_make("-o", "--output", gam_parse_output);
  options[6] = gam_option_make("-p", "--pregap", gam_parse_pregap);
  options[7] = gam_option_make("-t", "--trail", gam_parse_trail);
  int out = gam_run(instance, options, GAMDEC_OPTION_COUNT, gamdec_help,
                    gamdec_read, gamdec_act, gamdec_done);
  instance = gam_instance_free(instance);
  for (size_t index = 0; index < GAMDEC_OPTION_COUNT; index++) {
    options[index] = gam_option_free(options[index]);
  }
  free(options);
  return out;
}
#undef GAMDEC_OPTION_COUNT
