/**
 * @file @src/synth_parser.c
 */
#include <synth/synth_assert.h>
#include <synth/synth_errors.h>
#include <synth/synth_types.h>

#include <synth_internal/synth_lexer.h>
#include <synth_internal/synth_note.h>
#include <synth_internal/synth_parser.h>
#include <synth_internal/synth_types.h>
#include <synth_internal/synth_volume.h>

#include <stdlib.h>
#include <string.h>

#define TOKEN_MAX_STR 30
#define EXTRA_CHARS 5+6+2
/** Template used to generate a 'unespected token found' */
static char __synthParser_defaultMsg[] =
        "ERROR: Expected %s but got %s.\n"
        "       Line: %i\n"
        "       Position: %i\n"
        "       Last character: %c\n";
/** Template used to generate a generic error */
static char __synthParser_customMsg[] =
        "ERROR: %s\n"
        "       Line: %i\n"
        "       Position: %i\n"
        "       Last character: %c\n";
/** Error string returned to the user */
static char __synthParser_errorMsg[TOKEN_MAX_STR * 2 +
        sizeof(__synthParser_defaultMsg) + EXTRA_CHARS];

/**
 * Assert that the expected token was retrieved
 * 
 * A single parameter is needed (the expected token), but there must be a few
 * variables on the context, with the following names/types:
 *  - synthCtx *pCtx
 *  - synthParserCtx *pParser
 *  - synth_error rv
 * Also, there must be a label __err, before the function's return
 */
#define SYNTH_ASSERT_TOKEN(Expected) \
  do { \
    synth_token tk; \
    rv = synthLexer_lookupToken(&tk, &(pCtx->lexCtx)); \
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv); \
    if (tk != Expected) { \
      pParser->errorFlag = SYNTH_TRUE; \
      pParser->expected = Expected; \
      pParser->gotten = tk; \
      rv = SYNTH_UNEXPECTED_TOKEN; \
      goto __err; \
    } \
  } while (0)


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
synth_err synthParser_init(synthParserCtx *pParser, synthCtx *pCtx) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pParser, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);

    /* Remove any error flag */
    pParser->errorFlag = SYNTH_FALSE;
    pParser->bpm = 60;
    pParser->octave = 4;
    pParser->duration = 4;
    pParser->keyoff = 75;
    pParser->pan = 50;
    pParser->wave = W_SQUARE;

    /* Set the volume to half the maximum possible */
    rv = synthVolume_getConst(&(pParser->pVolume), pCtx, 0x7f);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);
    
    rv = SYNTH_OK;
__err:
    pParser->errorCode = rv;
    if (rv != SYNTH_OK) {
        pParser->errorFlag = SYNTH_TRUE;
    }
    return rv;
}

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
        synthCtx *pCtx) {
    char lastChar;
    int curLine, curPosition;
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(ppError, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pParser, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);
    /* Check that an error happened */
    SYNTH_ASSERT_ERR(pParser->errorFlag == SYNTH_TRUE, SYNTH_NO_ERRORS);

    /* Retrieve the current status of the lexer */
    rv = synthLexer_getCurrentLine(&curLine, &(pCtx->lexCtx));
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);
    rv = synthLexer_getCurrentLinePosition(&curPosition, &(pCtx->lexCtx));
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);
    rv = synthLexer_getLastCharacter(&lastChar, &(pCtx->lexCtx));
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    /* Generate an error message according to the error */
    if (pParser->errorCode == SYNTH_UNEXPECTED_TOKEN) {
        char *pExpected, *pGotten;

        /* Retrieve the names of the related tokens */
        rv = synthLexer_printToken(&pExpected, pParser->expected);
        SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);
        rv = synthLexer_printToken(&pGotten, pParser->gotten);
        SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

        /* Finally, generate the error string */
        sprintf(__synthParser_errorMsg, __synthParser_defaultMsg, pExpected,
                pGotten, curLine, curPosition, lastChar);
    }
    else {
        char *pError;

        /* Retrieve the error */
        switch (pParser->errorCode) {
            case SYNTH_EOF: {
                pError = "File ended before parsing ended";
            } break;
            case SYNTH_EOS: {
                pError = "Stream ended before parsing ended";
            } break;
            case SYNTH_UNEXPECTED_TOKEN: {
                pError = "Unexpected token";
            } break;
            case SYNTH_EMPTY_SEQUENCE: {
                    pError = "Got a track without notes";
            } break;
            case SYNTH_INVALID_WAVE: {
                pError = "Invalid wave type";
            } break;
            default: {
                pError = "Unkown error";
            }
        }

        /* Finally, generate the error string */
        sprintf(__synthParser_errorMsg, __synthParser_customMsg, pError,
                curLine, curPosition, lastChar);
    }

    /* Set the return */
    *ppError = __synthParser_errorMsg;
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Retrieve the first token on the song
 * 
 * Parsing rule: T_MML
 * 
 * This token doesn't produce any output, but it must be found before anything else
 * 
 * @param  [ in]pParser The parser context
 * @param  [ in]pCtx    The synthesizer context
 * @return              SYNTH_OK, SYNTH_UNEXPECTED_TOKEN
 */
