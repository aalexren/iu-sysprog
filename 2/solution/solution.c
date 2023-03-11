#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "command.h"
#include "stack.h"
#include "pair.h"


int
exec_cmds(struct cmd **comms, int count)
{
    for (int i = 0; i < count; ++i)
    {
        if (strcmp(cmd_get_name(comms[i]), "cd") == 0)
        {
            if (chdir(cmd_get_argv(comms[i])[0]) != 0)
            {
                fprintf(stderr, "cd: no such file or directory: %s\n", cmd_get_argv(comms[i])[0]);
                return 1;
            }
        }
    }
    
    return 0;
}

void
shell_loop()
{
    for (;;)
    {
        printf("$> ");
        /* Parse tokens from input string. */
        char *raw_input = read_line();
        struct pair *tokens = parse_line(raw_input);
        int tokens_len = *(int*)snd_pair(tokens);
        char **tokens_args = (char**)fst_pair(tokens);

        /* Parse command stuctures from tokens. */
        struct pair *commands = parse_cmds(tokens_args, tokens_len);
        struct cmd **commands_array = NULL; int commands_count = 0;
        if (fst_pair(commands) != NULL) {
            commands_array = (struct cmd**)fst_pair(commands);
            commands_count = *(int*)snd_pair(commands);
        }

        exec_cmds(commands_array, commands_count);

        /* Free allocated memory to avoid memory leak. */
        free(raw_input);
        free(tokens);
        free(commands);
        for (int i = 0; i < commands_count; ++i)
        {
            cmd_free(commands_array[i]);
        }
        free(commands_array);
    }
}


int main(int argc, char **argv)
{

    shell_loop();

    // printf("%s", read_line());
    // return 0;
    struct pair *p_ = parse_line(read_line());
    int len = *(int*)snd_pair(p_);
    char **args = (char**)fst_pair(p_);
    // printf("\nnumber of args: %d\n", len);
    // for (int i = 0; i < len; ++i)
    // {
    //     printf("%s\n", args[i]);
    // }
    parse_cmds(args, len);
    // struct pair *cmds_ = parse_cmds(p_);
    // printf("%d", *(int*)snd_pair(cmds_));
    // printf("\n%p", cmds);
    // cmd_print(cmds[0]);
    // struct pair *p_ = parse_line("echo 123\x20'123'\\\"456\\\"");
    // struct pair *p_ = parse_line("mv my\\ name\\ 1.txt my\\ name\\ 2.txt");
    
    // free(p_);
    // printf("%s, %d", ((char**)fst_pair(p_))[0], *(int*)snd_pair(p_));

    // return 0;
    // char *keks[] = {"echo", "dzin", NULL};
    // int a = 10;
    // struct pair* p = make_pair(keks, &a);
    // printf("%s, %d", ((char**)fst_pair(p))[0], *(int*)snd_pair(p));
    // return 0;
    // struct cmd **cmds = parse_cmds("echo \x7 123 \x7 > \x7 456 \x7");
    // cmd_print(cmds[0]);
    // // cmd_print(cmds[1]);
    
    return 0;
    
    
    // pid_t child = fork();

    // if (child == 0) {
    //     /* child proccess */
    //     char* args[] = {"echo", s, NULL};
    //     int status_code = execvp("echo", args);
    //     // char *args[] = {"rm", s, NULL};
    //     // int status_code = execvp("rm", args);

    //     if (status_code == -1) {
    //         printf("Oh, no, something goes wrong...%s\n", strerror(errno));
    //     }
    // }

    // // printf("PID: %d\n", child);
    // free(s);
    // free(rs);

    return 0;
}