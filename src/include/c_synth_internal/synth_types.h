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
#  ifndef __SYNTHENVELOPE_ENUM__
#  define __SYNTHENVELOPE_ENUM__
     typedef enum enSynthEnvelope synth_envelope;
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
    W_NOISE_BEST_BASS,
    W_NOISE_BEST_HIGHPITCH,
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
    T_SET_ENVELOPE,
    T_NEW_MML,
    T_DECL_MACRO,
    T_MACRO_ID,
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

/** Controler for outputting stuff with the parser. Mostly used to copy the
 * current state in a macro */
struct stSynthParserCtl {
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
    /** Current wave */
    synth_wave wave;
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
    /** Compass' time signature in binary fixed point notation */
    int timeSignature;
    /** Length of the current compass in binary fixed point notation */
    int curCompassLength;
    /** Whether the new envelope mode should be used */
    synth_bool useNewEnvelope;
    /** Controls how notes are parsed */
    struct stSynthParserCtl ctl;
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
    /** Whether the new envelope mode should be used. Only temporarialy used
     * while rendering a song. */
    synth_bool useNewEnvelope;
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
    /** Whether the new envelope mode should be used */
    synth_bool useNewEnvelope;
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
    /** Index to either a value between 0x0 and 0xff or a envelop */
    int volume;
};

/**
 * Various states that the volume envelope may be. The volumes are interpolated
 * as following:
 *
 * ENV_ATTACK = preAttack ~ hold
 * ENV_HOLD = hold ~ decay
 * ENV_DECAY = decay ~ release
 * ENV_RELEASE = release ~ postRelease
 */
enum enSynthEnvelope {
    ENV_ATTACK = 0,
    ENV_HOLD,
    ENV_DECAY,
    ENV_RELEASE,
    ENV_MAX
};

/**
 * Define a simple note envelop, which becomes the following graph:
 *
 * Amp
 *  ^
 *  |   /^^^^^\
 *  |  /       \
 *  | /  |   |  \
 *  |/           \
 *  |    |   |    \
 *  |              \_______
 *  |    |   |
 *  |------------------------> time
 *  t0   t1  t2    t3    t4
 *
 * Where:
 *  t0 = preAttack (starting volume)
 *  t1 = hold (volume after the attack)
 *  t2 = decay (volume after the 'hold' period, before decaying)
 *  t3 = release (starting volume on note release)
 *  t4 = postRelease (final volume)
 *
 * Note that the evenlope doesn't have to start and end at 0. If so desired, it
 * may ring until the end.
 */
struct stSynthVolume {
    /** Initial volume */
    int ini;
    /** Final volume */
    int fin;
    /** === NEW CONTROLS ===== */
    int preAttack;
    int hold;
    int decay;
    int release;
    int postRelease;
};

#endif /* __SYNTH_INTERNAL_TYPES_H__ */

