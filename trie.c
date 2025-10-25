#include <trie.h>
#include <stddef.h>
#include <stdlib.h>

void trie_insert(struct trie *t, char *s, int32_t value)
{
    if (t->children == NULL) {
        t->children = malloc(sizeof(struct trie) * 128);
    }

    while ((*s) != '\0') {
        if (t->children[*s] == NULL) {
            t->children[*s] = malloc(sizeof(struct trie));
            t->children[*s]->children = malloc(sizeof(struct trie) * 128);
        }
        
        t = t->children[*s];
        t->value = -1;
        s++;
    }

    t->value = value;
}

int32_t trie_get(struct trie *t, char *s)
{
    while ((*s) != '\0') {
        t = t->children[*s];
        if (t == NULL) return -1;
        s++;
    }
    
    return t->value;
}
