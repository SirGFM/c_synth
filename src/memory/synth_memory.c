/**
 * @project c_synth
 * @license zlib license
 * @file    src/memory/synth_memory.c
 *
 * @summary   Container where parsed input is stored
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
 */
#include <c_synth_internal/synth_memory.h>
/** Required for the internal types */
#include <c_synth_internal/synth_types.h>
/** Required for synth_assert */
#include <c_synth_internal/synth_error.h>

/** Required for size_t */
#include <stddef.h>
/** Required for memset */
#include <string.h>

/** Objects' reference */
synth_memory *pMemory = 0;
/** Amount of memory required by the memory */
const size_t synth_memorySize = synth_align32(sizeof(synth_memory));

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
        , int tracksCount, int nodesCount) {
    int len;

/** Compute the size of a region, aligned to 32 bits */
#define SIZEOF_REGION(_region_) \
  (synth_align32(_region_##sCount * sizeof(synth_##_region_)))

    len = SIZEOF_REGION(instrument);
    len += SIZEOF_REGION(song);
    len += SIZEOF_REGION(track);
    len += SIZEOF_REGION(node);

#undef GET_LEN

    return len;
}

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
void synth_setupMemory(void *pBase, int instrumentsNum, int songsNum
        , int tracksNum, int nodesNum, int stringsNum, int stackNum) {
    int instrumentsLen,  songsLen, tracksLen, nodesLen, stringsLen
            , stackLen;

    synth_assert(pBase);

/** Retrieve the actual amount of bytes required by a given region */
#define CONVERT_SIZE(_region_) \
  _region_##sLen = ( _region_##sNum * sizeof(synth_##_region_) )

    /* Convert the number of member into bytes */
    CONVERT_SIZE(instrument);
    CONVERT_SIZE(song);
    CONVERT_SIZE(track);
    CONVERT_SIZE(node);
    /* Those last two are only here to maintain the pattern */
    stringsLen = stringsNum;
    stackLen = stackNum;

    pMemory = (synth_memory*)pBase;
    memset(pMemory, 0x0, synth_memorySize + instrumentsLen + songsLen
            + tracksLen + nodesLen + stringsLen + stackLen);

/** Put region '_to_' on the start of the memory */
#define SET_BASE(_to_) \
  do { \
    pMemory->_to_.offset = (int)synth_memorySize; \
    pMemory->_to_.len = _to_##Num; \
  } while (0)
/** Put region '_to_' right after region '_from_' */
#define SET_OFFSET(_to_, _from_) \
  do { \
    /* Offset it by the actual byte count */ \
    pMemory->_to_.offset = pMemory->_from_.offset + _from_##Len; \
    /* But store the number of members */ \
    pMemory->_to_.len = _to_##Num; \
  } while (0)

    SET_BASE(instruments);
    SET_OFFSET(songs, instruments);
    SET_OFFSET(tracks, songs);
    SET_OFFSET(nodes, tracks);
    SET_OFFSET(strings, nodes);
    SET_OFFSET(stack, strings);

#undef SET_BASE
#undef SET_OFFSET
}

/**
 * Search for a null-terminated string on the memory.
 *
 * @param  [ in]pString The string
 * @return              Position of the string or -1, if not found
 */
int synth_getStringPosition(char *pString) {
    /* Points to the start of the strings region */
    char *pTmp;
    /* Index of the stored string being compared */
    int i;
    /* Current position within both strings */
    int j;

    synth_assert(pString);
    synth_assert(strlen(pString) > 0);

    pTmp = (char*)synth_getRegion(strings);

    i = 0;
    j = 0;
    while (i < pMemory->strings.used) {
        if (pString[j] == '\0' && pTmp[i + j] == '\0') {
            /* String found */
            return i;
        }
        else if (pString[j] != pTmp[i + j]) {
            /* Move over to the next string */
            while (j < pMemory->strings.used && pTmp[i + j] != '\0') {
                j++;
            }
            i = j + 1;
            /* Go back to the start of both strings */
            j = 0;
        }

        j++;
    }

    /* String not found */
    return -1;
}

