/**
 * @project c_synth
 * @license zlib license
 * @file    src/include/c_synth_internal/synth_memory.h
 *
 * Helps to manage memory for objects.
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
    int offset;
    int len;
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
static inline int synth_align32(int value) {
    return ((value + 3) & ~4)
}

/**
 * Retrieves a memory region within the objects' memory
 *
 * @param  [ in]offset Offset within objects' memory
 * @return             Pointer to the start of that region
 */
static inline void* synth_getRegion(int offset) {
    return (void*)( ((uint8_t*)pObjects) + synth_memorySize + offset);
}

#endif /* __SYNTH_MEMORY_H__ */

