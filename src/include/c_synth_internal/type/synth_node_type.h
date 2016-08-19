/**
 * @project   c_synth
 * @license   zlib license
 * @file      src/include/c_synth_internal/type/synth_node_type.h
 *
 * @summary   Types that are mainly used within nodes.
 *
 * @typedef   synth_waveform  Waveform for synthesizing notes.
 * @typedef   synth_envelope  Envelope for the note's amplitude.
 * @typedef   synth_note_data Musical note within a song.
 * @typedef   synth_loop_data Defines loops within songs.
 * @typedef   synth_node_data Generic data for a node.
 *
 * This header shouldn't be directly included into another! In order to
 * use it, include c_synth_internal/synth_types.h.
 */
#ifndef __SYNTH_NODE_TYPE_H__
#define __SYNTH_NODE_TYPE_H__

/**
 * Waveform for synthesizing notes
 *
 * WF_*_PW waves define pulse waves of a given duty cycle. The duty
 * cicle defines for how long the wave is kept on high amplitude.
 */
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
 *
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
 * A musical note within a song.
 *
 * Differently from a synth_note, this structure holds the note's pitch,
 * its duration and its octave. Therefore, it carries all information
 * necessary to play the note.
 *
 * Even though the amplitude isn't store within a note, "extended notes"
 * may be implemented by setting a track's amplitude before playing the
 * note.
 */
struct stSynth_note_data {
    synth_note note;
    uint8_t duration;
    uint8_t octave;
};
typedef struct stSynth_note_data synth_note_data;

/**
 * Defines loops within songs.
 *
 * It gets parsed from STK_LOOP_START, STK_LOOP_END and STK_REPEAT
 *
 * Basically, this node is either used to set another loop data
 * (essentially resetting it) or defines the next position of the track
 * that should be played (i.e., jumps to that position).
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
 * Generic data for a node.
 *
 * How it should be interpreted (i.e., which of its "fields" should be
 * acessed) depends on the node type.
 */
union unSynth_node_data {
    synth_note_data note;
    synth_envelope envelope;
    synth_loop_data loop;
    uint16_t value;
};
typedef union unSynth_node_data synth_node_data;

#endif /* __SYNTH_NODE_TYPE_H__ */

