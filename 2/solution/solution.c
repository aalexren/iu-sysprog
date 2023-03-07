#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "command.h"
#include "stack.h"
#include "pair.h"


int main(int argc, char **argv)
{
    // printf("%s", read_line());
    // return 0;
    struct pair *p_ = parse_line(read_line());
    // struct pair *p_ = parse_line("echo 123\x20'123'\\\"456\\\"");
    // struct pair *p_ = parse_line("mv my\\ name\\ 1.txt my\\ name\\ 2.txt");
    int len = *(int*)snd_pair(p_);
    char **args = (char**)fst_pair(p_);
    printf("number of args: %d\n", len);
    for (int i = 0; i < len; ++i)
    {
        printf("%s\n", args[i]);
    }
    // printf("%s, %d", ((char**)fst_pair(p_))[0], *(int*)snd_pair(p_));

    return 0;
    char *keks[] = {"echo", "dzin", NULL};
    int a = 10;
    struct pair* p = make_pair(keks, &a);
    printf("%s, %d", ((char**)fst_pair(p))[0], *(int*)snd_pair(p));
    return 0;
    struct cmd **cmds = parse_cmds("echo \x7 123 \x7 > \x7 456 \x7");
    cmd_print(cmds[0]);
    // cmd_print(cmds[1]);
    
    return 0;
    
    
    pid_t child = fork();

    if (child == 0) {
        /* child proccess */
        char* args[] = {"echo", s, NULL};
        int status_code = execvp("echo", args);
        // char *args[] = {"rm", s, NULL};
        // int status_code = execvp("rm", args);

        if (status_code == -1) {
            printf("Oh, no, something goes wrong...%s\n", strerror(errno));
        }
    }

    // printf("PID: %d\n", child);
    free(s);
    free(rs);

    return 0;
}