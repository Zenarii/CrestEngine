//################
//# Zenarii 2020 #
//################


struct crest_offscreen_buffer {
    void* Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

//needs the timing, Controller/Keyboard, bitmap buffer, sound buffer
internal void
GameUpdateAndRender(crest_offscreen_buffer* ScreenBuffer);
