/**
 * @project   c_synth
 * @license   zlib license
 * @file      src/include/c_synth_internal/type/synth_parser_type.h
 *
 * @summary   Types that are mainly generated by the parser.
 *
 * @typedef   synth_node       Node/instruction within a track.
 * @typedef   synth_instrument Controls how notes ares synthesized.
 * @typedef   synth_track      Track/Voice within a song.
 * @typedef   synth_song       A collection of tracks and their speed
 *
 * This header shouldn't be directly included into another! In order to
 * use it, include c_synth_internal/synth_types.h.
 */
#ifndef __SYNTH_PARSER_TYPE_H__
#define __SYNTH_PARSER_TYPE_H__

/** A node within a track */
struct st_synth_node {
    synth_token type;
    synth_node_data data;
};
typedef struct st_synth_node synth_node;

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

/**
 * A track/voice within a song
 *
 * Each track has its own controllers for position and instrument so
 * they may loop and play independently.
 */
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
     *
     * This implies that a track will never simultaneously play more
     * than once.
     */
    uint16_t position;
    /**
     * Number of nodes on the track.
     *
     * 16 bits enables 65535 nodes per track. Considering a 4/4 song at
     * 120 bpm that only has 1/64 notes, it would be 34 minutes long. It
     * seems to be way more than enough for any common cases.
     */
    uint16_t numNodes;
};
typedef struct stSynth_track synth_track;

/** A collection of tracks and its speed */
struct stSynth_song {
    /**
     * Time signature.
     *
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

#endif /* __SYNTH_PARSER_TYPE_H__ */

