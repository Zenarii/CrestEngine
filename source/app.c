#include "debug.c"
#include "shader.c"
#define STB_TRUETYPE_IMPLEMENTATION
#define ALLOW_UNALIGNED_TRUETYPE
#include "stb/stb_truetype.h"
#include "ui/ui_renderer.c"
#include "ui/ui.c"

#include "C2D/2DRenderer.c"

typedef struct app {
    b32 Initialised;
    r32 Delta;

    CrestUI UI;
    ui_renderer UIRenderer;

    C2DRenderer Renderer;
} app;

internal b32
AppUpdate(Platform * platform) {
    b32 AppShouldQuit = 0;
    app * App = platform->PermenantStorage;

    if(!App->Initialised) {
        App->Initialised = 1;
        CrestUIRendererInit(&App->UIRenderer);
        CrestUIRendererLoadFont(&App->UIRenderer, "../assets/LiberationMono-Regular.ttf");
        C2DInit(&App->Renderer);
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glClearColor(0.1, 0.1, 0.1, 1.0f);
        platform->TargetFPS = 60.0f;
    }

    //Note(Zen): Per-Frame initialisation
    {

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        App->UIRenderer.Width = platform->ScreenWidth;
        App->UIRenderer.Height = platform->ScreenHeight;
        App->Renderer.Width = platform->ScreenWidth;
        App->Renderer.Height = platform->ScreenHeight;
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

    for(i32 x = 0; x < (platform->ScreenWidth/25); ++x) {
        for(i32 y = 0; y < (platform->ScreenHeight/25); ++y) {
            v2 Position = v2((r32)x * 25.0f, (r32)y * 25.0f);
            v2 Size = v2(20.0f, 20.0f);
            C2DDrawColouredRect(&App->Renderer, Position, Size, v3(Position.x/platform->ScreenWidth, 0.5f, Position.y/platform->ScreenWidth));
        }
    }

    i32 DrawCalls = C2DEndFrame(&App->Renderer);



    CrestUIBeginFrame(&App->UI, &UIIn, &App->UIRenderer);
    {
        //Note(Zen): Diagnostic Panel
        r32 PanelWidth = 8.f * 3.f + 128.f * 2.f;
        CrestUIPushPanel(&App->UI, v2(platform->ScreenWidth - PanelWidth - 10.0f, 10.0f), 0.0f);
        {
            CrestUIPushRow(&App->UI, v2(platform->ScreenWidth - PanelWidth - 10.0f, 10.0f), v2(128.f, 32.f), 2);
            {
                r32 FPS = 1.f / platform->TimeTaken;
                char FPSString[16];
                sprintf(FPSString, "%.2fFPS\0", FPS);
                char TimeTakenForFrameString[32];
                sprintf(TimeTakenForFrameString, "%.6fus\0", platform->TimeTakenForFrame * 1000000.f);
                char DrawCallsString[8];
                sprintf(DrawCallsString, "%d", DrawCalls);


                CrestUITextLabel(&App->UI, GENERIC_ID(0), "FPS:");
                CrestUITextLabel(&App->UI, GENERIC_ID(0), FPSString);
                CrestUITextLabel(&App->UI, GENERIC_ID(0), "Time:");
                CrestUITextLabel(&App->UI, GENERIC_ID(0), TimeTakenForFrameString);
                CrestUITextLabel(&App->UI, GENERIC_ID(0), "Draw Calls:");
                CrestUITextLabel(&App->UI, GENERIC_ID(0), DrawCallsString);
            }
            CrestUIPopRow(&App->UI);
        }
        CrestUIPopPanel(&App->UI);
    }
    CrestUIEndFrame(&App->UI, &App->UIRenderer);




    return AppShouldQuit;
}
