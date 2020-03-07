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

enum APP_PANEL {
    STYLE_PANEL,
    BUTTON_PANEL
};

global b32 StylePanelOpen = 1;
global b32 AppShouldQuit = {0};

internal b32
doButtonPanel(app * App, r32 Precedence) {
    static v2 ButtonPanelPosition = {10.0f, 10.0f};
    ButtonPanelPosition = CrestUIDnDBoxP(&App->UI, GENERIC_ID(0), Precedence, v4(ButtonPanelPosition.x, ButtonPanelPosition.y, 220.0f + 16.0f, 32.0f), "Button Test");

    CrestUIPushPanel(&App->UI, v2(ButtonPanelPosition.x, ButtonPanelPosition.y + 32.0f), Precedence);
    {
        CrestUIPushRow(&App->UI, v2(ButtonPanelPosition.x, ButtonPanelPosition.y + 32.0f), v2(220.f, 32.f), 1);
        {
            if(StylePanelOpen) {
                StylePanelOpen = CrestUIButton(&App->UI, GENERIC_ID(0), "Close Style Editor") ? 0 : StylePanelOpen;
                if(!StylePanelOpen) {
                    App->UI.active = CrestUIIDNull();
                    App->UI.hot = CrestUIIDNull();
                }
            }
            AppShouldQuit = CrestUIButton(&App->UI, GENERIC_ID(0), "Close App");
        }
        CrestUIPopRow(&App->UI);
    }
    r32 width = App->UI.PanelStack[App->UI.PanelStackPosition-1].Width;
    r32 height = App->UI.PanelStack[App->UI.PanelStackPosition-1].Height;
    CrestUIPopPanel(&App->UI);

    b32 MouseOver = (App->UI.MouseX >= ButtonPanelPosition.x &&
                       App->UI.MouseY >= ButtonPanelPosition.y &&
                       App->UI.MouseX <= ButtonPanelPosition.x + width &&
                       App->UI.MouseY <= ButtonPanelPosition.y + height + 32.0f);

    return (MouseOver && App->UI.LeftMouseDown);
}


