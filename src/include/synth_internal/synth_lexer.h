/**
 * @file src/include/synth_internal/synth_lexer.h
 * 
 * Module for tokenizing a mml file/string
 */
#ifndef __SYNTH_LEXER_H__
#define __SYNTH_LEXER_H__

#include <synth/synth_errors.h>
#include <synth_internal/synth_types.h>

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

