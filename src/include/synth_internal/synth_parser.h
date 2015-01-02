/**
 * @file @src/include/synth_internal/synth_parser.h
 */
#ifndef __SYNTH_PARSER_H_
#define __SYNTH_PARSER_H_

#include <synth/synth_errors.h>
#include <synth/synth_types.h>
#include <synth_internal/synth_track.h>

typedef struct stSynthParserCtx synthParserCtx;

/**
 * Init parsing of a mml string
 * 
 * @param ctx Variable that will store the context
 * @param mml String with the mml audio (it needn't be null-terminated)
 * @param len String's length
 * @return The error code
 */
synth_err synth_parser_initParses(synthParserCtx **ctx, char *mml, int len);
/**
 * Init parsing of a mml file
 * 
 * @param ctx Variable that will store the context
 * @param filename MML's filename
 * @return The error code
 */
synth_err synth_parser_initParsef(synthParserCtx **ctx, char *filename);

/**
 * Clean up memory allocated during initialization
 * 
 * @param ctx The context
 */
void synth_parser_clean(synthParserCtx **ctx);

/**
 * Parse a new track from the context
 * 
 * @param track Track parsed
 * @param ctx The context
 * @return The error code
 */
synth_err synth_parser_getTrack(synthTrack **track, synthParserCtx *ctx);

/**
 * Parse the bpm. Not required, but must be at the start of the stream
 * 
 * @param ctx The context
 * @return The error code
 */
synth_err synth_parser_getBPM(synthParserCtx *ctx);

#endif

