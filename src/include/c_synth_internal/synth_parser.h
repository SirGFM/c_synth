/**
 * @project c_synth
 * @license zlib license
 * @file    src/include/c_synth_internal/synth_parser.h
 *
 * The language's parser.
 */
#ifndef __SYNTH_PARSER_H__
#define __SYNTH_PARSER_H__

/** Required for fixed-width sizes */
#include <stdint.h>
/** Required for synth_err */
#include <c_synth/synth_error.h>
/** Required for synth_token */
#include <c_synth_internal/synth_lexer.h>

/**
 * Possible values set on error
 */
union unSynth_parserErrorData {
    /**
     * The expected token. Used when rv == SYNTH_PARSER_ERROR.
     *
     * The retrieved one may be accessed on pLexer->token.token.
     */
    synth_token expected;
    /**
     * Maximum value allowed. Used when rv == SYNTH_VALUE_RANGE.
     *
     * The retrieved one may be accessed on pLexer->token.data.
     */
    uint16_t maxValue;
};
typedef union unSynth_parserErrorData synth_parserErrorData;

/** Context with information about the last error */
struct stSynth_parserError {
    /** Token being parsed when an error happened */
    synth_token context;
    /** The latest error, if any */
    synth_error rv;
    /**
     * Data that triggered the error.
     *
     * Which value should be read depends on rv.
     */
    synth_parserErrorData data;
};
typedef struct stSynth_parserError synth_parserError;

/** The parser context */
struct stSynth_parserCtx {
    synth_parserError error;

    int octave;
    int duration;
    int startPosition;
};
typedef struct stSynth_parserCtx synth_parserCtx;

extern synth_parserCtx *pParser;

#endif /* __SYNTH_PARSER_H__ */

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

