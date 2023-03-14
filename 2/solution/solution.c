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
    /* Make pipes */
    int **pipes = NULL;
    int len = 0; int pipe_idx = 0;
    for (int i = 0; i < count; ++i)
    {
        if (cmd_get_special(comms[i]) == 0)
        {
            len++;
            pipes = realloc(pipes, sizeof(int*) * len);
            if (pipes == NULL)
            {
                fprintf(stderr, "Error during fork() call: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            pipes[len-1] = malloc(sizeof(int) * 2);
            if (pipe(pipes[len-1]) == -1)
            {
                fprintf(stderr, "Error during fork() call: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
    }

    /* Save all pids to wait for them later */
    pid_t *pids = malloc(sizeof(*pids) * len);

    for (int i = 0; i < count; ++i)
    {
        if (cmd_get_special(comms[i]) == 0)
        {
            continue;
        }

        if (strcmp(cmd_get_name(comms[i]), "exit") == 0 && count == 1)
        {
            return 1;
        }
        /* cd command handling */
        else if (strcmp(cmd_get_name(comms[i]), "cd") == 0)
        {
            if (chdir(cmd_get_argv(comms[i])[0]) != 0)
            {
                fprintf(stderr, "cd: no such file or directory: %s\n", cmd_get_argv(comms[i])[0]);
            }
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
                /* FILE DESCRIPTORS KEEPED GLOABLLY BY OS */
                if (pipe_idx > 0)
                {
                    /* в предудщий писать не надо, закрываем */
                    close(pipes[pipe_idx-1][1]);
                    /**
                     * перенаправляем читать stdin из предыдущего fd;
                     * создаём новый дескриптор, который ассоциирован с newfd;
                     * 
                     * читаем из предыдущего;
                     */
                    dup2(pipes[pipe_idx-1][0], STDIN_FILENO);
                    /* закрываем старый дескриптор предыдущего на чтение */
                    close(pipes[pipe_idx-1][0]);
                }
                if (pipe_idx + 1 < len)
                {
                    /* из текущего нечего читать ещё, закрываем */
                    close(pipes[pipe_idx][0]);
                    /**
                     * перенаправляем выводить stdout из текущего fd;
                     * создаём новый дескриптор, который ассоциирован с newfd;
                     * 
                     * выводим в текущий;
                     */
                    dup2(pipes[pipe_idx][1], STDOUT_FILENO);
                    /* закрыаем старый дескриптор текущего pipe на запись */
                    close(pipes[pipe_idx][1]);
                }

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
                pipe_idx++;
                pids[pipe_idx-1] = child_pid;
                // printf("Child process finished with status code %d\n", WEXITSTATUS(status));
            }
            
            // if (name_args != NULL)
            // {
            //     for (int j = 0; j < argc + 1; ++j)
            //     {
            //         free(name_args[j]);
            //     }
            //     free(name_args);
            // }
        }
    }

    /* Close all pipes */
    for (int i = 0; i < len; ++i)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for (int i = 0; i < len; ++i)
    {
        int status;
        waitpid(pids[i], &status, 0);
    }

    /* Free allocated memory */
    for (int i = 0; i < len; ++i)
    {
        free(pipes[i]);
    }
    free(pipes);
    free(pids);
    
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

    shell_loop();

    printf("Number of not freed allocations: %llu\n", heaph_get_alloc_count());
    return 0;
}