//Just for the demo
//~
internal b32
doStylePanel(app * App, r32 Precedence) {

        //Note(Zen): Style Editor Panel
        static v2 StyleEditorPanelPosition = {100.0f, 100.0f};
        StyleEditorPanelPosition = CrestUIDnDBoxP(&App->UI, GENERIC_ID(0), Precedence, v4(StyleEditorPanelPosition.x, StyleEditorPanelPosition.y, 256.0f + 24.0f, 32.0f), "Style Panel");

        CrestUIPushPanel(&App->UI, v2(StyleEditorPanelPosition.x, StyleEditorPanelPosition.y + 32.0f), Precedence);
        {
            //Note(Zen):
            CrestUIPushRow(&App->UI, v2(StyleEditorPanelPosition.x, StyleEditorPanelPosition.y + 32.f), v2(128.f, 32.f), 2);
            {
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

            //Note(Zen): Style Editor Panel
            static b32 ShowStyleEditToggle = 0;
            CrestUIPushRow(&App->UI, v2(StyleEditorPanelPosition.x, StyleEditorPanelPosition.y + 32.0f), v2(256.f + 8.f, 32.f), 1);
            {
                ShowStyleEditToggle = CrestUIButton(&App->UI, GENERIC_ID(0), "Style Edit") ? !ShowStyleEditToggle : ShowStyleEditToggle;
                if(ShowStyleEditToggle)
                    CrestUITextLabel(&App->UI, GENERIC_ID(0), "--Button--");
            }
            CrestUIPopRow(&App->UI);

            if(ShowStyleEditToggle) {

                CrestUIPushRow(&App->UI, v2(StyleEditorPanelPosition.x, StyleEditorPanelPosition.y + 32.f), v2(60.f, 32.f), 4);
                {
                    //Note(Zen): Button Colour
                    {
                        char String[8];

                        CrestUITextLabel(&App->UI, GENERIC_ID(0), "Normal");
                        sprintf(String, "%.2f", DefaultStyle.ButtonColour.x);
                        DefaultStyle.ButtonColour.x = CrestUISlider(&App->UI, GENERIC_ID(0), DefaultStyle.ButtonColour.x, String);
                        sprintf(String, "%.2f", DefaultStyle.ButtonColour.y);
                        DefaultStyle.ButtonColour.y = CrestUISlider(&App->UI, GENERIC_ID(0), DefaultStyle.ButtonColour.y, String);
                        sprintf(String, "%.2f", DefaultStyle.ButtonColour.z);
                        DefaultStyle.ButtonColour.z = CrestUISlider(&App->UI, GENERIC_ID(0), DefaultStyle.ButtonColour.z, String);
                    }

                    //Note(Zen): Button Hot Colour
                    {
                        char String[8];

                        CrestUITextLabel(&App->UI, GENERIC_ID(0), "Hot");
                        sprintf(String, "%.2f", DefaultStyle.ButtonHotColour.x);
                        DefaultStyle.ButtonHotColour.x = CrestUISlider(&App->UI, GENERIC_ID(0), DefaultStyle.ButtonHotColour.x, String);
                        sprintf(String, "%.2f", DefaultStyle.ButtonHotColour.y);
                        DefaultStyle.ButtonHotColour.y = CrestUISlider(&App->UI, GENERIC_ID(0), DefaultStyle.ButtonHotColour.y, String);
                        sprintf(String, "%.2f", DefaultStyle.ButtonHotColour.z);
                        DefaultStyle.ButtonHotColour.z = CrestUISlider(&App->UI, GENERIC_ID(0), DefaultStyle.ButtonHotColour.z, String);
                    }

                    //Note(Zen): Border Colour
                    {
                        char String[8];

                        CrestUITextLabel(&App->UI, GENERIC_ID(0), "Border");
                        sprintf(String, "%.2f", DefaultStyle.ButtonBorderColour.x);
                        DefaultStyle.ButtonBorderColour.x = CrestUISlider(&App->UI, GENERIC_ID(0), DefaultStyle.ButtonBorderColour.x, String);
                        sprintf(String, "%.2f", DefaultStyle.ButtonBorderColour.y);
                        DefaultStyle.ButtonBorderColour.y = CrestUISlider(&App->UI, GENERIC_ID(0), DefaultStyle.ButtonBorderColour.y, String);
                        sprintf(String, "%.2f", DefaultStyle.ButtonBorderColour.z);
                        DefaultStyle.ButtonBorderColour.z = CrestUISlider(&App->UI, GENERIC_ID(0), DefaultStyle.ButtonBorderColour.z, String);
                    }
                }
                CrestUIPopRow(&App->UI);

                CrestUIPushRow(&App->UI, v2(StyleEditorPanelPosition.x, StyleEditorPanelPosition.y + 32.0f), v2(256.f + 8.f, 32.f), 1);
                {
                    CrestUITextLabel(&App->UI, GENERIC_ID(0), "--Header--");
                }
                CrestUIPopRow(&App->UI);

                CrestUIPushRow(&App->UI, v2(StyleEditorPanelPosition.x, StyleEditorPanelPosition.y + 32.f), v2(60.f, 32.f), 4);
                {
                    //Note(Zen): Background Colour
                    {
                        char String[8];

                        CrestUITextLabel(&App->UI, GENERIC_ID(0), "Main");
                        sprintf(String, "%.2f", DefaultStyle.HeaderColour.x);
                        DefaultStyle.HeaderColour.x = CrestUISlider(&App->UI, GENERIC_ID(0), DefaultStyle.HeaderColour.x, String);
                        sprintf(String, "%.2f", DefaultStyle.HeaderColour.y);
                        DefaultStyle.HeaderColour.y = CrestUISlider(&App->UI, GENERIC_ID(0), DefaultStyle.HeaderColour.y, String);
                        sprintf(String, "%.2f", DefaultStyle.HeaderColour.z);
                        DefaultStyle.HeaderColour.z = CrestUISlider(&App->UI, GENERIC_ID(0), DefaultStyle.HeaderColour.z, String);
                    }

                    //Note(Zen): Border Colour
                    {
                        char String[8];

                        CrestUITextLabel(&App->UI, GENERIC_ID(0), "Border");
                        sprintf(String, "%.2f", DefaultStyle.HeaderBorderColour.x);
                        DefaultStyle.HeaderBorderColour.x = CrestUISlider(&App->UI, GENERIC_ID(0), DefaultStyle.HeaderBorderColour.x, String);
                        sprintf(String, "%.2f", DefaultStyle.HeaderBorderColour.y);
                        DefaultStyle.HeaderBorderColour.y = CrestUISlider(&App->UI, GENERIC_ID(0), DefaultStyle.HeaderBorderColour.y, String);
                        sprintf(String, "%.2f", DefaultStyle.HeaderBorderColour.z);
                        DefaultStyle.HeaderBorderColour.z = CrestUISlider(&App->UI, GENERIC_ID(0), DefaultStyle.HeaderBorderColour.z, String);
                    }
                }
                CrestUIPopRow(&App->UI);

                CrestUIPushRow(&App->UI, v2(StyleEditorPanelPosition.x, StyleEditorPanelPosition.y + 32.0f), v2(256.f + 8.f, 32.f), 1);
                {
                    CrestUITextLabel(&App->UI, GENERIC_ID(0), "--Panel--");
                }
                CrestUIPopRow(&App->UI);

                CrestUIPushRow(&App->UI, v2(StyleEditorPanelPosition.x, StyleEditorPanelPosition.y + 32.f), v2(60.f, 32.f), 4);
                {
                    //Note(Zen): Background Colour
                    {
                        char String[8];

                        CrestUITextLabel(&App->UI, GENERIC_ID(0), "Main");
                        sprintf(String, "%.2f", DefaultStyle.PanelColour.x);
                        DefaultStyle.PanelColour.x = CrestUISlider(&App->UI, GENERIC_ID(0), DefaultStyle.PanelColour.x, String);
                        sprintf(String, "%.2f", DefaultStyle.PanelColour.y);
                        DefaultStyle.PanelColour.y = CrestUISlider(&App->UI, GENERIC_ID(0), DefaultStyle.PanelColour.y, String);
                        sprintf(String, "%.2f", DefaultStyle.PanelColour.z);
                        DefaultStyle.PanelColour.z = CrestUISlider(&App->UI, GENERIC_ID(0), DefaultStyle.PanelColour.z, String);
                    }
                }
                CrestUIPopRow(&App->UI);
            }
        }
        r32 width = App->UI.PanelStack[App->UI.PanelStackPosition-1].Width;
        r32 height = App->UI.PanelStack[App->UI.PanelStackPosition-1].Height;
        CrestUIPopPanel(&App->UI);

        b32 MouseOver = (App->UI.MouseX >= StyleEditorPanelPosition.x &&
                           App->UI.MouseY >= StyleEditorPanelPosition.y &&
                           App->UI.MouseX <= StyleEditorPanelPosition.x + width &&
                           App->UI.MouseY <= StyleEditorPanelPosition.y + height + 32.0f);

        return (MouseOver && App->UI.LeftMouseDown);
}


//~
internal b32
AppUpdate(Platform * platform) {
    AppShouldQuit = 0;
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

        //static enum APP_PANEL CurrentPanel = STYLE_PANEL;
        static enum APP_PANEL CurrentPanel = STYLE_PANEL;
        if(CurrentPanel == STYLE_PANEL) {
            CurrentPanel = doButtonPanel(App, -0.1f) ? BUTTON_PANEL : CurrentPanel;
            if(StylePanelOpen) {
                CurrentPanel = doStylePanel(App, -0.2f) ? STYLE_PANEL : CurrentPanel;
            }
        }
        else {
            if(StylePanelOpen) {
                CurrentPanel = doStylePanel(App, -0.1f) ? STYLE_PANEL : CurrentPanel;
            }
            CurrentPanel = doButtonPanel(App, -0.2f) ? BUTTON_PANEL : CurrentPanel;
        }
    }
    CrestUIEndFrame(&App->UI, &App->UIRenderer);

    return AppShouldQuit;
}
