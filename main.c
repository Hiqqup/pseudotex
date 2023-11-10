#include "uthash.h"
#include <inttypes.h>
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
        }
        /* handle comments*/
        else if (fileContents[i] == '/' && fileContents[i + 1] == '/') {
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

    printf("%s\n", currentWord);
    printf("\\end{Verbatim}\n");
}

char* parseJsonColor(FILE* filePtr, char c)
{
    char* color = malloc(8 * sizeof(char));
    int colorPtr = 0;
    c = fgetc(filePtr);
    while (c != '"') {
        if (c == EOF) {
            perror("invalid json, didnt end string");
            exit(1);
        }
        color[colorPtr] = c;
        colorPtr++;
        c = fgetc(filePtr);
    }
    color[colorPtr] = '\0';
    c = fgetc(filePtr);
    return color;
}
struct keyword* parseJsonKeyword(FILE* filePtr, char c, char* color, struct keyword* keys)
{
    char word[MAX_INPUT];
    int wordPtr = 0;
    c = fgetc(filePtr);
    while (c != '"') {
        if (c == EOF) {
            perror("invalid json, didnt end string");
            exit(1);
        }
        word[wordPtr] = c;
        wordPtr++;
        c = fgetc(filePtr);
    }
    word[wordPtr] = '\0';
    c = fgetc(filePtr);

    /*overwrite key if it already exists*/
    struct keyword* tmp;
    HASH_FIND_STR(keys, word, tmp);
    if (tmp) {
        tmp->color = color;
    } else {
        /*add a new one*/
        keys = addKeyword(word, color, keys);
    }
    // printf("Added Key: %s to Color: %s\n", word, color);

    return keys;
}
struct keyword* parseJsonKeyfile(char* dir, struct keyword* keys)
{
    FILE* filePtr = fopen(dir, "r");

    if (filePtr == NULL) {
        perror("fopen() error");
        exit(1);
    }
    char c = fgetc(filePtr);
    int blockLevel = 0;
    char* currentColor;
    while (c != EOF) {
        if (c == '"' && blockLevel == 0) {
            currentColor = parseJsonColor(filePtr, c);
            // printf("current color set to: %s\n", currentColor);
        }
        if (c == '"' && blockLevel == 1) {
            keys = parseJsonKeyword(filePtr, c, currentColor, keys);
        }
        if (c == '{') {
            blockLevel++;
            if (blockLevel > 2) {
                perror("invalid json, deep to block\n");
            }
        }
        if (c == '}')
            blockLevel--;
        c = fgetc(filePtr);
    }
    if (blockLevel != 0) {
        perror("invalid json, didnt end block\n");
    }
    if (!feof(filePtr)) {
        perror("failed to parse keyfile\n");
    }
    fclose(filePtr);
    return keys;
}

struct keyword* keys = NULL;
char* fileName = NULL;
char* keyFileName = NULL;
bool appendKeys = false;
char* fileContents = NULL;
void parseArgs(int argc, char* argv[])
{
    /* parse input arguments for information and flags*/
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            if (fileName != NULL) {
                printf("Only one filename can be entered\n");
                exit(1);
            }
            fileName = argv[i];

        } else
            switch (argv[i][1]) {
            case 'k':
                /*keyfile flag*/
                if (argv[i][2] == 'a')
                    /*append to default flag*/
                    appendKeys = true;
                i++;
                if (i >= argc || argv[i][0] == '-') {
                    printf("Usage:\t -k <input file>\n");
                    exit(1);
                } else {
                    keyFileName = argv[i];
                }
                break;
            case 'm':
                /*parse string itead of input flag*/
                i++;
                fileContents = argv[i];
                break;
            case 'h':
                printf("Flags:\n");
                printf("\t-h\t for help\n");
                printf("\t-m\t to pass string instead of filepath\n");
                printf("\t-k\t to pass custom Json-keyfile\n");
                printf("\t-ka\t to pass custom Json-keyfile, and append and overwirte default\n");
                printf("\n");

                exit(0);
                break;
            }
    }
    if (fileName == NULL && fileContents == NULL) {
        printf("Usage:\t <input file> or use -m\n");
        exit(1);
    }
}

int main(int argc, char* argv[])
{
    parseArgs(argc, argv);
    /*set default values*/
    if (keyFileName == NULL || appendKeys) {
        keys = setDefaultKeywords();
    }
    /* parse keywords*/
    if (keyFileName != NULL) {
        keys = parseJsonKeyfile(keyFileName, keys);
    }

    struct keyword *tmp, *tmp2;

    /* parse keywords*/
    if (fileName != NULL) {
        fileContents = readFile(fileName /*put file path here*/);
    }
    if (fileContents != NULL) {
        parseForKeywords(keys, fileContents, tmp);
    }

    /* free the hash table contents */

    HASH_ITER(hh, keys, tmp, tmp2)
    {
        HASH_DEL(keys, tmp);
        free(tmp);
    }
    return 0;
}

// old part of maic how much space to allocaten functio
/*
    FILE* outputPtr = fopen("output", "w");

    if (outputPtr == NULL) {
        perror("fopen() error");
        exit(1);
    }
    fclose(outputPtr);
*/
