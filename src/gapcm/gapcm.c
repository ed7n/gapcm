// File operators stop on I/O anomalies. Echo, fade, and gain features are not
// supported; get their parameters here and apply them elsewhere.

#include "gapcm.h"
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#define GAPCM_SUCCESS 0

/** Represents a transcoding context. */
struct GaPcmIoContext {
  /** GAPCM header. */
  const struct GaPcmHeader *header;
  /** Output stream. */
  FILE *output;
  /** Source stream. */
  FILE *source;
  /** Block byte counts. */
  size_t *counts;
  /** Block indexes. */
  size_t *indexes;
  /** Consumer PCM buffers. */
  uint8_t *blocks;
  /** GAPCM sector buffer. */
  uint8_t *sector;
  /** Stream channel count. */
  uint16_t CHANNEL_COUNT;
};

/** Runs the given decode context for the given count of samples. */
static unsigned long long gapcm_decode_context_for(struct GaPcmIoContext *c,
                                                   unsigned long long count) {
  if (c->CHANNEL_COUNT <= 0) {
    return 0;
  }
  unsigned long long out = 0;
  bool error = false;
  while (!error && count >= c->CHANNEL_COUNT) {
    for (size_t channel = 0; !error && channel < c->CHANNEL_COUNT; channel++) {
      c->counts[channel] = gapcm_decode_sector(
          c->sector, fread(c->sector, 1, GAPCM_SECTOR_SIZE, c->source),
          &c->blocks[GAPCM_BLOCK_SIZE * channel]);
      if (c->counts[channel] != GAPCM_BLOCK_SIZE) {
        error = true;
      }
      if (c->counts[channel] > count / c->CHANNEL_COUNT) {
        c->counts[channel] = count / c->CHANNEL_COUNT;
      }
      c->indexes[channel] = 0;
    }
    for (size_t channel = 0;; channel = (channel + 1) % c->CHANNEL_COUNT) {
      const unsigned char origin = GAPCM_SAMPLE_ORIGIN;
      const void *ptr;
      if (c->indexes[channel] < c->counts[channel]) {
        ptr = &c->blocks[GAPCM_BLOCK_SIZE * channel + c->indexes[channel]++];
      } else if (channel > 0) {
        ptr = &origin;
      } else {
        break;
      }
      if (fwrite(ptr, 1, 1, c->output) != 1) {
        return out;
      }
      out++;
    }
    count -= c->counts[0] * c->CHANNEL_COUNT;
  }
  return out;
}

static unsigned long long
gapcm_decode_context_loop(struct GaPcmIoContext *context, int loop_count) {
  if (loop_count < 1) {
    return 0;
  }
  unsigned long long length_loop =
      context->header->length * context->CHANNEL_COUNT -
      GAPCM_BLOCK_SIZE * context->header->mark;
  unsigned long long out = 0;
  while (true) {
    unsigned long long count_decode =
        gapcm_decode_context_for(context, length_loop);
    out += count_decode;
    if (count_decode != length_loop || --loop_count < 1 ||
        gapcm_decode_seek(context->source, context->header->mark) !=
            GAPCM_SUCCESS) {
      break;
    }
  }
  return out;
}

/** Runs the given encode context for the given count of samples. */
static unsigned long long gapcm_encode_context_for(struct GaPcmIoContext *c,
                                                   unsigned long long count) {
  if (c->CHANNEL_COUNT <= 0) {
    return 0;
  }
  unsigned long long out = 0;
  bool error = false;
  while (!error && count >= c->CHANNEL_COUNT) {
    size_t count_block = count / c->CHANNEL_COUNT > GAPCM_BLOCK_SIZE
                             ? GAPCM_BLOCK_SIZE
                             : count / c->CHANNEL_COUNT;
    for (size_t channel = 0; channel < c->CHANNEL_COUNT; channel++) {
      c->counts[channel] = 0;
      c->indexes[channel] = 0;
    }
    for (size_t channel = 0; c->indexes[channel] < count_block;
         channel = (channel + 1) % c->CHANNEL_COUNT) {
      if (fread(&c->blocks[GAPCM_BLOCK_SIZE * channel + c->indexes[channel]++],
                1, 1, c->source) != 1) {
        error = true;
        break;
      }
      c->counts[channel]++;
    }
    for (size_t channel = 0;
         channel < c->CHANNEL_COUNT && c->counts[channel] > 0; channel++) {
      for (size_t index = c->counts[channel]; index < GAPCM_BLOCK_SIZE;
           index++) {
        c->blocks[GAPCM_BLOCK_SIZE * channel + index] = GAPCM_SAMPLE_ORIGIN;
      }
      count -= c->counts[channel];
      unsigned long count_write =
          fwrite(c->sector, 1,
                 gapcm_encode_sector(&c->blocks[GAPCM_BLOCK_SIZE * channel],
                                     GAPCM_BLOCK_SIZE, c->sector),
                 c->output);
      out += count_write;
      if (count_write / GAPCM_SECTOR_BLOCKS != GAPCM_BLOCK_SIZE) {
        return out;
      }
    }
  }
  return out;
}

