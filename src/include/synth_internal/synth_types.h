/**
 * Define all types used within the library, but not 'publicly exported' to the
 * user
 * 
 * @file src/include/synth_internal/synth_types.h
 */
#ifndef __SYNTH_INTERNAL_TYPES_H__
#define __SYNTH_INTERNAL_TYPES_H__

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
#  ifndef __SYNTHLIST_STRUCT__
#  define __SYNTHLIST_STRUCT__
     typedef struct stSynthList synthList;
#  endif /* __SYNTHLIST_STRUCT__ */
#  ifndef __SYNTHNOTE_STRUCT__
#  define __SYNTHNOTE_STRUCT__
     typedef struct stSynthNote synthNote;
#  endif /* __SYNTHNOTE_STRUCT__ */
#  ifndef __SYNTHTRACK_STRUCT__
#  define __SYNTHTRACK_STRUCT__
     typedef struct stSynthTrack synthTrack;
#  endif /* __SYNTHTRACK_STRUCT__ */
#  ifndef __SYNTHVOLUME_STRUCT__
#  define __SYNTHVOLUME_STRUCT__
     typedef struct stSynthVolume synthVolume;
#  endif /* __SYNTHVOLUME_STRUCT__ */

/* Now, define every struct/union; Note that the order may be relevant, as there
 * are statically alloc'ed stuff into the structs; Obviously, in case of
 * pointers, the order becomes irrelevant */

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
};

/** Define a track, which is almost simply a sequence of notes */
struct stSynthTrack {
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
     * Current position on the note.
     * If type is N_loop, represents note to which should jump.
     */
    int pos;
    /** After how many samples should the note be muted */
    int keyoff;
    /** Only used if type is N_loop; how many times has already looped */
    int numIterations;
    /** Exact 'note duration', so as to search the cached value */
    int duration;
    /** Wave type to be synthesized */
    synth_wave wave;
    /** Musical note to be played */
    synth_note note;
    /** Either a value between 0x0 and 0xff or a envelop */
    synthVolume *vol;
};

/** Define a simple note envelop */
struct stSynthVolume {
    /** Initial volume */
    char ini;
    /** Final volume */
    char fin;
};

#endif /* __SYNTH_INTERNAL_TYPES_H__ */

