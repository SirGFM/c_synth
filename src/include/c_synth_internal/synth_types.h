/**
 *
 * @typedef   synth_note        Notes within an octave.
 * @typedef   synth_token       Tokens recognized by the lexer.
 */
#ifndef __SYNTH_INTERNAL_TYPES_H__
#define __SYNTH_INTERNAL_TYPES_H__

/** Required for fixed-width sizes */
#include <stdint.h>


/* == LEXER COMPONENTS ============================================== */

/** Represents all possible notes within a single octave. */
enum enSynth_note {
    /** Equivalent to NT_B on a lower octave */
    NT_CB = 0,
    NT_C,
    NT_CS,
    NT_D,
    NT_DS,
    NT_E,
    NT_F,
    NT_FS,
    NT_G,
    NT_GS,
    NT_A,
    NT_AS,
    NT_B,
    /** Equivalent to NT_C on a higher octave */
    NT_BS,
    NT_REST,
    NT_MAX
};
typedef enum enSynth_note synth_note;

/**
 * List of tokens recognized by the lexer. Almost every token is mapped
 * to its respective characters. The exception are NOTE_TK, STRING_TK,
 * NUMBER_TK and COMMENT_TK. Both STRING_TK and COMMENT_TK are mapped to
 * the first character recognized by then. NOTE_TK and STRING_TK, on the
 * other hand, are simply mapped to available characters, without any
 * intrinsic meaning.
 */
enum enSynth_token {
    STK_HALF_DURATION   = '.',
    STK_NOTE_EXTENSION  = '^',
    STK_OCTAVE          = 'o',
    STK_INCREASE_OCTAVE = '>',
    STK_DECREASE_OCTAVE = '<',
    STK_DURATION        = 'l',
    STK_LOAD            = 'j',
    STK_INSTRUMENT      = 'i',
    STK_ENVELOPE        = 'v',
    STK_WAVE            = 'w',
    STK_PANNING         = 'p',
    STK_ATTACK          = 't',
    STK_KEYOFF          = 'k',
    STK_RELEASE         = 'q',
    STK_LOOP_START      = '[',
    STK_LOOP_END        = ']',
    STK_REPEAT          = '$',
    STK_MACRO           = 'm',
    STK_END             = ';',
    STK_BPM             = 'B',
    STK_KEY             = 'K',
    STK_TEMPO           = 'T',
    STK_STRING          = '"',
    STK_COMMENT         = '#',
    STK_NOTE            = 'a',
    STK_NUMBER          = 'n',
    STK_END_OF_INPUT    = '\0',
    STK_UNKNOWN         = '?'
};
typedef enum enSynth_token synth_token;


/* == SONG COMPONENTS =============================================== */

/** Waveform for synthesizing notes */
enum enSynth_waveform {
    WF_12_5_PW    = 0,
    WF_25_PW      = 1,
    WF_50_PW      = 2,
    WF_75_PW      = 3,
    WF_TRIANGLE   = 4,
    WF_SAWTOOTH   = 5,
    WF_WHITENOISE = 6,
    WF_MAX
};
typedef enum enSynth_waveform synth_waveform;

/**
 * Envelope for the note's amplitude.
 * The amplitude is linearly interpolated between the start and end
 * values. Also, each value is 4 bits long, so they range from [0,15].
 * It's important to note that the synthesizer may adjust the volume
 * according to the number of tracks currently playing.
 */
struct stSynth_envelope {
    uint8_t start : 4;
    uint8_t end   : 4;
};
typedef struct stSynth_envelope synth_envelope;

/**
 * Instrument that controls how notes ares synthesized.
 *
 * It defaults to:
 *   - .envelope = { .start=7, .end=12 }
 *   - .wave    = WF_50_PW
 *   - .pan     = 3
 *   - .attack  = 1
 *   - .keyoff  = 5
 *   - .release = 6
 */
struct stSynth_instrument {
    synth_envelope envelope;
    synth_waveform wave;
    /**
     * Amplitude's separation ratio between left and right channel.
     * It's represented by a 3 bit value, ranging from 0 to 6. 0 and 6
     * sets the track to play only on the left or right channel,
     * respectively, while 3 divide the song evenly between both
     * channels.
     */
    uint8_t pan : 3;
    /**
     * Percentage of the note's duration that takes to reach full
     * amplitude from silence.
     * It must be on the range [0, 7]. 0 makes the note start maxed out.
     * Also, note that this value is overriden by both keyoff and
     * release.
     */
    uint8_t attack : 3;
    /**
     * Percentage of the note's duration that takes the note to start
     * fading out. It must be on the range [0, 7]. Note that this value
     * is overriden by release.
     */
    uint8_t keyoff : 3;
    /**
     * Percentage of the note's duration when it goes completely
     * silent. It must be on the range [0, 7].
     */
    uint8_t release : 3;
};
typedef struct stSynth_instrument synth_instrument;

