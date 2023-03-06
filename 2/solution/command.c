#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "command.h"

struct cmd *
parse_cmds(char *line)
{
    int index = 0;
    size_t len = strlen(line);

    if (len == 1) {
        // TODO
    }

    /** Firstly count how many commands do we have to
     * allocate enough memory for list struct cmd *. */
    size_t cmds_count = 0;
    for (int i = 1; i < len; ++i) {
        if (line[i-1] == '\\' && line[i] == '|') continue;

    }
    
    /* Skip tabs, spaces etc. */
    for (;index < len && isalpha(line[index]) == 0; index++) {}

    size_t cmd_name_len = 0;
    /* Find command name first. */
    for (; index < len; index++)
    {
        if (isalpha(line[index])) cmd_name_len++;
        else break;
    }

}