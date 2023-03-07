#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "command.h"
#include "stack.h"
#include "pair.h"

#define true 1
#define false 0

struct cmd {
    char *name;
    char **argv;
    int argc;
};

struct cmd **
parse_cmds(struct pair* args)
{
    return NULL;
}

void 
cmd_print(struct cmd* cmd_)
{
    printf("name: %s\argc: %d\n", cmd_->name, cmd_->argc);
    for (int i = 0; i < cmd_->argc; ++i)
    {
        printf("%d. %s\n", i, cmd_->argv[i]);
    }
}