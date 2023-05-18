/**
 * GAPCM: Header model and transcode functions.
 *
 * File operators start from the current position and--unless otherwise
 * mentioned--return their count of output bytes. Those that flush and
 * seek--either for looping or `gapcm_decode_seek`--set `errno` on error. For
 * PCM transcodes from and to signed 8-bit, change `GAPCM_SAMPLE_ORIGIN` to `0`.
 * `0x80` for unsigned. Consumer PCM refers to PCM of this format.
 *
 * GA is short for GAME ARTS Co., Ltd.
 */
#ifndef _GAPCM_H
#define _GAPCM_H

/** Consumer PCM sample origin. */
#define GAPCM_SAMPLE_ORIGIN 0x80

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/** Block size in bytes. Must be half of sector's. */
#define GAPCM_BLOCK_SIZE 1024
/** Mono stream format. */
#define GAPCM_FORMAT_MONO 2
/** Stereo stream format. */
#define GAPCM_FORMAT_STEREO 1
/** Maximum count of char units needed for header stringification. */
#define GAPCM_HEADER_STRING_CAPACITY 198
/** Sector size in bytes. */
#define GAPCM_SECTOR_SIZE 2048
/** Two blocks in a sector. */
#define GAPCM_SECTOR_BLOCKS (GAPCM_SECTOR_SIZE / GAPCM_BLOCK_SIZE)

#define GAPCM_ERROR_FORMAT "The format is invalid."
#define GAPCM_ERROR_LENGTH                                                     \
  "The stream length is less than the loop start position."
#define GAPCM_ERROR_MARK                                                       \
  "The loop start position is more than the logical maximum."

/**
 * Represents a GAPCM header. A block spans 1024 samples, a frame spans one
 * sample for mono, two for stereo, and a tick spans 7.8 ms.
 */
struct GaPcmHeader {
  /** Stream format. 1: stereo, 2: mono. */
  uint16_t format;
  /** Loop start position in blocks. */
  uint32_t mark;
  /** Length between stream start and loop end in frames. */
  uint32_t length;
  /** Echo pans for channels 3 to 8. Low nibble: left, high nibble: right. */
  uint8_t echo_pans[6];
  /** First echo delay in ticks. */
  uint8_t echo_pregap;
  /** Echo delay in ticks. */
  uint8_t echo_delay;
  /** Echo levels for channel pairs 3 and 4 to 7 and 8. */
  uint8_t echo_levels[3];
  /** Artificial silence length in blocks. */
  uint8_t pregap;
};

/**
 * Decodes a header from the given sector to the given model and returns
 * `GAPCM_SECTOR_SIZE` on success.
 */
size_t gapcm_decode_header(uint8_t *sector, struct GaPcmHeader *header);

/** Decodes the given loop defined by the given header to the given output. */
unsigned long long gapcm_decode_loop(const struct GaPcmHeader *header,
                                     FILE *source, FILE *output,
                                     int loop_count);

/** Writes the given count of silent blocks to the given file. */
unsigned long long gapcm_decode_pregap(uint8_t pregap, FILE *file);

/**
 * Translates the given GAPCM sign–magnitude sample. [0x00, 0x7f] maps to [-128,
 * -1], and [0x80, 0xff] to [0, 127]. The returned format depends on
 * `GAPCM_SAMPLE_ORIGIN`.
 */
uint8_t gapcm_decode_sample(uint8_t sample);

/**
 * Decodes the given sector to the given block and returns `GAPCM_SECTOR_SIZE`
 * on success.
 */
size_t gapcm_decode_sector(const uint8_t *sector, size_t count, uint8_t *block);

/**
 * Flushes then seeks the given file to the given position in blocks and returns
 * its success.
 */
int gapcm_decode_seek(FILE *file, uint32_t position);

/** Decodes the given stream defined by the given header to the given output. */
unsigned long long gapcm_decode_stream(const struct GaPcmHeader *header,
                                       FILE *source, FILE *output,
                                       int loop_count);

/** Decodes the given stream for the given count of frames. */
unsigned long long gapcm_decode_stream_for(const struct GaPcmHeader *header,
                                           FILE *source, FILE *output,
                                           uint32_t count);

/**
 * Encodes the given header to the given sector and returns `GAPCM_SECTOR_SIZE`
 * on success.
 */
size_t gapcm_encode_header(struct GaPcmHeader *header, uint8_t *sector);

/**
 * Translates the given sample. The returned format depends on
 * `GAPCM_SAMPLE_ORIGIN`.
 */
uint8_t gapcm_encode_sample(uint8_t sample);

/** Encodes the given block to the given sector. */
size_t gapcm_encode_sector(const uint8_t *block, size_t count, uint8_t *sector);

/** Encodes the given stream defined by the given header to the given output. */
unsigned long long gapcm_encode_stream(const struct GaPcmHeader *header,
                                       FILE *source, FILE *output);

/** Encodes the given stream for the given count of frames. */
unsigned long long gapcm_encode_stream_for(const struct GaPcmHeader *header,
                                           FILE *source, FILE *output,
                                           uint32_t count);

/**
 * Checks the given header and returns its success. The given non-NULL `error`
 * points to the message of the first encountered error.
 */
bool gapcm_header_check(const struct GaPcmHeader *header, const char **error);

/** Frees the given GAPCM header. */
struct GaPcmHeader *gapcm_header_free(struct GaPcmHeader *header);

/** Makes a GAPCM header. */
struct GaPcmHeader *gapcm_header_make(void);

/**
 * Stringifies the given header to the given string and returns the resulting
 * length. This requires at most `GAPCM_HEADER_STRING_CAPACITY` char units.
 */
int gapcm_header_stringify(const struct GaPcmHeader *header, char *string);

/** Translates the given stream format to channel count. */
uint16_t gapcm_to_channelcount(uint16_t format);

/** Translates the given channel count to stream format. */
uint16_t gapcm_to_format(uint16_t channel_count);

#endif
