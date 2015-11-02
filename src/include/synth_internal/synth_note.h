/**
 * @file src/include/synth_internal/synth_note.h
 *
 * Representation of a single note in a track
 */
#ifndef __SYNTH_NOTE_H__
#define __SYNTH_NOTE_H__

#include <synth/synth.h>
#include <synth/synth_errors.h>

#include <synth_internal/synth_types.h>
#include <synth_internal/synth_volume.h>

/**
 * Retrieve a new note pointer, so it can be later initialized
 * 
 * @param  [out]ppNote The new note
 * @param  [ in]pCtx   The synthesizer context
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_MEM_ERR
 */
synth_err synthNote_init(synthNote **ppNote, synthCtx *pCtx);

/**
 * Retrieve a new note pointer, already initialized as a loop
 * 
 * @param  [out]ppNote   The new note
 * @param  [ in]pCtx     The synthesizer context
 * @param  [ in]repeat   How many times the loop should repeat
 * @param  [ in]position Note to which the song should jump back, on loop
 * @return               SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_MEM_ERR
 */
synth_err synthNote_initLoop(synthNote **ppNote, synthCtx *pCtx, int repeat,
        int position);

/**
 * Set the note panning
 * 
 * Defines which channel, if any, should be louder; If the value is outside the
 * expected [0, 100] range, it's automatically clamped
 * 
 * @param [ in]pNote The note
 * @param [ in]pan   Panning level; 0 means completelly to the left and 100
 *                   means completelly to the right
 * @return           SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthNote_setPan(synthNote *pNote, char pan);

/**
 * Set the note octave
 * 
 * Define higher the pitch, the highed the numeric representation; The value is
 * clamped to the range [1, 8]
 * 
 * @param [ in]pNote  The note
 * @param [ in]octave The octave
 * @return            SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthNote_setOctave(synthNote *pNote, char octave);

/**
 * Set the note wave
 * 
 * If the wave isn't valid, it will be set to noise!
 * 
 * @param [ in]pNote The note
 * @param [ in]wave  The wave
 * @return           SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthNote_setWave(synthNote *pNote, synth_wave wave);

/**
 * Set the musical note
 * 
 * @param [ in]pNote The note
 * @param [ in]note  The musical note
 * @return           SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthNote_setNote(synthNote *pNote, synth_note note);

/**
 * Set the note duration
 * 
 * NOTE: The duration is stored in samples
 * 
 * @param [ in]pNote    The note
 * @param [ in]pCtx     The synthesizer context
 * @param [ in]bpm      The song's speed in beats-per-minute
 * @param [ in]duration Bitfield for the duration. Each bit represents a
 *                      fraction of the duration;
 * @return              SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthNote_setDuration(synthNote *pNote, synthCtx *pCtx, int bpm,
        int duration);

/**
 * Set the note's keyoff time
 * 
 * NOTE: This parameter must be set after the duration
 * 
 * Calculate (and store) after how many samples this note should be released;
 * The value must be a number in the range [0, 100], represeting the percentage
 * of the note that it must keep playing
 * 
 * @param  [ in]pNote  The note
 * @param  [ in]keyoff The percentage of the note duration before it's released
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthNote_setKeyoff(synthNote *pNote, int keyoff);

/**
 * Set the volume envelop
 * 
 * @param  [ in]pNote The note
 * @param  [ in]pVol  The volume
 * @return            SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthNote_setVolume(synthNote *pNote, synthVolume *pVol);

/**
 * Check if the note is a loop point
 * 
 * @param  [ in]pNote The note
 * @return            SYNTH_TRUE, SYNTH_FALSE
 */
synth_bool synthNote_isLoop(synthNote *pNote);

/**
 * Retrieve the note duration, in samples
 * 
 * @param  [out]pVal  The duration
 * @param  [ in]pNote The note
 * @return            SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthNote_getDuration(int *pVal, synthNote *pNote);

/**
 * Retrieve the panning of the note, where 0 means completely on the left
 * channel and 100 means completely on the right channel
 * 
 * @param  [out]pVal  The panning
 * @param  [ in]pNote The note
 * @return            SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthNote_getPan(char *pVal, synthNote *pNote);

/**
 * Retrieve the number of times this loop should repeat
 * 
 * @param  [out]pVal  The repeat count
 * @param  [ in]pNote The note
 * @return            SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthNote_getRepeat(int *pVal, synthNote *pNote);

/**
 * Retrieve the position, in the track, to which it should jump on loop
 * 
 * @param  [out]pVal  The repeat position
 * @param  [ in]pNote The note
 * @return            SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthNote_getJumpPosition(int *pVal, synthNote *pNote);

/**
 * Render a note into a buffer
 * 
 * The buffer must have at least 'synthNote_getDuration' bytes time the number
 * of bytes required by the mode
 * 
 * @param  [ in]pBuf      Buffer that will be filled with the track
 * @param  [ in]pNote     The note
 * @param  [ in]mode      Desired mode for the wave
 * @param  [ in]synthFreq Synthesizer's frequency
 * @return                SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthNote_render(char *pBuf, synthNote *pNote, synthBufMode mode,
        int synthFreq);

#if 0

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
 * Get the note panning.
 * 
 * @param note The note
 */
char synth_note_getPan(synthNote *note);

/**
 * Set the note octave
 * 
 * @param note The note
 * @param octave The note octave, 1 being the lowest and 8 being the one of
 * highest pitch
 */
void synth_note_setOctave(synthNote *note, char octave);

/**
 * Get the note octave
 * 
 * @param note The note
 */
char synth_note_getOctave(synthNote *note);

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
 * Get the note duration.
 * 
 * @param note The note
 */
int synth_note_getDuration(synthNote *note);

/**
 * Get the note length in samples
 * 
 * @param note The note
 */
int synth_note_getLen(synthNote *note);

/**
 * Set the kind of wave this note should play
 * 
 * @param note The note
 * @param wave The wave
 */
void synth_note_setWave(synthNote *note, synth_wave wave);

/**
 * Get the kind of wave this note should play
 * 
 * @param note The note
 */
synth_wave synth_note_getWave(synthNote *note);

/**
 * Set the musical note that should be played
 * 
 * @param note The note
 * @param N The musical note to be played
 */
void synth_note_setNote(synthNote *note, synth_note N);

/**
 * Get the musical note that should be played
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
 * Get how the volume behaves
 * 
 * @param note The note
 */
synthVolume* synth_note_getVolume(synthNote *note);

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
 * Set the keyoff for the note; must be set after the duration!
 * 
 * @param note The note
 */
int synth_note_getKeyoff(synthNote *note);

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

synth_bool synth_note_didFinishHacky(synthNote *note, int _pos);

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

int synth_note_synthesizeHacky(synthNote *note, int samples, uint16_t *left,
    uint16_t *right, int *_pos);

/**
 * Get how many samples a note would have
 * 
 * @param bpm Beats per minute
 * @param duration Bitfield for the duration. Each bit represents its 1/2^n
 * duration; i.e., 1/8 = (1000)b; 1/16. = (110000)b. It's as straight forward
 * @return The length
 */
int synth_note_getSampleSize(int bpm, int duration);

#endif /* 0 */

#endif /* __SYNTH_NOTE_H__ */