static synth_err synthParser_mml(synthParserCtx *pParser, synthCtx *pCtx) {
    synth_err rv;

    /* Check that it's a MML token */
    SYNTH_ASSERT_TOKEN(T_MML);

    /* Read the next token */
    rv = synthLexer_getToken(&(pCtx->lexCtx));
    SYNTH_ASSERT(rv == SYNTH_OK);

    rv = SYNTH_OK;
__err:
    pParser->errorCode = rv;
    if (rv != SYNTH_OK) {
        pParser->errorFlag = SYNTH_TRUE;
    }
    return rv;
}

/**
 * Parse the audio's bpm, if any
 * 
 * Parsing rule: bmp = (T_SET_BPM T_NUMBER)?
 * 
 * @param  [ in]pParser The parser context
 * @param  [ in]pCtx    The synthesizer context
 * @return              SYNTH_OK, SYNTH_UNEXPECTED_TOKEN
 */
static synth_err synthParser_bpm(synthParserCtx *pParser, synthCtx *pCtx) {
    synth_err rv;
    synth_token token;

    /* Retrieve the current token */
    rv = synthLexer_lookupToken(&token, &(pCtx->lexCtx));
    SYNTH_ASSERT(rv == SYNTH_OK);

    if (token == T_SET_BPM) {
        /* If the token was found, try to read its value */
        rv = synthLexer_getToken(&(pCtx->lexCtx));
        SYNTH_ASSERT(rv == SYNTH_OK);
        /* The following token MUST be a T_NUMBER */
        SYNTH_ASSERT_TOKEN(T_NUMBER);

        /* Store the retrieved value */
        rv = synthLexer_getValuei(&(pParser->bpm), &(pCtx->lexCtx));
        SYNTH_ASSERT(rv == SYNTH_OK);

        /* Get the next token */
        rv = synthLexer_getToken(&(pCtx->lexCtx));
        SYNTH_ASSERT(rv == SYNTH_OK);
    }

    rv = SYNTH_OK;
__err:
    pParser->errorCode = rv;
    if (rv != SYNTH_OK) {
        pParser->errorFlag = SYNTH_TRUE;
    }
    return rv;
}

/**
 * Check if the next structure is a sequence
 * 
 * @param  [ in]pCtx The synthesizer context
 * @return           SYNTH_TRUE, SYNTH_FALSE
 */
static synth_bool synthParser_isSequence(synthCtx *pCtx) {
    synth_bool rv;
    synth_err srv;
    synth_token token;

    /* Get the next token */
    srv = synthLexer_lookupToken(&token, &(pCtx->lexCtx));
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, SYNTH_FALSE);

    /* Check if the next token belongs to a sequence */
    switch (token) {
        case T_SET_DURATION:
        case T_SET_OCTAVE:
        case T_SET_REL_OCTAVE:
        case T_SET_VOLUME:
        case T_OPEN_BRACKET:
        case T_CLOSE_BRACKET:
        case T_SET_KEYOFF:
        case T_SET_PAN:
        case T_SET_WAVE:
        case T_NOTE:
        case T_SET_LOOP_START:
            rv = SYNTH_TRUE;
        break;
        default:
            rv = SYNTH_FALSE;
    }

__err:
    return rv;
}

/**
 * Parse a note into the context
 * 
 * Parsing rule: note = T_NOTE T_NUMBER? T_DURATION?
 * 
 * @param  [ in]pParser   The parser context
 * @param  [ in]pCtx      The synthesizer context
 * @return                SYNTH_OK, SYNTH_UNEXPECTED_TOKEN, SYNTH_MEM_ERR
 */
