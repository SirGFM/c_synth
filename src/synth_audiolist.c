/**
 * @file src/synth_audiolist.c
 */
#include <synth/synth_assert.h>
#include <synth/synth_audio.h>
#include <synth/synth_backend.h>
#include <synth/synth_errors.h>
#include <synth/synth_types.h>
#include <synth_internal/synth_audio.h>
#include <synth_internal/synth_audiolist.h>
#include <synth_internal/synth_buffer.h>
#include <synth_internal/synth_thread.h>

#include <stdlib.h>

/**
 * Linked list for audios
 */
struct stSynthList {
    /**
     * Whether it's a simple audio or a sfx (that should have a single track
     * and the position is controlled by the node itself)
     */
    synth_audio_type type;
    /**
     * The audio to be played
     */
    synthAudio *aud;
    /**
     * It's a linked list, so...
     */
    struct stSynthList *next;
    // TODO position for sfx
};
typedef struct stSynthList synthList;

/**
 * Currently playing audios
 */
synthList *audios = 0;
/**
 * List where audios are added, so we can add audio and play it without many
 * locks
 */
synthList *backlist = 0;
/**
 * Unused nodes (lessen alloc/free)
 */
synthList *recycled = 0;
/**
 * Currently playing bgm
 */
synthAudio *bgm = 0;
/**
 * Bgm that will start playing
 */
synthAudio *newBgm = 0;

/**
 * Get (either recycling or alloc'ing) a new node
 * 
 * @param node Reference to the returned node
 * @return Error code
 */
static synth_err synth_list_getNode(synthList **node);

/**
 * Initialize necessary stuff
 * (does nothing, for now)
 */
synth_err synth_list_init() {
    return SYNTH_OK;
}

/**
 * Clean up memory used by linked lists
 */
void synth_list_clean() {
    
    // Simple macro for freeing a linked list
    #define FREELIST(list) \
      while (list) { \
        synthList *tmp; \
        \
        tmp = list; \
        list = list->next; \
        \
        free(tmp); \
      } \
      list = 0
    
    // Free every list
    FREELIST(audios);
    FREELIST(recycled);
    FREELIST(backlist);
}

/**
 * Thread-safe. Set up a bgm change
 * 
 * @param aud The new bgm
 * @return Error code
 */
void synth_list_setBgm(synthAudio *aud) {
    // Lock and set the to be played bgm
    synth_thread_lockAudio();
    newBgm = aud;
    synth_thread_unlockAudio();
    // Unpause (or continue) the backend
    synth_bkend_unpause();
}

/**
 * Thread-safe. Stop the currently playing bgm
 */
void synth_list_stopBgm() {
    // Lock and remove the playing bgm
    synth_thread_lockAudio();
    bgm = 0;
    synth_thread_unlockAudio();
}

/**
 * Thread-safe. Set up an audio addition.
 * The audio will be reset, so be sure it's not playing already
 * 
 * @param aud The audio to be added
 * @return Error code
 */
synth_err synth_list_addAudio(synthAudio *aud) {
    synth_err rv;
    synthList *node;
    
    // Get a node list
    rv = synth_list_getNode(&node);
    SYNTH_ASSERT(rv == SYNTH_OK);
    
    synth_audio_reset(aud);
    node->aud = aud;
    node->type = A_AUDIO;
    
    // Lock and add the node to the back list
    synth_thread_lockAudio();
    node->next = backlist;
    backlist = node;
    synth_thread_unlockAudio();
    
    // Unpause (or continue) the backend
    synth_bkend_unpause();
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Thread-safe. Set up a sfx addition.
 * Different from an audio, this keep track of the current position on the
 * audio, but must be a single track.
 * 
 * @param aud The sfx to be added
 * @return Error code
 */
synth_err synth_list_addSfx(synthAudio *aud) {
    return SYNTH_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Get (either recycling or alloc'ing) a new node and add it to the list
 * 
 * @param node Reference to the returned node
 * @return Error code
 */
static synth_err synth_list_getNode(synthList **node) {
    synth_err rv;
    synthList *tmp;
    
    tmp = 0;
    
    // Get the first unused node (if any), or alloc a new one)
    synth_thread_lockAudio();
    if (recycled) {
        tmp = recycled;
        recycled = recycled->next;
    }
    synth_thread_unlockAudio();
    
    // If there were no nodes to be recycled
    if (!tmp) {
        tmp = (synthList*)malloc(sizeof(synthList));
        SYNTH_ASSERT_ERR(tmp, SYNTH_MEM_ERR);
    }
    
    // Set return
    *node = tmp;
    rv = SYNTH_OK;
__err:
    // Clean up memory, on error
    if (rv != SYNTH_OK) {
        if (tmp)
            free(tmp);
    }
    return rv;
}

/**
 * Thread-safe. Merge the the back list to the playing one. Also, update the playing
 * bgm
 */
void synth_list_merge() {
    synth_thread_lockAudio();
    if (backlist) {
        synthList *tmp;
        
        // Get the last audio to be added
        tmp = backlist;
        while (tmp->next)
            tmp = tmp->next;
        
        // Merge both lists
        tmp->next = audios;
        audios = backlist;
        
        backlist = 0;
    }
    // Update the bgm
    if (newBgm) {
        bgm = newBgm;
        newBgm = 0;
        
        synth_audio_reset(bgm);
    }
    
    // If there are no audios, pause the backend
    if (!bgm && !audios)
        synth_bkend_pause();
    
    synth_thread_unlockAudio();
}

/**
 * Synthesize every 
 * 
 * @param samples How many samples will be synthesized
 * @param left The left buffer
 * @param right The right buffer
 */
void synth_list_synthesize(int samples, uint16_t *left, uint16_t *right) {
    synthList *tmp;
    synthList *prev;
    int i;
    
    // Lock the buffer, so we can 'feed' it
    synth_thread_lockBuffer();
    
    // Loop through every audio
    prev = 0;
    tmp = audios;
    while (tmp) {
        // Synthesize the current audio
        synth_audio_synthesize
            (
            tmp->aud,
            samples,
            left,
            right
            );
        
        // If the current audio finished, add its node to the recycled list
        if (synth_audio_didFinish(tmp->aud) == SYNTH_TRUE) {
            if (prev)
                prev->next = tmp->next;
            else
                audios = tmp->next;
            
            // TODO check if there can be a deadlock here
            // Add the node the the recycled list
            synth_thread_lockAudio();
            tmp->next = recycled;
            recycled = tmp;
            synth_thread_unlockAudio();
        }
        
        // Go to the next one
        tmp = tmp->next;
    }
    
    if (bgm) {
        synth_audio_synthesize(bgm, samples, left, right);
        
        if (synth_audio_didFinish(bgm) == SYNTH_TRUE)
            bgm = 0;
    }
    
    synth_thread_unlockBuffer();
}

