/**
 * @file @src/synth_parser.c
 */
#include <synth/synth_assert.h>
#include <synth/synth_errors.h>
#include <synth/synth_types.h>
#include <synth_internal/synth_lexer.h>
#include <synth_internal/synth_parser.h>

#include <stdio.h>
#include <stdlib.h>

/**
 * Marker for where compiling is done
 */
#define COMPILER /**/

#define TOKEN_MAX_STR 30
#define EXTRA_CHARS 5+6+2
static char parser_def_msg[] = "ERROR: Expected %s but got %s.\n"
                               "       Line: %i\n"
                               "       Position: %i\n"
                               "       Last character: %c\n";
static char parser_rv_msg[] = "ERROR: %s\n"
                               "       Line: %i\n"
                               "       Position: %i\n"
                               "       Last character: %c\n";
static char parser_error[TOKEN_MAX_STR*2+sizeof(parser_def_msg)+EXTRA_CHARS];

struct stSynthParserCtx {
    /**
     * Lexer context
     */
    synthLexCtx *lexCtx;
    /**
     * Expected token (only valid on error)
     */
    synth_token expected;
    /**
     * Gotten token (only valid on error)
     */
    synth_token gotten;
    /**
     * Whether an error occured or note
     */
    synth_bool errorFlag;
    /**
     * Which error code was raised
     */
    synth_err errorCode;
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
    
    tmp->errorFlag = SYNTH_FALSE;
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
    if (rv != SYNTH_OK) {
        ctx->errorFlag = SYNTH_TRUE;
        ctx->errorCode = rv;
    }
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
    
    // Parse the first track
    rv = synth_parser_track(ctx);
    SYNTH_ASSERT(rv == SYNTH_OK);
    
