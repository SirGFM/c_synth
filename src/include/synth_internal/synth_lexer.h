/**
 * @file src/include/synth_internal/synth_lexer.h
 * 
 * Module for tokenizing a mml file/string
 */
#ifndef __SYNTH_LEXER_H_
#define __SYNTH_LEXER_H_

#include <synth/synth_errors.h>
#include <synth/synth_types.h>

/**
 * Context for a tokenification operation
 */
typedef struct stSynthLexCtx synthLexCtx;

/**
 * Setup a context for tokenizing a mml file
 * 
 * @param ctx Variable that will hold the allocated context. Must be freed
 * afterward, by calling synth_lex_freeCtx
 * @param filename MML's filename
 * @return Error code
 */
synth_err synth_lex_tokenizef(synthLexCtx **ctx, char *filename);
/**
 * Setup a context for tokenizing a mml file
 * 
 * @param ctx Variable that will hold the allocated context. Must be freed
 * afterward, by calling synth_lex_freeCtx
 * @param mml String with the mml audio (it needn't be null-terminated)
 * @param len String's length
 * @return Error code
 */
synth_err synth_lex_tokenizes(synthLexCtx **ctx, char *mml, int len);

/**
 * Clean up memory allocated during setup (on tokenize)
 * 
 * @param ctx The contex to be freed
 */
void synth_lex_freeCtx(synthLexCtx **ctx);

/**
 * Get the next token on the context and its value (if any)
 * 
 * @param tk The token that was read
 * @param ctx The context
 * @return Error code
 */
synth_err synth_lex_getToken(synth_token *tk, synthLexCtx *ctx);

/**
 * Get the last read integer value
 * 
 * @param ctx The context
 * @return The value
 */
int synth_lex_getCurValuei(synthLexCtx *ctx);

/**
 * Read the character on the current position
 * 
 * @param c The character that was read
 * @param ctx The context
 * @return Error code
 */
synth_err synth_lex_getChar(char *c, synthLexCtx *ctx);

/**
 * Return the character to the 'stream'
 * 
 * @param ctx The context
 * @param c The character that was previously read
 */
void synth_lex_ungetChar(synthLexCtx *ctx, char c);

/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
synth_bool synth_lex_isSetBPM(synthLexCtx *ctx);
/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
synth_bool synth_lex_isSetDuration(synthLexCtx *ctx);
/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
synth_bool synth_lex_isSetOctave(synthLexCtx *ctx);
/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
synth_bool synth_lex_isSetRelOctave(synthLexCtx *ctx);
/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
synth_bool synth_lex_isSetLoopPoint(synthLexCtx *ctx);
/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
synth_bool synth_lex_isEndOfTrack(synthLexCtx *ctx);
/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
synth_bool synth_lex_isSetVolume(synthLexCtx *ctx);
/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
synth_bool synth_lex_isSetRelVolume(synthLexCtx *ctx);
/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
synth_bool synth_lex_isSetKeyoff(synthLexCtx *ctx);
/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
synth_bool synth_lex_isSetPan(synthLexCtx *ctx);
/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
synth_bool synth_lex_isSetLoopStart(synthLexCtx *ctx);
/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
synth_bool synth_lex_isSetLoopEnd(synthLexCtx *ctx);
/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
synth_bool synth_lex_isSetWave(synthLexCtx *ctx);
/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
synth_bool synth_lex_isNote(synthLexCtx *ctx);
/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
synth_bool synth_lex_isDotDuration(synthLexCtx *ctx);
/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
synth_bool synth_lex_isNumber(synthLexCtx *ctx);

/**
 * Check if there are any more characters in the 'stream' or not
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
synth_bool synth_lex_didFinish(synthLexCtx *ctx);

/**
 * Returns a printable string for a given token
 * 
 * @param tk The token
 * @return A null-terminated string representing the token
 */
char *synth_lex_printToken(synth_token tk);

/**
 * Get the current line number
 * 
 * @param ctx The contex
 */
int synth_lex_getCurrentLine(synthLexCtx *ctx);
/**
 * Get the current position inside the line
 * 
 * @param ctx The contex
 */
int synth_lex_getCurrentLinePosition(synthLexCtx *ctx);
/**
 * Get the last chracter read (that probably triggered an error)
 */
char synth_lex_getLastCharacter(synthLexCtx *ctx);

#endif

