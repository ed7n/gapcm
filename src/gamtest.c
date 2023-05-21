/**
 * GAPCM: Unit Tests
 */

#include "common/constants.h"
#include "gapcm/gapcm.h"
#include <assert.h>
#include <stdlib.h>

#define GAMTEST_SECTOR_BYTES 4

#define O GAPCM_SAMPLE_ORIGIN
static const uint8_t gamtest_decode_table[] = {
    O - 1,   O - 2,   O - 3,   O - 4,   O - 5,   O - 6,   O - 7,   O - 8,
    O - 9,   O - 10,  O - 11,  O - 12,  O - 13,  O - 14,  O - 15,  O - 16,
    O - 17,  O - 18,  O - 19,  O - 20,  O - 21,  O - 22,  O - 23,  O - 24,
    O - 25,  O - 26,  O - 27,  O - 28,  O - 29,  O - 30,  O - 31,  O - 32,
    O - 33,  O - 34,  O - 35,  O - 36,  O - 37,  O - 38,  O - 39,  O - 40,
    O - 41,  O - 42,  O - 43,  O - 44,  O - 45,  O - 46,  O - 47,  O - 48,
    O - 49,  O - 50,  O - 51,  O - 52,  O - 53,  O - 54,  O - 55,  O - 56,
    O - 57,  O - 58,  O - 59,  O - 60,  O - 61,  O - 62,  O - 63,  O - 64,
    O - 65,  O - 66,  O - 67,  O - 68,  O - 69,  O - 70,  O - 71,  O - 72,
    O - 73,  O - 74,  O - 75,  O - 76,  O - 77,  O - 78,  O - 79,  O - 80,
    O - 81,  O - 82,  O - 83,  O - 84,  O - 85,  O - 86,  O - 87,  O - 88,
    O - 89,  O - 90,  O - 91,  O - 92,  O - 93,  O - 94,  O - 95,  O - 96,
    O - 97,  O - 98,  O - 99,  O - 100, O - 101, O - 102, O - 103, O - 104,
    O - 105, O - 106, O - 107, O - 108, O - 109, O - 110, O - 111, O - 112,
    O - 113, O - 114, O - 115, O - 116, O - 117, O - 118, O - 119, O - 120,
    O - 121, O - 122, O - 123, O - 124, O - 125, O - 126, O - 127, O - 128,
    O + 0,   O + 1,   O + 2,   O + 3,   O + 4,   O + 5,   O + 6,   O + 7,
    O + 8,   O + 9,   O + 10,  O + 11,  O + 12,  O + 13,  O + 14,  O + 15,
    O + 16,  O + 17,  O + 18,  O + 19,  O + 20,  O + 21,  O + 22,  O + 23,
    O + 24,  O + 25,  O + 26,  O + 27,  O + 28,  O + 29,  O + 30,  O + 31,
    O + 32,  O + 33,  O + 34,  O + 35,  O + 36,  O + 37,  O + 38,  O + 39,
    O + 40,  O + 41,  O + 42,  O + 43,  O + 44,  O + 45,  O + 46,  O + 47,
    O + 48,  O + 49,  O + 50,  O + 51,  O + 52,  O + 53,  O + 54,  O + 55,
    O + 56,  O + 57,  O + 58,  O + 59,  O + 60,  O + 61,  O + 62,  O + 63,
    O + 64,  O + 65,  O + 66,  O + 67,  O + 68,  O + 69,  O + 70,  O + 71,
    O + 72,  O + 73,  O + 74,  O + 75,  O + 76,  O + 77,  O + 78,  O + 79,
    O + 80,  O + 81,  O + 82,  O + 83,  O + 84,  O + 85,  O + 86,  O + 87,
    O + 88,  O + 89,  O + 90,  O + 91,  O + 92,  O + 93,  O + 94,  O + 95,
    O + 96,  O + 97,  O + 98,  O + 99,  O + 100, O + 101, O + 102, O + 103,
    O + 104, O + 105, O + 106, O + 107, O + 108, O + 109, O + 110, O + 111,
    O + 112, O + 113, O + 114, O + 115, O + 116, O + 117, O + 118, O + 119,
    O + 120, O + 121, O + 122, O + 123, O + 124, O + 125, O + 126, O + 127};
#undef O

