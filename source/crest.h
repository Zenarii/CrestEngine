//################
//# Zenarii 2020 #
//################


struct game_offscreen_buffer {
    void* Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};



//needs the timing, Controller/Keyboard, bitmap buffer, sound buffer
internal void
GameUpdateAndRender(game_offscreen_buffer* buffer);
