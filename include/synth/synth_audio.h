/**
 * @file include/synth/synth_audio.h
 * 
 * An audio structure. Can play multiple tracks simutaneously.
 */
#ifndef __SYNTH_AUDIO_H_
#define __SYNTH_AUDIO_H_

#include <synth/synth_errors.h>
#include <synth/synth_types.h>

/**
 * Forward declaration of the audio type
 */
typedef struct stSynthAudio synthAudio;

/**
 * Parse a mml file into an audio
 * 
 * @param ctx Variable that will store the audio
 * @param filename MML's filename
 * @return The error code
 */
synth_err synth_audio_loadf(synthAudio **audio, char *filename);

/**
 * Parse a mml string into an audio
 * 
 * @param ctx Variable that will store the audio
 * @param mml String with the mml audio (it needn't be null-terminated)
 * @param len String's length
 * @return The error code
 */
synth_err synth_audio_loads(synthAudio **audio, char *mml, int len);

/**
 * Free up an audio structure
 * 
 * @param audio Reference to the audio pointer
 */
void synth_audio_free(synthAudio **audio);

/**
 * Reset and play an audio. The audio shouldn't be played, otherwise the
 * synthesizer will have unexpected behaviour
 * 
 * @param audio The audio
 * @return The error code
 */
synth_err synth_audio_playAudio(synthAudio *audio);

/**
 * Play a single track audio. Different from playAudio, this can be called with
 * an already playing audio
 * 
 * @param audio The audio
 * @return The error code
 */
synth_err synth_audio_playSFX(synthAudio *audio);

/**
 * Reset and play an audio as a bgm
 * 
 * @param audio The audio
 */
void synth_audio_playBGM(synthAudio *audio);

/**
 * Stop the currently playing bgm
 */
void synth_audio_stopBgm();

/**
 * Whether the audio finished playing
 * 
 * @param audio The audio
 * @return Error code
 */
synth_bool synth_audio_didFinish(synthAudio *audio);

#endif

