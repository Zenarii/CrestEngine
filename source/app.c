#include "shader.c"
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

        glEnable (GL_BLEND); glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    //Note(Zen): Per-Frame initialisation
    {
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        App->UIRenderer.Width = platform->ScreenWidth;
        App->UIRenderer.Height = platform->ScreenHeight;
    }

    //Note(Zen): Copy across UI Input
    CrestUIInput UIIn = {0};
    {
        UIIn.MouseX = platform->MouseEndX;
        UIIn.MouseY = platform->MouseEndY;
    }

    CrestUIBegin(&App->UI, &UIIn);
    CrestUIRendererStartFrame(&App->UIRenderer);
    {
        v2 position = v2(platform->MouseEndX - 50.0f, platform->MouseEndY - 50.0f);
        v4 colour = platform->LeftMouseDown ? v4(1.0f, 1.0f, 1.0f, 0.0f) : v4(0.5f, 1.0f, 1.0f, 1.0f);
        CrestPushFilledRect(&App->UIRenderer, colour, position, v2(100.0f, 100.0f));
    }
    CrestUIRender(&App->UIRenderer);



    return AppShouldQuit;
}