/** A collection of tracks and its speed */
struct stSynth_song {
    /**
     * Time signature.
     * This information is relevant even after a song is successfully
     * parsed so the BMP may be correctly modified.
     */
    int signature;
    uint16_t trackIndex;
    /**
     * The song speed in beats per minutes.
     */
    uint8_t bmp;
    uint8_t numTracks;
};
typedef struct stSynth_song synth_song;

struct stSynth_track {
    /** Starting index of the track's notes within pMemory->nodes. */
    uint32_t nodeIndex;
    /**
     * Index of the track's own instrument.
     *
     * Whenever a node directly modifies the track's instrument, the
     * current instrument gets copied into its default instrument and
     * then it gets modified. Therefore, as long a "common" instrument
     * (when that was loaded or the default one) is used, there's no
     * copy nor store involved, it's as simple as setting a pointer.
     * However when a "common" instrument is in use and it is to be
     * modified, the entire instrument will get copied, but any
     * following operation won't require copying it again.
     */
    uint16_t defaultInstrument;
    /** Index of the instrument in use by the track */
    uint16_t currentInstrument;
    /**
     * Position of the current node being playing.
     * This implies that a track will never simultaneously play more
     * than once.
     */
    uint16_t position;
    /**
     * Number of nodes on the track.
     * 16 bits enables 65535 nodes per track. Considering a 4/4 song at
     * 120 bpm that only has 1/64 notes, it would be 34 minutes long. It
     * seems to be way more than enough for any common cases.
     */
    uint16_t numNodes;
};
typedef struct stSynth_track synth_track;

/** A note that can be played */
struct stSynth_note_data {
    synth_note note;
    uint8_t duration;
    uint8_t octave;
};
typedef struct stSynth_note_data synth_note_data;

/** Loop data.
 * It gets parsed from STK_LOOP_START, STK_LOOP_END and STK_REPEAT
 */
struct stSynth_loop_data {
    /** 
     * Position within the track.
     *
     * On STK_LOOP_START, it's the position of its respective
     * STK_LOOP_END node, on which 'repeatCount' shall be set.
     *
     * On STK_LOOP_END and STK_REPEAT, it's the jump offset, from the
     * start of the track.
     */
    uint16_t position;
    /**
     * How many times should the loop repeat.
     *
     * Whenever this node is interpreted, this field gets decremented.
     * On doing so, if it becomes 0, the track continues to play
     * (instead of looping to 'position'). However, if this field starts
     * as 0, it will loop indefinitely.
     */
    uint16_t repeatCount;
};
typedef struct stSynth_loop_data synth_loop_data;

/**
 * Values of nodes within a track.
 *
 * Which field to use, and how to cast it, will depend on the node's
 * 'type'.
 */
union unSynth_node_data {
    synth_note_data note;
    synth_envelope envelope;
    synth_loop_data loop;
    uint16_t value;
};
typedef union unSynth_node_data synth_node_data;

/**
 * Types of nodes within a track.
 *
 * If a parsed token doesn't take effect instantly (as setting the
 * note's duration would), it will become a node with one of these
 * types.
 *
 * This isn't even necessary, since a node's type may be inferred from
 * its token.
 */
enum enSynth_node_type {
    /** Note to be played */
    NDT_NOTE = 0,
    /** Changes a track's configuration (e.g., load an instrument) */
    NDT_CONF,
    /** Change the flow of the track (i.e., jump to some position) */
    NDT_FLOW,
    NDT_MAX
};
typedef enum enSynth_node_type synth_node_type;

/** A node within a track */
struct st_synth_node {
    synth_token type;
    synth_node_data data;
};
typedef struct st_synth_node synth_node;


/**
 * Initialize a non-constant instrument
 *
 * @param  [out]pInstrument The instrument
 */
inline void synth_setDefaultInstrument(synth_instrument *pInstrument);

#endif /* __SYNTH_INTERNAL_TYPES_H__ */













/**
 * Define all types used within the library, but not 'publicly exported' to the
 * user
 * 
 * @file src/include/synth_internal/synth_types.h
 */
#ifndef __SYNTH_INTERNAL_TYPES_H__
#define __SYNTH_INTERNAL_TYPES_H__

/* Required because of a FILE* */
#include <stdio.h>

#if defined(USE_SDL2)
#  include <SDL2/SDL_rwops.h>
#endif