void gamtest_sector(const uint8_t *sector) {
#if GAPCM_SAMPLE_BYTES == 2
  const uint8_t answer[] = {
      gapcm_decode_sample(sector[0]), gapcm_decode_sample(sector[1]),
      gapcm_decode_sample(sector[2]), gapcm_decode_sample(sector[3])};
#else
  const uint8_t answer[] = {gapcm_decode_sample(sector[1]),
                            gapcm_decode_sample(sector[3]), 0xab, 0xcd};
#endif
  uint8_t decode[] = {0xab, 0xcd, 0xef, 0xbc};
  size_t decode_count =
      gapcm_decode_sector(sector, GAMTEST_SECTOR_BYTES, decode);
  uint8_t encode[] = {0xab, 0xcd, 0xef, 0xbc};
  size_t encode_count = gapcm_encode_sector(decode, decode_count, encode);
  printf("  [0x%02x, 0x%02x, 0x%02x, 0x%02x] -> [0x%02x, 0x%02x, 0x%02x, "
         "0x%02x] ([0x%02x, 0x%02x, 0x%02x, 0x%02x]) -> [0x%02x, 0x%02x, "
         "0x%02x, 0x%02x]" EOL,
         sector[0], sector[1], sector[2], sector[3], decode[0], decode[1],
         decode[2], decode[3], answer[0], answer[1], answer[2], answer[3],
         encode[0], encode[1], encode[2], encode[3]);
  assert(decode_count == GAMTEST_SECTOR_BYTES / GAPCM_SECTOR_BLOCKS);
  for (size_t index = 0; index < decode_count; index++) {
    assert(decode[index] == answer[index]);
  }
  assert(encode_count == GAMTEST_SECTOR_BYTES);
  for (size_t index = 0; index < encode_count;
       index += GAPCM_SAMPLE_BYTES + GAPCM_SAMPLE_BYTES_PAD) {
#if GAPCM_SAMPLE_BYTES == 2
    assert(encode[index] == sector[index]);
#else
    assert(encode[index] == 0);
#endif
    assert(encode[index + 1] == sector[index + 1]);
  }
}

int main() {
  printf("Sample transcode for origin `0x%02x`." EOL, GAPCM_SAMPLE_ORIGIN);
  for (uint8_t sample = 0; sample < UINT8_MAX; sample++) {
    uint8_t decode = gapcm_decode_sample(sample);
    uint8_t encode = gapcm_encode_sample(decode);
    printf("  0x%02x -> 0x%02x (0x%02x) -> 0x%02x" EOL, sample, decode,
           gamtest_decode_table[sample], encode);
    assert(decode == gamtest_decode_table[sample]);
    assert(encode == sample);
  }
  printf("Sector transcode for origin `0x%02x` and sample byte count of "
         "`%u`." EOL,
         GAPCM_SAMPLE_ORIGIN, GAPCM_SAMPLE_BYTES);
  uint8_t sector[] = {0xab, 0xcd, 0xef, 0xbc};
  gamtest_sector(sector);
  sector[0] = 0xbc;
  sector[1] = 0xef;
  sector[2] = 0xcd;
  sector[3] = 0xab;
  gamtest_sector(sector);
  sector[0] = 0xcd;
  sector[1] = 0xab;
  sector[2] = 0xbc;
  sector[3] = 0xef;
  gamtest_sector(sector);
  sector[0] = 0xef;
  sector[1] = 0xbc;
  sector[2] = 0xab;
  sector[3] = 0xcd;
  gamtest_sector(sector);
  sector[0] = 0x00;
  sector[1] = 0x7f;
  sector[2] = 0x80;
  sector[3] = 0xff;
  gamtest_sector(sector);
  sector[0] = 0xff;
  sector[1] = 0x80;
  sector[2] = 0x7f;
  sector[3] = 0x00;
  gamtest_sector(sector);
  sector[0] = 0x7f;
  sector[1] = 0x00;
  sector[2] = 0xff;
  sector[3] = 0x80;
  gamtest_sector(sector);
  sector[0] = 0x80;
  sector[1] = 0xff;
  sector[2] = 0x00;
  sector[3] = 0x7f;
  gamtest_sector(sector);
  puts("Done.");
  return EXIT_SUCCESS;
}
