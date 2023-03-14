/**
 * @file parser.c
 * @author Artyom Chernitsa
 * @brief List of ASCII codes: https://web.itu.edu.tr/~sgunduz/courses/mikroisl/ascii.html
 * @version 0.1
 * @date 2023-03-08
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "parser.h"
#include "stack.h"
#include "pair.h"

#define true 1
#define false 0

struct pair *
parse_line(char *rs)
{
    struct char_stack* stack = cs_init();
    struct char_stack* wstack = cs_init(); /* stack for single word */
    struct char_stack *wstack_r = NULL;
    size_t len = strlen(rs);

    char **args = NULL; int argc = 0;

    int single_quote = false; // opened single quote -> literal meaning
    int double_quote = false; // opened double quote -> with respect to '\'

    size_t idx = 0;
    for (; idx < len; ++idx)
    {
        /* Skip first consecutive useless whitespaces. */
        if (!single_quote && !double_quote && (rs[idx] == ' ' || rs[idx] == '\t'))
        {
            continue;
        }

        /* Comment out of enclosing quotes declines the rest of the line. */
        if (!single_quote && !double_quote && rs[idx] == '#')
        {
            break;
        }

        /* Pipe is a separate command. */
        if (!single_quote && !double_quote && rs[idx] == '|')
        {
            /* || case command */
            if (idx + 1 < len && rs[idx + 1] == '|')
            {
                cs_push(wstack, rs[idx]);
                idx++;
            }
            cs_push(wstack, rs[idx]);
            cs_push(wstack, '\0'); /* put end of line */
            argc++; /* increase number of recognized tokens */
            args = realloc(args, sizeof(char*) * argc); /* expand memory */
            if (args == NULL) {
                fprintf(stderr, "Error during realloc(): %s", strerror(errno));
                exit(-1);
            }
            wstack_r = cs_reverse(wstack);
            args[argc - 1] = cs_splice(wstack_r); /* save token */
            cs_free(wstack); /* clear stack */
            cs_free(wstack_r);
            free(wstack_r);
            wstack_r = NULL;
            continue;
        }

        /* Redirection is a separate command. */
        if (!single_quote && !double_quote && rs[idx] == '>')
        {
            /* >> case command */
            if (idx + 1 < len && rs[idx + 1] == '>')
            {
                cs_push(wstack, rs[idx]);
                idx++;
            }
            cs_push(wstack, rs[idx]);
            cs_push(wstack, '\0'); /* put end of line */
            argc++; /* increase number of recognized tokens */
            args = realloc(args, sizeof(char*) * argc); /* expand memory */
            if (args == NULL) {
                fprintf(stderr, "Error during realloc(): %s", strerror(errno));
                exit(-1);
            }
            wstack_r = cs_reverse(wstack);
            args[argc - 1] = cs_splice(wstack_r); /* save token */
            cs_free(wstack); /* clear stack */
            cs_free(wstack_r);
            free(wstack_r);
            wstack_r = NULL;
            continue;
        }

        /* Background is a separate command. */
        if (!single_quote && !double_quote && rs[idx] == '&')
        {
            /* && case command */
            if (idx + 1 < len && rs[idx + 1] == '&')
            {
                cs_push(wstack, rs[idx]);
                idx++;
            }
            cs_push(wstack, rs[idx]);
            cs_push(wstack, '\0'); /* put end of line */
            argc++; /* increase number of recognized tokens */
            args = realloc(args, sizeof(char*) * argc); /* expand memory */
            if (args == NULL) {
                fprintf(stderr, "Error during realloc(): %s", strerror(errno));
                exit(-1);
            }
            wstack_r = cs_reverse(wstack);
            args[argc - 1] = cs_splice(wstack_r); /* save token */
            cs_free(wstack); /* clear stack */
            cs_free(wstack_r);
            free(wstack_r);
            wstack_r = NULL;
            continue;
        }

        /* Found enclosing single quote. Set up flag and go next. */
        if (!single_quote && !double_quote && rs[idx] == '\'')
        {
            single_quote = true;
            continue;
        }

        /* With single quote flag enabled save all enclosed characters. */
        if (single_quote && !double_quote && rs[idx] != '\'')
        {
            cs_push(wstack, rs[idx]);
            continue;
        }

        /* Found second enclosing single quote. Save result as a token. */
        if (single_quote && !double_quote && rs[idx] == '\'')
        {
            single_quote = false;
            cs_push(wstack, '\0'); /* put end of line */
            argc++; /* increase number of recognized tokens */
            args = realloc(args, sizeof(char*) * argc); /* expand memory */
            if (args == NULL) {
                fprintf(stderr, "Error during realloc(): %s", strerror(errno));
                exit(-1);
            }
            wstack_r = cs_reverse(wstack);
            args[argc - 1] = cs_splice(wstack_r); /* save token */
            cs_free(wstack); /* clear stack */
            cs_free(wstack_r);
            free(wstack_r);
            wstack_r = NULL;
            continue;
        }

        /* Found enclosing double quote. Set up flag and go next.
         * Pay respect to \ character. */
        if (!single_quote && !double_quote && rs[idx] == '\"')
        {
            double_quote = true;
            continue;
        }

        /* Found second enclosing double quote. Save result as a token. */
        if (!single_quote && double_quote && rs[idx] == '\"' && cs_isempty(stack))
        {
            double_quote = false;
            cs_push(wstack, '\0'); /* put end of line */
            argc++; /* increase number of recognized tokens */
            args = realloc(args, sizeof(char*) * argc); /* expand memory */
            if (args == NULL) {
                fprintf(stderr, "Error during realloc(): %s", strerror(errno));
                exit(-1);
            }
            wstack_r = cs_reverse(wstack);
            args[argc - 1] = cs_splice(wstack_r); /* save token */
            cs_free(wstack); /* clear stack */
            cs_free(wstack_r);
            free(wstack_r);
            wstack_r = NULL;
            continue;
        }

        /* Found escape character among enclosing double quote. No escapes put on stack. */
        if (!single_quote && double_quote && rs[idx] == '\\' && cs_isempty(stack))
        {
            cs_push(stack, '\\');
            continue;
        }

        /* Found common character with empty stack. Put character to word stack. */
        if (!single_quote && double_quote && rs[idx] != '\\' && cs_isempty(stack))
        {
            cs_push(wstack, rs[idx]);
            continue;
        }

        /* Found common character among enclosing double quotes with escape character on stack. */
        if (!single_quote && double_quote && !cs_isempty(stack))
        {
            if (rs[idx] == '\n' && cs_peek(stack) == '\\')
            {
                /**
                 * with enclosing ": '\' + '\n' => ''
                 * $> echo "123
                 *      456\
                 *      678"
                 * 123
                 * 456678
                 */
                cs_pop(stack);
            }
            else if (rs[idx] == '\\' && cs_peek(stack) == '\\') 
            {
                cs_pop(stack);
                cs_push(wstack, '\\');
            }
            else if (rs[idx] == '\"' && cs_peek(stack) == '\\')
            {
                cs_pop(stack);
                cs_push(wstack, '\"');
            }
            else if (rs[idx] == 'n' && cs_peek(stack) == '\\')
            {
                cs_pop(stack);
                cs_push(wstack, '\n');
            }
            else if (rs[idx] == 't' && cs_peek(stack) == '\\')
            {
                cs_pop(stack);
                cs_push(wstack, '\t');
            }
            else if (rs[idx] == 'r' && cs_peek(stack) == '\\')
            {
                cs_pop(stack);
                cs_push(wstack, '\r');
            }
            else {
                /**
                 * $> echo "123\ 4"
                 * 123\ 4
                 */
                cs_push(wstack, cs_peek(stack));
                cs_pop(stack);
                cs_push(wstack, rs[idx]);
            }
            continue;
        }

        /**
         * Non-quoted, pay respect to \ that preserves literal value of next character.
         * Keep it single token until space or tab is founded.
         * Omit any double or single quotes, '\n' '\t' etc.
         * 
         * $> touch 123\ '567'"456"
         * $> ls
         * 123 567456 # one file, not two!!!
         */
        if (!single_quote && !double_quote)
        {
            for (;idx < len && rs[idx] != ' ' && rs[idx] != '\t';)
            {
                if (rs[idx] == '\\')
                {
                    idx++;
                    if (idx < len && rs[idx] == '\\')
                    {
                        cs_push(wstack, '\\');
                        idx++;
                    }
                    else if (idx < len && rs[idx] == '\n')
                    {
                        idx++;
                    }
                    else if (idx < len && rs[idx] == '|')
                    {
                        /**
                         * $> echo 123\
                         * 456\
                         * | grep 2
                         * 123456 | grep 2
                         */
                        if (!cs_isempty(wstack))
                        {
                            idx--;
                            break;
                        }
                        idx++;
                        if (idx < len && rs[idx] == '|')
                        {
                            cs_push(wstack, rs[idx]);
                        }
                        else 
                        {
                            idx--;
                        }

                        cs_push(wstack, rs[idx]);
                        break;
                    }
                    else if (idx < len && rs[idx] == '>')
                    {
                        if (!cs_isempty(wstack)) 
                        {
                            idx--;
                            break;
                        }
                        idx++;
                        if (idx < len && rs[idx] == '>')
                        {
                            cs_push(wstack, rs[idx]);
                        }
                        else 
                        {
                            idx--;
                        }

                        cs_push(wstack, rs[idx]);
                        break;
                    }
                    else if (idx < len && rs[idx] == '&')
                    {
                        if (!cs_isempty(wstack)) 
                        {
                            idx--;
                            break;
                        }
                        idx++;
                        if (idx < len && rs[idx] == '&')
                        {
                            cs_push(wstack, rs[idx]);
                        }
                        else 
                        {
                            idx--;
                        }

                        cs_push(wstack, rs[idx]);
                        break;
                    }
                    else if (idx < len)
                    {
                        cs_push(wstack, rs[idx]);
                        idx++;
                    }
                }
                /** Ignore case. Cause I don't understand parsing rule for this shit.
                 * $> echo \\n
                 * >
                 * > # print just '\n'
                 */
                else if (rs[idx] == '\"' || rs[idx] == '\'')
                {
                    idx++;
                }
                else if (rs[idx] == '|')
                {
                    /**
                     * $> echo 100|grep 100
                     * 100
                     */
                    if (!cs_isempty(wstack)) 
                    {
                        idx--;
                        break;
                    }
                    idx++;
                    if (idx < len && rs[idx] == '|')
                    {
                        cs_push(wstack, rs[idx]);
                    }

                    cs_push(wstack, rs[idx]);
                    break;
                }
                else if (rs[idx] == '&')
                {
                    /**
                     * $> echo 100&grep 100
                     * 100
                     */
                    if (!cs_isempty(wstack)) 
                    {
                        idx--;
                        break;
                    }
                    idx++;
                    if (idx < len && rs[idx] == '&')
                    {
                        cs_push(wstack, rs[idx]);
                    }

                    cs_push(wstack, rs[idx]);
                    break;
                }
                else if (rs[idx] == '>')
                {
                    /**
                     * $> echo 100>text.txt
                     * 
                     */
                    if (!cs_isempty(wstack)) 
                    {
                        idx--;
                        break;
                    }
                    idx++;
                    if (idx < len && rs[idx] == '>')
                    {
                        cs_push(wstack, rs[idx]);
                    }

                    cs_push(wstack, rs[idx]);
                    break;
                }
                else 
                {
                    cs_push(wstack, rs[idx]);
                    idx++;
                }
            }
            cs_push(wstack, '\0'); /* put end of line */
            argc++; /* increase number of recognized tokens */
            args = realloc(args, sizeof(char*) * argc); /* expand memory */
            if (args == NULL) {
                fprintf(stderr, "Error during realloc(): %s", strerror(errno));
                exit(-1);
            }
            wstack_r = cs_reverse(wstack);
            args[argc - 1] = cs_splice(wstack_r); /* save token */
            cs_free(wstack); /* clear stack */
            cs_free(wstack_r);
            free(wstack_r);
            wstack_r = NULL;
            continue;
        }
    }

    cs_free(stack);
    cs_free(wstack);
    free(stack);
    free(wstack);
    if (wstack_r != NULL) free(wstack_r);
    
    int pass = argc;
    struct pair* p = make_pair(args, &pass);
    
    return p;
}

