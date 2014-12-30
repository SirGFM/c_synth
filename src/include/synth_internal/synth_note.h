/**
 * @file src/include/synth_internal/synth_note.h
 *
 * Representation of a single note in a track
 */
#ifndef __SYNTH_NOTE_H_
#define __SYNTH_NOTE_H_

#include <synth/synth_types.h>
#include <synth_internal/synth_volume.h>

struct stSynthNote {
    /**
     * Value between 0 and 100, where 0 means only left channel and 100 means
     * only right channel
     */
    char pan;
    /**
     * Octave at which the note should play, from 1 to 8
     */
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
    /**
     * After how many samples should the note be muted
     */
    int keyoff;
    /**
     * Only used if type is N_loop; how many times has already looped
     */
    int numIterations;
    /**
     * Wave type to be synthesized
     */
    synth_wave wave;
    /**
     * Musical note to be played
     */
    synth_note note;
    /**
     * Either a value between 0x0 and 0xff or a envelop.
     */
    synthVolume *vol;
};
typedef struct stSynthNote synthNote;

/**
 * Initialize everything to default values.
 * 
 * @param note The note
 */
void synth_note_init(synthNote *note);

/**
 * Clean up. Since nothing is instantiated, it isn't needed, for now...
 * 
 * @param note The note
 */
void synth_note_clean(synthNote *note);


/**
 * Set the note panning.
 * 
 * @param note The note
 * @param pan Panning level. 0 is completelly to the left and 100 is
 * completelly to the right. The value is corretly clamped.
 */
void synth_note_setPan(synthNote *note, char pan);

/**
 * Set the note octave
 * 
 * @param note The note
 * @param octave The note octave, 1 being the lowest and 8 being the one of
 * highest pitch
 */
void synth_note_setOctave(synthNote *note, char octave);

/**
 * Set the note duration. Since it uses the current backend frequency, should
 * only be called after it was setuped
 * 
 * @param note The note
 * @param bpm Beats per minute
 * @param duration Bitfield for the duration. Each bit represents its 1/2^n
 * duration; i.e., 1/8 = (1000)b; 1/16. = (110000)b. It's as straight forward
 * as it seems
 */
void synth_note_setDuration(synthNote *note, int bpm, int duration);

/**
 * Set the kind of wave this note should play
 * 
 * @param note The note
 * @param wave The wave
 */
void synth_note_setWave(synthNote *note, synth_wave wave);

/**
 * Set the musical note that should be played
 * 
 * @param note The note
 * @param N The musical note to be played
 */
void synth_note_setNote(synthNote *note, synth_note N);

/**
 * Set the musical note that should be played
 * 
 * @param note The note
 * @return The current musical note
 */
synth_note synth_note_getNote(synthNote *note);

/**
 * Set how the volume behaves
 * 
 * @param note The note
 * @param vol The volume; must be caller released
 */
void synth_note_setVolume(synthNote *note, synthVolume *vol);

/**
 * Set the keyoff for the note; must be set after the duration!
 * 
 * @param note The note
 * @param keyoff Percentage (a integer between 0 and 100) of the duration that
 * should be played until the "note is released" (in a physical sense...
 * associate with how a keyboard note is pressed/released)
 */
void synth_note_setKeyoff(synthNote *note, int keyoff);

/**
 * Set how many times the 'looper' should count until continuing
 * 
 * @param note The note
 * @param num How many times it should loop
 */
void synth_note_setRepeatTimes(synthNote *note, int num);

/**
 * To which position the looper should jump, when it still must repeat.
 * 
 * @param note The note
 * @param pos Position into the track that the repeatition starts
 */
void synth_note_setJumpPosition(synthNote *note, int pos);

/**
 * Get the jump position and increase the jump counter
 * 
 * @param note The note
 * @return The position where the track should jump to
 */
int synth_note_jumpToPosition(synthNote *note);

/**
 * Check if the note is a loop and if there are any jumps left.
 *
 * @param note The note
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
synth_bool synth_note_doLoop(synthNote *note);

/**
 * Check whether a note finished playing or not
 *
 * @param note The note
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
synth_bool synth_note_didFinish(synthNote *note);

/**
 * Reset the note so it can be played again at a later time
 * 
 * @param note The note
 */
void synth_note_reset(synthNote *note);

/**
 * Synthesize part of this note to a buffer
 * 
 * @param note Note to be synthesized
 * @param samples How many samples there still are in this "slice"
 * @param left Left output buffer
 * @param right Right output buffer
 * @return How many samples from the next note should be synthesized (leftover)
 */
int synth_note_synthesize(synthNote *note, int samples, uint16_t *left,
    uint16_t *right);

#endif

