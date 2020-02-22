#include "shader.c"
#include "renderer.c"

typedef struct App {
    b32 Initialised;
} App;

internal b32
AppUpdate(Platform * platform) {
    b32 appShouldQuit = 0;
    App * app = platform->PermenantStorage;

    if(!app->Initialised) {
        app->Initialised = 1;
        CrestRendererInit();
    }

    //Note(Zen): Per-Frame initialisation
    {
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }


    CrestRendererStartFrame();
    {
        if(!platform->KeyDown[KEY_A]) {
            CrestPushFilledRect(v4(1.0f, 1.0f, 1.0f, 1.0f), v2(0.0f, 0.0f), v2(1.0f, 1.0f));
            CrestPushFilledRect(v4(0.5f, 0.5f, 1.0f, 1.0f), v2(0.0f, 0.0f), v2(-1.0f, -1.0f));
            CrestPushFilledRect(v4(0.5f, 0.5f, 0.5f, 1.0f), v2(0.0f, 0.0f), v2(-1.0f, 1.0f));
        }
        else {
            CrestPushFilledRect(v4(0.0f, 0.8f, 0.8f, 1.0f), v2(-0.5f, -0.5), v2(1.0f, 1.0f));
        }
    }
    CrestRender();



    return appShouldQuit;
}