    // Parse every other track
    while (synth_lex_lookupToken(ctx->lexCtx) == T_END_OF_TRACK) {
        rv = synth_lex_getToken(ctx->lexCtx);
        SYNTH_ASSERT(rv == SYNTH_OK);
        
        rv = synth_parser_track(ctx);
        SYNTH_ASSERT(rv == SYNTH_OK);
    }
    
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Check if the next structure is a sequence
 * 
 * @param ctx The context
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
static synth_bool synth_parser_isSequence(synthParserCtx *ctx) {
    switch (synth_lex_lookupToken(ctx->lexCtx)) {
        case T_SET_DURATION:
        case T_SET_OCTAVE:
        case T_SET_REL_OCTAVE:
        case T_SET_VOLUME:
        case T_SET_REL_VOLUME:
        case T_SET_KEYOFF:
        case T_SET_PAN:
        case T_SET_WAVE:
        case T_NOTE:
        case T_SET_LOOP_START:
            return SYNTH_TRUE;
        break;
        default:
            return SYNTH_FALSE;
    }
}

/**
 * Parse a track into the context
 * Parsing rule: sequence | sequence? T_SET_LOOPPOINT sequence
 * 
 * @param ctx The context
 * @return Error code
 */
synth_err synth_parser_track(synthParserCtx *ctx) {
    synth_err rv;
    int loopRequired;
    
    // Loop is required unless there was a sequence
    loopRequired = 1;
    
COMPILER // TODO spawn a new track
    
    // Parse a sequence, if it's the next token
    if (synth_parser_isSequence(ctx) == SYNTH_TRUE) {
        rv = synth_parser_sequence(ctx);
        SYNTH_ASSERT(rv == SYNTH_OK);
        
        // Set loop as not required
        loopRequired = 0;
    }
    
    // If no sequence has been read, a LOOPPOINT token is required
    if (loopRequired)
        SYNTH_ASSERT_TOKEN(T_SET_LOOPPOINT);
    
    // Parse another sequence, if there's a looppoint token
    if (synth_lex_lookupToken(ctx->lexCtx) == T_SET_LOOPPOINT) {
        // Get the next token
        rv = synth_lex_getToken(ctx->lexCtx);
        SYNTH_ASSERT(rv == SYNTH_OK);
        
COMPILER // TODO set up the track loop point
        
        // Parse the looping sequence
        rv = synth_parser_sequence(ctx);
        SYNTH_ASSERT(rv == SYNTH_OK);
    }
    
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Parse a sequence into the context
 * Parsing rule: ( mod | note | loop )+
 * 
 * @param ctx The context
 * @return Error code
 */
synth_err synth_parser_sequence(synthParserCtx *ctx) {
    synth_err rv;
    int gotNoteOrLoop;
    
    gotNoteOrLoop = 0;
    
    // Next token must be one of sequence
    SYNTH_ASSERT_ERR(synth_parser_isSequence(ctx) == SYNTH_TRUE,
        SYNTH_UNEXPECTED_TOKEN);
    
    // Fun stuff. Lookup next token and do whatever it requires
    while (synth_parser_isSequence(ctx) == SYNTH_TRUE) {
        // Anything not a note or loop will be parsed as mod
        switch (synth_lex_lookupToken(ctx->lexCtx)) {
            case T_NOTE:
                rv = synth_parser_note(ctx);
                gotNoteOrLoop = 1;
            break;
            case T_SET_LOOP_START:
                rv = synth_parser_loop(ctx);
                gotNoteOrLoop = 1;
            break;
            default:
                rv = synth_parser_mod(ctx);
        }
        SYNTH_ASSERT(rv == SYNTH_OK);
    }
    
    // At least one note or loop must be on the sequence, otherwise it's invalid
    SYNTH_ASSERT_ERR(gotNoteOrLoop == 1, SYNTH_EMPTY_SEQUENCE);
    
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Parse a loop into the context
 * Parsing rule: T_SET_LOOP_START sequence T_SET_LOOP_END T_NUMBER?
 * 
 * @param ctx The context
 * @return Error code
 */
synth_err synth_parser_loop(synthParserCtx *ctx) {
    synth_err rv;
    int loopPosition;
    int count;
    
    // We're sure to have this token, but...
    SYNTH_ASSERT_TOKEN(T_SET_LOOP_START);
    
    // Set basic loop count to 2 (default)
    count = 2;
    
    // Store the current position in track to set it on the loop 'note'
COMPILER loopPosition = 0;// TODO read value from track
    
    // Read the next token
    rv = synth_lex_getToken(ctx->lexCtx);
    SYNTH_ASSERT(rv == SYNTH_OK);
    
    // After the loop start, there must be a sequence
    SYNTH_ASSERT_ERR(synth_parser_isSequence(ctx) == SYNTH_TRUE,
        SYNTH_INVALID_TOKEN);
    
    // Parse the sequence
    rv = synth_parser_sequence(ctx);
    SYNTH_ASSERT(rv == SYNTH_OK);
    
    // Afterwards, a loop end must come
    SYNTH_ASSERT_TOKEN(T_SET_LOOP_END);
    
    // Get the next token
    rv = synth_lex_getToken(ctx->lexCtx);
    SYNTH_ASSERT(rv == SYNTH_OK);
    
    // If the next token is a number, it's how many times the loop runs
    if (synth_lex_lookupToken(ctx->lexCtx) == T_NUMBER) {
        // Store the loop count
COMPILER    count = synth_lex_getValuei(ctx->lexCtx);
        
        // Get the next token
        rv = synth_lex_getToken(ctx->lexCtx);
        SYNTH_ASSERT(rv == SYNTH_OK);
    }
    
    // Add a 'loop note' to the track
COMPILER // TODO actually set the loop
    
    rv = SYNTH_OK;
__err:
    return rv;
}

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
synth_err synth_parser_mod(synthParserCtx *ctx) {
    synth_err rv;
    
    switch (synth_lex_lookupToken(ctx->lexCtx)) {
        case T_SET_DURATION:
            // Read the following number
            rv = synth_lex_getToken(ctx->lexCtx);
            SYNTH_ASSERT(rv == SYNTH_OK);
            SYNTH_ASSERT_TOKEN(T_NUMBER);
            
            // Set the default duration for notes
COMPILER    ctx->duration = synth_lex_getValuei(ctx->lexCtx);
        break;
        case T_SET_OCTAVE:
            // Read the following number
            rv = synth_lex_getToken(ctx->lexCtx);
            SYNTH_ASSERT(rv == SYNTH_OK);
            SYNTH_ASSERT_TOKEN(T_NUMBER);
            
            // Set the octave for notes
COMPILER    ctx->octave = synth_lex_getValuei(ctx->lexCtx);
        break;
        case T_SET_REL_OCTAVE:
            // This token already have the variation, so simply use it
            
            // Increase or decrease the octave
COMPILER    ctx->octave += synth_lex_getValuei(ctx->lexCtx);
        break;
        case T_SET_VOLUME:
COMPILER // TODO set volume
        break;
        case T_SET_REL_VOLUME:
COMPILER // TODO >__<
        break;
        case T_SET_KEYOFF:
            // Read the following number
            rv = synth_lex_getToken(ctx->lexCtx);
            SYNTH_ASSERT(rv == SYNTH_OK);
            SYNTH_ASSERT_TOKEN(T_NUMBER);
            
            // Set the keyoff value
COMPILER    ctx->keyoff = synth_lex_getValuei(ctx->lexCtx);
        break;
        case T_SET_PAN:
            // Read the following number
            rv = synth_lex_getToken(ctx->lexCtx);
            SYNTH_ASSERT(rv == SYNTH_OK);
            SYNTH_ASSERT_TOKEN(T_NUMBER);
            
            // Set the pan
COMPILER    ctx->pan = synth_lex_getValuei(ctx->lexCtx);
        break;
        case T_SET_WAVE:
            // Read the following number
            rv = synth_lex_getToken(ctx->lexCtx);
            SYNTH_ASSERT(rv == SYNTH_OK);
            SYNTH_ASSERT_TOKEN(T_NUMBER);
            
            // Parse the number into a wave
COMPILER    switch (synth_lex_getValuei(ctx->lexCtx)) {
COMPILER        case 0: ctx->wave = W_SQUARE; break;
COMPILER        case 1: ctx->wave = W_PULSE_12_5; break;
COMPILER        case 2: ctx->wave = W_PULSE_25; break;
COMPILER        case 3: ctx->wave = W_PULSE_75; break;
COMPILER        case 4: ctx->wave = W_TRIANGLE; break;
COMPILER        case 5: ctx->wave = W_NOISE; break;
COMPILER        default: SYNTH_ASSERT_ERR(0, SYNTH_INVALID_WAVE);
COMPILER    }
        break;
        default:
            SYNTH_ASSERT_ERR(0, SYNTH_UNEXPECTED_TOKEN);
    }
    
    // Read the next token
    rv = synth_lex_getToken(ctx->lexCtx);
    SYNTH_ASSERT(rv == SYNTH_OK);
    
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Parse a note into the context
 * Parsing rule: T_NOTE T_NUMBER? T_DURATION?
 * 
 * @param ctx The context
 * @return Error code
 */
synth_err synth_parser_note(synthParserCtx *ctx) {
    synth_err rv;
    synth_note note;
    int duration;
    
    // Callee function already assures this, but...
    SYNTH_ASSERT_TOKEN(T_NOTE);
    
    // Set initial duration to whatever the default is
    duration = ctx->duration;
    
    // Store the note to be played
    note = (synth_note)synth_lex_getValuei(ctx->lexCtx);
    
    // Get next token
    rv = synth_lex_getToken(ctx->lexCtx);
    SYNTH_ASSERT(rv == SYNTH_OK);
    
    // Any number following the note will override the note
    if (synth_lex_lookupToken(ctx->lexCtx) == T_NUMBER) {
        // Store the new duration and read the next token
        duration = synth_lex_getValuei(ctx->lexCtx);
        
        rv = synth_lex_getToken(ctx->lexCtx);
        SYNTH_ASSERT(rv == SYNTH_OK);
    }
    
    // If there are any '.', add half the duration every time
    if (synth_lex_lookupToken(ctx->lexCtx) == T_NUMBER) {
        int dots;
        
        dots = synth_lex_getValuei(ctx->lexCtx);
        
        // TODO do something with that value
        
        // Read whatever the next token is
        rv = synth_lex_getToken(ctx->lexCtx);
        SYNTH_ASSERT(rv == SYNTH_OK);
    }
    
COMPILER // TODO create and add the note to the track
    
    rv = SYNTH_OK;
__err:
    return rv;
}

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
        SYNTH_ASSERT(rv == SYNTH_OK);
    }
    
    rv = SYNTH_OK;
__err:
    return rv;
}

static char *synth_parser_getErrorMessage(synthParserCtx *ctx) {
    switch (ctx->errorCode) {
        case SYNTH_EOF: return "File ended before parsing ended"; break;
        case SYNTH_EOS: return "Stream ended before parsing ended"; break;
        case SYNTH_UNEXPECTED_TOKEN: return "Unexpected token"; break;
        case SYNTH_EMPTY_SEQUENCE: return "Got a track without notes"; break;
        case SYNTH_INVALID_WAVE: return "Invalid wave type"; break;
        default: return "Unkown error";
    }
}

char* synth_parser_getErrorString(synthParserCtx *ctx) {
    // If no error has occured, return nothing
    if (ctx->errorFlag == SYNTH_FALSE)
        return 0;
    
    if (ctx->errorCode == SYNTH_UNEXPECTED_TOKEN) {
        sprintf
            (
            parser_error,
            parser_def_msg,
            synth_lex_printToken(ctx->expected),
            synth_lex_printToken(ctx->gotten),
            synth_lex_getCurrentLine(ctx->lexCtx),
            synth_lex_getCurrentLinePosition(ctx->lexCtx),
            synth_lex_getLastCharacter(ctx->lexCtx)
            );
    }
    else {
        sprintf
            (
            parser_error,
            parser_rv_msg,
            synth_parser_getErrorMessage(ctx),
            synth_lex_getCurrentLine(ctx->lexCtx),
            synth_lex_getCurrentLinePosition(ctx->lexCtx),
            synth_lex_getLastCharacter(ctx->lexCtx)
            );
    }
    // Return the error string
    return parser_error;
}

