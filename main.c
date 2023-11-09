#include "uthash.h"
#include <limits.h>
#include <stdbool.h>
#include <stdio.h> /* printf */
#include <stdlib.h> /* malloc */
#include <string.h> /* strcpy */
#include <unistd.h>

struct keyword {
    const char* key; /* key */
    char* color;
    UT_hash_handle hh; /* makes this structure hashable */
};

int maxKeywordLength = 0;
struct keyword* addKeyword(char* key, char* color, struct keyword* keys)
{
    if (strlen(key) > maxKeywordLength)
        maxKeywordLength = strlen(key);
    struct keyword* s = malloc(sizeof *s);
    s->key = key;
    s->color = color;
    HASH_ADD_KEYPTR(hh, keys, key, strlen(key), s);
    return keys;
}
struct keyword* setDefaultKeywords()
{
    struct keyword* keys = NULL;
    keys = addKeyword("fuction", "magenta", keys);
    keys = addKeyword("for", "magenta", keys);
    keys = addKeyword("do", "magenta", keys);
    keys = addKeyword("if", "magenta", keys);
    keys = addKeyword("then", "magenta", keys);
    keys = addKeyword("end", "magenta", keys);
    keys = addKeyword("return", "magenta", keys);
    keys = addKeyword("int", "cyan", keys);
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
void parseForKeywords(struct keyword* keys, char* fileContents, struct keyword* tmp)
{

    printf("\\begin{Verbatim}[commandchars=\\\\\\{\\}]\n");

    char currentWord[maxKeywordLength];
    int currentWordIndex = 0;
    for (int i = 0; i < strlen(fileContents); i++) {
        if (
            fileContents[i] >= 65 && fileContents[i] <= 90 || //
            fileContents[i] >= 97 && fileContents[i] <= 122) {
            currentWord[currentWordIndex] = fileContents[i];
            currentWordIndex++;
        } else if (fileContents[i] == '/' && fileContents[i + 1] == '/') {
            printf("\\textcolor{gray}{");
            while (fileContents[i] != '\n') {
                printf("%c", fileContents[i]);
                i++;
            }
            printf("}\n");
            i++;

        } else {
            currentWord[currentWordIndex] = '\0';
            currentWordIndex = 0;
            HASH_FIND_STR(keys, currentWord, tmp);
            if (tmp) {
                printf("\\textcolor{%s}{%s}", tmp->color, currentWord);
            } else {
                printf("%s", currentWord);
            }
            printf("%c", fileContents[i]);
        }
    }
    printf("\\end{Verbatim}\n");
}
int main(int argc, char* argv[])
{
    struct keyword* tmp;
    struct keyword* keys = setDefaultKeywords();

    FILE* outputPtr = fopen("output", "w");

    if (outputPtr == NULL) {
        perror("fopen() error");
        exit(1);
    }
    fclose(outputPtr);
    char* fileContents = readFile("input");
    parseForKeywords(keys, fileContents, tmp);

    /* free the hash table contents */
    struct keyword* tmp2;
    HASH_ITER(hh, keys, tmp, tmp2)
    {
        HASH_DEL(keys, tmp);
        free(tmp);
    }
    return 0;
}