/* First, define the name (i.e., typedef) of every type */

#  ifndef __SYNTHAUDIO_STRUCT__
#  define __SYNTHAUDIO_STRUCT__
     typedef struct stSynthAudio synthAudio;
#  endif /* __SYNTHAUDIO_STRUCT__ */
#  ifndef __SYNTHBUFFER_UNION__
#  define __SYNTHBUFFER_UNION__
     typedef union unSynthBuffer synthBuffer;
#  endif /* __SYNTHBUFFER_UNION__ */
#  ifndef __SYNTHCTX_STRUCT__
#  define __SYNTHCTX_STRUCT__
     typedef struct stSynthCtx synthCtx;
#  endif /* __SYNTHCTX_STRUCT__ */
#  ifndef __SYNTHLEXCTX_STRUCT__
#  define __SYNTHLEXCTX_STRUCT__
     typedef struct stSynthLexCtx synthLexCtx;
#  endif /* __SYNTHLEXCTX_STRUCT__ */
#  ifndef __SYNTHLIST_STRUCT__
#  define __SYNTHLIST_STRUCT__
     typedef struct stSynthList synthList;
#  endif /* __SYNTHLIST_STRUCT__ */
#  ifndef __SYNTHNOTE_STRUCT__
#  define __SYNTHNOTE_STRUCT__
     typedef struct stSynthNote synthNote;
#  endif /* __SYNTHNOTE_STRUCT__ */
#  ifndef __SYNTHPARSERCTX_STRUCT__
#  define __SYNTHPARSERCTX_STRUCT__
     typedef struct stSynthParserCtx synthParserCtx;
#  endif /* __SYNTHPARSERCTX_STRUCT__ */
#  ifndef __SYNTHRENDERERCTX_STRUCT__
#  define __SYNTHRENDERERCTX_STRUCT__
typedef struct stSynthRendererCtx synthRendererCtx;
#  endif /* __SYNTHRENDERERCTX_STRUCT__ */
#  ifndef __SYNTHPRNG_STRUCT__
#  define __SYNTHPRNG_STRUCT__
     typedef struct stSynthPRNGCtx synthPRNGCtx;
#  endif /* __SYNTHPRNG_STRUCTRUCT__ */
#  ifndef __SYNTHSOURCE_UNION__
#  define __SYNTHSOURCE_UNION__
     typedef union unSynthSource synthSource;
#  endif /* __SYNTHSOURCE_UNION__ */
#  ifndef __SYNTHSOURCETYPE_ENUM__
#  define __SYNTHSOURCETYPE_ENUM__
     typedef enum enSynthSourceType synthSourceType;
#  endif /* __SYNTHSOURCETYPE_ENUM__ */
#  ifndef __SYNTHSTRING_STRUCT__
#  define __SYNTHSTRING_STRUCT__
     typedef struct stSynthString synthString;
#  endif /* __SYNTHSTRING_STRUCT__ */
#  ifndef __SYNTHTRACK_STRUCT__
#  define __SYNTHTRACK_STRUCT__
     typedef struct stSynthTrack synthTrack;
#  endif /* __SYNTHTRACK_STRUCT__ */
#  ifndef __SYNTHVOLUME_STRUCT__
#  define __SYNTHVOLUME_STRUCT__
     typedef struct stSynthVolume synthVolume;
#  endif /* __SYNTHVOLUME_STRUCT__ */
#  ifndef __SYNTHBOOL_ENUM__
#  define __SYNTHBOOL_ENUM__
     typedef enum enSynthBool synth_bool;
#  endif /* __SYNTHBOOL_ENUM__ */
#  ifndef __SYNTHWAVE_ENUM__
#  define __SYNTHWAVE_ENUM__
     typedef enum enSynthWave synth_wave;
#  endif /* __SYNTHWAVE_ENUM__ */
#  ifndef __SYNTHNOTE_ENUM__
#  define __SYNTHNOTE_ENUM__
     typedef enum enSynthNote synth_note;
#  endif /* __SYNTHNOTE_ENUM__ */
#  ifndef __SYNTHTOKEN_ENUM__
#  define __SYNTHTOKEN_ENUM__
     typedef enum enSynthToken synth_token;
#  endif /* __SYNTHTOKEN_ENUM__ */

/* Define every enumeration */

/* Simple boolean value */
enum enSynthBool {
    SYNTH_TRUE = 0,
    SYNTH_FALSE
};

