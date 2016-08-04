/**
 * @project c_synth
 * @license zlib license
 * @file    src/lexer/synth_lexerDict.h
 *
 * Dictionary for the lexer's tokens.
 *
 * This is mostly intended for debug. 
 */
#include <c_synth_internal/synth_lexer.h>

static char *_synth_noteDict[NT_MAX] = {
    [NT_CB]   = "Cb",
    [NT_C]    = "C",
    [NT_CS]   = "C#",
    [NT_D]    = "D",
    [NT_DS]   = "D#",
    [NT_E]    = "E",
    [NT_F]    = "F",
    [NT_FS]   = "F#",
    [NT_G]    = "G",
    [NT_GS]   = "G#",
    [NT_A]    = "A",
    [NT_AS]   = "A#",
    [NT_B]    = "B",
    [NT_BS]   = "B#",
    [NT_REST] = "rest"
};

/**
 * Dictionary of notes.
 *
 * @param  [ in]note The note
 * @return           Static string with the note's name
 */
char* synth_noteDict(synth_note note) {
    if (note < NT_MAX) {
        return _synth_noteDict[note];
    }

    return "invalid note";
}

/**
 * Dictionary of tokens.
 *
 * @param  [ in]token The token.
 * @return            Static string with the token's name
 */
char* synth_tokenDictionary(synth_token token) {
#define case2str(name) \
    case name: return #name;

    switch (token) {
        case2str(STK_HALF_DURATION);
        case2str(STK_NOTE_EXTENSION);
        case2str(STK_OCTAVE);
        case2str(STK_INCREASE_OCTAVE);
        case2str(STK_DECREASE_OCTAVE);
        case2str(STK_DURATION);
        case2str(STK_LOAD);
        case2str(STK_INSTRUMENT);
        case2str(STK_ENVELOPE);
        case2str(STK_WAVE);
        case2str(STK_PANNING);
        case2str(STK_ATTACK);
        case2str(STK_KEYOFF);
        case2str(STK_RELEASE);
        case2str(STK_LOOP_START);
        case2str(STK_LOOP_END);
        case2str(STK_REPEAT);
        case2str(STK_MACRO);
        case2str(STK_END);
        case2str(STK_BPM);
        case2str(STK_KEY);
        case2str(STK_TEMPO);
        case2str(STK_STRING);
        case2str(STK_COMMENT);
        case2str(STK_NOTE);
        case2str(STK_NUMBER);
        case2str(STK_END_OF_INPUT);
        case2str(STK_UNKNOWN);
        default: return "invalid token";
    }

#undef case2str
}

