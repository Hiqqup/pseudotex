#include "uthash.h"
#include <limits.h>
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
char* fullInputDir(char* filename)
{
    char currentDir[PATH_MAX];
    if (getcwd(currentDir, sizeof(currentDir)) == NULL) {
        perror("getcwd() error");
        exit(1);
    }
    char* dirPtr = malloc((strlen(currentDir) + strlen(filename) + 1) * sizeof(char));
    for (int i = 0; i < strlen(currentDir); i++) {
        dirPtr[i] = currentDir[i];
    }
    dirPtr[strlen(currentDir)] = '/';
    for (int i = 0; i <= strlen(filename); i++) {
        dirPtr[strlen(currentDir) + i + 1] = filename[i];
    }
    return dirPtr;
}
int main(int argc, char* argv[])
{
    struct keyword* tmp;
    struct keyword* keys = setDefaultKeywords();

    printf("cwd: %s\n", fullInputDir("input"));
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