/* Wave types for a note */
enum enSynthWave {
    W_SQUARE = 0,
    W_PULSE_12_5,
    W_PULSE_25,
    W_PULSE_75,
    W_TRIANGLE,
    W_NOISE,
    W_NOISE_SQUARE,
    W_NOISE_12_5,
    W_NOISE_25,
    W_NOISE_75,
    W_NOISE_TRIANGLE,
    SYNTH_MAX_WAVE
};

/* Musical notes */
enum enSynthNote {
    N_CB = 0,   /* Required for decreasing octave */
    N_C,
    N_CS,
    N_D,
    N_DS,
    N_E,
    N_F,
    N_FS,
    N_G,
    N_GS,
    N_A,
    N_AS,
    N_B,
    N_BS,   /* Required for increasing octave */
    N_REST,
    N_LOOP
};

/* Tokens used on a song's compilation */
enum enSynthToken {
    T_MML = 0,
    T_SET_BPM,
    T_SET_DURATION,
    T_SET_OCTAVE,
    T_SET_REL_OCTAVE,
    T_SET_LOOPPOINT,
    T_END_OF_TRACK,
    T_SET_VOLUME,
    T_SET_ATTACK,
    T_SET_KEYOFF,
    T_SET_RELEASE,
    T_SET_PAN,
    T_SET_LOOP_START,
    T_SET_LOOP_END,
    T_SET_WAVE,
    T_OPEN_BRACKET,
    T_CLOSE_BRACKET,
    T_NOTE,
    T_DURATION,
    T_NUMBER,
    T_COMMA,
    T_DONE,
    T_EXTEND,
    TK_MAX
};

/* Now, define every struct/union; Note that the order may be relevant, as there
 * are statically alloc'ed stuff into the structs; Obviously, in case of
 * pointers, the order becomes irrelevant */

/** Define a simple static string for parsing */
struct stSynthString {
    /** Total length of the string */
    int len;
    /** Current position on the string */
    int pos;
    /** Pointer to the static (and NULL-terminated) string */
    char *pStr;
};

/** Define a source for a MML audio, which can either be a file or a string */
union unSynthSource {
#if defined(USE_SDL2)
    /** SDL's SDL_RWops, so it works on mobile! */
    SDL_RWops *sdl;
#endif
    /** A file */
    FILE *file;
    /** A static string, with its current position and length */
    synthString str;
};

/** Defines all posible input types for the lexer */
enum enSynthSourceType {
    SST_NONE = 0,
    SST_FILE,
    SST_STR,
    SST_SDL,
    SST_MAX
};

/** Define the context for the lexer */
struct stSynthLexCtx {
    /** Last read character */
    char lastChar;
    /** Current input type */
    synthSourceType type;
    /** Current line on the stream */
    int line;
    /** Position inside the current line */
    int linePos;
    /** Token read on the privous getToken call */
    synth_token lastToken;
    /** Integer value gotten when reading a token */
    int ivalue;
    /** MML's source; either a file descriptor or a string */
    synthSource source;
};

/** Define the context for the parser */
struct stSynthParserCtx {
    /** Expected token (only valid on error) */
    synth_token expected;
    /** Gotten token (only valid on error) */
    synth_token gotten;
    /** Whether an error occured or note */
    synth_bool errorFlag;
    /** Which error code was raised */
    synth_err errorCode;
    /** Current octave */
    int octave;
    /** Default duration (when not specified) */
    int duration;
    /** Index of the default volume */
    int volume;
    /** Current attack */
    int attack;
    /** Current keyoff */
    int keyoff;
    /** Current release */
    int release;
    /** Current pan */
    int pan;
    /** Compass' time signature in binary fixed point notation */
    int timeSignature;
    /** Length of the current compass in binary fixed point notation */
    int curCompassLength;
    /** Current wave */
    synth_wave wave;
};

/** Struct with data about the currently rendering song/track */
struct stSynthRendererCtx {
    /** Number of samples per compass */
    int samplesPerCompass;
    /** Current length of the compass in samples */
    int curCompassLength;
    /** Audio time signature */
    int timeSignature;
    /** Current position within the compass */
    int curCompassPosition;
};

/** Defines the types of noise wave generators */
enum enNoiseWaveType {
    NW_NONE = 0,
    NW_BOXMULLER,
    NW_ZIGGURAT
};

/** Struct with the static parameters required by the Box-Muller algorithm */
struct stBoxMullerParams {
    /** A point, generated on the previous iteration */
    double z0;
    /** The other point, generated on the previous iteration */
    double z1;
    /** Whether the points where generated on the previous iteration */
    int didGenerate;
};

/** Struct with the static parameters required by the ziggurat algorithm */
struct stZigguratParams {
    /* TODO */
    int nil;
};

