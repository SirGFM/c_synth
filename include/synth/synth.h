/**
 * @file include/synth/synth.h
 */
#ifndef __SYNTHCTX_STRUCT__
#define __SYNTHCTX_STRUCT__

/** 'Export' the synthCtx struct */
typedef struct stSynthCtx synthCtx;

#endif /* __SYNTHCTX_STRUCT__ */

#ifndef __SYNTH_H__
#define __SYNTH_H__

#include <synth/synth_errors.h>
#include <synth/synth_types.h>

/**
 * Retrieve the total size for a context
 * 
 * This allows an application to alloc it however it wants; In memory constraint
 * environments, it might be desired not to use dynamic memory, so this function
 * call can determined how much memory would be required for a context with some
 * restrictions
 * 
 * @param  [out]pSize      The size of the context struct in bytes
 * @param  [ in]maxSongs   How many songs can be compiled at the same time
 * @param  [ in]maxTracks  How many tracks can be used through all songs
 * @param  [ in]maxNotes   How many notes can be used through all tracks
 * @param  [ in]maxVolumes How many volumes can be used through all tracks
 * @return                 SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synth_getStaticContextSize(int *pSize, int maxSongs, int maxTracks,
        int maxNotes, int maxVolumes);

/**
 * Check how many bytes the context is currently using
 * 
 * @param  [out]pSize The size of the context struct in bytes
 * @param  [ in]pCtx  The synthesizer context
 * @return                SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synth_getContextSize(int *pSize, synthCtx *pCtx);

/**
 * Initialize the synthesizer context from a previously alloc'ed memory from the
 * user
 * 
 * This function call initializes a context that enforces restrictions over the
 * use of memory by the synthesizer; It won't alloc no extra memory, so it's
 * highly advised that the synthesizer is first tested using its dynamic
 * version, so the required memory to whatever is desired is calculated, before
 * trying to use this mode;
 * 
 * @param  [out]ppCtx      The new synthesizer context
 * @param  [ in]pMem       'synth_getContextSize' bytes or NULL, if the library
 *                         should alloc the structure however it wants
 * @param  [ in]freq       Synthesizer frequency, in samples per seconds
 * @param  [ in]maxSongs   How many songs can be compiled at the same time
 * @param  [ in]maxTracks  How many tracks can be used through all songs
 * @param  [ in]maxNotes   How many notes can be used through all tracks
 * @param  [ in]maxVolumes How many volumes can be used through all tracks
 * @return                 SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_MEM_ERR
 */
synth_err synth_initStatic(synthCtx **ppCtx, void *pMem, int freq, int maxSongs,
        int maxTracks, int maxNotes, int maxVolumes);

/**
 * Alloc and initialize the synthesizer
 * 
 * @param  [out]ppCtx    The new synthesizer context
 * @param  [ in]freq     Synthesizer frequency, in samples per seconds
 * @return               SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_MEM_ERR
 */
synth_err synth_init(synthCtx **ppCtx, int freq);

/**
 * Release any of the submodules in use and then release any other memory
 * alloc'ed by the library
 * 
 * @param  [ in]ppCtx The synthesizer context that will be dealloc'ed
 * @return            SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synth_free(synthCtx **ppCtx);

/**
 * Parse a file into a compiled song
 * 
 * The compiled song can later be used to playback the audio, get its samples
 * (i.e., buffer the whole song) or to export it to WAVE or OGG
 * 
 * @param  [out]pHandle   Handle of the loaded song
 * @param  [ in]pCtx      The synthesizer context
 * @param  [ in]pFilename File with the song's MML
 * @param                 SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_MEM_ERR, ...
 */
synth_err synth_compileSongFromFile(int *pHandle, synthCtx *pCtx,
        char *pFilename);

/**
 * Parse a string into a compiled song
 * 
 * The compiled song can later be used to playback the audio, get its samples
 * (i.e., buffer the whole song) or to export it to WAVE or OGG
 * 
 * @param  [out]pHandle Handle of the loaded song
 * @param  [ in]pCtx    The synthesizer context
 * @param  [ in]pString Song's MML
 * @param  [ in]length  The string's length (must contain the NULL-terminator!)
 * @param               SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_MEM_ERR, ...
 */
synth_err synth_compileSongFromString(int *pHandle, synthCtx *pCtx,
        char *pString, int length);

#define synth_compileSongFromStringStatic(pHandle, pCtx, pString) \
  synth_compileSongFromString(pHandle, pCtx, pString, sizeof(pString))

/**
 * Return a string representing the compiler error raised
 * 
 * This string is statically allocated and mustn't be freed by user
 * 
 * @param  [out]ppError The error string
 * @param  [ in]pCtx    The synthesizer context
 * @return              SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_NO_ERRORS
 */
synth_err synth_getCompilerErrorString(char **ppError, synthCtx *pCtx);

/**
 * Return the number of tracks in a song
 * 
 * @param  [out]pNum   The number of tracks
 * @param  [ in]pCtx   The synthesizer context
 * @param  [ in]handle Handle of the audio
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_INVALID_INDEX
 */
synth_err synth_getAudioTrackCount(int *pNum, synthCtx *pCtx, int handle);

/**
 * Retrieve the number of samples in a track
 * 
 * @param  [out]pLen   The length of the track in samples
 * @param  [ in]pCtx   The synthesizer context
 * @param  [ in]handle Handle of the audio
 * @param  [ in]track  Track index
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_INVALID_INDEX
 */
synth_err synth_getTrackLength(int *pLen, synthCtx *pCtx, int handle,
        int track);

/**
 * Retrieve the number of samples until a track's loop point
 * 
 * @param  [out]pLen   The length of the track's intro
 * @param  [ in]pCtx   The synthesizer context
 * @param  [ in]handle Handle of the audio
 * @param  [ in]pTrack The track
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synth_getTrackIntroLength(int *pLen, synthCtx *pCtx, int handle,
        int track);

#if 0
/**
 * Initialize the synthesizer, including buffering thread and other
 * buffers
 * 
 * @param freq At which frequency (samples per minute) should synthesizer work
 * @param doBuf Whether the buffering thread should run or not
 * @param size How many samples should be buffered per channel. Must be at least
 *             synth_bkend_getSamplesPerChannel()
 * @param doBkend Whether should start the compiled backend
 * @return Error code
 */
synth_err synth_init(int freq, synth_bool doBuf, int size, synth_bool doBkend);

/**
 * Clean up any allocated memory and the buffering thread
 */
synth_err synth_clean();

/**
 * Get the frequency being used
 * 
 * @return The frequency
 */
int synth_getFrequency();

/**
 * Get how many samples should be buffered
 * 
 * @return The number of samples
 */
int synth_getSamples();

/**
 * Signal the buffering thread to buffer more samples
 */
void synth_requestBuffering();

/**
 * Try to lock buffer, allowing it to be read. SYNTH_COULDNT_LOCK means that the
 * buffer is already in use.
 * 
 * @return Error code
 */
synth_err synth_lockBuffer();

/**
 * Unlock the buffer
 */
void synth_unlockBuffer();

/**
 * Read some samples from the buffer. If it returns
 * SYNTH_BUFFER_NOT_ENOUGH_SAMPLES then more samples were requested than there
 * are currently buffered.
 * 
 * @param left Returns the left channel buffer
 * @param right Returns the right channel buffer
 * @param samples How many samples should be read
 * @return Error code
 */
synth_err synth_getBuffer(uint16_t **left, uint16_t **right, int samples);
#endif /* 0 */

#endif /* __SYNTH_H__ */

