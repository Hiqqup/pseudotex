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
char* readFile(char* dir)
{
    FILE* filePtr = fopen(dir, "r");

    if (filePtr == NULL) {
        perror("fopen() error");
        exit(1);
    }

    fseek(filePtr, 0, SEEK_END);
    long numChars = ftell(filePtr);
    fseek(filePtr, 0, SEEK_SET);

    char* charArray = malloc(numChars * sizeof(char));
    if (charArray == NULL) {
        perror("Memory allocation failed.\n");
        exit(1);
    }
    fread(charArray, sizeof(char), numChars, filePtr);

    fclose(filePtr);
    return charArray;
}
int main(int argc, char* argv[])
{
    struct keyword* tmp;
    struct keyword* keys = setDefaultKeywords();

    char* fileContents = readFile("input");
    printf("the file: %s\n", fileContents);
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
