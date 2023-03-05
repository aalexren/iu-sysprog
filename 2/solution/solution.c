#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "command.h"
#include "stack.h"

char const str[245] = "keks";

int main(int argc, char **argv)
{
    struct char_stack *stack = cs_init();
    cs_push(stack, 's');
    cs_push(stack, 't');
    cs_push(stack, 'a');
    cs_push(stack, 'c');
    cs_push(stack, 'k');
    cs_print(stack);
    cs_pop(stack);
    cs_pop(stack);
    cs_pop(stack);
    cs_pop(stack);
    cs_pop(stack);
    free(stack);

    return 0;
    char *s = read_line();
    printf("%s\n", s);
    
    
    // parse_line(s);
    
    
    pid_t child = fork();

    if (child == 0) {
        /* child proccess */
        char* args[] = {"ls", s, NULL};
        int status_code = execvp("ls", args);

        if (status_code == -1) {
            printf("Oh, no, something goes wrong...%s\n", strerror(errno));
        }
    }

    // printf("PID: %d\n", child);
    free(s);
    return 0;

    return 0;
}