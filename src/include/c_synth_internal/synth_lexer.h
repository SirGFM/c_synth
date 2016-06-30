#ifndef __SYNTH_LEXER_H__
#define __SYNTH_LEXER_H__

#include <c_synth/synth_errors.h>

/**
 * List of tokens recognized by the lexer. Almost every token is mapped to its
 * respective characters. The exception are NOTE_TK, STRING_TK, NUMBER_TK and
 * COMMENT_TK. Both STRING_TK and COMMENT_TK are mapped to the first character
 * recognized by then. NOTE_TK and STRING_TK, on the other hand, are simply
 * mapped to available characters, without any intrinsic meaning.
 */
enum enSynthToken {
    STK_HALF_DURATION   = '.',
    STK_NOTE_EXTENSION  = '^',
    STK_SHARP           = '+',
    STK_FLAT            = '-',
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
typedef enum enSynthToken synthToken;

#if defined(ENABLE_STRING_INPUT)
/** A simple string that may be used as input for the lexre */
struct stSynthString {
    /** Total length of the string */
    unsigned int len;
    /** Current position on the string */
    unsigned int pos;
    /** Pointer to the static (and NULL-terminated) string */
    char *pStr;
};
typedef struct stSynthString synthString;
#endif

/** An input that shall be tokenized by the lexer */
union unSynthInput {
#if defined(USE_SDL2)
    /** SDL's SDL_RWops, so it works on mobile! */
    SDL_RWops *sdl;
#endif
    /** A file */
    FILE *file;
#if defined(ENABLE_STRING_INPUT)
    /** A static string, with its current position and length */
    synthString str;
#endif
};
typedef union unSynthInput synthInput;

/** List of types available as input */
enum enSynthInputType {
    SST_NONE = 0,
    SST_FILE,
    SST_STR,
    SST_SDL,
    SST_MAX
};
typedef enum enSynthInputType synthInputType;

struct stSynthLexerCtx {
    synthToken token;
    synthInputType inputType;
    int data;
    int line;
    int linePos;
    synthInput input;
};
typedef struct stSynthLexerCtx synthLexerCtx;

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

