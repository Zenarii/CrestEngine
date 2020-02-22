#include "shader.c"
#include "renderer.c"

typedef struct app {
    b32 Initialised;
    r32 Delta;
    renderer Renderer;
} app;

internal b32
AppUpdate(Platform * platform) {
    b32 AppShouldQuit = 0;
    app * App = platform->PermenantStorage;

    if(!App->Initialised) {
        App->Initialised = 1;
        CrestRendererInit(&App->Renderer);
    }

    //Note(Zen): Per-Frame initialisation
    {
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        App->Renderer.Width = platform->ScreenWidth;
        App->Renderer.Height = platform->ScreenHeight;
    }


    CrestRendererStartFrame(&App->Renderer);
    {
        CrestPushFilledRect(&App->Renderer, v4(1.0f, 1.0f, 1.0f, 1.0f), v2(0.0f, 0.0f), v2(App->Renderer.Width, App->Renderer.Height));
    }
    CrestRender(&App->Renderer);



    return AppShouldQuit;
}
