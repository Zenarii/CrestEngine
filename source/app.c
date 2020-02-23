#include "shader.c"
#include "ui/ui_renderer.c"
#include "ui/ui.c"

typedef struct app {
    b32 Initialised;
    r32 Delta;

    CrestUI UI;
    ui_renderer UIRenderer;
    b32 toggle;
} app;


internal b32
AppUpdate(Platform * platform) {
    b32 AppShouldQuit = 0;
    app * App = platform->PermenantStorage;

    if(!App->Initialised) {
        App->Initialised = 1;
        CrestUIRendererInit(&App->UIRenderer);

        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(0.2f, 0.3f, 0.2f, 0.0f);
        App->toggle = 0;
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
        if(CrestUIButton(&App->UI, GENERIC_ID(0), v4(10.0f, 10.0f, 100.0f, 100.0f), "Some Text")) {
            App->toggle = !App->toggle;
            OutputDebugString("Button 1 pressed\n");
        }
        if(App->toggle) {
            if(CrestUIButton(&App->UI, GENERIC_ID(0), v4(200.0f, 100.0f, 100.0f, 100.0f), "Some Text")) {
                AppShouldQuit = 1;
            }
        }
    }
    CrestUIEnd(&App->UI, &App->UIRenderer);
    CrestUIRender(&App->UIRenderer);



    return AppShouldQuit;
}
