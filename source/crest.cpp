//################
//# Zenarii 2020 #
//################

#include "crest.h"


internal void
RenderWeirdGradient(crest_offscreen_buffer * buffer) {
    int pitch = buffer->Width * buffer->BytesPerPixel;
    uint8 *row = (uint8 *)buffer->Memory;
    for(int y = 0; y < buffer->Height; ++y) {
        uint32 *pixel = (uint32 *)row;
        for(int x = 0; x < buffer->Width; ++x) {
            uint8 r = 0;
            uint8 g = 56;
            uint8 b = 168;

            if (y > (buffer->Height)/2) {
                r = 214;
                g = 200;
                b = 112;
            }
            else {
                r = 155;
                g = 79;
                b = 150;
            }
            //writes, then increments
            *pixel++ = ((r<<16)|(g<<8)|b);
        }
        row += pitch;
    }
}


internal void
GameUpdateAndRender(crest_offscreen_buffer* ScreenBuffer) {
    local_persist bool started = false;
    if(!started) {
        glClearColor(0.1f, 0.1f, 0.1f, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        RenderTriangle();
        started = true;
    }

}