/** Parameter specific to algorithms used by the PRNG */
union stPRNGParams {
    struct stBoxMullerParams boxMuller;
    struct stZigguratParams ziggurat;
};

/** Current context used by the pseudo random number generator */
struct stSynthPRNGCtx {
    /* Whether it was initialized */
    int isInit;
    /** Value used by the PRNG formula */
    unsigned int a;
    /** Value used by the PRNG formula */
    unsigned int c;
    /** Latest seed */
    unsigned int seed;
    /* The current noise wave generator */
    enum enNoiseWaveType type;
    /** Params used by the current generator algorithm */
    union stPRNGParams noiseParams;
};

/** Union with all possible buffers (i.e., list of items) */
union unSynthBuffer {
    /* Points to an array of audios */
    synthAudio *pAudios;
    /* Points to an array of notes */
    synthNote *pNotes;
    /* Points to an array of tracks */
    synthTrack *pTracks;
    /* Points to an array of volumes */
    synthVolume *pVolumes;
};

/** A generic list of a buffer */
struct stSynthList {
    /** How many itens may this list may hold, at most */
    int max;
    /** How big is the list; Useful if there's no limit */
    int len;
    /** How many itens are currently in use */
    int used;
    /* TODO Add a map of used items? */
    /** The actual list of itens */
    synthBuffer buf;
};

/* Define the main context */
struct stSynthCtx {
    /**
     * Whether the context was alloc'ed by the library (and, thus, must be
     * freed) or if it was alloc'ed by the user
     */
    int autoAlloced;
    /** Synthesizer frequency in samples per second */
    int frequency;
    /** List of songs */
    synthList songs;
    /** List of tracks */
    synthList tracks;
    /** List of notes */
    synthList notes;
    /** List of volumes */
    synthList volumes;
    /** Lexer context */
    synthLexCtx lexCtx;
    /** Parser context */
    synthParserCtx parserCtx;
    /** Pseudo-random number generator context */
    synthPRNGCtx prngCtx;
    /** Keep track of whatever is being rendered */
    synthRendererCtx renderCtx;
};

/** Define an audio, which is simply an aggregation of tracks */
struct stSynthAudio {
    /**
     * Index to the first track in the synthesizer context; An offset was
     * chosen, in place of a pointer, because the base pointer might change, if
     * dynamic allocations are allowed; So, since the main context should always
     * be available (i.e., passed as parameter to the function), this seemed
     * like the cleanest way to do this
     */
    int tracksIndex;
    /** How many tracks the song has */
    int num;
    /** Song's 'speed' in beats-per-minute */
    int bpm;
    /** Song's time signature */
    int timeSignature;
};

/** Define a track, which is almost simply a sequence of notes */
struct stSynthTrack {
    /** Cached length of the track, in samples */
    int cachedLength;
    /** Cached loop of the track, in samples */
    int cachedLoopPoint;
    /** Start point for repeating or -1, if shouldn't loop */
    int loopPoint;
    /**
     * Index to the first note in the synthesizer context; An offset was
     * chosen, in place of a pointer, because the base pointer might change, if
     * dynamic allocations are allowed; So, since the main context should always
     * be available (i.e., passed as parameter to the function), this seemed
     * like the cleanest way to do this
     */
    int notesIndex;
    /** Number of notes in this track */
    int num;
};

struct stSynthNote {
    /**
     * Value between 0 and 100, where 0 means only left channel and 100 means
     * only right channel
     */
    char pan;
    /** Octave at which the note should play, from 1 to 8 */
    char octave;
    /**
     * Duration of the note in samples (depends on the sample rate).
     * If type is N_loop, represent how many times should repeat.
     */
    int len;
    /**
     * Note's duration in binary fixed point notation; It uses 6 bits for the
     * fractional part
     */
    int duration;
    /** Cached duration of the note in samples */
    int samplesDuration;
    /** Only used if type is N_loop; Represents note to which should jump. */
    int jumpPosition;
    /** Time, in samples, until the note reaches its maximum amplitude */
    int attack;
    /** After how many samples should the note be muted */
    int keyoff;
    /** Time, in samples, until the note halts completely */
    int release;
    /** Only used if type is N_loop; how many times has already looped */
    int numIterations;
    /** Wave type to be synthesized */
    synth_wave wave;
    /** Musical note to be played */
    synth_note note;
    /** Index to either a value between 0x0 and 0xff or a envelope */
    int volume;
};

/** Define a simple note envelop */
struct stSynthVolume {
    /** Initial volume */
    int ini;
    /** Final volume */
    int fin;
};

#endif /* __SYNTH_INTERNAL_TYPES_H__ */

