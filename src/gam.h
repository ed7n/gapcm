/**
 * GAPCM: Application
 *
 * Parts shared by the applications that did not qualify to
 * `common/application.h`. See that for details.
 */
#ifndef _GAM_H
#define _GAM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define GAM_ALERT_STEREO "Output is stereo."
#define GAM_ERROR_EOF "Unexpected end-of-file."
#define GAM_ERROR_HEADER "A header can not be parsed."
#define GAM_ERROR_OUTPUT "The output is incomplete."
#define GAM_ERROR_READ "A read error has occurred."
#define GAM_ERROR_WRITE "A write error has occurred."
#define GAM_INFO_LISTEN "Now listening from pipe."

/** Exit code: quit. */
#define GAM_EXIT_QUIT 0xcdda
/** Preset loop count. */
#define GAM_LOOP_COUNT 2

/** Operation modes. */
enum GamMode { PARSE, READ, ACT, DONE };

/** Represents an instance. */
struct GamInstance {
  /** GAPCM header. */
  struct GaPcmHeader *header;
  /** Options. */
  struct GamOptions *options;
  /** Parse context. */
  struct ApplicationParseContext *parse;
  /** Output stream. */
  FILE *output;
  /** Source stream. */
  FILE *source;
  /** Count of bytes read. */
  unsigned long long read_count;
  /** Count of bytes written. */
  unsigned long long write_count;
};

/** Represents an option case. */
struct GamOption {
  /** Handler function. */
  int (*HANDLER)(struct ApplicationParseContext *, struct GamOptions *);
  /** Unix-style name. */
  const char *NAME;
  /** Long name. */
  const char *NAME_LONG;
};

/** Represents options. Not all are used in each application. */
struct GamOptions {
  /** Echo levels. */
  uint8_t echo_levels[3];
  /** Echo pans. */
  uint8_t echo_pans[6];
  /** Output stream. */
  char *output;
  /** Source stream. */
  char *source;
  /** Length frames. */
  uint32_t length;
  /** Mark blocks. */
  uint32_t mark;
  /** Channel count. */
  uint16_t channels;
  /** Loop count. */
  uint16_t loop;
  /** Echo delay ticks. */
  uint8_t echo_delay;
  /** Echo pregap ticks. */
  uint8_t echo_pregap;
  /** Stream pregap blocks. */
  uint8_t pregap;
  /** Channels present? */
  bool has_channels;
  /** Echo levels present? */
  bool has_echo_delay;
  /** Echo levels present? */
  bool has_echo_levels;
  /** Echo levels present? */
  bool has_echo_pans;
  /** Echo levels present? */
  bool has_echo_pregap;
  /** Length present? */
  bool has_length;
  /** Loop present? */
  bool has_loop;
  /** Mark present? */
  bool has_mark;
  /** Pregap present? */
  bool has_pregap;
  /** Print header? */
  bool info;
  /** Include trailing samples? */
  bool trail;
};

/** Checks the output and source files of the given instance. */
bool gam_check_files(struct GamInstance *instance, int *success);

int gam_error_header(const char *head);

/** Frees the given instance. */
struct GamInstance *gam_instance_free(struct GamInstance *instance);

/** Makes an instance. */
struct GamInstance *gam_instance_make(char *arguments[], int count);

/** Opens the given file by its name for writing to the given location. */
bool gam_open_output(const char *name, FILE **file, int *success);

/** Opens the given file by its name for reading to the given location. */
bool gam_open_source(const char *name, FILE **file, int *success);

/** Frees the given option case. */
struct GamOption *gam_option_free(struct GamOption *option);

/** Makes an option case with the given. */
struct GamOption *gam_option_make(
    const char *key, const char *name,
    int (*handler)(struct ApplicationParseContext *, struct GamOptions *));

/** Frees the given options model. */
struct GamOptions *gam_options_free(struct GamOptions *options);

/** Makes an options model. */
struct GamOptions *gam_options_make(void);

/**
 * Runs the given parse context against the given cases to the given location.
 */
int gam_parse(struct ApplicationParseContext *context, struct GamOption **cases,
              size_t count, struct GamOptions *options, int (*help)(void));

int gam_parse_channels(struct ApplicationParseContext *context,
                       struct GamOptions *options);

int gam_parse_echo_delay(struct ApplicationParseContext *context,
                         struct GamOptions *options);

int gam_parse_echo_levels(struct ApplicationParseContext *context,
                          struct GamOptions *options);

int gam_parse_echo_pans(struct ApplicationParseContext *context,
                        struct GamOptions *options);

int gam_parse_echo_pregap(struct ApplicationParseContext *context,
                          struct GamOptions *options);

int gam_parse_info(struct ApplicationParseContext *context,
                   struct GamOptions *options);

int gam_parse_length(struct ApplicationParseContext *context,
                     struct GamOptions *options);

int gam_parse_mark(struct ApplicationParseContext *context,
                   struct GamOptions *options);

/**
 * Parses an option from the given context against the given cases to the given
 * location and return its success.
 */
bool gam_parse_option(struct ApplicationParseContext *context,
                      struct GamOption **cases, size_t count,
                      struct GamOptions *options);

int gam_parse_output(struct ApplicationParseContext *context,
                     struct GamOptions *options);

int gam_parse_pregap(struct ApplicationParseContext *context,
                     struct GamOptions *options);

int gam_parse_trail(struct ApplicationParseContext *context,
                    struct GamOptions *options);

/**
 * Runs the given instance. Function parameters except for `help` correspond to
 * their operation mode.
 */
int gam_run(struct GamInstance *instance, struct GamOption **options,
            size_t count, int (*help)(void), int (*read)(struct GamInstance *),
            int (*act)(struct GamInstance *),
            int (*done)(struct GamInstance *));

#endif
