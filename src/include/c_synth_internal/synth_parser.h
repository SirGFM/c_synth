/**
 * @file @src/include/synth_internal/synth_parser.h
 */
#ifndef __SYNTH_PARSER_H__
#define __SYNTH_PARSER_H__

#include <c_synth_internal/synth_types.h>

/**
 * Initialize the parser
 * 
 * The default settings are as follows:
 *   - BMP: 60bmp
 *   - Octave: 4th
 *   - Duration: quarter note
 *   - Keyoff: 75% (i.e., pressed for that amount of the duration)
 *   - Pan: 50% (i.e., equal to both channels)
 *   - Wave: 50% square
 *   - Volume: 50%
 * 
 * @param  [out]pParser The parser context to be initialized
 * @param  [ in]pCtx    The synthesizer context
 * @return              SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_MEM_ERR
 */
synth_err synthParser_init(synthParserCtx *pParser, synthCtx *pCtx);

/**
 * Return the error string
 * 
 * This string is statically allocated and mustn't be freed by user
 * 
 * @param  [out]ppError The error string
 * @param  [ in]pParser The parser context
 * @param  [ in]pCtx    The synthesizer context
 * @return              SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_NO_ERRORS
 */
synth_err synthParser_getErrorString(char **ppError, synthParserCtx *pParser,
        synthCtx *pCtx);

/**
 * Parse the currently loaded file into an audio
 * 
 * This function uses a lexer to break the file into tokens, as it does
 * retrieve track, notes etc from the main synthesizer context
 * 
 * Note: The context's lexer must have already been initialized; Therefore,
 * it's safer to simply use 'synthAudio_compile' (which calls this function),
 * or, at least, look at how that function is implemented
 * 
 * @param  [ in]pParser The parser context
 * @param  [ in]pCtx    The synthesizer context
 * @param  [ in]pAudio  A clean audio object, that will be filled with the
 *                      parsed song
 * @return              SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_MEM_ERR, ...
 */
synth_err synthParser_getAudio(synthParserCtx *pParser, synthCtx *pCtx,
        synthAudio *pAudio);

#endif /* __SYNTH_PARSER_H__ */

