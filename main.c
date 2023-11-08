#include "uthash.h"
#include <stdio.h> /* printf */
#include <stdlib.h> /* malloc */
#include <string.h> /* strcpy */
#include <unistd.h>
struct keyword {
    const char* key; /* key */
    char* color;
    UT_hash_handle hh; /* makes this structure hashable */
};

struct keyword* addKeyword(char* key, char* color, struct keyword* keys)
{
    struct keyword* s = malloc(sizeof *s);
    s->key = key;
    s->color = color;
    HASH_ADD_KEYPTR(hh, keys, key, strlen(key), s);
    return keys;
}
struct keyword* setDefaultKeywords()
{
    struct keyword* keys = NULL;
    keys = addKeyword("for", "magenta", keys);
    keys = addKeyword("do", "magenta", keys);
    keys = addKeyword("if", "magenta", keys);
    keys = addKeyword("then", "magenta", keys);
    keys = addKeyword("end", "magenta", keys);
    keys = addKeyword("return", "magenta", keys);
    return keys;
}
int main(int argc, char* argv[])
{
    struct keyword* tmp;
    struct keyword* keys = setDefaultKeywords();

    char* getcwd(char* buf, size_t size);

    /* from the example */
    HASH_FIND_STR(keys, "for", tmp);
    if (tmp)
        printf("carpeted %s\n", tmp->color);
    /* end */

    /* free the hash table contents */
    struct keyword* tmp2;
    HASH_ITER(hh, keys, tmp, tmp2)
    {
        HASH_DEL(keys, tmp);
        free(tmp);
    }
    return 0;
}
