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
     * Duration of the note in samples (depends on the sample rate)
     */
    int len;
    /**
     * Current position on the note
     */
    int pos;
    /**
     * After how many samples should the note be muted
     */
    int keyoff;
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

void synth_note_init(synthNote *note);
void synth_note_clean(synthNote *note);

/**
 * Synthesize part of this note to the current backend
 * 
 * @param note Note to be synthesized
 * @param time How many samples there still are in this "slice"
 * @return How many samples from the next note should be synthesized (leftover)
 */
int synth_note_synthesize(synthNote *note, int time);

#endif