static synth_err synthParser_note(synthParserCtx *pParser, synthCtx *pCtx) {
    synth_err rv;
    synth_note note;
    synthNote *pNote;
    synth_token token;
    int duration, octave, tmp;

    /* Callee function already assures this, but... */
    SYNTH_ASSERT_TOKEN(T_NOTE);

    /* Set initial duration to whatever the current default is */
    duration = pParser->duration;
    octave = pParser->octave;

    /* Store the note to be played */
    rv = synthLexer_getValuei(&tmp, &(pCtx->lexCtx));
    SYNTH_ASSERT(rv == SYNTH_OK);
    note = (synth_note)tmp;

    /* Adjuste the note's octave */
    if (note == N_CB) {
        note = N_B;
        octave--;
    }
    else if (note == N_BS) {
        note = N_C;
        octave++;
    }

    /* Get next token */
    rv = synthLexer_getToken(&(pCtx->lexCtx));
    SYNTH_ASSERT(rv == SYNTH_OK);

    /* Any number following the note will override the note's duration */
    synthLexer_lookupToken(&token, &(pCtx->lexCtx));
    SYNTH_ASSERT(rv == SYNTH_OK);

    if (token == T_NUMBER) {
        rv = synthLexer_getValuei(&duration, &(pCtx->lexCtx));
        SYNTH_ASSERT(rv == SYNTH_OK);

        /* Get the next token */
        rv = synthLexer_getToken(&(pCtx->lexCtx));
        SYNTH_ASSERT(rv == SYNTH_OK);
    }

    /* If there are any '.', add half the duration every time */
    synthLexer_lookupToken(&token, &(pCtx->lexCtx));
    SYNTH_ASSERT(rv == SYNTH_OK);
    if (token == T_DURATION) {
        int dots, d;

        rv = synthLexer_getValuei(&dots, &(pCtx->lexCtx));
        SYNTH_ASSERT(rv == SYNTH_OK);
        d = duration;

        /* For each consecutive bit in the duration, add half the current
         * duration */
        while (dots > 0) {
            d <<= 1;
            dots >>=1;

            duration = duration | d;
        }

        /* Read whatever the next token is */
        rv = synthLexer_getToken(&(pCtx->lexCtx));
        SYNTH_ASSERT(rv == SYNTH_OK);
    }

    /* Retrieve a new note */
    rv = synthNote_init(&pNote, pCtx);
    SYNTH_ASSERT(rv == SYNTH_OK);
    /* Initialize the note */
    rv = synthNote_setPan(pNote, pParser->pan);
    SYNTH_ASSERT(rv == SYNTH_OK);
    rv = synthNote_setOctave(pNote, octave);
    SYNTH_ASSERT(rv == SYNTH_OK);
    rv = synthNote_setNote(pNote, note);
    SYNTH_ASSERT(rv == SYNTH_OK);
    rv = synthNote_setWave(pNote, pParser->wave);
    SYNTH_ASSERT(rv == SYNTH_OK);
    rv = synthNote_setDuration(pNote, pCtx, pParser->bpm, duration);
    SYNTH_ASSERT(rv == SYNTH_OK);
    rv = synthNote_setKeyoff(pNote, pParser->keyoff);
    SYNTH_ASSERT(rv == SYNTH_OK);
    rv = synthNote_setVolume(pNote, pParser->pVolume);
    SYNTH_ASSERT(rv == SYNTH_OK);

    rv = SYNTH_OK;
__err:
    pParser->errorCode = rv;
    if (rv != SYNTH_OK) {
        pParser->errorFlag = SYNTH_TRUE;
    }
    return rv;
}

/**
 * Parse a 'context modification' into the context
 * 
 * Parsing rule: mod = T_SET_DURATION T_NUMBER |
 *                     T_SET_OCTAVE T_NUMBER |
 *                     T_SET_REL_OCTAVE T_NUMBER |
 *                     T_SET_VOLUME T_NUMBER |
 *                     T_SET_VOLUME T_OPEN_BRACKETS T_NUMBER T_COMMA T_NUMBER
 *                             T_CLOSE_BRACKETS |
 *                     T_OPEN_BRACKETS |      // for relative volume
 *                     T_CLOSE_BRACKETS |     // for relative volume
 *                     T_SET_KEYOFF T_NUMBER |
 *                     T_SET_PAN T_NUMBER |
 *                     T_SET_WAVE T_NUMBER
 * 
 * @param  [ in]pParser   The parser context
 * @param  [ in]pCtx      The synthesizer context
 * @return                SYNTH_OK, SYNTH_UNEXPECTED_TOKEN, SYNTH_MEM_ERR
 */
