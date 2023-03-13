#pragma once

#include "pair.h"

struct cmd;

/**
 * Parse line to find all commands and their arguments.
 * Commands separated by |, && and ||.
 * 
 * Returns pointer to command array and pointer to length of this array.
 */
struct pair *parse_cmds(char **argv, int argc);

struct cmd *cmd_init(char *name);

/* There is no check for NULL pointer! */
char *cmd_get_name(struct cmd *cmd_);

/* There is no check for NULL pointer! */
char **cmd_get_argv(struct cmd *cmd_);

/* There is no check for NULL pointer! */
int cmd_get_argc(struct cmd *cmd_);

/* There is no check for NULL pointer! */
int cmd_get_special(struct cmd *cmd_);

void cmd_free(struct cmd *cmd_);

void cmd_print(struct cmd* cmd_);