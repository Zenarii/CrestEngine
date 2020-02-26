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

        //TEMP(ZEN): some component stuff
        EntityID ID = App->p.Count;
        PositionComponent pos = {ID, 512.0f, 512.0f};
        SpriteComponent sprite = {ID, 0.0f, 0.0f, 30.0f, 30.0f};
        PushPlayer(&App->p, pos, sprite);
        ID = App->p.Count;
        PositionComponent pos2 = {ID, 256.0f, 256.0f};
        SpriteComponent sprite2 = {ID, 0.0f, 0.0f, 40.0f, 40.0f};
        PushPlayer(&App->p, pos2, sprite2);
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
        UIIn.LeftMouseDown = platform->LeftMouseDown;
        UIIn.RightMouseDown = platform->RightMouseDown;
    }

    static r32 value;

    CrestUIBeginFrame(&App->UI, &UIIn, &App->UIRenderer);
    {
        CrestUIPushRow(&App->UI, v2(32.f, 32.f), v2(128.f, 32.f), 2);
        if(CrestUIButton(&App->UI, GENERIC_ID(0), "Button 1")) {
            OutputDebugString("Button 1 pressed\n");
        }
        if(CrestUIButton(&App->UI, GENERIC_ID(0), "Button 2")) {
            OutputDebugString("Button 2 pressed\n");
        }

        App->BackgroundColour.x = CrestUISlider(&App->UI, GENERIC_ID(0), App->BackgroundColour.x, "Red");
        App->BackgroundColour.y = CrestUISlider(&App->UI, GENERIC_ID(0), App->BackgroundColour.y, "Blue");
        App->BackgroundColour.z = CrestUISlider(&App->UI, GENERIC_ID(0), App->BackgroundColour.z, "Green");
        if(CrestUIButton(&App->UI, GENERIC_ID(0), "Button 3")) {
            OutputDebugString("Button 3 pressed\n");
        }
        CrestUIPopRow(&App->UI);
    }

    CrestUIEndFrame(&App->UI, &App->UIRenderer);




    return AppShouldQuit;
}
