//################
//# Zenarii 2020 #
//################

struct win32_offscreen_buffer {
    BITMAPINFO Info;
    void * Memory;
    int Height;
    int Width;
    int bytesPerPixel;
    int Pitch;
};

struct win32_window_dimension {
    int Width;
    int Height;
};


struct win32_sound_output {
    int SamplesPerSecond;
    int ToneHz;
    uint32 RunningSampleIndex;
    int16 ToneVolume;
    int BytesPerSample;
    int SecondaryBufferSize;
    int LatencySampleCount;
};
