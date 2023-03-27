#pragma once

#include "pair.h"

/**
 * Check if stream is set up to EOF,
 * returns 1 if true, 0 otherwise.
 */
int is_eof(char next);

/**
 * Check if end of line, i.e. '\n' character
 * was put on stream. If previous character
 * was \ then continue reading, because it's
 * new line reading.
 */
int is_eol(char next, char prev);

/**
 * Read line of stdin, returns pointer to char.
 * Converts all two-length sequences 
 * starting with \ to escape characters.
 * Memory allocated here should be freed!
 * 
 * eof_flag is set to 1 if there is closed stdin.
 */
char *read_line(int *eof_flag);

/**
 * https://docs.python.org/3/library/shlex.html#parsing-rules
 * Operating in POSIX mode.
 * 
 * Returns pair consists of pointers to tokens and pointer to number of tokens.
 */
struct pair *parse_line(char *rs);