/**
 * @project c_synth
 * @license zlib license
 * @file    src/include/c_synth_internal/synth_lexer.c
 *
 * The language's lexer.
 *
 * Breaks an input into tokens, which are used by the parser to
 * syntactically analyze songs.
 *
 * By design, there shall be only one lexer for each application that
 * links it, and it may be accessed within the library through the
 * global pointer 'pLexer'. There's no need to directly access it
 * outside the library, but one may retrieve the current line (and the
 * position within it) through the function 'synth_getLexerLine'.
 *
 * The user of the lexer is required to alloc the memory that it
 * requires, which must be at least 'synth_lexerSize' bytes long. Also,
 * even though the lexer may use outside resources (e.g., FILE*) as
 * input, it's responsibility of the caller to clean all that up. This
 * is intended to help cleaning up the whole synthesizer in one go,
 * since a single memory block may be alloc'ed and divided by the
 * caller.
 */

/** Required for fixed-width sizes */
#include <stdint.h>
/** Required for FILE* related stuff */
#include <stdio.h>
/** Required for memset */
#include <string.h>

#include <c_synth/synth_errors.h>
#include <c_synth_internal/synth_lexer.h>

/* == LEXER GLOBAL VARS ============================================ */

/** Lexer's reference */
synthLexer_ctx *pLexer = 0;
/** Amount of memory required by the lexer */
size_t synth_lexerSize = sizeof(synthLexer_ctx);

/* == LEXER FUNCTIONS ======================================= */

/**
 * Setup the lexer.
 *
 * Must be called only once during initialization.
 *
 * @param  [ in]pBaseMemory Position in memory where the lexer will be
 *                          stored. Must point to at least
 *                          synth_lexerSize.
 */
synth_err synth_setupLexer(void *pBaseMemory) {
    if (pLexer) {
        /* Should it be an error to setup it twice? */
    }

    pLexer = (synthLexer_ctx*)pBaseMemory;
    memset(pLexer, 0x0, sizeof(synthLexer_ctx));
}

/**
 * (Re)initializes the lexer.
 *
 * The input file is rewound and the first token is retrieved.
 *
 * @param  [ in]pFile Input file used by the lexer
 */
synth_err synth_initLexer(void *pFile) {
    if (!pFile || !pLexer) {
        /* Assert the error */
    }

    /* Start it at 1, instead of 0, because it aims to help report
     * errors (and pretty much every text editor starts lines and
     * position at 1) */
    pLexer->line = 1;
    pLexer->linePos = 1;
    pLexer->pInput = pFile;

    synth_rewindInput();
    return synth_getNextToken();
}

/**
 * Retrieve the next token.
 */
synth_err synth_getNextToken() {
    char c;

    /* Most tokens are mapped to their actual characters */
    c = synth_getNextChar();
    switch(c) {
        case STK_HALF_DURATION:
        case STK_NOTE_EXTENSION:
        case STK_OCTAVE:
        case STK_INCREASE_OCTAVE:
        case STK_DECREASE_OCTAVE:
        case STK_DURATION:
        case STK_LOAD:
        case STK_INSTRUMENT:
        case STK_ENVELOPE:
        case STK_WAVE:
        case STK_PANNING:
        case STK_ATTACK:
        case STK_KEYOFF:
        case STK_RELEASE:
        case STK_LOOP_START:
        case STK_LOOP_END:
        case STK_REPEAT:
        case STK_MACRO:
        case STK_END:
        case STK_BPM:
        case STK_KEY:
        case STK_TEMPO:
        case STK_END_OF_INPUT:
            pLexer->token.token = (synth_token)c;
            /* TODO Return OK */
            return;
        case STK_STRING: {
            pLexer->token.token = (synth_token)c;
            do {
                /* TODO Store the current character somewhere */
                c = synth_getNextChar();
            } while ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
                    || (c >= '0' && c <= '9'));
            synth_ungetChar();
            /* TODO Return OK */
            return;
        } break;
        case STK_COMMENT: {
            pLexer->token.token = (synth_token)c;
            do {
                c = synth_getNextChar();
            } while (c != '\n' && c != STK_END_OF_INPUT);
            synth_ungetChar();
            /* TODO Return OK */
            return;
        } break;
        /* STK_NUMBER */
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            uint16_t val = 0;

            do {
                val = val * 10 + c - '0';
                c = synth_getNextChar();
            } while (c >= '0' && c <= '9');
            synth_ungetChar();

            pLexer->token.token = STK_NUMBER;
            pLexer->token.data.numVal = val;

            /* TODO Return OK */
            return;
        } break;
        /* STK_NOTE */
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g': {
            /* TODO Fix this so it actually works */
            pLexer->token.data.note = (synthNote)c;
            c = synth_getNextChar();
            if (c == '+') {
                pLexer->token.data.note++;
            }
            else if (c == '-') {
                pLexer->token.data.note--;
            }
            else {
                synth_ungetChar();
            }
            pLexer->token.token = STK_NOTE;
            /* TODO Lookup modifier */
        } break;
        default:
            return UNKOWN;
    }
}

/**
 * Retrieves the current line in ASCII format.
 *
 * The returned string is actually two lines long. The first points to
 * the current position within the actual input and the second one is
 * the actual input until the current position (plus the line number).
 * E.g.:
 *
 * Actual line: "a+ f g+ a+ f g+ a+ f"
 * Line number: 70
 * Current position: 9
 *
 * Output: "           v\n70: a+ f g+ \0"
 * Printed output (2 spaces indent):
 * --           v
 * --70: a+ f g+ 
 *
 * This is mostly useful for reporting errors on parsing errors.
 *
 * @param  [out]pSize   Length of the string
 * @param  [out]pString The current line. If NULL, the maximum required
 *                      size will be returned.
 */
synth_err synth_getLexerLine(unsigned int *pSize, char *pString);

#endif

