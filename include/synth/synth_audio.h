/**
 * @file include/synth/synth_audio.h
 * 
 * An audio structure. Can play multiple tracks simutaneously.
 */
#ifndef __SYNTH_AUDIO_H_
#define __SYNTH_AUDIO_H_

#include <synth/synth_errors.h>

/**
 * Forward declaration of the audio type
 */
typedef struct stSynthAudio synthAudio;

synth_err synth_audio_loadf(synthAudio **audio, char *filename);
synth_err synth_audio_loads(synthAudio **audio, char *mml, int len);

/**
 * Free up an audio structure
 * 
 * @param audio Reference to the audio pointer
 */
void synth_audio_free(synthAudio **audio);

void synth_audio_play(synthAudio *audio);

#endif

