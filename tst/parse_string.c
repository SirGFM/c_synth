/**
 * @file tst/parse_string.c
 */
#include <SDL2/SDL.h>

#include <synth/synth.h>
#include <synth/synth_assert.h>
#include <synth/synth_audio.h>
#include <synth/synth_backend.h>
#include <synth/synth_errors.h>
#include <synth_internal/synth_audio.h>
#include <synth_internal/synth_parser.h>

#include <stdio.h>

static char mml[] = "t90 l16 o5 e e8 e r c e r g4 > g4 <";
int len = sizeof(mml);

#define TIME 3
#define FREQ 44100
#define BUFFERLEN FREQ*TIME

int main(int argc, char *argv[]) {
    int rv, i;
    synth_err srv;
    synthParserCtx *ctx = 0;
    synthAudio *aud;
    uint16_t *left, *right, *data;
    int bufferlen;
    int time;
    
    rv = SDL_Init(0);
    SYNTH_ASSERT(rv >= 0);
    
    srv = synth_init(FREQ, SYNTH_FALSE, 0);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, srv);
    
    if (argc > 1) {
        printf("Reading mml from file: %s\n", argv[1]);
        
        // Initialize the contex for parsing
        srv = synth_parser_initParsef(&ctx, argv[1]);
        SYNTH_ASSERT_ERR(srv == SYNTH_OK, srv);
    }
    else {
        printf("mml: %s\n", mml);
        
        // Initialize the contex for parsing
        srv = synth_parser_initParses(&ctx, mml, len);
        SYNTH_ASSERT_ERR(srv == SYNTH_OK, srv);
    }
    
    if (argc > 2) {
        char *t;
        int n;
        
        t = argv[2];
        n = 0;
        while (*t) {
            n = n * 10 + *t - '0';
            t++;
        }
        
        bufferlen = FREQ*n;
        time = n;
    }
    else {
        bufferlen = BUFFERLEN;
        time = TIME;
    }
    
    srv = synth_parser_audio(ctx);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, srv);
    
    aud = synth_parser_getAudio(ctx);
    
    
    srv = synth_bkend_setup(FREQ, 2);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, srv);
    
    synth_bkend_pause();
    
    left = (uint16_t*)malloc(sizeof(uint16_t)*bufferlen);
    SYNTH_ASSERT_ERR(left != NULL, SYNTH_MEM_ERR);
    
    right = (uint16_t*)malloc(sizeof(uint16_t)*bufferlen);
    SYNTH_ASSERT_ERR(left != NULL, SYNTH_MEM_ERR);
    
    memset(left, 0x0, bufferlen*sizeof(uint16_t));
    memset(right, 0x0, bufferlen*sizeof(uint16_t));
    
    synth_audio_synthesize(aud, bufferlen, left, right);
    
    data = (uint16_t*)malloc(sizeof(uint16_t)*bufferlen*2);
    SYNTH_ASSERT_ERR(data != NULL, SYNTH_MEM_ERR);
    
    memset(data, 0x0, bufferlen*sizeof(uint16_t)*2);
    
    i = 0;
    while (i < bufferlen) {
        data[i*2+0] = left[i];
        data[i*2+1] = right[i];
        i++;
    }
    
    synth_bkend_fillBuffer(data, sizeof(uint16_t)*bufferlen*2);
    
    SDL_Delay((time)*1000);
    
    rv = 0;
__err:
    if (left)
        free(left);
    if (right)
        free(right);
    if (data)
        free(data);
    
    if (rv != 0) {
        char *error;
        
        error = synth_parser_getErrorString(ctx);
        if (error)
            printf("%s\n", error);
    }
    
    synth_audio_free(&aud);
    
    synth_parser_clean(&ctx);
    
    synth_bkend_clean();
    SDL_Quit();
    
    synth_clean();
    
    return rv;
}

