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
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        App->BackgroundColour = v4(0.79, 0.95, 0.70, 1.0f);
        platform->TargetFPS = 60.0f;
    }

    //Note(Zen): Per-Frame initialisation
    {
        glClearColor(App->BackgroundColour.x, App->BackgroundColour.y, App->BackgroundColour.z, App->BackgroundColour.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
        //For Demo I need: 3 Panels
        //Panel 1: Change BackgroundColour + a button that opens a settings editor
        //Panel 2: Diagnostics e.g. frame time, FPS, number of draw calls, time to draw
        //Panel 3: Buttons example

        //Note(Zen): Diagnostic
        static v2 ButtonPanelPosition = {10.0f, 10.0f};
        ButtonPanelPosition = CrestUIDnDBoxP(&App->UI, GENERIC_ID(0), 0.0f, v4(ButtonPanelPosition.x, ButtonPanelPosition.y, 256.0f + 24.0f, 32.0f), "Button Test");

        CrestUIPushPanel(&App->UI, v2(ButtonPanelPosition.x, ButtonPanelPosition.y + 32.0f), 0.0f);
        {
            CrestUIPushRow(&App->UI, v2(ButtonPanelPosition.x, ButtonPanelPosition.y + 32.0f), v2(128.f, 32.f), 2);
            {
                CrestUIButton(&App->UI, GENERIC_ID(0), "Button 1");
                CrestUIButton(&App->UI, GENERIC_ID(0), "Button 2");
                CrestUIButton(&App->UI, GENERIC_ID(0), "Button 3");
            }
            CrestUIPopRow(&App->UI);
        }
        CrestUIPopPanel(&App->UI);


        //Note(Zen): Style Editor Panel
        static v2 StyleEditorPanelPosition = {100.0f, 100.0f};
        StyleEditorPanelPosition = CrestUIDnDBoxP(&App->UI, GENERIC_ID(0), -0.02f, v4(StyleEditorPanelPosition.x, StyleEditorPanelPosition.y, 256.0f + 24.0f, 32.0f), "Background Colour");

        CrestUIPushPanel(&App->UI, v2(StyleEditorPanelPosition.x, StyleEditorPanelPosition.y + 32.0f), -0.2f);
        {
            CrestUIPushRow(&App->UI, v2(StyleEditorPanelPosition.x, StyleEditorPanelPosition.y + 32.f), v2(128.f, 32.f), 2);
            {
                //Note(Zen): Get values of the sliders
                char RedAmount[8];
                sprintf(RedAmount, "%.2f\0", App->BackgroundColour.x);
                char BlueAmount[8];
                sprintf(BlueAmount, "%.2f\0", App->BackgroundColour.y);
                char GreenAmount[8];
                sprintf(GreenAmount, "%.2f\0", App->BackgroundColour.z);

                CrestUITextLabel(&App->UI, GENERIC_ID(0), "Red");
                App->BackgroundColour.x = CrestUISlider(&App->UI, GENERIC_ID(0), App->BackgroundColour.x, RedAmount);
                CrestUITextLabel(&App->UI, GENERIC_ID(0), "Blue");
                App->BackgroundColour.y = CrestUISlider(&App->UI, GENERIC_ID(0), App->BackgroundColour.y, BlueAmount);
                CrestUITextLabel(&App->UI, GENERIC_ID(0), "Green");
                App->BackgroundColour.z = CrestUISlider(&App->UI, GENERIC_ID(0), App->BackgroundColour.z, GreenAmount);
            }
            CrestUIPopRow(&App->UI);
        }
        CrestUIPopPanel(&App->UI);

        //Note(Zen): Diagnostic Panel
        r32 PanelWidth = 8.f * 3.f + 128.f * 2.f;
        CrestUIPushPanel(&App->UI, v2(platform->ScreenWidth - PanelWidth - 10.0f, 10.0f), 0.0f);
        {
            CrestUIPushRow(&App->UI, v2(platform->ScreenWidth - PanelWidth - 10.0f, 10.0f), v2(128.f, 32.f), 2);
            {
                r32 FPS = (platform->TargetFPS > platform->TimeTakenForFrame) ? platform->TargetFPS : 1.f / platform->TimeTakenForFrame;
                char FPSString[16];
                sprintf(FPSString, "%.2fFPS\0", FPS);
                char TimeTakenForFrameString[32];
                sprintf(TimeTakenForFrameString, "%.6fus\0", platform->TimeTakenForFrame * 1000000.f);

                CrestUITextLabel(&App->UI, GENERIC_ID(0), "FPS:");
                CrestUITextLabel(&App->UI, GENERIC_ID(0), FPSString);
                CrestUITextLabel(&App->UI, GENERIC_ID(0), "Time:");
                CrestUITextLabel(&App->UI, GENERIC_ID(0), TimeTakenForFrameString);
            }
            CrestUIPopRow(&App->UI);
        }
        CrestUIPopPanel(&App->UI);
    }
    CrestUIEndFrame(&App->UI, &App->UIRenderer);

    return AppShouldQuit;
}
