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
#include <c_synth_internal/synth_types.h>

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
 * @param  [ in]instrumentsNum Bytes reserved for instruments
 * @param  [ in]songsNum       Bytes reserved for songs
 * @param  [ in]tracksNum      Bytes reserved for tracks
 * @param  [ in]nodesNum       Bytes reserved for nodes
 * @param  [ in]stringsNum     Bytes reserved for strings
 * @param  [ in]stackNum       Bytes reserved for the stack
 */
void synth_expandMemory(int instrumentsNum, int songsNum, int tracksNum
        , int nodesNum, int stringsNum, int stackNum) {
    synth_memory *pOld = 0, *pNew = 0;
    int len;

    len = synth_computeRequiredMemory(instrumentsNum, songsNum
            , tracksNum, nodesNum);
    len += stringsNum + stackNum;
    pNew = (synth_memory*)malloc(synth_memorySize + len);

    pOld = pMemory;
    synth_setupMemory(pNew, instrumentsNum, songsNum, tracksNum
            , nodesNum, stringsNum, stackNum);
    if (!pOld) {
        return;
    }

/** Copy region '_attr_' from the old memory into the new one */
#define COPY_REGION(_attr_, _sizeof_) \
  do { \
    pNew->_attr_.used = pOld->_attr_.used; \
    if (pOld->_attr_.len > 0) { \
        memcpy(synth_getRegion(_attr_) \
            , synth_getMemory((uint8_t*)pOld, pOld->_attr_.offset) \
            , pOld->_attr_.len * _sizeof_); \
    } \
  } while (0)

    COPY_REGION(instruments, sizeof(synth_instrument));
    COPY_REGION(songs, sizeof(synth_song));
    COPY_REGION(tracks, sizeof(synth_track));
    COPY_REGION(nodes, sizeof(synth_node));
    COPY_REGION(strings, sizeof(char));
    COPY_REGION(stack, sizeof(char));

#undef COPY_REGION

    free(pOld);
}

/**
 * Dynamically expands the memory for instruments.
 *
 * If the memory hasn't been setup'ed, it will be properly alloc'ed.
 *
 * @param  [ in]num By how many units it should be expanded
 */
void synth_expandInstruments(int num) {
    if (!pMemory) {
        synth_expandMemory(num, 0, 0, 0, 0, 0);
    }
    else {
        synth_expandMemory(pMemory->instruments.len + num
                , pMemory->songs.len, pMemory->tracks.len
                , pMemory->nodes.len, pMemory->strings.len
                , pMemory->stack.len);
    }
}

/**
 * Dynamically expands the memory for songs.
 *
 * If the memory hasn't been setup'ed, it will be properly alloc'ed.
 *
 * @param  [ in]num By how many units it should be expanded
 */
void synth_expandSongs(int num) {
    if (!pMemory) {
        synth_expandMemory(0, num, 0, 0, 0, 0);
    }
    else {
        synth_expandMemory(pMemory->instruments.len
                , pMemory->songs.len + num, pMemory->tracks.len
                , pMemory->nodes.len,  pMemory->strings.len
                , pMemory->stack.len);
    }
}

/**
 * Dynamically expands the memory for tracks.
 *
 * If the memory hasn't been setup'ed, it will be properly alloc'ed.
 *
 * @param  [ in]num By how many units it should be expanded
 */
void synth_expandTracks(int num) {
    if (!pMemory) {
        synth_expandMemory(0, 0, num, 0, 0, 0);
    }
    else {
        synth_expandMemory(pMemory->instruments.len, pMemory->songs.len
                , pMemory->tracks.len + num, pMemory->nodes.len
                , pMemory->strings.len, pMemory->stack.len);
    }
}

/**
 * Dynamically expands the memory for nodes.
 *
 * If the memory hasn't been setup'ed, it will be properly alloc'ed.
 *
 * @param  [ in]num By how many units it should be expanded
 */
void synth_expandNodes(int num) {
    if (!pMemory) {
        synth_expandMemory(0, 0, 0, num, 0, 0);
    }
    else {
        synth_expandMemory(pMemory->instruments.len, pMemory->songs.len
                , pMemory->tracks.len, pMemory->nodes.len + num
                , pMemory->strings.len, pMemory->stack.len);
    }
}

/**
 * Dynamically expands the memory for strings.
 *
 * If the memory hasn't been setup'ed, it will be properly alloc'ed.
 *
 * @param  [ in]num By how many units it should be expanded
 */
void synth_expandStrings(int num) {
    if (!pMemory) {
        synth_expandMemory(0, 0, 0, 0, num, 0);
    }
    else {
        synth_expandMemory(pMemory->instruments.len, pMemory->songs.len
                , pMemory->tracks.len, pMemory->nodes.len
                , pMemory->strings.len + num, pMemory->stack.len);
    }
}

/**
 * Dynamically expands the memory for the stack.
 *
 * If the memory hasn't been setup'ed, it will be properly alloc'ed.
 *
 * @param  [ in]num By how many units it should be expanded
 */
void synth_expandStack(int num) {
    if (!pMemory) {
        synth_expandMemory(0, 0, 0, 0, 0, num);
    }
    else {
        synth_expandMemory(pMemory->instruments.len, pMemory->songs.len
                , pMemory->tracks.len, pMemory->nodes.len
                , pMemory->strings.len, pMemory->stack.len + num);
    }
}

/**
 * Clean up the dinamically allo'ec memory
 */
void synth_cleanMemory() {
    free(pMemory);
    pMemory = 0;
}

