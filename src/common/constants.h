/**
 * EDCC: Constants
 *
 * Shared independent constants.
 */
#ifndef _EDCC_CONSTANTS_H
#define _EDCC_CONSTANTS_H

#define BYTES_CAPACITY 4096
#define FAILURE 1
#define LINE_WIDTH 80
#define NUL_STRING ""
#define SPACE " "
#define STRING_CAPACITY 1024
#define SUCCESS 0

#ifdef _WIN32
#define EOL "\r\n"
#else
#define EOL "\n"
#endif

#endif
