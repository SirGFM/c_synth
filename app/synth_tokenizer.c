/**
 * @project c_synth
 * @license zlib license
 * @file    app/synth_tokenizer.c
 *
 * Application to test tokenization of files
 */
/** Required for operations on FILE* */
#include <stdio.h>
/** Required for alloc/free */
#include <stdlib.h>

#include <c_synth_internal/synth_lexer.h>

#define ASSERT(stmt) \
  do { \
    if (!(stmt)) { \
      goto __ret; \
    } \
  } while (0)

extern synth_lexerCtx *pLexer;
extern size_t synth_lexerSize;

int main(int argc, char *argv[]) {
    void *pMem = 0;
    FILE *pFile = 0;
    int rv;

    if (argc < 2) {
        printf("Expected a file but got nothing!\n");
        printf("Usage: synth_tokenizer <FILE>\n");
        return -1;
    }

    pMem = malloc(synth_lexerSize);
    ASSERT(pMem);

    synth_setupLexer(pMem);

    pFile = fopen(argv[1], "rt");
    ASSERT(pFile);
    synth_loadInput(pFile);

    do {
        switch (pLexer->token.token) {
            case STK_NOTE: {
                printf("%s:%s ",
                        synth_tokenDictionary(pLexer->token.token),
                        synth_noteDict(pLexer->token.data.note));
            } break;
            case STK_NUMBER: {
                printf("%s:%i ",
                        synth_tokenDictionary(pLexer->token.token),
                        pLexer->token.data.numVal);
            } break;
            case STK_STRING: {
                printf("%s:%s ",
                        synth_tokenDictionary(pLexer->token.token),
                        "NOT_YET_IMPLEMENTED");
                /* TODO Print the string */
            } break;
            default: {
                printf("%s ",
                        synth_tokenDictionary(pLexer->token.token));
            }
        }
        synth_getNextToken();
    } while (pLexer->token.token != STK_END_OF_INPUT &&
            pLexer->token.token != STK_UNKNOWN);

    rv = 0;
__ret:
    if (pFile) {
        fclose(pFile);
    }
    free(pMem);

    return rv;
}

