#include "pair.h"

struct cmd;

/**
 * Parse line to find all commands and their arguments.
 * Commands separated by |, && and ||.
 */
struct cmd **parse_cmds(struct pair* args);

void cmd_print(struct cmd* cmd_);