/** Frees the given transcoding context. */
static struct GaPcmIoContext *gapcm_iocontext_free(struct GaPcmIoContext *c) {
  free(c->blocks);
  free(c->counts);
  free(c->indexes);
  free(c->sector);
  free(c);
  return NULL;
}

/** Makes a transcoding context with the given. */
static struct GaPcmIoContext *
gapcm_iocontext_make(const struct GaPcmHeader *header, FILE *restrict stream,
                     FILE *restrict output) {
  struct GaPcmIoContext *out = malloc(sizeof(*out));
  out->CHANNEL_COUNT = gapcm_to_channelcount(header->format);
  out->blocks =
      malloc(sizeof(*out->blocks) * GAPCM_BLOCK_SIZE * out->CHANNEL_COUNT);
  out->counts = malloc(sizeof(*out->counts) * out->CHANNEL_COUNT);
  out->header = header;
  out->indexes = malloc(sizeof(*out->indexes) * out->CHANNEL_COUNT);
  out->output = output;
  out->sector = malloc(sizeof(*out->sector) * GAPCM_SECTOR_SIZE);
  out->source = stream;
  return out;
}

size_t gapcm_decode_header(uint8_t *restrict sector,
                           struct GaPcmHeader *header) {
  // Address alignment.
  uint32_t data = 0;
  header->format = ntohs(*(uint16_t *)&sector[0]);
  memcpy(&data, &sector[2], sizeof(data));
  header->mark = ntohl(data);
  memcpy(&data, &sector[6], sizeof(data));
  header->length = ntohl(data);
  memcpy(&(header->echo_pans), &sector[10], 6);
  header->echo_pregap = sector[16];
  header->echo_delay = sector[17];
  memcpy(&(header->echo_levels), &sector[18], 3);
  header->pregap = sector[21];
  return GAPCM_SECTOR_SIZE;
}

unsigned long long gapcm_decode_loop(const struct GaPcmHeader *header,
                                     FILE *restrict source,
                                     FILE *restrict output, int loop_count) {
  struct GaPcmIoContext *context = gapcm_iocontext_make(header, source, output);
  unsigned long long out = gapcm_decode_context_loop(context, loop_count);
  context = gapcm_iocontext_free(context);
  return out;
}

unsigned long long gapcm_decode_pregap(uint8_t pregap, FILE *restrict file) {
  const unsigned char origin = GAPCM_SAMPLE_ORIGIN;
  unsigned long long out = 0;
  while (pregap-- > 0) {
    for (size_t index = 0; index < GAPCM_BLOCK_SIZE; index++) {
      if (fwrite(&origin, 1, 1, file) != 1) {
        break;
      }
      out++;
    }
  }
  return out;
}

uint8_t gapcm_decode_sample(const uint8_t sample) {
  return sample & 0x80 ? GAPCM_SAMPLE_ORIGIN + (sample & 0x7f)
                       : GAPCM_SAMPLE_ORIGIN - (sample & 0x7f) - 1;
}

size_t gapcm_decode_sector(const uint8_t *restrict sector, const size_t count,
                           uint8_t *restrict block) {
  size_t out = 0;
  for (size_t offset = 1; offset < count; offset = ++out * 2 + 1) {
    block[out] = gapcm_decode_sample(sector[offset]);
  }
  return out;
}

#define GAPCM_OFFSET_MAXIMUM UINT32_MAX / GAPCM_SECTOR_SIZE
int gapcm_decode_seek(FILE *restrict file, uint32_t position) {
  int out = fflush(file);
  if (out != GAPCM_SUCCESS) {
    return out;
  }
  out = fseek(file, GAPCM_SECTOR_SIZE, SEEK_SET);
  if (out != GAPCM_SUCCESS) {
    return out;
  }
  while (position >= GAPCM_OFFSET_MAXIMUM) {
    out = fseek(file, GAPCM_SECTOR_SIZE * GAPCM_OFFSET_MAXIMUM, SEEK_CUR);
    if (out != GAPCM_SUCCESS) {
      return out;
    }
    position -= GAPCM_OFFSET_MAXIMUM;
  }
  return fseek(file, GAPCM_SECTOR_SIZE * position, SEEK_CUR);
}
#undef GAPCM_OFFSET_MAXIMUM

unsigned long long gapcm_decode_stream(const struct GaPcmHeader *header,
                                       FILE *restrict source,
                                       FILE *restrict output, int loop_count) {
  struct GaPcmIoContext *context = gapcm_iocontext_make(header, source, output);
  unsigned long long mark = GAPCM_BLOCK_SIZE * context->header->mark;
  unsigned long long out = gapcm_decode_context_for(context, mark);
  if (out == mark) {
    out += gapcm_decode_context_loop(context, loop_count);
  }
  context = gapcm_iocontext_free(context);
  return out;
}

