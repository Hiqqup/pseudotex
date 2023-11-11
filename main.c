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
    keys = addKeyword("function", "magenta", keys);
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

char* parseJsonColor(char* contents, int* i)
{
    char* color = malloc(8 * sizeof(char));
    int colorPtr = 0;
    *i += 1;
    while (contents[*i] != '"') {
        if (*i > strlen(contents)) {
            perror("invalid json, didnt end string");
            exit(1);
        }
        color[colorPtr] = contents[*i];
        colorPtr++;
        *i += 1;
    }
    color[colorPtr] = '\0';
    *i += 1;
    return color;
}
struct keyword* parseJsonKeyword(char* contents, int* i, char* color, struct keyword* keys)
{

    char word[MAX_INPUT];
    int wordPtr = 0;
    *i += 1;
    while (contents[*i] != '"') {
        if (*i >= strlen(contents)) {
            perror("invalid json, didnt end string");
            exit(1);
        }
        word[wordPtr] = contents[*i];
        wordPtr++;
        *i += 1;
    }

    word[wordPtr] = '\0';
    *i += 1;

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
    //

    /*idk its soo wierd code wont work without this linebreak*/
    printf("\n");
    return keys;
}
struct keyword* parseJsonKeyfile(char* contents, struct keyword* keys)
{

    int blockLevel = 0;
    char* currentColor = NULL;
    int* i = malloc(sizeof(int));
    for (*i = 0; *i < strlen(contents); *i += 1) {
        if (contents[*i] == '"' && blockLevel == 0) {
            // if (currentColor != NULL) {
            //     free(currentColor);
            // }
            currentColor = parseJsonColor(contents, i);
            // printf("current color set to: %s\n", currentColor);
        }
        if (contents[*i] == '"' && blockLevel == 1) {
            keys = parseJsonKeyword(contents, i, currentColor, keys);
        }
        if (contents[*i] == '{') {
            blockLevel++;
            if (blockLevel > 2) {
                perror("invalid json, deep to block\n");
                exit(1);
            }
        }
        if (contents[*i] == '}')
            blockLevel--;
    }
    if (blockLevel != 0) {
        perror("invalid json, didnt end block\n");
        exit(1);
    }
    // if (currentColor != NULL) {
    //     free(currentColor);
    // }

    free(i);

    return keys;
}

struct keyword* keys = NULL;
char* fileContents = NULL;
bool fileContentsAllocated = false;
void parseArgs(int argc, char* argv[])
{
    /* parse input arguments for information and flags*/
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            if (fileContents != NULL) {
                printf("Duplicate filename: %s\n", argv[i]);
                printf("Only one filename can be entered\n");
                exit(1);
            }
            fileContents = readFile(argv[i]);
            fileContentsAllocated = true;

        } else
            switch (argv[i][1]) {
            case 'k':
                /*keyfile flag*/
                switch (argv[i][2]) {
                case 's':
                    keys = setDefaultKeywords();
                    /*put code to parse string here*/

                    i++;

                    if (i >= argc || argv[i][0] == '-') {
                        printf("Usage:\t -k <keyword file>\n");
                        exit(1);
                    } else {
                        keys = parseJsonKeyfile(argv[i], keys);
                    }
                    break;
                case 'a':
                    keys = setDefaultKeywords();
                    break;
                case '\0':
                    i++;

                    if (i >= argc || argv[i][0] == '-') {
                        printf("Usage:\t -k <keyword file>\n");
                        exit(1);
                    } else {
                        char* tmp = readFile(argv[i]);
                        keys = parseJsonKeyfile(tmp, keys);
                        free(tmp);
                    }
                    break;
                default:
                    printf("Unkown Flag\n");
                    exit(1);
                    break;
                }
                break;
            case 'm':
                /*parse string itead of input flag*/
                i++;
                if (i >= argc || argv[i][0] == '-') {
                    printf("Usage:\t -m <input string>\n");
                    exit(1);
                } else {
                    fileContents = argv[i];
                }

                break;
            case 'h':
                printf("Flags:\n");
                printf("\t-h\t for help\n");
                printf("\t-m\t to pass string instead of filepath\n");
                printf("\t-k\t to pass custom Json-keyfile\n");
                printf("\t-ka\t to pass custom Json-keyfile, and append and overwirte default\n");
                printf("\t-ks\t to pass custom Json-string, and append and overwirte default\n");
                printf("\n");

                exit(0);
                break;
            }
    }
    if (fileContents == NULL) {
        printf("Usage:\t <input file> or use -m <input string>\n");
        exit(1);
    }
    if (keys == NULL) {
        keys = setDefaultKeywords();
    }
}

int main(int argc, char* argv[])
{

    parseArgs(argc, argv);

    struct keyword *tmp, *tmp2;

    /* parse keywords*/
    if (fileContents != NULL) {
        parseForKeywords(keys, fileContents, tmp);
    } else {
        printf("no input provided\n");
    }
    if (fileContentsAllocated) {
        free(fileContents);
    }

    /* free the hash table contents */

    HASH_ITER(hh, keys, tmp, tmp2)
    {
        HASH_DEL(keys, tmp);
        free(tmp);
    }
    return 0;
}