int 
is_eof(char next)
{ 
    if (next == EOF) return 1;
    if (next == '\x04') return 1; // CTRL-D
   
    return 0;
}

int 
is_eol(char next, char prev)
{
    if (next == '\n' && prev != '\\') return 1;

    return 0;
}

char *
read_line(int *eof_flag)
{
    int index = 0;
    int size = 128;
    char *raw_line = calloc(size, sizeof(char));
    char prev = 's', next = 's';

    int double_quote = false;
    for (;;)
    {
        next = getc(stdin);
        if (double_quote && next == '\"')
        {
            double_quote = false;
        }
        else if (!double_quote && next == '\"') 
        {
            double_quote = true;
        }
        else if (!double_quote && prev == '\\' && next == '\n')
        {
            /**
             * $> echo 123\
             * 456
             * 123456
             */
            prev = 's';
            continue;
        }
        else if (is_eof(next))
        {
            *eof_flag = 1;
            break;
        }
        else if (is_eol(next, prev) && !double_quote) break;

        // -1 just in case :-)
        if (index - 1 <= size) {
            size += 2;
            raw_line = realloc(raw_line, sizeof(char) * size);
            if (raw_line == NULL) {
                printf("Error: %s!\n", strerror(errno));
                exit(1);
            }
        }
        raw_line[index++] = next;
        prev = next;
    }
    raw_line[index++] = '\0';

    return raw_line;
}