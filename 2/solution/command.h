#include "pair.h"

struct cmd;

/**
 * Parse line to find all commands and their arguments.
 * Commands separated by |, && and ||.
 */
struct pair *parse_cmds(struct pair* args);

struct cmd *cmd_init(char *name);

void cmd_print(struct cmd* cmd_);