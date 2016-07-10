/**
 * @project c_synth
 * @license zlib license
 * @file    src/memory/synth_memory.c
 *
 * @summary Helps to manage memory for objects.
 *
 * This module should be used to logically separate the synthesizer's
 * data memory into a few different regions. Each region should have a
 * fixed size and shall be used by a single module (e.g., one region for
 * songs and another one for instruments).
 *
 * The default use-case expects this memory to be initialized with
 * exactly the required amount of memory, perfectly distributed between
 * each region. However, it's written in such a way that one may request
 * its memory to be expanded and it will automatically fix all pointers
 * within the module. This mode may be useful for the offline tools,
 * since it will be able to calculate the required memory for a few
 * songs/instruments without relying in aproximations or guesses by the
 * user.
 */
#include <c_synth_internal/synth_memory.h>

/** Required for size_t */
#include <stddef.h>
/** Required for memset */
#include <string.h>

/** Objects' reference */
synth_memory *pObjects = 0;
/** Amount of memory required by the memory */
const size_t synth_memorySize = synth_align32(sizeof(synth_memory));


/**
 * Setup the memory for storing objects
 *
 * Setup the statically alloc'ed memory. The memory is defined in such a
 * way that it may be later expanded into another region simply by
 * properly memcpy'ing it.
 *
 * @param  [ in]pMemory        Memory region for objects
 * @param  [ in]instrumentsLen Bytes reserved for instruments
 * @param  [ in]songsLen       Bytes reserved for songs
 * @param  [ in]tracksLen      Bytes reserved for tracks
 * @param  [ in]stringsLen     Bytes reserved for strings
 */
void synth_setupMemory(void *pMemory, int instrumentsLen, int songsLen,
        int tracksLen, int stringsLen) {
    pObjects = (synth_memory*)pMemory;
    memset(pObjects, 0x0, synth_memorySize + instrumentsLen + songsLen
            + tracksLen + stringsLen);

    pObjects->instruments.offset = (int)synth_memorySize;
    pObjects->instruments.len = instrumentsLen;
    pObjects->songs.offset = pObjects->instruments.offset + instrumentsLen;
    pObjects->songs.len = songsLen;
    pObjects->tracks.offset = pObjects->songs.offset + songsLen;
    pObjects->tracks.len = tracksLen;
    pObjects->strings.offset = pObjects->tracks.offset + tracksLen;
    pObjects->strings.len = stringsLen;
}

