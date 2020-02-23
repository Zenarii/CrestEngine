#include "shader.c"
#define STB_TRUETYPE_IMPLEMENTATION
#define ALLOW_UNALIGNED_TRUETYPE
#include "stb/stb_truetype.h"
#include "ui/ui_renderer.c"
#include "ui/ui.c"

typedef struct app {
    b32 Initialised;
    r32 Delta;

    CrestUI UI;
    ui_renderer UIRenderer;
} app;


internal b32
AppUpdate(Platform * platform) {
    b32 AppShouldQuit = 0;
    app * App = platform->PermenantStorage;

    if(!App->Initialised) {
        App->Initialised = 1;
        CrestUIRendererInit(&App->UIRenderer);
        CrestUIRendererLoadFont(&App->UIRenderer, "c:/windows/fonts/times.ttf");
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //glClearColor(0.96862745098f, 0.80392156863f, 0.96078431372549f, 1.0f);
        glClearColor(0.4f, 0.4f, 0.45f, 1.0f);
    }

    //Note(Zen): Per-Frame initialisation
    {
        glClear(GL_COLOR_BUFFER_BIT);

        App->UIRenderer.Width = platform->ScreenWidth;
        App->UIRenderer.Height = platform->ScreenHeight;
    }

    //Note(Zen): Copy across UI Input
    CrestUIInput UIIn = {0};
    {
        UIIn.MouseX = platform->MouseEndX;
        UIIn.MouseY = platform->MouseEndY;
        UIIn.LeftMouseDown = platform->LeftMouseDown;
        UIIn.RightMouseDown = platform->RightMouseDown;
    }

    CrestUIBegin(&App->UI, &UIIn);
    CrestUIRendererStartFrame(&App->UIRenderer);
    {
        if(CrestUIButton(&App->UI, GENERIC_ID(0), v4(10.0f, 10.0f, 128.0f, 32.0f), "Button 1")) {
            OutputDebugString("Button 1 pressed\n");
        }

        CrestUIButton(&App->UI, GENERIC_ID(0), v4(10.0f, 42.0f, 128.0f, 32.0f), "Somey Text");
    }
    CrestUIEnd(&App->UI, &App->UIRenderer);
    CrestUIRender(&App->UIRenderer);



    return AppShouldQuit;
}