static synth_err synthParser_mod(synthParserCtx *pParser, synthCtx *pCtx) {
    synth_err rv;
    synth_token token;
    int tmp;

    /* Check which configuration should be set */
    rv = synthLexer_lookupToken(&token, &(pCtx->lexCtx));
    SYNTH_ASSERT(rv == SYNTH_OK);
    switch (token) {
        case T_SET_DURATION: {
            /* Read the following number */
            rv = synthLexer_getToken(&(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);
            SYNTH_ASSERT_TOKEN(T_NUMBER);

            /* Set the default duration for notes */
            rv = synthLexer_getValuei(&(pParser->duration), &(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);
        } break;
        case T_SET_OCTAVE: {
            /* Read the following number */
            rv = synthLexer_getToken(&(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);
            SYNTH_ASSERT_TOKEN(T_NUMBER);

            /* Set the octave for notes */
            rv = synthLexer_getValuei(&(pParser->octave), &(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);
        } break;
        case T_SET_REL_OCTAVE: {
            /* This token already have the amount to increase the octave, so
             * simply use it */

            /* Increase or decrease the octave */
            rv = synthLexer_getValuei(&tmp, &(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);
            pParser->octave += tmp;
        } break;
        case T_SET_VOLUME: {
            int isConst, vol1;

            /* Initialy, set the volume as linear */
            isConst = 1;

            /* Read the following number */
            rv = synthLexer_getToken(&(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);

            /* Check if it's a tuple (or something more complex, when supported
             * or a linear volume */
            rv = synthLexer_lookupToken(&token, &(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);
            if (token == T_OPEN_BRACKET) {
                /* Set the volume as not constant */
                isConst = 0;
                /* Read the following number */
                rv = synthLexer_getToken(&(pCtx->lexCtx));
                SYNTH_ASSERT(rv == SYNTH_OK);
            }

            /* Get the initial volume, whether it's constant or not */
            SYNTH_ASSERT_TOKEN(T_NUMBER);

            rv = synthLexer_getValuei(&vol1, &(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);

            /* Read the following token */
            rv = synthLexer_getToken(&(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);

            /* Parse the rest of a 'complex' volume */
            if (!isConst) {
                int vol2;

                /* Check there's a separator */
                SYNTH_ASSERT_TOKEN(T_COMMA);

                /* Read the following number */
                rv = synthLexer_getToken(&(pCtx->lexCtx));
                SYNTH_ASSERT(rv == SYNTH_OK);
                SYNTH_ASSERT_TOKEN(T_NUMBER);

                rv = synthLexer_getValuei(&vol2, &(pCtx->lexCtx));
                SYNTH_ASSERT(rv == SYNTH_OK);

                /* Check that the tuple ended */
                rv = synthLexer_getToken(&(pCtx->lexCtx));
                SYNTH_ASSERT(rv == SYNTH_OK);
                SYNTH_ASSERT_TOKEN(T_CLOSE_BRACKET);

                /* Initialize/Search the volume */
                rv = synthVolume_getLinear(&(pParser->pVolume), pCtx, vol1,
                        vol2);
                rv = SYNTH_FUNCTION_NOT_IMPLEMENTED;
                SYNTH_ASSERT(rv == SYNTH_OK);
            }
            else {
                /* Simply initialize/search the constant volume */
                rv = synthVolume_getConst(&(pParser->pVolume), pCtx, vol1);
                SYNTH_ASSERT(rv == SYNTH_OK);
            }
            SYNTH_ASSERT(rv == SYNTH_OK);
        } break;
        case T_OPEN_BRACKET: {
            /* TODO >__< */
            rv = SYNTH_FUNCTION_NOT_IMPLEMENTED;
            SYNTH_ASSERT(rv == SYNTH_OK);
        } break;
        case T_CLOSE_BRACKET: {
            /* TODO >__< */
            rv = SYNTH_FUNCTION_NOT_IMPLEMENTED;
            SYNTH_ASSERT(rv == SYNTH_OK);
        } break;
        case T_SET_KEYOFF: {
            /* Read the following number */
            rv = synthLexer_getToken(&(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);
            SYNTH_ASSERT_TOKEN(T_NUMBER);

            /* Set the keyoff value */
            rv = synthLexer_getValuei(&(pParser->keyoff), &(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);
        } break;
        case T_SET_PAN: {
            /* Read the following number */
            rv = synthLexer_getToken(&(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);
            SYNTH_ASSERT_TOKEN(T_NUMBER);

            /* Set the pan */
            rv = synthLexer_getValuei(&(pParser->pan), &(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);
        } break;
        case T_SET_WAVE: {
            /* Read the following number */
            rv = synthLexer_getToken(&(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);
            SYNTH_ASSERT_TOKEN(T_NUMBER);

            /* Parse the number into a wave */
            rv = synthLexer_getValuei(&tmp, &(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);
            switch (tmp) {
                case 0: pParser->wave = W_SQUARE; break;
                case 1: pParser->wave = W_PULSE_12_5; break;
                case 2: pParser->wave = W_PULSE_25; break;
                case 3: pParser->wave = W_PULSE_75; break;
                case 4: pParser->wave = W_TRIANGLE; break;
                case 5: pParser->wave = W_NOISE; break;
                default: SYNTH_ASSERT_ERR(0, SYNTH_INVALID_WAVE);
            }
        } break;
        default: {
            SYNTH_ASSERT_ERR(0, SYNTH_UNEXPECTED_TOKEN);
        }
    }

    /* Read the next token */
    rv = synthLexer_getToken(&(pCtx->lexCtx));
    SYNTH_ASSERT(rv == SYNTH_OK);

    rv = SYNTH_OK;
__err:
    pParser->errorCode = rv;
    if (rv != SYNTH_OK) {
        pParser->errorFlag = SYNTH_TRUE;
    }
    return rv;
}

/* Forward declaration of 'synthParser_sequence', since synthParser_loop
 * requires this */
static synth_err synthParser_sequence(int *pNumNotes, synthParserCtx *pParser,
        synthCtx *pCtx);

/**
 * Parse a loop into the context
 * 
 * Parsing rule: T_SET_LOOP_START sequence T_SET_LOOP_END T_NUMBER?
 * 
 * @param  [out]pNumNotes The total number of notes in this track, must have
 *                        been initialized before hand!
 * @param  [ in]pParser   The parser context
 * @param  [ in]pCtx      The synthesizer context
 * @return                SYNTH_OK, SYNTH_UNEXPECTED_TOKEN, SYNTH_MEM_ERR
 */
synth_err synthParser_loop(int *pNumNotes, synthParserCtx *pParser,
        synthCtx *pCtx) {
    int count, loopPosition;
    synth_err rv;
    synthNote *pNote;
    synth_token token;

    /* We're sure to have this token, but... */
    SYNTH_ASSERT_TOKEN(T_SET_LOOP_START);

    /* Set basic loop count to 2 (default) */
    count = 2;

    /* Store the current position in track (so the loop position can be set
     * later on */
    loopPosition = *pNumNotes;

    /* Read the next token */
    rv = synthLexer_getToken(&(pCtx->lexCtx));
    SYNTH_ASSERT(rv == SYNTH_OK);
    /* After the loop start, there must be a sequence */
    SYNTH_ASSERT_ERR(synthParser_isSequence(pCtx) == SYNTH_TRUE,
        SYNTH_INVALID_TOKEN);
    
    /* Parse the sequence */
    rv = synthParser_sequence(pNumNotes, pParser, pCtx);
    SYNTH_ASSERT(rv == SYNTH_OK);

    /* Afterwards, a loop end must come */
    SYNTH_ASSERT_TOKEN(T_SET_LOOP_END);

    /* Get the next token */
    rv = synthLexer_getToken(&(pCtx->lexCtx));
    SYNTH_ASSERT(rv == SYNTH_OK);

    /* If the next token is a number, it's how many times the loop runs */
    synthLexer_lookupToken(&token, &(pCtx->lexCtx));
    SYNTH_ASSERT(rv == SYNTH_OK);
    if (token == T_NUMBER) {
        // Store the loop count
        rv = synthLexer_getValuei(&count, &(pCtx->lexCtx));
        SYNTH_ASSERT(rv == SYNTH_OK);

        // Get the next token
        rv = synthLexer_getToken(&(pCtx->lexCtx));
        SYNTH_ASSERT(rv == SYNTH_OK);
    }

    /* Add a 'loop note' to the track */
    rv = synthNote_initLoop(&pNote, pCtx, count, loopPosition);
    SYNTH_ASSERT(rv == SYNTH_OK);

    /* Increase the number of notes in the track */
    (*pNumNotes)++;

    rv = SYNTH_OK;
__err:
    pParser->errorCode = rv;
    if (rv != SYNTH_OK) {
        pParser->errorFlag = SYNTH_TRUE;
    }
    return rv;
}

/**
 * Parse a sequence into the context
 * 
 * Parsing rule: sequence = ( mod | note | loop )+
 * 
 * @param  [out]pNumNotes The total number of notes in this track, must have
 *                        been initialized before hand!
 * @param  [ in]pParser   The parser context
 * @param  [ in]pCtx      The synthesizer context
 * @return                SYNTH_OK, SYNTH_UNEXPECTED_TOKEN, SYNTH_MEM_ERR
 */
static synth_err synthParser_sequence(int *pNumNotes, synthParserCtx *pParser,
        synthCtx *pCtx) {
    synth_err rv;

    /* Make sure the next token is part of a sequence */
    SYNTH_ASSERT_ERR(synthParser_isSequence(pCtx) == SYNTH_TRUE,
        SYNTH_UNEXPECTED_TOKEN);

    /* Fun stuff. Lookup next token and do whatever it requires */
    while (synthParser_isSequence(pCtx) == SYNTH_TRUE) {
        synth_token token;

        synthLexer_lookupToken(&token, &(pCtx->lexCtx));
        SYNTH_ASSERT(rv == SYNTH_OK);

        /* Anything not a note or loop will be parsed as mod */
        switch (token) {
            case T_NOTE:
                /* Simply parse the current note */
                rv = synthParser_note(pParser, pCtx);
                (*pNumNotes)++;
            break;
            case T_SET_LOOP_START:
                /* Recursively parse a sub-sequence */
                rv = synthParser_loop(pNumNotes, pParser, pCtx);
            break;
            default:
                /* Modify the current context in some way */
                rv = synthParser_mod(pParser, pCtx);
        }
        SYNTH_ASSERT(rv == SYNTH_OK);
    }

    rv = SYNTH_OK;
__err:
    pParser->errorCode = rv;
    if (rv != SYNTH_OK) {
        pParser->errorFlag = SYNTH_TRUE;
    }
    return rv;
}

/**
 * Parse a track into the context
 * 
 * Parsing rule: sequence | sequence? T_SET_LOOPPOINT sequence
 * 
 * @param  [out]pTrack The parsed track handle
 * @param  [ in]pParam The parser context
 * @param  [ in]pCtx   The synthesizer context
 * @return              SYNTH_OK, SYNTH_UNEXPECTED_TOKEN, SYNTH_MEM_ERR
 */
static synth_err synthParser_track(int *pTrack, synthParserCtx *pParser,
        synthCtx *pCtx) {
    synth_err rv;
    synth_token token;
    int curTrack, didFindSequence, numNotes;

    /* Make sure there's enough space for another track */
    SYNTH_ASSERT_ERR(pCtx->tracks.max == 0 ||
            pCtx->tracks.used < pCtx->tracks.max, SYNTH_MEM_ERR);
    /* Retrieve the next track */
    if (pCtx->tracks.used >= pCtx->tracks.len) {
        /* 'Double' the current buffer; Note that this will never be called if
         * the context was pre-alloc'ed, since 'max' will be set; The '+1' is
         * for the first audio, in which len will be 0 */
        pCtx->tracks.buf.pTracks = (synthTrack*)realloc(
                pCtx->tracks.buf.pTracks, (1 + pCtx->tracks.len * 2) *
                sizeof(synthTrack));
        SYNTH_ASSERT_ERR(pCtx->tracks.buf.pTracks, SYNTH_MEM_ERR);
        /* Clear only the new part of the buffer */
        memset(&(pCtx->tracks.buf.pTracks[pCtx->tracks.used]), 0x0,
                (1 + pCtx->tracks.len) * sizeof(synthTrack));
        /* Actually increase the buffer length */
        pCtx->tracks.len += 1 + pCtx->tracks.len;
    }
    curTrack = pCtx->tracks.used;

    /* Initialize the track as not being looped and without any notes */
    pCtx->tracks.buf.pTracks[curTrack].loopPoint = -1;
    numNotes = 0;
    /* Also, set the index of the first note */
    pCtx->tracks.buf.pTracks[curTrack].notesIndex = pCtx->notes.used;

    didFindSequence = 0;
    /* The first token may either be a sequence or a T_SET_LOOPPOINT */
    if (synthParser_isSequence(pCtx) == SYNTH_TRUE) {
        /* Parse a sequence */
        rv = synthParser_sequence(&numNotes, pParser, pCtx);
        SYNTH_ASSERT(rv == SYNTH_OK);

        didFindSequence = 1;
    }

    /* If no sequence was found, then the next token must be T_SET_LOOPPOINT */
    if (!didFindSequence) {
        SYNTH_ASSERT_TOKEN(T_SET_LOOPPOINT);
    }

    /* Parse the looped sequence */
    rv = synthLexer_lookupToken(&token, &(pCtx->lexCtx));
    SYNTH_ASSERT(rv == SYNTH_OK);
    if (token == T_SET_LOOPPOINT) {
        /* Set the current position into the track as the looppoint */
        pCtx->tracks.buf.pTracks[curTrack].loopPoint = numNotes;

        /* Get the next token (since the previous was a T_SET_LOOPPOINT) */
        rv = synthLexer_getToken(&(pCtx->lexCtx));
        SYNTH_ASSERT(rv == SYNTH_OK);

        /* Parse the looping sequence */
        rv = synthParser_sequence(&numNotes, pParser, pCtx);
        SYNTH_ASSERT(rv == SYNTH_OK);
    }

    /* Store the total number of notes in the track */
    pCtx->tracks.buf.pTracks[curTrack].num = numNotes;

    pCtx->tracks.used++;
    *pTrack = curTrack;
    rv = SYNTH_OK;
__err:
    pParser->errorCode = rv;
    if (rv != SYNTH_OK) {
        pParser->errorFlag = SYNTH_TRUE;
    }
    return rv;
}

/**
 * Parse tracks into the context
 * 
 * Parsing rule: tracks = track ( T_END_OF_TRACK track )*
 * 
 * @param  [ in]pParam The parser context
 * @param  [ in]pCtx   The synthesizer context
 * @param  [ in]pAudio The audio
 * @return              SYNTH_OK, SYNTH_UNEXPECTED_TOKEN, SYNTH_MEM_ERR
 */
static synth_err synthParser_tracks(synthParserCtx *pParser, synthCtx *pCtx,
        synthAudio *pAudio) {
    int track;
    synth_err rv;
    synth_token token;

    /* Parse the first track */
    rv = synthParser_track(&track, pParser, pCtx);
    SYNTH_ASSERT(rv == SYNTH_OK);

    /* Set the audio's first track */
    pAudio->tracksIndex = track;
    pAudio->num = 1;

    /* Check if there are any tracks to be parsed */
    rv = synthLexer_lookupToken(&token, &(pCtx->lexCtx));
    SYNTH_ASSERT(rv == SYNTH_OK);
    while (token == T_END_OF_TRACK) {
        /* Parse every other track */
        rv = synthLexer_getToken(&(pCtx->lexCtx));
        SYNTH_ASSERT(rv == SYNTH_OK);

        /* Parse the current track (note that the track index will be ignored */
        rv = synthParser_track(&track, pParser, pCtx);
        SYNTH_ASSERT(rv == SYNTH_OK);

        /* Simply increase the number of tracks, since they are retrieved
         * sequentially */
        pAudio->num++;

        /* Get the next token */
        rv = synthLexer_lookupToken(&token, &(pCtx->lexCtx));
        SYNTH_ASSERT(rv == SYNTH_OK);
    }

    rv = SYNTH_OK;
__err:
    pParser->errorCode = rv;
    if (rv != SYNTH_OK) {
        pParser->errorFlag = SYNTH_TRUE;
    }
    return rv;
}

/**
 * Parse the currently loaded file into an audio
 * 
 * Parsing rule: T_MML bmp tracks
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
 * @return              SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_UNEXPECTED_TOKEN,
 *                      SYNTH_MEM_ERR, ...
 */
synth_err synthParser_getAudio(synthParserCtx *pParser, synthCtx *pCtx,
        synthAudio *pAudio) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pAudio, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pParser, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);

    /* Read the first token */
    rv = synthLexer_getToken(&(pCtx->lexCtx));
    SYNTH_ASSERT(rv == SYNTH_OK);

    /* Check that its actually a MML song */
    rv = synthParser_mml(pParser, pCtx);
    SYNTH_ASSERT(rv == SYNTH_OK);

    /* Parse the bpm (optional token) */
    rv = synthParser_bpm(pParser, pCtx);
    SYNTH_ASSERT(rv == SYNTH_OK);

    /* Parse every track in this audio */
    rv = synthParser_tracks(pParser, pCtx, pAudio);
    SYNTH_ASSERT(rv == SYNTH_OK);

    /* Check that parsing finished */
    SYNTH_ASSERT_TOKEN(T_DONE);

    rv = SYNTH_OK;
__err:
    pParser->errorCode = rv;
    if (rv != SYNTH_OK) {
        pParser->errorFlag = SYNTH_TRUE;
    }
    return rv;
}

#if 0
#include <synth/synth_audio.h>
#include <synth_internal/synth_cache.h>

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
    
    if ((*ctx)->audio)
        synth_audio_free(&((*ctx)->audio));
    
    if ((*ctx)->track)
        synth_track_clean((*ctx)->track);
    
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
    tmp->keyoff = 75;
    tmp->pan = 50;
    tmp->wave = W_SQUARE;
    
    tmp->audio = 0;
    tmp->track = 0;
    
    rv = synth_cache_getVolume(&(tmp->vol), 0x7f, 0x7f);
    SYNTH_ASSERT(rv == SYNTH_OK);
    
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
    
    // Create the audio structure
COMPILER rv = synth_audio_allocAudio(&(ctx->audio));
COMPILER SYNTH_ASSERT(rv == SYNTH_OK);
    
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
        synth_audio_free(&(ctx->audio));
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
    
COMPILER rv = synth_audio_addTrack(ctx->audio, ctx->track);
COMPILER SYNTH_ASSERT(rv == SYNTH_OK);
COMPILER ctx->track = 0;
    
    // Parse every other track
    while (synth_lex_lookupToken(ctx->lexCtx) == T_END_OF_TRACK) {
        rv = synth_lex_getToken(ctx->lexCtx);
        SYNTH_ASSERT(rv == SYNTH_OK);
        
        rv = synth_parser_track(ctx);
        SYNTH_ASSERT(rv == SYNTH_OK);
        
COMPILER rv = synth_audio_addTrack(ctx->audio, ctx->track);
COMPILER SYNTH_ASSERT(rv == SYNTH_OK);
COMPILER ctx->track = 0;
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
        case T_OPEN_BRACKET:
        case T_CLOSE_BRACKET:
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
    int notes;
    
    notes = 0;
    // Loop is required unless there was a sequence
    loopRequired = 1;
    
COMPILER ctx->track = (synthTrack*)malloc(sizeof(synthTrack));
COMPILER rv = synth_track_init(ctx->track);
COMPILER SYNTH_ASSERT(rv == SYNTH_OK);
    
    // Parse a sequence, if it's the next token
    if (synth_parser_isSequence(ctx) == SYNTH_TRUE) {
        rv = synth_parser_sequence(ctx, &notes);
        SYNTH_ASSERT(rv == SYNTH_OK);
        
        // Set loop as not required
        if (notes > 0)
            loopRequired = 0;
    }
    
    // If no sequence has been read, a LOOPPOINT token is required
    if (loopRequired)
        SYNTH_ASSERT_TOKEN(T_SET_LOOPPOINT);
    
    // Parse another sequence, if there's a looppoint token
    if (synth_lex_lookupToken(ctx->lexCtx) == T_SET_LOOPPOINT) {
COMPILER int loopPoint;
        
        // Get the next token
        rv = synth_lex_getToken(ctx->lexCtx);
        SYNTH_ASSERT(rv == SYNTH_OK);
        
COMPILER loopPoint = synth_track_getLength(ctx->track);
COMPILER synth_track_setLoopPoint(ctx->track, loopPoint);
        
        // Parse the looping sequence
        rv = synth_parser_sequence(ctx, &notes);
        SYNTH_ASSERT(rv == SYNTH_OK);
    }
    
    SYNTH_ASSERT_ERR(notes > 0, SYNTH_EMPTY_SEQUENCE);
    
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Parse a sequence into the context
 * Parsing rule: ( mod | note | loop )+
 * 
 * @param ctx The context
 * @param notes How many notes were added this sequence
 * @return Error code
 */
synth_err synth_parser_sequence(synthParserCtx *ctx, int *notes) {
    synth_err rv;
    
    // Next token must be one of sequence
    SYNTH_ASSERT_ERR(synth_parser_isSequence(ctx) == SYNTH_TRUE,
        SYNTH_UNEXPECTED_TOKEN);
    
    // Fun stuff. Lookup next token and do whatever it requires
    while (synth_parser_isSequence(ctx) == SYNTH_TRUE) {
        // Anything not a note or loop will be parsed as mod
        switch (synth_lex_lookupToken(ctx->lexCtx)) {
            case T_NOTE:
                rv = synth_parser_note(ctx);
                (*notes)++;
            break;
            case T_SET_LOOP_START:
                rv = synth_parser_loop(ctx, notes);
            break;
            default:
                rv = synth_parser_mod(ctx);
        }
        SYNTH_ASSERT(rv == SYNTH_OK);
    }
    
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Parse a loop into the context
 * Parsing rule: T_SET_LOOP_START sequence T_SET_LOOP_END T_NUMBER?
 * 
 * @param ctx The context
 * @param notes How many notes there are in this loop
 * @return Error code
 */
synth_err synth_parser_loop(synthParserCtx *ctx, int *notes) {
COMPILER int loopPosition;
COMPILER synthNote *loop;
COMPILER int count;
    synth_err rv;
    
    // We're sure to have this token, but...
    SYNTH_ASSERT_TOKEN(T_SET_LOOP_START);
    
    // Set basic loop count to 2 (default)
COMPILER count = 2;
    
    // Store the current position in track to set it on the loop 'note'
COMPILER loopPosition = synth_track_getLength(ctx->track);
    
    // Read the next token
    rv = synth_lex_getToken(ctx->lexCtx);
    SYNTH_ASSERT(rv == SYNTH_OK);
    
    // After the loop start, there must be a sequence
    SYNTH_ASSERT_ERR(synth_parser_isSequence(ctx) == SYNTH_TRUE,
        SYNTH_INVALID_TOKEN);
    
    // Parse the sequence
    rv = synth_parser_sequence(ctx, notes);
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
COMPILER rv = synth_cache_getLoop(&loop, loopPosition, count);
COMPILER SYNTH_ASSERT(rv == SYNTH_OK);
COMPILER rv = synth_track_addNote(ctx->track, loop);
COMPILER SYNTH_ASSERT(rv == SYNTH_OK);
    
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
        case T_SET_VOLUME: {
            int vol1, vol2 = -1;
            
            // Read the following number
            rv = synth_lex_getToken(ctx->lexCtx);
            SYNTH_ASSERT(rv == SYNTH_OK);
            
            if (synth_lex_lookupToken(ctx->lexCtx) == T_OPEN_BRACKET) {
                // Read the following number
                rv = synth_lex_getToken(ctx->lexCtx);
                SYNTH_ASSERT(rv == SYNTH_OK);
                
                SYNTH_ASSERT_TOKEN(T_NUMBER);
                vol1 = synth_lex_getValuei(ctx->lexCtx);
                
                // Read the following number
                rv = synth_lex_getToken(ctx->lexCtx);
                SYNTH_ASSERT(rv == SYNTH_OK);
                
                // TODO parse a comma
                
                // Read the following number
                rv = synth_lex_getToken(ctx->lexCtx);
                SYNTH_ASSERT(rv == SYNTH_OK);
                
                SYNTH_ASSERT_TOKEN(T_NUMBER);
                vol2 = synth_lex_getValuei(ctx->lexCtx);
                
                // Read the following number
                rv = synth_lex_getToken(ctx->lexCtx);
                SYNTH_ASSERT(rv == SYNTH_OK);
                SYNTH_ASSERT_TOKEN(T_CLOSE_BRACKET);
            }
            else {
                // If a '(' wasn't found, then there must be a number
                SYNTH_ASSERT_TOKEN(T_NUMBER);
                
                vol1 = synth_lex_getValuei(ctx->lexCtx);
            }
            
COMPILER if (vol2 != -1) {
COMPILER    rv = synth_cache_getVolume(&(ctx->vol), vol1, vol2);
COMPILER }
COMPILER else
COMPILER    rv = synth_cache_getVolume(&(ctx->vol), vol1, vol1);
COMPILER SYNTH_ASSERT(rv == SYNTH_OK);
        } break;
        case T_OPEN_BRACKET:
COMPILER // TODO >__<
        break;
        case T_CLOSE_BRACKET:
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
COMPILER synthNote *pNote;
    synth_err rv;
    synth_note note;
    int duration, octave;
    
    // Callee function already assures this, but...
    SYNTH_ASSERT_TOKEN(T_NOTE);
    
    // Set initial duration to whatever the default is
    duration = ctx->duration;
    octave = ctx->octave;
    
    // Store the note to be played
    note = (synth_note)synth_lex_getValuei(ctx->lexCtx);
    
    // Adjuste the note
    if (note == N_CB) {
        note = N_B;
        octave--;
    }
    else if (note == N_BS) {
        note = N_C;
        octave++;
    }
    
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
    if (synth_lex_lookupToken(ctx->lexCtx) == T_DURATION) {
        int dots, d;
        
        dots = synth_lex_getValuei(ctx->lexCtx);
        d = duration;
        
        // For each '1' in the duration, add half the current duration
        // TODO account for bizare durations?
        while (dots > 0) {
            d <<= 1;
            dots >>=1;
            
            duration = duration | d;
        }
        
        // Read whatever the next token is
        rv = synth_lex_getToken(ctx->lexCtx);
        SYNTH_ASSERT(rv == SYNTH_OK);
    }
    
COMPILER rv = synth_cache_getNote
COMPILER        (
COMPILER        &pNote,
COMPILER        note,
COMPILER        duration,
COMPILER        ctx->bpm,
COMPILER        octave,
COMPILER        ctx->pan,
COMPILER        ctx->keyoff,
COMPILER        ctx->vol,
COMPILER        ctx->wave
COMPILER        );
COMPILER SYNTH_ASSERT(rv == SYNTH_OK);
COMPILER rv = synth_track_addNote(ctx->track, pNote);
COMPILER SYNTH_ASSERT(rv == SYNTH_OK);
    
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

/**
 * Get the last parsed audio
 * 
 * @param ctx The context
 * @return The parsed audio
 */
synthAudio *synth_parser_getAudio(synthParserCtx *ctx) {
    synthAudio *aud;
    
    aud = ctx->audio;
    ctx->audio = 0;
    
    return aud;
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

#endif /* 0 */

