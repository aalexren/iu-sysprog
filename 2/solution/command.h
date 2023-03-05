struct cmd {
    const char *name;
    const char **argv;
    int argc;
};

/**
 * Parse line to find all commands and their arguments.
 * Commands separated by |, && and ||.
 */
struct cmd *parse_line(char *line);