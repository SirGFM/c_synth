/**
 * @project c_synth
 * @license zlib license
 * @file    src/memory/synth_dynamicmemory.c
 *
 * @summary   Container where parsed input is stored
 *
 * @description
 *
 * Implements the functions required to expand the object's memory.
 */
#include <c_synth_internal/synth_memory.h>

/** Required for fixed-width sizes */
#include <stdint.h>
/** Required for malloc/free */
#include <stdlib.h>
/** Required for memset */
#include <string.h>

/**
 * Dynamically expands the memory.
 *
 * If the memory hasn't been setup'ed, it will be properly alloc'ed.
 *
 * @param  [ in]instrumentsLen Bytes reserved for instruments
 * @param  [ in]songsLen       Bytes reserved for songs
 * @param  [ in]tracksLen      Bytes reserved for tracks
 * @param  [ in]stringsLen     Bytes reserved for strings
 * @param  [ in]stackLen       Bytes reserved for the stack
 */
void synth_expandMemory(int instrumentsLen, int songsLen, int tracksLen
        , int stringsLen, int stackLen) {
    synth_memory *pOld, *pNew;

    pNew = malloc(synth_memorySize + instrumentsLen + songsLen
            + tracksLen + stringsLen + stackLen);

    pOld = pMemory;
    synth_setupMemory(pNew, instrumentsLen, songsLen, tracksLen
            , stringsLen, stackLen);
    if (!pOld) {
        return;
    }

/** Copy region '_attr_' from the old memory into the new one */
#define COPY_REGION(_attr_) \
  do { \
    pNew->_attr_.used = pOld->_attr_.used; \
    if (pOld->_attr_.len > 0) { \
        memcpy(synth_getRegion(_attr_) \
            , synth_getMemory((uint8_t*)pOld, pOld->_attr_.offset) \
            , pOld->_attr_.len); \
    } \
  } while (0)

    COPY_REGION(instruments);
    COPY_REGION(songs);
    COPY_REGION(tracks);
    COPY_REGION(strings);
    COPY_REGION(stack);

#undef COPY_REGION

    free(pOld);
}

/**
 * Dynamically expands the memory for instruments.
 *
 * If the memory hasn't been setup'ed, it will be properly alloc'ed.
 *
 * @param  [ in]len Bytes reserved for instruments
 */
void synth_expandInstruments(int len) {
    if (!pMemory) {
        synth_expandMemory(len, 0, 0, 0, 0);
    }
    else {
        synth_expandMemory(len, pMemory->songs.len, pMemory->tracks.len
                , pMemory->strings.len, pMemory->stack.len);
    }
}

/**
 * Dynamically expands the memory for songs.
 *
 * If the memory hasn't been setup'ed, it will be properly alloc'ed.
 *
 * @param  [ in]len Bytes reserved for songs
 */
void synth_expandSongs(int len) {
    if (!pMemory) {
        synth_expandMemory(0, len, 0, 0, 0);
    }
    else {
        synth_expandMemory(pMemory->instruments.len, len
                , pMemory->tracks.len, pMemory->strings.len
                , pMemory->stack.len);
    }
}

/**
 * Dynamically expands the memory for tracks.
 *
 * If the memory hasn't been setup'ed, it will be properly alloc'ed.
 *
 * @param  [ in]len Bytes reserved for tracks
 */
void synth_expandTracks(int len) {
    if (!pMemory) {
        synth_expandMemory(0, 0, len, 0, 0);
    }
    else {
        synth_expandMemory(pMemory->instruments.len, pMemory->songs.len
                , len, pMemory->strings.len, pMemory->stack.len);
    }
}

/**
 * Dynamically expands the memory for strings.
 *
 * If the memory hasn't been setup'ed, it will be properly alloc'ed.
 *
 * @param  [ in]len Bytes reserved for strings
 */
void synth_expandStrings(int len) {
    if (!pMemory) {
        synth_expandMemory(0, 0, 0, len, 0);
    }
    else {
        synth_expandMemory(pMemory->instruments.len, pMemory->songs.len
                , pMemory->tracks.len, len, pMemory->stack.len);
    }
}

/**
 * Dynamically expands the memory for the stack.
 *
 * If the memory hasn't been setup'ed, it will be properly alloc'ed.
 *
 * @param  [ in]len Bytes reserved for the stack
 */
void synth_expandStack(int len) {
    if (!pMemory) {
        synth_expandMemory(0, 0, 0, 0, len);
    }
    else {
        synth_expandMemory(pMemory->instruments.len, pMemory->songs.len
                , pMemory->tracks.len, pMemory->strings.len, len);
    }
}

/**
 * Clean up the dinamically allo'ec memory
 */
void synth_cleanMemory() {
    free(pMemory);
    pMemory = 0;
}

