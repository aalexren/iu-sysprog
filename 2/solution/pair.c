#include <stdlib.h>
#include "pair.h"

struct pair {
    void *first;
    void *second;
};

struct pair*
make_pair(void *first, void *second)
{
    struct pair* p = malloc(sizeof(struct pair));
    p->first = first;
    p->second = second;

    return p;
}

void *
fst_pair(struct pair* p)
{
    if (p != NULL) return p->first;

    return NULL;
}

void *
snd_pair(struct pair* p)
{
    if (p != NULL) return p->second;
    
    return NULL;
}