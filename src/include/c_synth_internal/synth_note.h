/**
 * @file src/include/synth_internal/synth_note.h
 *
 * Representation of a single note in a track
 */
#ifndef __SYNTH_NOTE_H__
#define __SYNTH_NOTE_H__

#include <c_synth/synth.h>
#include <c_synth/synth_errors.h>

#include <c_synth_internal/synth_types.h>
#include <c_synth_internal/synth_volume.h>

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
 * Set the characteristics of the note's duration
 * 
 * NOTE: This parameter must be set after the duration
 * 
 * All values must be in the range [0, 100]. The attack is campled to the range
 * [0, keyoff] and the release is campled to the range [keyoff, 100]. Although
 * the parameter express the percentage of the note's duration, the value is
 * stored in samples.
 * 
 * @param  [ in]pNote   The note
 * @param  [ in]attack  The percentage of the note duration before it reaches
 *                      its full amplitude
 * @param  [ in]keyoff  The percentage of the note duration before it's released
 * @param  [ in]release The percentage of the note duration before it halts
 *                      completely
 * @return              SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthNote_setKeyoff(synthNote *pNote, int attack, int keyoff,
        int release);

/**
 * Set the volume envelop
 * 
 * @param  [ in]pNote  The note
 * @param  [ in]volume The volume
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthNote_setVolume(synthNote *pNote, int volume);

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
 * @param  [ in]pCtx      The synthesizer context
 * @param  [ in]mode      Desired mode for the wave
 * @param  [ in]synthFreq Synthesizer's frequency
 * @return                SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthNote_render(char *pBuf, synthNote *pNote, synthCtx *pCtx,
        synthBufMode mode, int synthFreq);

#endif /* __SYNTH_NOTE_H__ */

