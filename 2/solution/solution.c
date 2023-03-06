#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"
// #include "command.h"
#include "stack.h"

char const str[245] = "keks";

int main(int argc, char **argv)
{
    char *rs = read_line();
    printf("raw string:\t%s\n", rs);
    char *s = parse_line(rs);
    printf("parsed string:\t%s\n", s);
    
    // return 0;
    
    
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