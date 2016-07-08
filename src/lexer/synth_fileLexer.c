/**
 * @project c_synth
 * @license zlib license
 * @file    src/include/c_synth_internal/synth_fileLexer.c
 *
 * Backend for all accesses to inputs as files, within the lexer.
 */
#include <c_synth_internal/synth_lexer.h>

/** Required for everything FILE* related */
#include <stdio.h>

/**
 * (Re)initializes the lexer.
 *
 * The input file is rewound and the first token is retrieved.
 *
 * @param  [ in]pInput Input file used by the lexer
 */
void synth_loadFileInput(void *pInput) {
    pLexer->pInput = pInput;

    synth_rewindFileInput();
    synth_getNextToken();
}

/**
 * Rewinds the input to its start
 */
void synth_rewindFileInput() {
    /* Start it at 1, instead of 0, because it aims to help report
     * errors (and pretty much every text editor starts lines and
     * position at 1) */
    pLexer->line = 0;
    pLexer->linePos = 1;

    rewind((FILE*)pLexer->pInput);
}

/**
 * Reads the next character from the input.
 *
 * Any kind of whitespace (' ', '\t', '\n', '\r') is completely ignored.
 * Also, pLexer's line and linePos are updated accordingly.
 *
 * @return The character read. On error and on end of input, '\0' is
 *         returned.
 */
char synth_getNextCharFile() {
    int c;

    do {
        c = fgetc((FILE*)pLexer->pInput);

        switch (c) {
            case (int)'\r':
            break;
            case (int)'\n': {
                pLexer->linePos = 0;
                pLexer->line++;
            } break;
            case (int)' ':
            case (int)'\t': {
                pLexer->linePos++;
            } break;
            case EOF: {
                return '\0';
            } break;
            default: {
                pLexer->linePos++;
                return (char)c;
            }
        }
    } while (1);
}

/**
 * Move the current position within the input back.
 *
 * This function updates pLexer's line and linePos. Other than that,
 * it's useful when parsing tokens that are/may be longer than one
 * character long.
 *
 * @return The character that was on the current position.
 */
char synth_ungetCharFile() {
    int c;

    /* TODO Check for 'underflows' (i.e., reading past the begin of the
     * file) */

    fseek((FILE*)pLexer->pInput, -1, SEEK_CUR);
    c = fgetc((FILE*)pLexer->pInput);

    fseek((FILE*)pLexer->pInput, -2, SEEK_CUR);
    pLexer->linePos--;
    if (c == '\n') {
        pLexer->line--;
        /* TODO Query the line position, somehow? */
        pLexer->linePos = 10;
    }
    else if (c == EOF) {
        c = '\0';
    }

    return (char)c;
}

