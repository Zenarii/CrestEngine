#include "debug.c"
#include "shader.c"
#define STB_TRUETYPE_IMPLEMENTATION
#define ALLOW_UNALIGNED_TRUETYPE
#include "stb/stb_truetype.h"
#include "ui/ui_renderer.c"
#include "ui/ui.c"

#include "ExampleECS/Player.h"

typedef struct app {
    b32 Initialised;
    r32 Delta;

    CrestUI UI;
    ui_renderer UIRenderer;

    v4 BackgroundColour;
    PlayerSet p;

} app;

#include "ExampleECS/Player.c"

internal b32
AppUpdate(Platform * platform) {
    b32 AppShouldQuit = 0;
    app * App = platform->PermenantStorage;

    if(!App->Initialised) {
        App->Initialised = 1;
        CrestUIRendererInit(&App->UIRenderer);
        CrestUIRendererLoadFont(&App->UIRenderer, "../assets/LiberationMono-Regular.ttf");
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


        App->BackgroundColour = v4(0.79, 0.95, 0.70, 1.0f);
        platform->TargetFPS = 60.0f;
    }

    //Note(Zen): Per-Frame initialisation
    {
        glClearColor(App->BackgroundColour.x, App->BackgroundColour.y, App->BackgroundColour.z, App->BackgroundColour.w);
        glClear(GL_COLOR_BUFFER_BIT);

        App->UIRenderer.Width = platform->ScreenWidth;
        App->UIRenderer.Height = platform->ScreenHeight;
        App->Delta = 1.f / platform->TargetFPS;
    }

    //Note(Zen): Copy across UI Input
    CrestUIInput UIIn = {0};
    {
        UIIn.MouseX = platform->MouseEndX;
        UIIn.MouseY = platform->MouseEndY;
        UIIn.MouseStartX = platform->MouseStartX;
        UIIn.MouseStartY = platform->MouseStartY;
        UIIn.LeftMouseDown = platform->LeftMouseDown;
        UIIn.RightMouseDown = platform->RightMouseDown;
    }

    CrestUIBeginFrame(&App->UI, &UIIn, &App->UIRenderer);
    {


        static v2 BoxPosition = {100.0f, 100.0f};
        BoxPosition = CrestUIDnDBoxP(&App->UI, GENERIC_ID(0), v4(BoxPosition.x, BoxPosition.y, 256.0f, 32.0f), "Drag ME");

        static int NumButtons = 2;
        CrestUIPushRow(&App->UI, v2(BoxPosition.x, BoxPosition.y + 32.f), v2(128.f, 32.f), NumButtons);
        {
            App->BackgroundColour.x = CrestUISlider(&App->UI, GENERIC_ID(0), App->BackgroundColour.x, "Red");
            App->BackgroundColour.y = CrestUISlider(&App->UI, GENERIC_ID(0), App->BackgroundColour.y, "Blue");
            App->BackgroundColour.z = CrestUISlider(&App->UI, GENERIC_ID(0), App->BackgroundColour.z, "Green");
        }
        CrestUIPopRow(&App->UI);


    }

    CrestUIEndFrame(&App->UI, &App->UIRenderer);
    CrestPushFilledRect3D(&App->UIRenderer, PANEL_COLOUR, v3(100.0f, 100.f, 0.5f), v2(64.0f, 64.0f));
    CrestUIRender(&App->UIRenderer);


    return AppShouldQuit;
}
