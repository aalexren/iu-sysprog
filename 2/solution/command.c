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

    /**
     * default -> 0
     * |  -> 1
     * >  -> 1
     * >> -> 1
     * && -> 1
     * || -> 1
     * &  -> 1
     */
    int special;
};

struct pair*
parse_cmds(char **argv, int argc)
{
    // char **argv = (char **)fst_pair(args);
    // int argc = *(int *)snd_pair(args);

    if (argc < 1) return NULL;

    struct cmd **cmds = malloc(sizeof(struct cmd*));
    int size = 1;
    int index = 0;

    struct cmd* temp = cmd_init(argv[0]);
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], ">") == 0  ||
            strcmp(argv[i], ">>") == 0)
        {
            size += 1;
            cmds = realloc(cmds, sizeof(struct cmd*) * size);

            // /* put NULL to the end of array */
            // temp->argv = realloc(temp->argv, sizeof(char *) * (temp->argc + 1));
            // temp->argv[temp->argc] = NULL;
            // temp->argc += 1;

            /* save current command. */
            cmds[index++] = temp;

            /* start next command. */
            temp = cmd_init(argv[i]);
            temp->special = true;
        }
        else if (strcmp(argv[i], "||") == 0 ||
                 strcmp(argv[i], "&&") == 0 ||
                 strcmp(argv[i], "|") == 0  ||
                 strcmp(argv[i], "&") == 0)
        {
            size += 2;
            cmds = realloc(cmds, sizeof(struct cmd*) * size);

            // /* put NULL to the end of array */
            // temp->argv = realloc(temp->argv, sizeof(char *) * (temp->argc + 1));
            // temp->argv[temp->argc] = NULL;
            // temp->argc += 1;

            /* save current command. */
            cmds[index++] = temp; 

            /* start next command and save it. */
            temp = cmd_init(argv[i]);
            temp->special = true;
            cmds[index++] = temp;

            /* start next command. */
            if (i + 1 < argc) {
                temp = cmd_init(argv[i + 1]);
                i++;
            }
            else {
                index--; /* this is the last command. */
            }
        }
        else {
            temp->argv = realloc(temp->argv, sizeof(char *) * (temp->argc + 1));
            temp->argv[temp->argc] = argv[i];
            temp->argc += 1;
        }
    }
    // /* put NULL to the end of array */
    // temp->argv = realloc(temp->argv, sizeof(char *) * (temp->argc + 1));
    // temp->argv[temp->argc] = NULL;
    // temp->argc += 1;
    cmds[index] = temp;

    // printf("index: %d\n", index);
    // for (int i = 0; i < index + 1; ++i)
    // {
    //     cmd_print(cmds[i]);
    // }

    int len = index + 1;
    return make_pair(cmds, &len);
}

struct cmd*
cmd_init(char *name)
{
    struct cmd* cmd_ = malloc(sizeof(*cmd_));
    cmd_->name = name;
    cmd_->argv = NULL;
    cmd_->argc = 0;
    cmd_->special = 0;

    return cmd_;
}

char *
cmd_get_name(struct cmd *cmd_)
{
    return cmd_->name;
}

char **
cmd_get_argv(struct cmd *cmd_)
{
    return cmd_->argv;
}

int
cmd_get_argc(struct cmd *cmd_)
{
    return cmd_->argc;
}

int
cmd_get_special(struct cmd *cmd_)
{
    return cmd_->special;
}

void
cmd_free(struct cmd *cmd_)
{
    free(cmd_->name);
    for (int i = 0; i < cmd_->argc + 1; ++i)
    {
        free(cmd_->argv[i]);
    }
    free(cmd_->argv);
    free(cmd_);
}

void 
cmd_print(struct cmd* cmd_)
{
    printf("name: %s,\targc: %d,\tspec: %d,\targv = [", cmd_->name, cmd_->argc, cmd_->special);
    for (int i = 0; i < cmd_->argc + 1; ++i)
    {
        printf("%s, ", cmd_->argv[i]);
    }
    printf("]\n");
}