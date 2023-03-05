#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "parser.h"

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
read_line()
{
    // TODO
    /**
     * Идём по строке, далее:
     * 0. Итерируемся по строке -> шаг 1
     * 1. Видим \, идём на шаг 2, иначе на шаг 4
     * 2. Кладём \ в стек, идём на шаг 3
     * 3. Далее видим escape символы, кроме `n`, напр. " или ' -> 
     *    вытаскиваем из стека \ и добавляем к строке \" или \' и т.д.
     *    если видим `n`, просто убираем из стека \ -> на шаг 0
     * 4. Видим " или ' -> на шаг 5, иначе на шаг 6
     * 5. Если стек пустой, кладём туда кавычку и дальше идём по строке -> на шаг 0
     *    Если стек содержит парную кавычку, то удаляем первую из стека -> на шаг 0
     *    Если лежит \, то убираем его из стека и к строке добавляем escape character -> на шаг 0
     * 6. Видим всё остальное, значит просто добавляем к строке -> на шаг 0
     */
    int index = 0;
    int size = 128;
    char *raw_line = calloc(size, sizeof(char));
    char prev = 's', next = 's';
    for (;;)
    {
        next = getc(stdin);
        if (is_eof(next)) break;
        if (is_eol(next, prev)) break;

        // if (prev == '\\' && next == '"')
        // {
        //     raw_line[--index] = '\"';
        // }
        // if (prev == '\\' && next == '\'')
        // {
        //     raw_line[--index] == '\'';
        // }
        // if (prev == '')

        if (index == size) {
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

    // TODO
    return raw_line;
}