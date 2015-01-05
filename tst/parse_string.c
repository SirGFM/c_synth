/**
 * @file tst/parse_string.c
 */
#include <synth/synth_assert.h>
#include <synth/synth_errors.h>
#include <synth_internal/synth_parser.h>

#include <stdio.h>

static char mml[] = "t90 l16 o5 e e8 e r c e r g4 > g4 <";
int len = sizeof(mml);

int main(int argc, char *argv[]) {
    int rv;
    synth_err srv;
    synthParserCtx *ctx = 0;
    
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
    
    srv = synth_parser_audio(ctx);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, srv);
    
    rv = 0;
__err:
    if (rv != 0) {
        char *error;
        
        error = synth_parser_getErrorString(ctx);
        if (error)
            printf("%s\n", error);
    }
    synth_parser_clean(&ctx);
    
    return rv;
}

