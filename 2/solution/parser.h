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
 */
char* read_line();