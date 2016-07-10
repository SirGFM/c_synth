/**
 * @project c_synth
 * @license zlib license
 * @file    src/include/c_synth_internal/synth_memory.h
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
#ifndef __SYNTH_MEMORY_H__
#define __SYNTH_MEMORY_H__

/** Required for fixed-width sizes */
#include <stdint.h>
/** Required for size_t */
#include <stddef.h>

/* == MEMORY TYPES ================================================== */

/** Defines a region within the base memory */
struct stSynth_region {
    /** Offset from the memory's base */
    int offset;
    /** Size of the memory region */
    int len;
#  ifdef ENABLE_MALLOC
    /** Amount of used memory */
    int used;
#  endif
};
typedef struct stSynth_region synth_region;

/** Memory used for objects */
struct stSynth_memory {
    /** Instruments parsed */
    synth_region instruments;
    /** Songs parsed */
    synth_region songs;
    /** Tracks parsed */
    synth_region tracks;
    /** Strings tokenized by the lexer */
    synth_region strings;
};
typedef struct stSynth_memory synth_memory;

/* == MEMORY GLOBAL VARS ============================================ */

/** Objects' reference */
extern synth_memory *pObjects;
/** Amount of memory required by the memory */
extern const size_t synth_memorySize;

/* == MEMORY FUNCTIONS ============================================== */

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
        int tracksLen, int stringsLen);

#  ifdef ENABLE_MALLOC
/* == DYNAMIC MEMORY FUNCTIONS ====================================== */

/**
 * Dynamically expands the memory.
 *
 * If the memory hasn't been setup'ed, it will be properly alloc'ed.
 *
 * @param  [ in]instrumentsLen Bytes reserved for instruments
 * @param  [ in]songsLen       Bytes reserved for songs
 * @param  [ in]tracksLen      Bytes reserved for tracks
 * @param  [ in]stringsLen     Bytes reserved for strings
 */
void synth_expandMemory(int instrumentsLen, int songsLen, int tracksLen,
        int stringsLen);

/**
 * Dynamically expands the memory for instruments.
 *
 * If the memory hasn't been setup'ed, it will be properly alloc'ed.
 *
 * @param  [ in]len Bytes reserved for instruments
 */
void synth_expandInstruments(int len);

/**
 * Dynamically expands the memory for songs.
 *
 * If the memory hasn't been setup'ed, it will be properly alloc'ed.
 *
 * @param  [ in]len Bytes reserved for songs
 */
void synth_expandSongs(int len);

/**
 * Dynamically expands the memory for tracks.
 *
 * If the memory hasn't been setup'ed, it will be properly alloc'ed.
 *
 * @param  [ in]len Bytes reserved for tracks
 */
void synth_expandTracks(int len);

/**
 * Dynamically expands the memory for strings.
 *
 * If the memory hasn't been setup'ed, it will be properly alloc'ed.
 *
 * @param  [ in]len Bytes reserved for strings
 */
void synth_expandStrings(int len);

/**
 * Clean up the dinamically allo'ec memory
 */
void synth_cleanMemory();

#  endif

/* == MEMORY INLINE FUNCTIONS ======================================= */

/**
 * Align a value to the next 32 bits aligned value
 *
 * Note that if the value passed was already 32 bits aligned, it will be
 * returned unchanged.
 *
 * @param  [ in]value Value to be aligned
 * @return            Aligned value
 */
#define synth_align32(value) ((value + 3) & ~4)

/**
 * Retrieves a memory region within another region
 *
 * @param  [ in]pMemory Base memory region
 * @param  [ in]offset  Offset within the memory
 * @return              Pointer to the start of that region
 */
#define synth_getMemory(pMemory, offset) ((void*)(pMemory + offset))

/**
 * Retrieves a memory region within the objects' memory
 *
 * @param  [ in]offset Offset within objects' memory
 * @return             Pointer to the start of that region
 */
#define synth_getRegion(offset) (synth_getMemory((uint8_t*)pObjects, offset))

#endif /* __SYNTH_MEMORY_H__ */

