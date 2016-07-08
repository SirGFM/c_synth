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

#include <c_synth_internal/synth_lexer.h>

/* == LEXER GLOBAL VARS ============================================ */

/** Lexer's reference */
synth_lexerCtx *pLexer = 0;
/** Amount of memory required by the lexer */
size_t synth_lexerSize = sizeof(synth_lexerCtx);

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
void synth_setupLexer(void *pBaseMemory) {
    pLexer = (synth_lexerCtx*)pBaseMemory;
    memset(pLexer, 0x0, sizeof(synth_lexerCtx));
}

/**
 * Retrieve the next token.
 */
synth_token synth_getNextToken() {
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
            return (synth_token)c;
        case STK_STRING: {
            pLexer->token.token = (synth_token)c;

            do {
                /* TODO Store the current character somewhere */
                /* TODO Assert that it's valid */
                c = synth_getNextChar();
            } while (c != STK_STRING);
            synth_ungetChar();

            return STK_STRING;
        } break;
        case STK_COMMENT: {
            int curLine;

            pLexer->token.token = (synth_token)c;
            curLine = pLexer->line;
            do {
                c = synth_getNextChar();
            } while (pLexer->line == curLine && c != STK_END_OF_INPUT);
            synth_ungetChar();

            return STK_COMMENT;
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
                val = val * 10 + (uint16_t)(c - '0');
                c = synth_getNextChar();
            } while (c >= '0' && c <= '9');
            synth_ungetChar();

            pLexer->token.token = STK_NUMBER;
            pLexer->token.data.numVal = val;

            return STK_NUMBER;
        } break;
        /* STK_NOTE */
        case 'a': pLexer->token.data.note = NT_A;
        case 'b': if (c == 'b') { pLexer->token.data.note = NT_B; }
        case 'c': if (c == 'c') { pLexer->token.data.note = NT_C; }
        case 'd': if (c == 'd') { pLexer->token.data.note = NT_D; }
        case 'e': if (c == 'e') { pLexer->token.data.note = NT_E; }
        case 'f': if (c == 'f') { pLexer->token.data.note = NT_F; }
        case 'g': if (c == 'g') { pLexer->token.data.note = NT_G; }

            /* Check sharp/flat modifier */
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
            return STK_NOTE;
        break;
        default:
            return STK_UNKNOWN;
    }
}

/* == LEXER DEBUG FUNCTION ========================================= */

/**
 * Dictionary of notes.
 *
 * @param  [ in]note The note
 * @return           Static string with the note's name
 */
char* synth_noteDict(synth_note note);

/**
 * Dictionary of tokens.
 *
 * @param  [ in]token The token.
 * @return            Static string with the token's name
 */
char* synth_tokenDictionary(synth_token token);

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
void synth_getLexerLine(unsigned int *pSize, char *pString);

