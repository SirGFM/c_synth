/**
 * @file @src/synth_parser.c
 */
#include <c_synth/synth_assert.h>
#include <c_synth/synth_errors.h>

#include <c_synth_internal/synth_lexer.h>
#include <c_synth_internal/synth_note.h>
#include <c_synth_internal/synth_parser.h>
#include <c_synth_internal/synth_track.h>
#include <c_synth_internal/synth_types.h>
#include <c_synth_internal/synth_volume.h>

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
 * Revert the parser back to its default configuration
 * 
 * @param  [ in]pParser The parser context to be initialized
 * @param  [ in]pCtx    The synthesizer context
 * @return              SYNTH_OK, SYNTH_MEM_ERR
 */
static synth_err synthParser_setDefault(synthParserCtx *pParser, synthCtx *pCtx) {
    synth_err rv;

    pParser->ctl.octave = 4;
    pParser->ctl.duration = 4;
    pParser->ctl.attack = 0;
    pParser->ctl.keyoff = 75;
    pParser->ctl.release = 0;
    pParser->ctl.pan = 50;
    pParser->ctl.wave = W_SQUARE;
    /* Set the time signature to a whole note ('brevissima'); This should work
     * for any simple time signature (1/4, 2/4, 4/4 etc) */
    pParser->timeSignature = 1 << 6;
    pParser->curCompassLength = 0;
    rv = synthVolume_getConst(&(pParser->ctl.volume), pCtx, 64);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    rv = SYNTH_OK;
__err:
    return rv;
}

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

    memset(pParser->macros, 0x0, sizeof(pParser->macros));

    /* Remove any error flag */
    pParser->errorFlag = SYNTH_FALSE;
    rv = synthParser_setDefault(pParser, pCtx);
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
            case SYNTH_COMPASS_OVERFLOW: {
                pError = "Compass duration overflown";
            } break;
            case SYNTH_BAD_LOOP_START: {
                pError = "Loop start didn't sync with compass start";
            } break;
            case SYNTH_BAD_LOOP_END: {
                pError = "Loop end didn't sync with compass end";
            } break;
            case SYNTH_BAD_LOOP_POINT: {
                pError = "Loop point didn't sync with compass start";
            } break;
            case SYNTH_BAD_VERSION: {
                pError = "Invalid MML version";
            } break;
            case SYNTH_BAD_MACRO: {
                pError = "Invalid macro declaration: multiple notes or none";
            } break;
            case SYNTH_UNDEF_MACRO: {
                pError = "Undefined macro";
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
 * Parsing rule: T_MML | T_NEW_MML T_NUMBER
 * 
 * This token doesn't produce any output, but it must be found before anything
 * else. T_NEW_MML may be used to specify a newer MML version (e.g., one with
 * macro and envelope support).
 * 
 * @param  [ in]pParser The parser context
 * @param  [ in]pCtx    The synthesizer context
 * @return              SYNTH_OK, SYNTH_UNEXPECTED_TOKEN
 */
static synth_err synthParser_mml(synthParserCtx *pParser, synthCtx *pCtx) {
    synth_err rv;
    synth_token token;

    rv = synthLexer_lookupToken(&token, &(pCtx->lexCtx));
    SYNTH_ASSERT(rv == SYNTH_OK);
    switch (token) {
        case T_MML: {
            pParser->useNewEnvelope = SYNTH_FALSE;
        } break;
        case T_NEW_MML: {
            int val;

            rv = synthLexer_getToken(&(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);
            SYNTH_ASSERT_TOKEN(T_NUMBER);

            rv = synthLexer_getValuei(&val, &(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);

            switch (val) {
                case 0:
                case 1: {
                    pParser->useNewEnvelope = SYNTH_FALSE;
                } break;
                case 2: {
                    pParser->useNewEnvelope = SYNTH_TRUE;
                } break;
                default: {
                    SYNTH_ASSERT_ERR(0, SYNTH_BAD_VERSION);
                }
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
    return rv;
}

/**
 * Parse the audio's bpm, if any
 * 
 * Parsing rule: bmp = (T_SET_BPM T_NUMBER)?
 * 
 * @param  [ in]pParser The parser context
 * @param  [ in]pCtx    The synthesizer context
 * @param  [ in]pAudio  The audio
 * @return              SYNTH_OK, SYNTH_UNEXPECTED_TOKEN
 */
static synth_err synthParser_bpm(synthParserCtx *pParser, synthCtx *pCtx,
        synthAudio *pAudio) {
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
        rv = synthLexer_getValuei(&(pAudio->bpm), &(pCtx->lexCtx));
        SYNTH_ASSERT(rv == SYNTH_OK);

        /* Get the next token */
        rv = synthLexer_getToken(&(pCtx->lexCtx));
        SYNTH_ASSERT(rv == SYNTH_OK);
    }

    rv = SYNTH_OK;
__err:
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
        case T_SET_ATTACK:
        case T_SET_KEYOFF:
        case T_SET_RELEASE:
        case T_SET_PAN:
        case T_SET_WAVE:
        case T_NOTE:
        case T_SET_LOOP_START:
        case T_SET_ENVELOPE:
        case T_DECL_MACRO:
        case T_MACRO_ID:
            rv = SYNTH_TRUE;
        break;
        default:
            rv = SYNTH_FALSE;
    }

__err:
    return rv;
}

/**
 * Output a note into the current compass (from a note controller). Afterwards,
 * check whether the note duration matches the compass.
 *
 * @param  [ in]pParser The parser context
 * @param  [ in]pCtx The synthesizer context
 * @param  [out]pNote The note being read
 */
static synth_err synthParser_outputCtldNote(synthParserCtx *pParser,
        synthCtx *pCtx, synthNote *pNote) {
    synthNote *pOutNote;
    synth_err rv;
    int duration;

    /* Retrieve a new note */
    rv = synthNote_init(&pOutNote, pCtx);
    SYNTH_ASSERT(rv == SYNTH_OK);

    memset(pOutNote, 0x0, sizeof(synthNote));
    memcpy(pOutNote, pNote, sizeof(struct stSynthNoteCtl));

    /* Update the position within the compass */
    rv = synthNote_getDuration(&duration, pOutNote);
    pParser->curCompassLength += duration;
    SYNTH_ASSERT_ERR(pParser->curCompassLength <= pParser->timeSignature,
            SYNTH_COMPASS_OVERFLOW);
    if (pParser->curCompassLength == pParser->timeSignature) {
        /* Reset the compass if we just reached the next one */
        pParser->curCompassLength = 0;
    }

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Initializes a non-extended note control from the current parser.
 *
 * @param  [ in]pParser   The parser context
 * @param  [ in]pCtx      The synthesizer context
 * @param  [out]pNote     The note getting initialized
 * @param  [ in]octave    The note's octave
 * @param  [ in]note      The note to be played (e.g., C, A)
 * @param  [ in]duration  The note's duration (in fixed point)
 */
static synth_err synthParser_getNoteCtl(synthParserCtx *pParser,
        synthNote *pNote, int octave, synth_note note, int duration) {
    synth_err rv;

    rv = synthNote_setDefault(pNote);
    SYNTH_ASSERT(rv == SYNTH_OK);

    rv = synthNote_setPan(pNote, pParser->ctl.pan);
    SYNTH_ASSERT(rv == SYNTH_OK);
    rv = synthNote_setOctave(pNote, octave);
    SYNTH_ASSERT(rv == SYNTH_OK);
    rv = synthNote_setNote(pNote, note);
    SYNTH_ASSERT(rv == SYNTH_OK);
    rv = synthNote_setWave(pNote, pParser->ctl.wave);
    SYNTH_ASSERT(rv == SYNTH_OK);
    rv = synthNote_setDuration(pNote, duration);
    SYNTH_ASSERT(rv == SYNTH_OK);
    rv = synthNote_setKeyoff(pNote, pParser->ctl.attack,
            pParser->ctl.keyoff, pParser->ctl.release);
    SYNTH_ASSERT(rv == SYNTH_OK);
    rv = synthNote_setVolume(pNote, pParser->ctl.volume);
    SYNTH_ASSERT(rv == SYNTH_OK);

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Do actually output the note and check if the note duration matches the compass
 *
 * @param  [ in]pParser   The parser context
 * @param  [ in]pCtx      The synthesizer context
 * @param  [ in]doExtend  Whether the should is extended, and in which part
 * @param  [ in]octave    The note's octave
 * @param  [ in]note      The note to be played (e.g., C, A)
 * @param  [ in]duration  The note's duration (in fixed point)
 */
static synth_err synthParser_outputNote(synthParserCtx *pParser, synthCtx *pCtx,
        int doExtend, int octave, synth_note note, int duration) {
    synth_err rv;
    synthNote stNote;

    /* Initialize the note */
    rv = synthParser_getNoteCtl(pParser, &stNote, octave, note, duration);
    SYNTH_ASSERT(rv == SYNTH_OK);
    /* NOTE: First part of extended notes is always done in synthParser_note */
    if (doExtend == 2) {
        /* Seconds part of extended note: play it fully */
        rv = synthNote_setKeyoff(&stNote, 0, 100, 100);
    }
    else if (doExtend == 3) {
        /* Last part of extended note: set keyoff and release */
        rv = synthNote_setKeyoff(&stNote, 0, pParser->ctl.keyoff,
                pParser->ctl.release);
    }
    SYNTH_ASSERT(rv == SYNTH_OK);

    rv = synthParser_outputCtldNote(pParser, pCtx, &stNote);
    SYNTH_ASSERT(rv == SYNTH_OK);

__err:
    return rv;
}

/**
 * Retrieve a duration with any number of 'dots'
 *
 * @param  [out]pDuration the retrieved duration
 * @param  [ in]pParser   The parser context
 * @param  [ in]pCtx      The synthesizer context
 * @return                SYNTH_OK, SYNTH_UNEXPECTED_TOKEN
 */
static synth_err synthParser_getDuration(int *pDuration,
        synthParserCtx *pParser, synthCtx *pCtx) {
    int duration;
    synth_err rv;
    synth_token token;

    rv = synthLexer_lookupToken(&token, &(pCtx->lexCtx));
    SYNTH_ASSERT(rv == SYNTH_OK);
    if (token != T_NUMBER && token != T_DURATION) {
        SYNTH_ASSERT_ERR(0, SYNTH_UNEXPECTED_TOKEN);
    }

    /* Retrieve the basic duration */
    if (token == T_NUMBER) {
        rv = synthLexer_getValuei(&duration, &(pCtx->lexCtx));
        SYNTH_ASSERT(rv == SYNTH_OK);

        /* Get the next token */
        rv = synthLexer_getToken(&(pCtx->lexCtx));
        SYNTH_ASSERT(rv == SYNTH_OK);
    }
    else {
        duration = pParser->ctl.duration;
    }

    /* If there are any '.', add half the duration every time */
    rv = synthLexer_lookupToken(&token, &(pCtx->lexCtx));
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

    *pDuration = duration;
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Parse a note into a note controller.
 *
 * Parsing rule: noteCtl = T_NOTE T_NUMBER? T_DURATION?
 *
 * @param  [ in]pParser   The parser context
 * @param  [ in]pCtx      The synthesizer context
 * @param  [out]pNote     The parsed note
 */
static synth_err synthParser_noteCtl(synthParserCtx *pParser, synthCtx *pCtx,
        synthNote *pNote) {
    synth_err rv;
    synth_note note;
    synth_token token;
    int duration, octave, tmp;

    /* Callee function already assures this, but... */
    SYNTH_ASSERT_TOKEN(T_NOTE);

    /* Set initial duration to whatever the current default is */
    duration = pParser->ctl.duration;
    octave = pParser->ctl.octave;

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
    rv = synthLexer_lookupToken(&token, &(pCtx->lexCtx));
    SYNTH_ASSERT(rv == SYNTH_OK);

    /* Retrieve the note's duration, if not the default one */
    if (token == T_NUMBER || token == T_DURATION) {
        rv = synthParser_getDuration(&duration, pParser, pCtx);
        SYNTH_ASSERT(rv == SYNTH_OK);
    }

    rv = synthParser_getNoteCtl(pParser, pNote, octave, note, duration);
    SYNTH_ASSERT(rv == SYNTH_OK);

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Parse a note into the context
 *
 * Parsing rule: note = noteCtl (T_EXTEND T_NUMBER T_DURATION?)*
 *
 * @param  [out]pNumNotes The number of parsed notes (since extends counts as
 *                        new notes)
 * @param  [ in]pParser   The parser context
 * @param  [ in]pCtx      The synthesizer context
 * @return                SYNTH_OK, SYNTH_UNEXPECTED_TOKEN, SYNTH_MEM_ERR
 */
static synth_err synthParser_note(int *pNumNotes, synthParserCtx *pParser,
        synthCtx *pCtx) {
    synthNote note;
    synth_token token;
    synth_err rv;
    int doExtend = 0;

    rv = synthParser_noteCtl(pParser, pCtx, &note);
    SYNTH_ASSERT(rv == SYNTH_OK);

    /* Check if the note will be extended */
    rv = synthLexer_lookupToken(&token, &(pCtx->lexCtx));
    SYNTH_ASSERT(rv == SYNTH_OK);
    if (token == T_EXTEND) {
        doExtend = 1;
        /* First part of extended note: do attack only */
        rv = synthNote_setKeyoff(&note, pParser->ctl.attack, 100, 100);
    }

    rv = synthParser_outputCtldNote(pParser, pCtx, &note);
    SYNTH_ASSERT(rv == SYNTH_OK);
    *pNumNotes = 1;
    while (doExtend != 0 && doExtend != 3) {
        int duration;

        /* The only way to get into this section is through a T_EXTEND */
        SYNTH_ASSERT_TOKEN(T_EXTEND);
        /* Get next token */
        rv = synthLexer_getToken(&(pCtx->lexCtx));
        SYNTH_ASSERT(rv == SYNTH_OK);

        /* T_EXTEND must be followed by the extended duration */
        rv = synthParser_getDuration(&duration, pParser, pCtx);
        SYNTH_ASSERT(rv == SYNTH_OK);

        /* Check if the next token is another T_EXTEND */
        rv = synthLexer_lookupToken(&token, &(pCtx->lexCtx));
        SYNTH_ASSERT(rv == SYNTH_OK);
        if (token == T_EXTEND) {
            doExtend = 2;
        }
        else {
            doExtend = 3;
        }

        /* Output the current note */
        rv = synthParser_outputNote(pParser, pCtx, doExtend,
                note.ctl.octave, note.ctl.note, duration);
        SYNTH_ASSERT(rv == SYNTH_OK);

        (*pNumNotes)++;
    }

    rv = SYNTH_OK;
__err:
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
 *                     T_SET_ATTACK T_NUMBER |
 *                     T_SET_KEYOFF T_NUMBER |
 *                     T_SET_RELEASE T_NUMBER |
 *                     T_SET_PAN T_NUMBER |
 *                     T_SET_WAVE T_NUMBER |
 *                     T_SET_ENVELOPE T_OPEN_BRACKETS 4(T_NUMBER T_COMMA)
 *                             T_NUMBER T_CLOSE_BRACKETS
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
            rv = synthLexer_getValuei(&(pParser->ctl.duration),
                    &(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);
        } break;
        case T_SET_OCTAVE: {
            /* Read the following number */
            rv = synthLexer_getToken(&(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);
            SYNTH_ASSERT_TOKEN(T_NUMBER);

            /* Set the octave for notes */
            rv = synthLexer_getValuei(&(pParser->ctl.octave), &(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);
        } break;
        case T_SET_REL_OCTAVE: {
            /* This token already have the amount to increase the octave, so
             * simply use it */

            /* Increase or decrease the octave */
            rv = synthLexer_getValuei(&tmp, &(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);
            pParser->ctl.octave += tmp;
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
                /* The old enveloped volume shouldn't be used with the new
                 * envelop enabled */
                SYNTH_ASSERT_ERR(pParser->useNewEnvelope == SYNTH_FALSE,
                        SYNTH_UNEXPECTED_TOKEN);
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

            /* Parse the rest of a 'complex' volume */
            if (!isConst) {
                int vol2;

                /* Read the following token */
                rv = synthLexer_getToken(&(pCtx->lexCtx));
                SYNTH_ASSERT(rv == SYNTH_OK);

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
                rv = synthVolume_getLinear(&(pParser->ctl.volume), pCtx, vol1,
                        vol2);
                SYNTH_ASSERT(rv == SYNTH_OK);
            }
            else {
                /* Simply initialize/search the constant volume */
                rv = synthVolume_getConst(&(pParser->ctl.volume), pCtx, vol1);
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
        case T_SET_ATTACK: {
            /* Read the following number */
            rv = synthLexer_getToken(&(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);
            SYNTH_ASSERT_TOKEN(T_NUMBER);

            /* Set the keyoff value */
            rv = synthLexer_getValuei(&(pParser->ctl.attack), &(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);
        } break;
        case T_SET_KEYOFF: {
            /* Read the following number */
            rv = synthLexer_getToken(&(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);
            SYNTH_ASSERT_TOKEN(T_NUMBER);

            /* Set the keyoff value */
            rv = synthLexer_getValuei(&(pParser->ctl.keyoff), &(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);
        } break;
        case T_SET_RELEASE: {
            /* Read the following number */
            rv = synthLexer_getToken(&(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);
            SYNTH_ASSERT_TOKEN(T_NUMBER);

            /* Set the keyoff value */
            rv = synthLexer_getValuei(&(pParser->ctl.release), &(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);
        } break;
        case T_SET_PAN: {
            /* Read the following number */
            rv = synthLexer_getToken(&(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);
            SYNTH_ASSERT_TOKEN(T_NUMBER);

            /* Set the pan */
            rv = synthLexer_getValuei(&(pParser->ctl.pan), &(pCtx->lexCtx));
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
            SYNTH_ASSERT_ERR(tmp < SYNTH_MAX_WAVE, SYNTH_INVALID_WAVE);
            pParser->ctl.wave = tmp;
        } break;
        case T_SET_ENVELOPE: {
            struct stSynthVolume newVol;
            int i;

            rv = synthLexer_getToken(&(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);
            SYNTH_ASSERT_TOKEN(T_OPEN_BRACKET);

            /* Since the envelope is simply a bunch of ints, parse it as such */
            i = 0;
            while (i < sizeof(newVol) / sizeof(int) - 2) {
                int *pInt = (int*)&newVol;

                rv = synthLexer_getToken(&(pCtx->lexCtx));
                SYNTH_ASSERT(rv == SYNTH_OK);
                SYNTH_ASSERT_TOKEN(T_NUMBER);

                rv = synthLexer_getValuei(&pInt[i+2], &(pCtx->lexCtx));
                SYNTH_ASSERT(rv == SYNTH_OK);

                /* There must be a ',' between each number (except the last) */
                if (i < sizeof(newVol) / sizeof(int) - 3) {
                    rv = synthLexer_getToken(&(pCtx->lexCtx));
                    SYNTH_ASSERT(rv == SYNTH_OK);

                    SYNTH_ASSERT_TOKEN(T_COMMA);
                }
                i++;
            }

            rv = synthLexer_getToken(&(pCtx->lexCtx));
            SYNTH_ASSERT(rv == SYNTH_OK);
            SYNTH_ASSERT_TOKEN(T_CLOSE_BRACKET);

            /* Initialize/Search the volume */
            rv = synthVolume_getEnvelope(&(pParser->ctl.volume), pCtx, &newVol);
            SYNTH_ASSERT(rv == SYNTH_OK);
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
static synth_err synthParser_loop(int *pNumNotes, synthParserCtx *pParser,
        synthCtx *pCtx) {
    int count, loopPosition;
    synth_err rv;
    synthNote *pNote;
    synth_token token;

    /* We're sure to have this token, but... */
    SYNTH_ASSERT_TOKEN(T_SET_LOOP_START);
    /* As in music scores, loops must sync with the compass */
    SYNTH_ASSERT_ERR(pParser->curCompassLength == 0, SYNTH_BAD_LOOP_START);

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
    /* Again, a compass must have just ended */
    SYNTH_ASSERT_ERR(pParser->curCompassLength == 0, SYNTH_BAD_LOOP_END);

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
    return rv;
}

/**
 * Parse a macro declaration, which may be used from this moment onward.
 *
 * Parsing rule: declareMacro = T_DECL_MACRO T_MACRO_ID mod* noteCtl mod*
 *                                      T_END_OF_TRACK
 *
 * @param  [ in]pParser   The parser context
 * @param  [ in]pCtx      The synthesizer context
 */
static synth_err synthParser_declareMacro(synthParserCtx *pParser,
        synthCtx *pCtx) {
    synth_err rv;
    int flags, macro;

#define DONE     0x01
#define GOT_NOTE 0x02

    /* We're sure to have this token, but... */
    SYNTH_ASSERT_TOKEN(T_DECL_MACRO);

    rv = synthLexer_getToken(&(pCtx->lexCtx));
    SYNTH_ASSERT(rv == SYNTH_OK);
    SYNTH_ASSERT_TOKEN(T_MACRO_ID);

    /* Retrieve the macro and convert it to an index */
    rv = synthLexer_getValuei(&macro, &(pCtx->lexCtx));
    SYNTH_ASSERT(rv == SYNTH_OK);
    macro -= 'A';

    /* Start parsing everything into the macro */
    rv = synthLexer_getToken(&(pCtx->lexCtx));
    SYNTH_ASSERT(rv == SYNTH_OK);

    flags = 0;
    do {
        synth_token token;

        rv = synthLexer_lookupToken(&token, &(pCtx->lexCtx));
        SYNTH_ASSERT(rv == SYNTH_OK);
        switch (token) {
            case T_NOTE: {
                synthNote *pNote;

                SYNTH_ASSERT_ERR(!(flags & GOT_NOTE), SYNTH_BAD_MACRO);

                pNote = (synthNote*)&(pParser->macros[macro].note);
                rv = synthParser_noteCtl(pParser, pCtx, pNote);

                flags |= GOT_NOTE;
            } break;
            case T_END_OF_TRACK: {
                rv = SYNTH_OK;
                flags |= DONE;

                /* Get next token */
                rv = synthLexer_getToken(&(pCtx->lexCtx));
                SYNTH_ASSERT(rv == SYNTH_OK);
            } break;
            default: {
                rv = synthParser_mod(pParser, pCtx);
            }
        }
        SYNTH_ASSERT(rv == SYNTH_OK);

    } while (!(flags & DONE));

    SYNTH_ASSERT_ERR(flags & GOT_NOTE, SYNTH_BAD_MACRO);

    /* Store the current context into the macro, so it may be later recovered */
    do {
        synthParserCtx *tmpParser;

        tmpParser = (synthParserCtx*)&(pParser->macros[macro].parser);
        memcpy(tmpParser, pParser, sizeof(struct stSynthParserCtl));
    } while (0);

    pParser->macros[macro].defined = 1;

#undef DONE
#undef GOT_NOTE

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Parse a macro, outputting it and recovering the parser state.
 *
 * Parsing rule: macro = T_MACRO_ID
 *
 * @param  [out]pNumNotes The number of parsed notes (since extends counts as
 *                        new notes). For now, it will always be 1. However,
 *                        it's here if extended notes is ever added to macros.
 * @param  [ in]pParser   The parser context
 * @param  [ in]pCtx      The synthesizer context
 */
static synth_err synthParser_macro(int *pNumNotes, synthParserCtx *pParser,
        synthCtx *pCtx) {
    synthParserCtx *tmpParser;
    synthNote *pNote;
    synth_err rv;
    int macro;

    /* Callee function already assures this, but... */
    SYNTH_ASSERT_TOKEN(T_MACRO_ID);

    /* Convert the macro to an index, and check that it's valid */
    rv = synthLexer_getValuei(&macro, &(pCtx->lexCtx));
    SYNTH_ASSERT(rv == SYNTH_OK);
    macro -= 'A';
    SYNTH_ASSERT_ERR(pParser->macros[macro].defined, SYNTH_UNDEF_MACRO);

    /* Simply output the macro and retrieve its parsing context */
    pNote = (synthNote*)&(pParser->macros[macro].note);
    rv = synthParser_outputCtldNote(pParser, pCtx, pNote);
    SYNTH_ASSERT(rv == SYNTH_OK);
    *pNumNotes = 1;

    tmpParser = (synthParserCtx*)&(pParser->macros[macro].parser);
    memcpy(pParser, tmpParser, sizeof(struct stSynthParserCtl));

    /* Read whatever the next token is */
    rv = synthLexer_getToken(&(pCtx->lexCtx));
    SYNTH_ASSERT(rv == SYNTH_OK);

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Parse a sequence into the context
 * 
 * Parsing rule: sequence = ( mod | note | loop | declareMacro | macro )+
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

        rv = synthLexer_lookupToken(&token, &(pCtx->lexCtx));
        SYNTH_ASSERT(rv == SYNTH_OK);

        /* Anything not a note or loop will be parsed as mod */
        switch (token) {
            case T_NOTE: {
                int numNotes;

                /* Simply parse the current note */
                rv = synthParser_note(&numNotes, pParser, pCtx);

                *pNumNotes += numNotes;
            } break;
            case T_SET_LOOP_START: {
                /* Recursively parse a sub-sequence */
                rv = synthParser_loop(pNumNotes, pParser, pCtx);
            } break;
            case T_DECL_MACRO: {
                rv = synthParser_declareMacro(pParser, pCtx);
            } break;
            case T_MACRO_ID: {
                int numNotes;

                rv = synthParser_macro(&numNotes, pParser, pCtx);

                *pNumNotes += numNotes;
            } break;
            default: {
                /* Modify the current context in some way */
                rv = synthParser_mod(pParser, pCtx);
            }
        }
        SYNTH_ASSERT(rv == SYNTH_OK);
    }

    rv = SYNTH_OK;
__err:
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
static synth_err synthParser_track(int *pTrackHnd, synthParserCtx *pParser,
        synthCtx *pCtx) {
    synthTrack *pTrack;
    synth_err rv;
    synth_token token;
    int curTrack, didFindSequence, numNotes;

    /* Retrieve a new track */
    rv = synthTrack_init(&pTrack, pCtx);
    SYNTH_ASSERT(rv == SYNTH_OK);
    curTrack = pCtx->tracks.used - 1;

    /* Revert the parser to its initial state */
    rv = synthParser_setDefault(pParser, pCtx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    numNotes = 0;

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
        /* Loop point must be found on compass start */
        SYNTH_ASSERT_ERR(pParser->curCompassLength == 0, SYNTH_BAD_LOOP_POINT);
        /* Set the current position into the track as the looppoint */
        pTrack->loopPoint = numNotes;

        /* Get the next token (since the previous was a T_SET_LOOPPOINT) */
        rv = synthLexer_getToken(&(pCtx->lexCtx));
        SYNTH_ASSERT(rv == SYNTH_OK);

        /* Parse the looping sequence */
        rv = synthParser_sequence(&numNotes, pParser, pCtx);
        SYNTH_ASSERT(rv == SYNTH_OK);
    }

    /* Store the total number of notes in the track */
    pTrack->num = numNotes;

    *pTrackHnd = curTrack;
    rv = SYNTH_OK;
__err:
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
    return rv;
}

/**
 * Parse the currently loaded file into an audio
 * 
 * Parsing rule: mml bmp newEnv tracks
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

    /* Set the time signature */
    pAudio->timeSignature = pParser->timeSignature;

    /* Read the first token */
    rv = synthLexer_getToken(&(pCtx->lexCtx));
    SYNTH_ASSERT(rv == SYNTH_OK);

    /* Check that its actually a MML song */
    rv = synthParser_mml(pParser, pCtx);
    SYNTH_ASSERT(rv == SYNTH_OK);

    /* Set the audio's envelope based on the current version */
    pAudio->useNewEnvelope = pParser->useNewEnvelope;

    /* Parse the bpm (optional token) */
    rv = synthParser_bpm(pParser, pCtx, pAudio);
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

