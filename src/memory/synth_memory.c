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
synth_memory *pMemory = 0;
/** Amount of memory required by the memory */
const size_t synth_memorySize = synth_align32(sizeof(synth_memory));


/**
 * Setup the memory for storing objects
 *
 * Setup the statically alloc'ed memory. The memory is defined in such a
 * way that it may be later expanded into another region simply by
 * properly memcpy'ing it.
 *
 * @param  [ in]pBase          Memory region for objects
 * @param  [ in]instrumentsLen Bytes reserved for instruments
 * @param  [ in]songsLen       Bytes reserved for songs
 * @param  [ in]tracksLen      Bytes reserved for tracks
 * @param  [ in]stringsLen     Bytes reserved for strings
 * @param  [ in]stackLen       Bytes reserved for the stack
 */
void synth_setupMemory(void *pBase, int instrumentsLen, int songsLen,
        int tracksLen, int stringsLen, int stackLen) {
    pMemory = (synth_memory*)pBase;
    memset(pMemory, 0x0, synth_memorySize + instrumentsLen + songsLen
            + tracksLen + stringsLen + stackLen);

    /* Put region '_to_' on the start of the memory */
#define SET_BASE(_to_) \
  do { \
    pMemory->_to_.offset = (int)synth_memorySize; \
    pMemory->_to_.len = _to_##Len; \
  } while (0)
    /* Put region '_to_' right after region '_from_' */
#define SET_OFFSET(_to_, _from_) \
  do { \
    pMemory->_to_.offset = pMemory->_from_.offset + _from_##Len; \
    pMemory->_to_.len = _to_##Len; \
  } while (0)

    SET_BASE(instruments);
    SET_OFFSET(songs, instruments);
    SET_OFFSET(tracks, songs);
    SET_OFFSET(strings, tracks);
    SET_OFFSET(stack, strings);

#undef SET_BASE
#undef SET_OFFSET
}

