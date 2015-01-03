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
 * Parse an audio into the context
 * Parsing rule: audio - bpm tracks T_DONE
 * 
 * @param ctx The context
 * @return Error code
 */
synth_err synth_parser_audio(synthParserCtx *ctx);

/**
 * Parse tracks into the context
 * Parsing rule: track ( T_END_OF_TRACK track )*
 * 
 * @param ctx The context
 * @return Error code
 */
synth_err synth_parser_tracks(synthParserCtx *ctx);

/**
 * Parse a track into the context
 * Parsing rule: sequence | sequence? T_SET_LOOPPOINT sequence
 * 
 * @param ctx The context
 * @return Error code
 */
synth_err synth_parser_track(synthParserCtx *ctx);

/**
 * Parse a sequence into the context
 * Parsing rule: ( mod | note | loop )+
 * NOTE needs reworking!! It annoys me that there can be a track without notes
 * 
 * @param ctx The context
 * @return Error code
 */
synth_err synth_parser_sequence(synthParserCtx *ctx);

/**
 * Parse a loop into the context
 * Parsing rule: T_LOOP_START sequence T_LOOP_END T_NUMBER?
 * 
 * @param ctx The context
 * @return Error code
 */
synth_err synth_parser_loop(synthParserCtx *ctx);

/**
 * Parse a 'context modification' into the context
 * Parsing rule: 
 *   T_SET_DURATION T_NUMBER |
 *   T_SET_OCTAVE T_NUMBER |
 *   T_SET_REL_OCTAVE T_NUMBER |
 *   T_SET_VOLUME T_NUMBER | 
 *   T_SET_VOLUME T_OPEN_BRACKETS T_NUMBER T_COMMA T_NUMBER T_CLOSE_BRACKETS |
 *   T_OPEN_BRACKETS |      // for relative volume
 *   T_CLOSE_BRACKETS |     // for relative volume
 *   T_SET_KEYOFF T_NUMBER |
 *   T_SET_PAN T_NUMBER |
 *   T_SET_WAVE T_NUMBER
 * 
 * @param ctx The context
 * @return Error code
 */
synth_err synth_parser_mod(synthParserCtx *ctx);

/**
 * Parse a note into the context
 * Parsing rule: T_NOTE T_NUMBER? T_DURATION?
 * 
 * @param ctx The context
 * @return Error code
 */
synth_err synth_parser_note(synthParserCtx *ctx);

/**
 * Parse the audio's bpm
 * Parsing rule: ( T_SET_BPM T_NUMBER )?
 * 
 * @param ctx The context
 * @return The error code
 */
synth_err synth_parser_bpm(synthParserCtx *ctx);

#endif