unsigned long long gapcm_decode_stream_for(const struct GaPcmHeader *header,
                                           FILE *restrict source,
                                           FILE *restrict output,
                                           const uint32_t count) {
  struct GaPcmIoContext *context = gapcm_iocontext_make(header, source, output);
  unsigned long long out =
      gapcm_decode_context_for(context, count * context->CHANNEL_COUNT);
  context = gapcm_iocontext_free(context);
  return out;
}

size_t gapcm_encode_header(struct GaPcmHeader *header, uint8_t *sector) {
  uint32_t longg = htonl(header->mark);
  uint16_t shortt = htons(header->format);
  memcpy(&sector[0], &shortt, sizeof(shortt));
  memcpy(&sector[2], &longg, sizeof(longg));
  longg = htonl(header->length);
  memcpy(&sector[6], &longg, sizeof(longg));
  memcpy(&sector[10], &(header->echo_pans), 6);
  sector[16] = header->echo_pregap;
  sector[17] = header->echo_delay;
  memcpy(&sector[18], &(header->echo_levels), 3);
  sector[21] = header->pregap;
  for (size_t index = 22; index < GAPCM_SECTOR_SIZE; index++) {
    sector[index] = 0;
  }
  return GAPCM_SECTOR_SIZE;
}

uint8_t gapcm_encode_sample(const uint8_t sample) {
  return sample & 0x80 ? 0x80 | (sample - GAPCM_SAMPLE_ORIGIN)
                       : 0x7f & (GAPCM_SAMPLE_ORIGIN - sample - 1);
}

size_t gapcm_encode_sector(const uint8_t *restrict pointer, const size_t count,
                           uint8_t *restrict block) {
  size_t out = 0;
  for (size_t index = 0; index < count; out = ++index * 2) {
    block[out] = 0;
    block[out + 1] = gapcm_encode_sample(pointer[index]);
  }
  return out;
}

unsigned long long gapcm_encode_stream(const struct GaPcmHeader *header,
                                       FILE *restrict source,
                                       FILE *restrict output) {
  return gapcm_encode_stream_for(header, source, output, header->length);
}

unsigned long long gapcm_encode_stream_for(const struct GaPcmHeader *header,
                                           FILE *restrict source,
                                           FILE *restrict output,
                                           const uint32_t count) {
  struct GaPcmIoContext *context = gapcm_iocontext_make(header, source, output);
  unsigned long long out =
      gapcm_encode_context_for(context, count * context->CHANNEL_COUNT);
  context = gapcm_iocontext_free(context);
  return out;
}

bool gapcm_header_check(const struct GaPcmHeader *header, const char **error) {
  uint16_t channel_count = gapcm_to_channelcount(header->format);
  if (channel_count == 0) {
    if (error != NULL) {
      *error = GAPCM_ERROR_FORMAT;
    }
    return false;
  }
  uint16_t block_frames = GAPCM_BLOCK_SIZE / channel_count;
  if (header->mark > UINT32_MAX / block_frames) {
    if (error != NULL) {
      *error = GAPCM_ERROR_MARK;
    }
    return false;
  }
  if (header->length / block_frames < header->mark) {
    if (error != NULL) {
      *error = GAPCM_ERROR_LENGTH;
    }
    return false;
  }
  return true;
}

struct GaPcmHeader *gapcm_header_free(struct GaPcmHeader *header) {
  free(header);
  return NULL;
}

struct GaPcmHeader *gapcm_header_make(void) {
  return calloc(1, sizeof(struct GaPcmHeader));
}

int gapcm_header_stringify(const struct GaPcmHeader *h, char *restrict string) {
  return snprintf(
      string, GAPCM_HEADER_STRING_CAPACITY,
      "channel_count : %u\nmark          : %u\nlength        : "
      "%u\necho_pans     : %02x %02x %02x %02x %02x %02x\necho_pregap   : "
      "%u\necho_delay    : %u\necho_levels   : %u %u %u\npregap        : %u",
      gapcm_to_channelcount(h->format), h->mark, h->length, h->echo_pans[0],
      h->echo_pans[1], h->echo_pans[2], h->echo_pans[3], h->echo_pans[4],
      h->echo_pans[5], h->echo_pregap, h->echo_delay, h->echo_levels[0],
      h->echo_levels[1], h->echo_levels[2], h->pregap);
}

uint16_t gapcm_to_channelcount(const uint16_t format) {
  switch (format) {
  case GAPCM_FORMAT_MONO:
    return 1;
  case GAPCM_FORMAT_STEREO:
    return 2;
  }
  return 0;
}

uint16_t gapcm_to_format(const uint16_t channel_count) {
  switch (channel_count) {
  case 1:
    return GAPCM_FORMAT_MONO;
  case 2:
    return GAPCM_FORMAT_STEREO;
  }
  return 0;
}
