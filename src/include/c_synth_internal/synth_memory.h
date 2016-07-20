/**
 * @project c_synth
 * @license zlib license
 * @file    src/include/c_synth_internal/synth_memory.h
 *
 * @summary   Container where parsed input is stored
 *
 * @typedef   synth_region     Position of region within the container.
 * @typedef   synth_memory     The container, divided into regions.
 *
 * @globalvar pMemory          Reference to the data container.
 * @globalvar synth_memorySize Amount of bytes require by the container.
 *
 * @description
 *
 * Describes how an user-supplied memory region should be divided
 * between all parsed data. The memory is divided as a container for the
 * following objects:
 *   - instruments;
 *   - songs;
 *   - tracks;
 *   - nodes;
 *   - strings;
 *   - other temporary data.
 *
 * Instruments, songs and tracks are the logical representation of
 * parsed structures. Take a look at the wiki for more info about them.
 * Similarly, nodes represent a unit within the track, which is either
 * a control node, an envelope one or a note. Strings are usually used
 * to point at instruments, but only while parsing an input. "Other
 * temporary data" may be anything (e.g., the string just tokenized by
 * the lexer or a macro).
 *
 * When the synthesizer is used as a library, setting up this module
 * requires knowing exactly how much memory each sub-region will need.
 * Although this may prove slightly cumbersome, it aims to help
 * targeting devices with restricted memory (mobile, HTML5, older
 * consoles) and ensuring that there are no memory leaks.
 *
 * To ease that restriction, some of the synthesizer's tools (located on
 * the apps directory) use dynamic memory and report the used memory.
 * This way, one could even call those apps from their build script,
 * redirect the memory requirement to a file and use that as input on
 * their final application.
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
    /** Amount of used memory */
    int used;
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
    /** Nodes within tracks */
    synth_region nodes;
    /** Strings saved for later use (e.g., an instrument's name) */
    synth_region strings;
    /** Shared region for any needed memory */
    synth_region stack;
};
typedef struct stSynth_memory synth_memory;

/* == MEMORY GLOBAL VARS ============================================ */

/** Objects' reference */
extern synth_memory *pMemory;
/** Amount of memory required by the memory */
extern const size_t synth_memorySize;

/* == MEMORY FUNCTIONS ============================================== */

/**
 * Calculate how many bytes each memory region requires, given how many
 * units they have.
 *
 * The returned number of bytes may be slightly greater than the exact
 * amount required. This is in order to align every region.
 *
 * @param  [ in]instrumentsCount Number of instruments available
 * @param  [ in]songsCount       Number of songs available
 * @param  [ in]tracksCount      Number of tracks available
 * @param  [ in]nodesCount       Number of nodes available
 * @return                       The length in bytes for all regions
 */
int synth_computeRequiredMemory(int instrumentsCount, int songsCount
        , int tracksCount, int nodesCount);

/**
 * Setup the memory for storing objects
 *
 * Setup the statically alloc'ed memory. The memory is defined in such a
 * way that it may be later expanded into another region simply by
 * properly memcpy'ing it.
 *
 * @param  [ in]pBase          Memory region for objects
 * @param  [ in]instrumentsNum Number of instruments available
 * @param  [ in]songsNum       Number of songs available
 * @param  [ in]tracksNum      Number of tracks available
 * @param  [ in]nodesNum       Number of nodes available
 * @param  [ in]stringsNum     Number of bytes reserved for strings
 * @param  [ in]stackNum       Number of bytes reserved for the stack
 */
void synth_setupMemory(void *pBase, int InstrumentsNum, int SongsNum
        , int TracksNum, int NodesNum, int stringsNum, int stackNum);

/* == DYNAMIC MEMORY FUNCTIONS ====================================== */

/**
 * Dynamically expands the memory.
 *
 * If the memory hasn't been setup'ed, it will be properly alloc'ed.
 *
 * @param  [ in]instrumentsLen Bytes reserved for instruments
 * @param  [ in]songsLen       Bytes reserved for songs
 * @param  [ in]tracksLen      Bytes reserved for tracks
 * @param  [ in]nodesNum       Bytes reserved for nodes
 * @param  [ in]stringsLen     Bytes reserved for strings
 * @param  [ in]stackLen       Bytes reserved for the stack
 */
void synth_expandMemory(int instrumentsLen, int songsLen, int tracksLen
        , int nodesNum, int stringsNum, int stackNum);

/**
 * Dynamically expands the memory for instruments.
 *
 * If the memory hasn't been setup'ed, it will be properly alloc'ed.
 *
 * @param  [ in]num By how many units it should be expanded
 */
void synth_expandInstruments(int num);

/**
 * Dynamically expands the memory for songs.
 *
 * If the memory hasn't been setup'ed, it will be properly alloc'ed.
 *
 * @param  [ in]num By how many units it should be expanded
 */
void synth_expandSongs(int num);

/**
 * Dynamically expands the memory for tracks.
 *
 * If the memory hasn't been setup'ed, it will be properly alloc'ed.
 *
 * @param  [ in]num By how many units it should be expanded
 */
void synth_expandTracks(int num);

/**
 * Dynamically expands the memory for nodes.
 *
 * If the memory hasn't been setup'ed, it will be properly alloc'ed.
 *
 * @param  [ in]num By how many units it should be expanded
 */
void synth_expandNodes(int num);

/**
 * Dynamically expands the memory for strings.
 *
 * If the memory hasn't been setup'ed, it will be properly alloc'ed.
 *
 * @param  [ in]num By how many units it should be expanded
 */
void synth_expandStrings(int num);

/**
 * Dynamically expands the memory for the stack.
 *
 * If the memory hasn't been setup'ed, it will be properly alloc'ed.
 *
 * @param  [ in]num By how many units it should be expanded
 */
void synth_expandStack(int num);

/**
 * Clean up the dinamically allo'ec memory
 */
void synth_cleanMemory();

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
#define synth_align32(value) ((value + 3) & ~3)

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
 * @param  [ in]_region_ Region to be retrieved
 * @return               Pointer to the start of that region
 */
#define synth_getRegion(_region_) \
    (synth_getMemory((uint8_t*)pMemory, pMemory->_region_.offset))

/**
 * Check if a given region has at least _size_ bytes free
 *
 * @param  [ in]_region_ Region to be checked
 * @param  [ in]_size_   Amount of bytes that should be available
 * @return               1 if there's enough space, 0 otherwise
 */
#define synth_checkOverflow(_region_, _size_) \
    (pMemory->_region_.used + _size_ > pMemory->_region_.len)

/**
 * Check if a give region is full
 *
 * @param  [ in]_region_ Region to be checked
 * @return               1 if it's full, 0 otherwise
 */
#define synth_isFull(_region_, _size_) \
    (pMemory->_region_.used >= pMemory->_region_.len)

#endif /* __SYNTH_MEMORY_H__ */

