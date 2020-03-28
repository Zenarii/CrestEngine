#include "debug.c"

#define STB_TRUETYPE_IMPLEMENTATION
#define ALLOW_UNALIGNED_TRUETYPE
#include "stb/stb_truetype.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "shader.c"
#include "ui/ui_renderer.c"
#include "ui/ui.c"
#include "CAssets/Textures.c"
#include "C2D/2DRenderer.c"

#include "Pong/pong.h"

typedef struct app {
    b32 Initialised;
    r32 Delta;
    b32 KeyDown[CREST_KEY_MAX];

    CrestUI UI;
    ui_renderer UIRenderer;

    C2DRenderer Renderer;

    game_data Game;
} app;

#include "Pong/pong.c"

enum Textures {
    TEXTURE_WHITE,
    TEXTURE_MENU,

    TEXTURE_COUNT
};

internal b32
AppUpdate(Platform * platform) {
    b32 AppShouldQuit = 0;
    app * App = platform->PermenantStorage;
    static b32 OnMenu = 1;

    if(!App->Initialised) {
        App->Initialised = 1;
        CrestUIRendererInit(&App->UIRenderer);
        CrestUIRendererLoadFont(&App->UIRenderer, "../assets/LiberationMono-Regular.ttf");
        C2DInit(&App->Renderer);
        App->Renderer.Textures[TEXTURE_WHITE] = CasLoadTexture("../assets/White.png", GL_NEAREST);
        App->Renderer.Textures[TEXTURE_MENU] = CasLoadTexture("../assets/title_text.png", GL_NEAREST);
        App->Renderer.ActiveTextures = TEXTURE_COUNT;
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        platform->TargetFPS = 60.0f;


        //Note(Zen): Demo Setup
        App->Renderer.Width = platform->ScreenWidth;
        App->Renderer.Height = platform->ScreenHeight;
        App->Game = initGame(App);
    }

    //Note(Zen): Per-Frame initialisation
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        App->UIRenderer.Width = platform->ScreenWidth;
        App->UIRenderer.Height = platform->ScreenHeight;
        App->Renderer.Width = platform->ScreenWidth;
        App->Renderer.Height = platform->ScreenHeight;
        App->Delta = 1.f / platform->TargetFPS;
        memcpy(App->KeyDown, platform->KeyDown, sizeof(b32) * CREST_KEY_MAX);
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
    static i32 InWinState;
    if(!OnMenu) {
        if(App->KeyDown[KEY_R]) {
            App->Game = initGame(App);
            InWinState = 0;
        }
        if(!InWinState) InWinState = doGame(App) ? 1 : InWinState;
        else doWin(App);
    }
    else {
        if(App->KeyDown[KEY_R]) {
            App->Game = initGame(App);
            InWinState = 0;
            OnMenu = 0;

        }
        v2 Size = v2(37.f * 10.f, 11.f * 10.f);
        v2 Position = v2(App->Renderer.Width * 0.5f - Size.x * 0.5f, 100.f);
        C2DDrawTexturedRect(&App->Renderer, Position, Size, TEXTURE_MENU);
    }

    i32 DrawCalls = C2DEndFrame(&App->Renderer);

    //Note(Zen): UI Diagnostics
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
