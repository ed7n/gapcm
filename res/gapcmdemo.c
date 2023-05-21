// This sample decoder does basic looping. Stereo is not supported.
//
// As seen on [https://www.youtube.com/watch?v=iPH80UoAIxE].
//
// $ cc -Wall -Wpedantic -Wextra gapcmdemo.c

#include <arpa/inet.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GAPCM_BLOCK_BYTES 1024
#define GAPCM_SAMPLE_ORIGIN 0x80
#define GAPCM_SECTOR_BYTES 2048

int main(int argument_count, char *arguments[]) {
  if (argument_count < 2) {
    fputs("No file.\n", stderr);
    return EXIT_FAILURE;
  }
  FILE *file = fopen(arguments[1], "rb");
  if (file == NULL) {
    fprintf(stderr, "%s: %s\n", arguments[1], strerror(errno));
    return EXIT_FAILURE;
  }
  uint8_t block[GAPCM_BLOCK_BYTES];
  uint8_t sector[GAPCM_SECTOR_BYTES];
  unsigned long long count = 0;
  fread(&sector, 1, GAPCM_SECTOR_BYTES, file);
  uint32_t marker = ntohl(*(uint32_t *)&sector[2]);
  uint32_t length = ntohl(*(uint32_t *)&sector[6]);
  fprintf(stderr, "marker: %u\nlength: %u\n", marker, length);
  while (fread(&sector, 1, GAPCM_SECTOR_BYTES, file) == GAPCM_SECTOR_BYTES) {
    for (size_t index = 0; index < GAPCM_BLOCK_BYTES; index++) {
      uint8_t sample = sector[index * 2 + 1];
      block[index] = sample & 0x80 ? GAPCM_SAMPLE_ORIGIN + (sample & 0x7f)
                                   : GAPCM_SAMPLE_ORIGIN - (sample & 0x7f) - 1;
    }
    count += GAPCM_BLOCK_BYTES;
    if (count >= length) {
      fwrite(&block, GAPCM_BLOCK_BYTES - (count + length), 1, stdout);
      fseek(file, GAPCM_SECTOR_BYTES * (marker + 1), SEEK_SET);
      count = GAPCM_BLOCK_BYTES * marker;
    } else {
      fwrite(&block, GAPCM_BLOCK_BYTES, 1, stdout);
    }
  }
  return EXIT_SUCCESS;
}
