/**
 * @project c_synth
 * @license zlib license
 * @file    src/include/c_synth_internal/synth_lexer.h
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
#ifndef __SYNTH_LEXER_H__
#define __SYNTH_LEXER_H__

/** Required for fixed-width sizes */
#include <stdint.h>
#include <c_synth/synth_errors.h>

typedef struct stSynth_lexerCtx synth_lexerCtx;

/* == LEXER GLOBAL VARS ==================================================== */

/** Pointer to the start of the lexer within the application's memory */
extern synthLexer_ctx *pLexer;
/** Amount of memory required by the lexer */
extern size_t synth_lexerSize;

/* == LEXER TYPES ========================================================== */

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
    NT_REST
};
typedef enum enSynth_note synth_note;

/**
 * List of tokens recognized by the lexer. Almost every token is mapped to its
 * respective characters. The exception are NOTE_TK, STRING_TK, NUMBER_TK and
 * COMMENT_TK. Both STRING_TK and COMMENT_TK are mapped to the first character
 * recognized by then. NOTE_TK and STRING_TK, on the other hand, are simply
 * mapped to available characters, without any intrinsic meaning.
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
    STK_STRING          = '_',
    STK_COMMENT         = '#',
    STK_NOTE            = 'a',
    STK_NUMBER          = 'n',
    STK_END_OF_INPUT    = '\0'
};
typedef enum enSynth_token synth_token;

/** Possible representations for a token's data */
union unSynth_tokenData {
    synthNote note;
    uint16_t numVal;
};
typedef union unSynth_tokenData synth_tokenData;

/** Token and its value (if any) packed into a single 32 bits struct */
struct stSynth_packedToken {
    /** The token */
    synth_token token : 16;
    /** The token's data (only used on STK_NOTE and STK_NUMBER) */
    synth_tokenData data : 16;
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

/* == LEXER FUNCTIONS ====================================================== */

/**
 * Setup the lexer.
 *
 * Must be called only once during initialization.
 *
 * @param  [ in]pBaseMemory Position in memory where the lexer will be
 *                          stored. Must be at least synth_lexerSize.
 */
synth_err synth_setupLexer(void *pBaseMemory);

/**
 * (Re)initializes the lexer.
 *
 * The input file is rewound and the first token is retrieved.
 *
 * @param  [ in]pFile Input file used by the lexer
 */
synth_err synth_initLexer(void *pFile);

/**
 * Retrieve the next token.
 */
synth_err synth_getNextToken();

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


/* ========================================================================== */


/**
 * @file src/include/synth_internal/synth_lexer.h
 * 
 * Module for tokenizing a mml file/string
 */
#ifndef __SYNTH_LEXER_H__
#define __SYNTH_LEXER_H__

#include <c_synth/synth_errors.h>

#include <c_synth_internal/synth_types.h>

/**
 * Initialize the lexer, reading tokens from a SDL_RWops
 * 
 * If the lexer has already been initialized, it will be reset and
 * re-initialized with this new source
 * 
 * @param  [ in]pCtx  The lexer context, to be initialized
 * @param  [ in]pFile The file
 * @return            SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_OPEN_FILE_ERR
 */
synth_err synthLexer_initFromSDL_RWops(synthLexCtx *pCtx, void *pFile);

/**
 * Initialize the lexer, reading tokens from a file
 * 
 * If the lexer has already been initialized, it will be reset and
 * re-initialized with this new source
 * 
 * @param  [ in]pCtx      The lexer context, to be initialized
 * @param  [ in]pFilename The file
 * @return                SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_OPEN_FILE_ERR
 */
synth_err synthLexer_initFromFile(synthLexCtx *pCtx, char *pFilename);

/**
 * Initialize the lexer, reading tokens from a string
 * 
 * If the lexer has already been initialized, it will be reset and
 * re-initialized with this new source
 * 
 * @param  [ in]pCtx    The lexer context, to be initialized
 * @param  [ in]pString The string
 * @param  [ in]len     The string's length
 * @return              SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthLexer_initFromString(synthLexCtx *pCtx, char *pString, int len);

/**
 * Clear a lexer and all of its resources
 * 
 * This functions only needs really to be called when using a file as input
 * source, since, otherwise, everything is kept in RAM;
 * 
 * @param  [ in]pCtx      The lexer context, to be initialized
 * @return                SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthLexer_clear(synthLexCtx *pCtx);

/**
 * Get the current line number
 * 
 * @param [out]pVal The current line
 * @param [ in]pCtx The contex
 * return           SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthLexer_getCurrentLine(int *pVal, synthLexCtx *pCtx);

/**
 * Get the current position inside the line
 * 
 * @param [out]pVal The the current position
 * @param [ in]pCtx The contex
 * return           SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthLexer_getCurrentLinePosition(int *pVal, synthLexCtx *pCtx);

/**
 * Get the last chracter read (that probably triggered an error)
 * 
 * @param [out]pVal The last character
 * @param [ in]pCtx The contex
 * return           SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthLexer_getLastCharacter(char *pVal, synthLexCtx *pCtx);

/**
 * Get the token read on the previous getToken call
 * 
 * @param  [out]pToken The last read token
 * @param  [ in]pCtx   The context
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthLexer_lookupToken(synth_token *pToken, synthLexCtx *pCtx);

/**
 * Get the last read integer value
 * 
 * @param  [out]pVal The read integer
 * @param  [ in]pCtx The context
 * @return           SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthLexer_getValuei(int *pVal, synthLexCtx *pCtx);

/**
 * Returns a printable string for a given token
 * 
 * That that even if the token is invalid, the function returns OK; In that
 * case, though, the returned string says "unknown token"
 * 
 * @param  [out]ppStr The null-terminated, static string
 * @param  [ in]token The token
 * @return           SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthLexer_printToken(char **ppStr, synth_token token);

/**
 * Get the next token on the context and its value (if any)
 * 
 * @param  [ in]pCtx The context
 * return            SYNTH_OK, SYNTH_INVALID_TOKEN
 */
synth_err synthLexer_getToken(synthLexCtx *pCtx);

#endif /* __SYNTH_LEXER_H__ */

