#ifndef TRIE_H
#define TRIE_H

#include <stdint.h>

struct trie {
    struct trie **children;
    int64_t value;
};

void trie_insert(struct trie *t, char *s, int32_t value);
int32_t trie_get(struct trie *t, char *s);

#endif
