#pragma once

struct pair;

struct pair *make_pair(void *first, void *second);

void *fst_pair(struct pair* p);

void *snd_pair(struct pair* p);