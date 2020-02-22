#include "shader.c"
#include "ui_renderer.c"

typedef struct app {
    b32 Initialised;
    r32 Delta;
    ui_renderer UIRenderer;
} app;


internal b32
AppUpdate(Platform * platform) {
    b32 AppShouldQuit = 0;
    app * App = platform->PermenantStorage;

    if(!App->Initialised) {
        App->Initialised = 1;
        CrestUIRendererInit(&App->UIRenderer);
    }

    //Note(Zen): Per-Frame initialisation
    {
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        App->UIRenderer.Width = platform->ScreenWidth;
        App->UIRenderer.Height = platform->ScreenHeight;
    }


    CrestUIRendererStartFrame(&App->UIRenderer);
    {
        CrestPushFilledRect(&App->UIRenderer, v4(1.0f, 1.0f, 1.0f, 1.0f), v2(0.0f, 0.0f), v2(10.0f, 10.0f));
    }
    CrestRender(&App->UIRenderer);



    return AppShouldQuit;
}
