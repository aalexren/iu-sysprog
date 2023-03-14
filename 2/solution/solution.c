#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>

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

/**
 * @brief We make pipe for comamnds and close extra
 * descriptors, since even we have a copy of descriptors 
 * in any moment, right before we call execvp we will have
 * decreased fd, and then execvp exited it will decrease number
 * of references to descriptors, and the rest will closed by
 * parent process.
 * 
 * E.g.:
 * pipe                 -> fd[0]: 1 refs, fd[1]: 1 refs
 * fork                 -> fd[0]: 2 refs, fd[1]: 2 refs
 * child, close(fd[0])  -> fd[0]: 1 refs, fd[1]: 2 refs
 * child, execvp        -> fd[0]: 0 refs, fd[1]: 1 refs
 * parent, close(fd[1]) -> fd[0]: 0 refs, fd[1]: 0 refs
 * 
 * @param comms 
 * @param count 
 * @return int code of execution
 */
int
exec_cmds(struct cmd **comms, int count)
{
    /* Make pipes, 0 - read, 1 - write */
    int fd[2]; int fd_p = -1;

    /* Save all pids to wait for them later */
    pid_t *pids = NULL; int len = 0;

    for (int i = 0; i < count; ++i)
    {
        if (cmd_get_special(comms[i]) != 0)
        {
            continue;
        }

        if (strcmp(cmd_get_name(comms[i]), "exit") == 0 && count == 1)
        {
            return 1;
        }

        /* cd command handling */
        if (strcmp(cmd_get_name(comms[i]), "cd") == 0)
        {
            if (chdir(cmd_get_argv(comms[i])[0]) != 0)
            {
                fprintf(stderr, "cd: no such file or directory: %s\n", cmd_get_argv(comms[i])[0]);
            }
            continue;
        }

        /* execute command using pipe */
        if (pipe(fd) < 0) {
            fprintf(stderr, "Error during pipe() call: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

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

            if (i > 0 && i + 1 < count)
            {
                /* not first, not last command */
                close(fd[0]);
                dup2(fd_p, STDIN_FILENO);
                
                /* > and >> case */
                if (i + 1 < count && strcmp(cmd_get_name(comms[i + 1]), ">") == 0)
                {
                    char *fname = cmd_get_argv(comms[i + 1])[0];
                    int file = open(fname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    dup2(file, STDOUT_FILENO);
                }
                else if (i + 1 < count && strcmp(cmd_get_name(comms[i + 1]), ">>") == 0)
                {
                    char *fname = cmd_get_argv(comms[i + 1])[0];
                    int file = open(fname, O_WRONLY | O_CREAT | O_APPEND, 0644);
                    dup2(file, STDOUT_FILENO);
                }
                else
                {
                    dup2(fd[1], STDOUT_FILENO);
                }
            }
            else if (i > 0)
            {
                /* last command */
                close(fd[0]);
                dup2(fd_p, STDIN_FILENO);
            }
            else if (i == 0 && count > 1)
            {
                /* first command and not last one */
                close(fd[0]);
                
                /* > and >> case */
                if (i + 1 < count && strcmp(cmd_get_name(comms[i + 1]), ">") == 0)
                {
                    char *fname = cmd_get_argv(comms[i + 1])[0];
                    int file = open(fname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    dup2(file, STDOUT_FILENO);
                }
                else if (i + 1 < count && strcmp(cmd_get_name(comms[i + 1]), ">>") == 0)
                {
                    char *fname = cmd_get_argv(comms[i + 1])[0];
                    int file = open(fname, O_WRONLY | O_CREAT | O_APPEND, 0644);
                    dup2(file, STDOUT_FILENO);
                }
                else 
                {
                    dup2(fd[1], STDOUT_FILENO);
                }
            }
            else
            {
                /* single command */
                close(fd[0]);
            }
            
            execvp(name, name_args);

            fprintf(stderr, "Error during execvp() call: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        else 
        {
            len++;
            pids = realloc(pids, sizeof(pid_t) * len);
            if (pids == NULL)
            {
                fprintf(stderr, "Error during realloc(): %s", strerror(errno));
                exit(EXIT_FAILURE);
            }
            pids[len-1] = child_pid;

            if (i > 0 && i + 1 < count)
            {
                close(fd_p);
                close(fd[1]);
                fd_p = fd[0];
            }
            else if (i > 0)
            {
                close(fd[1]);
                close(fd_p);
            }
            else if (i == 0 && count > 1)
            {
                fd_p = fd[0];
                close(fd[1]);
            }
            else
            {
                close(fd[1]);
            }
            
            // printf("Child process finished with status code %d\n", WEXITSTATUS(status));
        }
    }

    for (int i = 0; i < len; ++i)
    {
        int status;
        /**
         * HINT: even more, we shoouldn't save pid_t of childs.
         * Hence pids array are extra, we can remove it, and replace
         * for calling `waitpid(-1, &status, 0)`.
         * 
         * Especially make sense when we have looped output:
         * $> yes bigdata | head -n 100000 | wc -l
         */
        waitpid(pids[i], &status, 0);
    }

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
        if (status != 0) return;

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
    }
}


int main(int argc, char **argv)
{

    heaph_init();

    shell_loop();

    printf("Number of not freed allocations: %llu\n", heaph_get_alloc_count());
    return 0;
}