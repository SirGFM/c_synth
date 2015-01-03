/**
 * @file @src/synth_parser.c
 */
#include <synth/synth_assert.h>
#include <synth/synth_errors.h>
#include <synth/synth_types.h>
#include <synth_internal/synth_lexer.h>
#include <synth_internal/synth_parser.h>

#include <stdlib.h>

struct stSynthParserCtx {
    /**
     * Lexer context
     */
    synthLexCtx *lexCtx;
    /**
     * Song BPM
     */
    int bpm;
    /**
     * Current octave
     */
    int octave;
    /**
     * Default duration (when not specified)
     */
    int duration;
    // TODO volume!!
    /**
     * Current keyoff
     */
    int keyoff;
    /**
     * Current pan
     */
    int pan;
    /**
     * Current wave
     */
    synth_wave wave;
};

static synth_err synth_parser_initStruct(synthParserCtx **ctx);

/**
 * Init parsing of a mml string
 * 
 * @param ctx Variable that will store the context
 * @param mml String with the mml audio (it needn't be null-terminated)
 * @param len String's length
 * @return The error code
 */
synth_err synth_parser_initParses(synthParserCtx **ctx, char *mml, int len) {
    synth_err rv;
    synthParserCtx *tmp = 0;
    
    // Alloc and init the context
    rv = synth_parser_initStruct(&tmp);
    SYNTH_ASSERT(rv == SYNTH_OK);
    
    // Create the lexer for this mml
    rv = synth_lex_tokenizes(&(tmp->lexCtx), mml, len);
    SYNTH_ASSERT(rv == SYNTH_OK);
    
    *ctx = tmp;
    rv = SYNTH_OK;
__err:
    if (rv != SYNTH_OK)
        synth_parser_clean(&tmp);
    return rv;
}

/**
 * Init parsing of a mml file
 * 
 * @param ctx Variable that will store the context
 * @param filename MML's filename
 * @return The error code
 */
synth_err synth_parser_initParsef(synthParserCtx **ctx, char *filename) {
    synth_err rv;
    synthParserCtx *tmp = 0;
    
    // Alloc and init the context
    rv = synth_parser_initStruct(&tmp);
    SYNTH_ASSERT(rv == SYNTH_OK);
    
    // Create the lexer for this mml
    rv = synth_lex_tokenizef(&(tmp->lexCtx), filename);
    SYNTH_ASSERT(rv == SYNTH_OK);
    
    *ctx = tmp;
    rv = SYNTH_OK;
__err:
    if (rv != SYNTH_OK)
        synth_parser_clean(&tmp);
    return rv;
}

/**
 * Clean up memory allocated during initialization
 * 
 * @param ctx The context
 */
void synth_parser_clean(synthParserCtx **ctx) {
    SYNTH_ASSERT(ctx);
    SYNTH_ASSERT(*ctx);
    
    synth_lex_freeCtx(&((*ctx)->lexCtx));
    free(*ctx);
    *ctx = 0;
__err:
    return;
}

static synth_err synth_parser_initStruct(synthParserCtx **ctx) {
    synth_err rv;
            synthParserCtx *tmp = 0;
    
    // Create the parser ctx structure
    tmp = (synthParserCtx*)malloc(sizeof(synthParserCtx));
    SYNTH_ASSERT_ERR(tmp, SYNTH_MEM_ERR);
    
    tmp->lexCtx = 0;
    tmp->bpm = 60;
    tmp->octave = 4;
    tmp->duration = 4;
    // TODO volume!!
    tmp->keyoff = 75;
    tmp->pan = 50;
    tmp->wave = W_SQUARE;
    
    *ctx = tmp;
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Parse an audio into the context
 * Parsing rule: audio - bpm tracks T_DONE
 * 
 * @param ctx The context
 * @return Error code
 */
synth_err synth_parser_audio(synthParserCtx *ctx) {
    synth_err rv;
    
    // Get the first token
    rv = synth_lex_getToken(ctx->lexCtx);
    SYNTH_ASSERT(rv == SYNTH_OK);
    
    // Parse(optional) the bpm
    rv = synth_parser_bpm(ctx);
    SYNTH_ASSERT(rv == SYNTH_OK);
    
    // Parse every track in this audio
    rv = synth_parser_tracks(ctx);
    SYNTH_ASSERT(rv == SYNTH_OK);

    // Check that parsing finished
    SYNTH_ASSERT_TOKEN(T_DONE);
    
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Parse tracks into the context
 * Parsing rule: track ( T_END_OF_TRACK track )*
 * 
 * @param ctx The context
 * @return Error code
 */
synth_err synth_parser_tracks(synthParserCtx *ctx) {
    synth_err rv;
    
    rv = synth_parser_track(ctx);
    SYNTH_ASSERT(rv == SYNTH_OK);
    
__err:
    return rv;
}

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
synth_err synth_parser_bpm(synthParserCtx *ctx) {
    synth_err rv;
    
    // Check if is BPM
    if (synth_lex_lookupToken(ctx->lexCtx) == T_SET_BPM) {
        // Try to read the BPM
        rv = synth_lex_getToken(ctx->lexCtx);
        SYNTH_ASSERT(rv == SYNTH_OK);
        SYNTH_ASSERT_TOKEN(T_NUMBER);
        
        ctx->bpm = synth_lex_getValuei(ctx->lexCtx);
        
        // Get the next token
        rv = synth_lex_getToken(ctx->lexCtx);
    }
    
    rv = SYNTH_OK;
__err:
    return rv;
}

