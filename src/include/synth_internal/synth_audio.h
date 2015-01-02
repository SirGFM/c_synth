/**
 * @file src/include/synth_internal/synth_audio.h
 */
#ifndef __SYNTH_AUDIO_H_
#define __SYNTH_AUDIO_H_

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
typedef stSynthAudio synthAudio;

/**
 * 
 */
synth_err synth_audio_allocAudio(synthAudio **audio);
synth_err synth_audio_addTrack(synthAudio *audio, synthTrack *track);

#endif

