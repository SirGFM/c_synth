/**
 * @project   c_synth
 * @license   zlib license
 * @file      src/include/c_synth_internal/synth_lexer.h
 *
 * @summary   The language's lexer.
 *
 * @typedef   synth_note        Notes within an octave.
 * @typedef   synth_token       Tokens recognized by the lexer.
 * @typedef   synth_tokenData   Data that may accompany a token.
 * @typedef   synth_packedToken A token and its data (if any).
 * @typedef   synth_lexerCtx    The lexer context
 *
 * @globalvar pLexer            Reference to the lexer.
 * @globalvar synth_lexerSize   Amount of memory required by the lexer.
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
#ifndef __SYNTH_LEXER_H__
#define __SYNTH_LEXER_H__

/** Required for fixed-width sizes */
#include <stdint.h>
/** Required for size_t */
#include <stddef.h>

/* == LEXER TYPES =================================================== */

/** Represents all possible notes within a single octave. */
enum enSynth_note {
    /** Equivalent to NT_B on a lower octave */
    NT_CB = 0,
    NT_C,
    NT_CS,
    NT_D,
    NT_DS,
    NT_E,
    NT_F,
    NT_FS,
    NT_G,
    NT_GS,
    NT_A,
    NT_AS,
    NT_B,
    /** Equivalent to NT_C on a higher octave */
    NT_BS,
    NT_REST,
    NT_MAX
};
typedef enum enSynth_note synth_note;

/**
 * List of tokens recognized by the lexer. Almost every token is mapped
 * to its respective characters. The exception are NOTE_TK, STRING_TK,
 * NUMBER_TK and COMMENT_TK. Both STRING_TK and COMMENT_TK are mapped to
 * the first character recognized by then. NOTE_TK and STRING_TK, on the
 * other hand, are simply mapped to available characters, without any
 * intrinsic meaning.
 */
enum enSynth_token {
    STK_HALF_DURATION   = '.',
    STK_NOTE_EXTENSION  = '^',
    STK_OCTAVE          = 'o',
    STK_INCREASE_OCTAVE = '>',
    STK_DECREASE_OCTAVE = '<',
    STK_DURATION        = 'l',
    STK_LOAD            = 'j',
    STK_INSTRUMENT      = 'i',
    STK_ENVELOPE        = 'v',
    STK_WAVE            = 'w',
    STK_PANNING         = 'p',
    STK_ATTACK          = 't',
    STK_KEYOFF          = 'k',
    STK_RELEASE         = 'q',
    STK_LOOP_START      = '[',
    STK_LOOP_END        = ']',
    STK_REPEAT          = '$',
    STK_MACRO           = 'm',
    STK_END             = ';',
    STK_BPM             = 'B',
    STK_KEY             = 'K',
    STK_TEMPO           = 'T',
    STK_STRING          = '"',
    STK_COMMENT         = '#',
    STK_NOTE            = 'a',
    STK_NUMBER          = 'n',
    STK_END_OF_INPUT    = '\0',
    STK_UNKNOWN         = '?'
};
typedef enum enSynth_token synth_token;

/** Possible representations for a token's data */
union unSynth_tokenData {
    synth_note note;
    uint16_t numVal;
};
typedef union unSynth_tokenData synth_tokenData;

/** Token and its value (if any) packed into a single 32 bits struct */
struct stSynth_packedToken {
    /** The token */
    synth_token token;
    /** The token's data (only used on STK_NOTE and STK_NUMBER) */
    synth_tokenData data;
};
typedef struct stSynth_packedToken synth_packedToken;

/** The lexer context */
struct stSynth_lexerCtx {
    /** The retrieved token */
    synth_packedToken token;
    /** Current line withing the input (useful for error logging) */
    int line;
    /** Current position within a line (useful for error logging) */
    int linePos;
    /** FILE* from where the song is read */
    void *pInput;
};
typedef struct stSynth_lexerCtx synth_lexerCtx;

/* == LEXER GLOBAL VARS ============================================= */

/** Lexer's reference */
extern synth_lexerCtx *pLexer;
/** Amount of memory required by the lexer */
extern const size_t synth_lexerSize;

/* == LEXER FUNCTIONS =============================================== */

/**
 * Setup the lexer.
 *
 * Must be called only once during initialization.
 *
 * @param  [ in]pBase Position in memory where the lexer will be stored.
 *                    Must point to at least synth_lexerSize.
 */
void synth_setupLexer(void *pBase);

/**
 * Retrieve the next token.
 *
 * @return The retrieved token (for ease of use).
 */
synth_token synth_getNextToken();

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

/* == LEXER BACKEND FUNCTIONS ======================================= */

/**
 * Define the backend functions
 */
#define synth_loadInput(pInput) synth_loadFileInput(pInput)
#define synth_rewindInput() synth_rewindFileInput()
#define synth_getNextChar() synth_getNextCharFile()
#define synth_ungetChar() synth_ungetCharFile()

/**
 * (Re)initializes the lexer.
 *
 * The input file is rewound and the first token is retrieved.
 *
 * @param  [ in]pInput Input file used by the lexer
 */
void synth_loadFileInput(void *pInput);

/**
 * Rewinds the input to its start
 */
void synth_rewindFileInput(void);

/**
 * Reads the next character from the input.
 *
 * Any kind of whitespace (' ', '\t', '\n', '\r') is completely ignored.
 * Also, pLexer's line and linePos are updated accordingly.
 *
 * @return The character read. On error and on end of input, '\0' is
 *         returned.
 */
char synth_getNextCharFile(void);

/**
 * Move the current position within the input back.
 *
 * This function updates pLexer's line and linePos. Other than that,
 * it's useful when parsing tokens that are/may be longer than one
 * character long.
 *
 * @return The character that was on the current position.
 */
char synth_ungetCharFile(void);

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

#endif

