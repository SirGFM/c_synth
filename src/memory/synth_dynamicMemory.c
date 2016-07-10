/**
 * @project c_synth
 * @license zlib license
 * @file    src/memory/synth_dynamicmemory.c
 *
 * @summary Helps to manage memory for objects.
 *
 * Implements the functions required to expand the object's memory.
 */
#define ENABLE_MALLOC
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
 */
void synth_expandMemory(int instrumentsLen, int songsLen, int tracksLen,
        int stringsLen) {
    synth_memory *pOld, *pNew;

    pNew = malloc(synth_memorySize + instrumentsLen + songsLen
            + tracksLen + stringsLen);

    pOld = pObjects;
    synth_setupMemory(pNew, instrumentsLen, songsLen, tracksLen
            , stringsLen);
    if (!pOld) {
        return;
    }

#define COPY_REGION(attribute) \
  do { \
    pNew->attribute.used = pOld->attribute.used; \
    memcpy(synth_getRegion(pNew->attribute.offset), \
        synth_getMemory((uint8_t*)pOld, pOld->attribute.offset), \
        pOld->attribute.len); \
  } while (0)

    COPY_REGION(instruments);
    COPY_REGION(songs);
    COPY_REGION(tracks);
    COPY_REGION(strings);

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
    synth_expandMemory(len, pObjects->songs.len, pObjects->tracks.len
            , pObjects->strings.len);
}

/**
 * Dynamically expands the memory for songs.
 *
 * If the memory hasn't been setup'ed, it will be properly alloc'ed.
 *
 * @param  [ in]len Bytes reserved for songs
 */
void synth_expandSongs(int len) {
    synth_expandMemory(pObjects->instruments.len, len
            , pObjects->tracks.len, pObjects->strings.len);
}

/**
 * Dynamically expands the memory for tracks.
 *
 * If the memory hasn't been setup'ed, it will be properly alloc'ed.
 *
 * @param  [ in]len Bytes reserved for tracks
 */
void synth_expandTracks(int len) {
    synth_expandMemory(pObjects->instruments.len, pObjects->songs.len
            , len, pObjects->strings.len);
}

/**
 * Dynamically expands the memory for strings.
 *
 * If the memory hasn't been setup'ed, it will be properly alloc'ed.
 *
 * @param  [ in]len Bytes reserved for strings
 */
void synth_expandStrings(int len) {
    synth_expandMemory(pObjects->instruments.len, pObjects->songs.len
            , pObjects->tracks.len, len);
}

/**
 * Clean up the dinamically allo'ec memory
 */
void synth_cleanMemory() {
    free(pObjects);
    pObjects = 0;
}

