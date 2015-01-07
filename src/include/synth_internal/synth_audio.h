/**
 * @file src/include/synth_internal/synth_audio.h
 */
#ifndef __SYNTH_INTERNAL_AUDIO_H_
#define __SYNTH_INTERNAL_AUDIO_H_

#include <synth/synth_audio.h>
#include <synth/synth_errors.h>
#include <synth_internal/synth_track.h>

struct stSynthAudio {
    synthTrack **tracks;
    /**
     * How many tracks are currently in use
     */
    int len;
    /**
     * How many slots there actually are in tracks
     */
    int tracksLen;
};
typedef struct stSynthAudio synthAudio;

/**
 * Alloc a new audio structure and initialize it
 * 
 * @param audio Reference to the audio pointer
 * @return Error code
 */
synth_err synth_audio_allocAudio(synthAudio **audio);
/**
 * Adds a new track to the audio
 * Though the track will be released automatically, its notes and volume won't
 * 
 * @param audio The audio
 * @param track The track
 * @return Error code
 */
synth_err synth_audio_addTrack(synthAudio *audio, synthTrack *track);

#endif

