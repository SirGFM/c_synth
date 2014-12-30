/**
 * @file src/synth_note.c
 */
#include <synth/synth_assert.h>
#include <synth/synth_backend.h>
#include <synth/synth_errors.h>
#include <synth/synth_types.h>
#include <synth_internal/synth_note.h>
#include <synth_internal/synth_volume.h>

/**
 * Static array with note frequencies; to get lower octaves one, right-shift
 * by the number of octaves going down
 */
static int note_frequency[] = 
{
 4186, // C 8
 4435, // C#8
 4699, // D 8
 4978, // D#8
 5274, // E 8
 5588, // F 8
 5920, // F#8
 6272, // G 8
 6645, // G#8
 7040, // A 8
 7459, // A#8
 7902  // B 8
};

/**
 * Initialize everything to default values.
 * 
 * @param note The note
 */
void synth_note_init(synthNote *note) {
    synth_note_setPan(note, 50);
    synth_note_setOctave(note, 4);
    synth_note_setDuration(note, 60, 4);
    synth_note_setWave(note, W_SQUARE);
    synth_note_setNote(note, N_A);
    synth_note_setVolume(note, 0);
    synth_note_setKeyoff(note, 75);
    
    note->numIterations = 0;
    note->pos = 0;
}

/**
 * Clean up. Since nothing is instantiated, it isn't needed, for now...
 * 
 * @param note The note
 */
void synth_note_clean(synthNote *note) {
    synth_note_init(note);
}


/**
 * Set the note panning.
 * 
 * @param note The note
 * @param pan Panning level. 0 is completelly to the left and 100 is
 * completelly to the right. The value is corretly clamped.
 */
void synth_note_setPan(synthNote *note, char pan) {
    if (pan < 0)
        pan = 0;
    else if (pan > 100)
        pan = 100;
    note->pan = pan;
}

/**
 * Set the note octave
 * 
 * @param note The note
 * @param octave The note octave, 1 being the lowest and 8 being the one of
 * highest pitch
 */
void synth_note_setOctave(synthNote *note, char octave) {
    if (octave < 1)
        octave = 1;
    else if (octave > 8)
        octave = 8;
    note->octave = octave;
}

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
void synth_note_setDuration(synthNote *note, int bpm, int duration) {
    int freq, time, nSamples;
    
    // Reset the note length
    note->len = 0;
    SYNTH_ASSERT(bpm > 0);
    
    // Get the global frequency
    freq = synth_bkend_getFrequency();
    // Calculate how long (in miliseconds) a semibreve note should last
    time = 1000 * 4 * 60 / bpm;
    // Calculate how many samples a semibreve should last
    nSamples = freq * time;
    
    // Accumulate the duration of the note, in samples
    while (duration > 0) {
        // If the note has this note value
        if ((duration & 1) == 1)
            // Accumulate the number of samples for thie note value
            note->len += nSamples / 1000;
        
        // Get the next note value (semibreve->minim->etc)
        duration >>= 1;
        nSamples >>= 1;
    }
    
__err:
    return;
}

/**
 * Set the kind of wave this note should play
 * 
 * @param note The note
 * @param wave The wave
 */
void synth_note_setWave(synthNote *note, synth_wave wave) {
    note->wave = wave;
}

/**
 * Set the musical note that should be played
 * 
 * @param note The note
 * @param N The musical note to be played
 */
void synth_note_setNote(synthNote *note, synth_note N) {
    note->note = N;
}

/**
 * Set the musical note that should be played
 * 
 * @param note The note
 * @return The current musical note
 */
synth_note synth_note_getNote(synthNote *note) {
    return note->note;
}

/**
 * Set how the volume behaves
 * 
 * @param note The note
 * @param vol The volume; must be caller released
 */
void synth_note_setVolume(synthNote *note, synthVolume *vol) {
    note->vol = vol;
}

/**
 * Set the keyoff for the note; must be set after the duration!
 * 
 * @param note The note
 * @param keyoff Percentage (a integer between 0 and 100) of the duration that
 * should be played until the "note is released" (in a physical sense...
 * associate with how a keyboard note is pressed/released)
 */
void synth_note_setKeyoff(synthNote *note, int keyoff) {
    note->keyoff = note->len * keyoff / 100;
}

/**
 * Set how many times the 'looper' should count until continuing
 * 
 * @param note The note
 * @param num How many times it should loop
 */
void synth_note_setRepeatTimes(synthNote *note, int num) {
    note->len = num;
}

/**
 * To which position the looper should jump, when it still must repeat.
 * 
 * @param note The note
 * @param pos Position into the track that the repeatition starts
 */
void synth_note_setJumpPosition(synthNote *note, int pos) {
    note->pos = pos;
}

/**
 * Get the jump position and increase the jump counter
 * 
 * @param note The note
 * @return The position where the track should jump to
 */
int synth_note_jumpToPosition(synthNote *note) {
    note->numIterations++;
    
    return note->pos;
}

/**
 * Check if the note is a loop and if there are any jumps left.
 *
 * @param note The note
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
synth_bool synth_note_doLoop(synthNote *note) {
    if (note->note != N_LOOP || note->numIterations >= note->len)
        return SYNTH_FALSE;
    return SYNTH_TRUE;
}

/**
 * Check whether a note finished playing or not
 *
 * @param note The note
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
synth_bool synth_note_didFinish(synthNote *note) {
    if (note->pos >= note->len || note->note == N_LOOP)
        return SYNTH_TRUE;
    return SYNTH_FALSE;
}

/**
 * Reset the note so it can be played again at a later time
 * 
 * @param note The note
 */
void synth_note_reset(synthNote *note) {
    if (note->note == N_LOOP)
        note->numIterations = 0;
    else
        note->pos = 0;
}

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
    uint16_t *right) {
    int rem;
    
    // Set the remainder for a case it returns before actually modifing it
    rem = samples;
    // Simply pass through, if it's a "loop note"
    SYNTH_ASSERT(note->note != N_LOOP);
    
    // Actually buffer the note
    if (note->note == N_REST) {
        note->pos += rem;
        if (note->pos > note->len) {
            rem = note->pos - note->len;
            note->pos = note->len;
        }
        else
            rem = 0;
    }
    else {
        int freq, spc, i;
        
        // See note bellow about samples per cycle
        freq = synth_bkend_getFrequency();
        spc = freq / (note_frequency[note->note] >> (8 - note->octave));
        
        i = 0;
        while (i < rem && note->pos + i  < note->len) {
            int perc;
            char amp;
            
            // Get the position in the cycle as a 'percentage' in [0,1024)
            perc = (note->pos + i) % spc;
            perc = (perc << 10) / spc;
            
            // Get the amplitude in [0, 255] for the wave at that point
            switch (note->wave) {
                case W_SQUARE:
                    amp = (perc < 512)?synth_vol_getVolume(note->vol, perc):0;
                    break;
                // TODO other waves
                // TODO modularize
                default:
                    amp = 0;
            }
            
            // Pan the sound and buffer it
            if (note->pos + i < note->keyoff) {
                left[i] += 0x7fff * (100 - note->pan) * amp / 25600;
                right[i] += 0x7fff * note->pan * amp / 25600;
            }
            
            i++;
        }
        
        rem -= i;
        note->pos += i;
    }
    
__err:
    return rem;
}

