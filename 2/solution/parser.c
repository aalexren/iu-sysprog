#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "parser.h"
#include "stack.h"

#define true 1
#define false 0

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
parse_line(char *rs)
{
    struct char_stack* stack = cs_init();
    size_t rs_len = strlen(rs);
    char *s = (char *)malloc(sizeof(char) * rs_len);
    size_t s_idx = 0;

    int single_quote = false; // opened single quote -> literal meaning
    int double_quote = false; // opened double quote -> with respect to '\'

    for (size_t idx = 0; idx < rs_len; ++idx)
    {
        /* Found enclosing single quote. Wait for the second one. */
        if (!single_quote && !double_quote && rs[idx] == '\'')
        {
            single_quote = true;
            continue;
        }
        if (single_quote && !double_quote && rs[idx] != '\'')
        {
            s[s_idx++] = rs[idx];
            continue;
        }
        if (single_quote && !double_quote && rs[idx] == '\'')
        {
            single_quote = false;
            continue;
        }
        /* Found enclosing double quote. Wait for the second one. 
         * Pay respect to \ character. */
        if (!single_quote && !double_quote && rs[idx] == '\"')
        {
            double_quote = true;
            continue;
        }
        if (!single_quote && double_quote && rs[idx] == '\"' && cs_isempty(stack))
        {
            double_quote = false;
            continue;
        }
        if (!single_quote && double_quote && rs[idx] == '\\' && cs_isempty(stack))
        {
            cs_push(stack, '\\');   
            continue;
        }
        if (!single_quote && double_quote && rs[idx] != '\\' && cs_isempty(stack))
        {
            s[s_idx++] = rs[idx];
            continue;
        }
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
                s[s_idx++] = '\\';
            }
            else if (rs[idx] == '\"' && cs_peek(stack) == '\\')
            {
                cs_pop(stack);
                s[s_idx++] = '\"';
            }
            else if (rs[idx] == 'n' && cs_peek(stack) == '\\')
            {
                cs_pop(stack);
                s[s_idx++] = '\n';
            }
            else if (rs[idx] == 't' && cs_peek(stack) == '\\')
            {
                cs_pop(stack);
                s[s_idx++] = '\t';
            }
            else if (rs[idx] == 'r' && cs_peek(stack) == '\\')
            {
                cs_pop(stack);
                s[s_idx++] = '\r';
            }
            else {
                s[s_idx++] = cs_peek(stack);
                cs_pop(stack);
                s[s_idx++] = rs[idx];
            }
            continue;
        }
        /* Non-quoted \ preserve literal value of next character. */
        if (!single_quote && !double_quote && rs[idx] == '\\')
        {
            cs_push(stack, rs[idx]);
            continue;
        }
        if (!single_quote && !double_quote && rs[idx] == '\n')
        {
            if (!cs_isempty(stack) && cs_peek(stack) == '\\')
            {
                cs_pop(stack);
            }
            continue;
        }
        /* Non-quoted, without \ character. */
        if (!single_quote && !double_quote && rs[idx] != '\\')
        {
            s[s_idx++] = rs[idx];
            continue;
        }
    }
    s[s_idx] = '\0';

    free(stack);
    return s;
}

char *
read_line()
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
        else if (is_eof(next)) break;
        else if (is_eol(next, prev) && !double_quote) break;

        // -1 just in case :-)
        if (index - 1 == size) {
            size *= 2;
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