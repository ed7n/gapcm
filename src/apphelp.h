/**
 * GAPCM: Application Help
 *
 * Application help messages.
 */
#ifndef _APPHELP_H
#define _APPHELP_H

#include "gapcm/gapcm.h"

#if GAPCM_SAMPLE_BYTES == 2
#define APPHELP_BIT_COUNT "16"
#else
#define APPHELP_BIT_COUNT "8"
#endif
#if GAPCM_SAMPLE_ORIGIN == 0
#define APPHELP_SIGNEDNESS "signed"
#else
#define APPHELP_SIGNEDNESS "unsigned"
#endif
/** Syntax to explanation. */
#define APPHELP_INVITATION "`--help` for more information."
/** Explanation to syntax. */
#define APPHELP_EXPLANATION                                                    \
  "\n\
Program Arguments:\n\
  -h, --help              Display this help message.\n\
  --                      Stop parsing options.\n"

#endif
