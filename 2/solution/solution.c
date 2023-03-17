#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "heap_help.h"
#include "parser.h"
#include "command.h"
#include "stack.h"
#include "pair.h"


#define true 1
#define false 0

/**
 * @brief Since execvp needs to have command name as a first
 * argument of array of arguments it creates new temporary
 * array and add name at first position to it.
 * 
 * Should be freed!
 */
char **
add_name_to_argv(char *name, char **argv, int argc)
{
    /**
     * Allocate memory for name of command and terminated NULL pointer.
     * NULL pointer MUST be only one in the array and only the last one!
     */
    char **temp = malloc(sizeof(char *) * (argc + 2));
    temp[0] = strdup(name);

    for (int i = 0; i < argc; ++i)
    {
        temp[i+1] = strdup(argv[i]);
    }
    temp[argc+2-1] = NULL;
    
    return temp;
}

int
exec_cmds(struct cmd **comms, int count)
{
    for (int i = 0; i < count; ++i)
    {
        /* cd command handling */
        if (strcmp(cmd_get_name(comms[i]), "cd") == 0)
        {
            if (chdir(cmd_get_argv(comms[i])[0]) != 0)
            {
                fprintf(stderr, "cd: no such file or directory: %s\n", cmd_get_argv(comms[i])[0]);
            }
        }
        else if (strcmp(cmd_get_name(comms[i]), "exit") == 0)
        {
            return 1;
        }
        else 
        {
            pid_t child_pid = fork();
            int argc = 0;
            char *name = NULL, **args = NULL, **name_args = NULL;

            if (child_pid < 0)
            {
                fprintf(stderr, "Error during fork() call: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            else if (child_pid == 0)
            {
                name = cmd_get_name(comms[i]);
                args = cmd_get_argv(comms[i]);
                argc = cmd_get_argc(comms[i]);
                name_args = add_name_to_argv(name, args, argc);
                
                execvp(name, name_args);

                fprintf(stderr, "Error during execvp() call: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            else 
            {
                int status;
                waitpid(child_pid, &status, 0);
                // printf("Child process finished with status code %d\n", WEXITSTATUS(status));
                
            }
            
            if (name_args != NULL)
            {
                for (int j = 0; j < argc + 1; ++j)
                {
                    free(name_args[j]);
                }
                free(name_args);
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

        int status = exec_cmds(commands_array, commands_count);

        /* Free allocated memory to avoid memory leak. */
        for (int i = 0; i < commands_count; ++i)
        {
            cmd_free(commands_array[i]);
        }
        free(commands_array);
        free(commands);
        free(raw_input);
        for (int i = 0; i < tokens_len; ++i)
        {
            free(tokens_args[i]);
        }
        free(tokens_args);
        free(tokens);

        // printf("Number of not freed allocations: %llu\n", heaph_get_alloc_count());

        if (status != 0) return;
    }
}


int main(int argc, char **argv)
{

    heaph_init();

    // struct char_stack *s = cs_init();
    // struct char_stack *sr = NULL;
    // // for (int i = 0; i < 10; ++i)
    // // {
    // //     cs_push(s,'a');cs_push(s,'b');cs_push(s,'c');
    // //     sr = cs_reverse(s);

    // //     char *ss = cs_splice(sr);
    // //     free(ss);

    // //     cs_free(sr);
    // //     cs_free(s);
    // //     free(sr);
    // //     sr = NULL;
    // // }
    // // free(s);
    // // printf("%p", sr);
    // if (sr != NULL) free(sr);
    shell_loop();

    printf("Number of not freed allocations: %llu\n", heaph_get_alloc_count());
    return 0;
}