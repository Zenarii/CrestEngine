#include "debug.c"

#define STB_TRUETYPE_IMPLEMENTATION
#define ALLOW_UNALIGNED_TRUETYPE
#include "stb/stb_truetype.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "CMaths/Vectors.c"
#include "CMaths/Matrices.c"
#include "CMaths/ProjectionMatrices.c"

#include "shader.c"
#include "ui/ui_renderer.c"
#include "ui/ui.c"

#include "CAssets/Textures.c"
#include "C3D/3DRenderer.c"

#include "Zeravia/Zeravia.h"



typedef struct app {
    b32 Initialised;
    r32 Delta;
    r32 ScreenWidth;
    r32 ScreenHeight;
    v2 MousePosition;
    b32 LeftMouseDown;
    b32 RightMouseDown;
    b32 KeyDown[CREST_KEY_MAX];
    b32 KeyWasDown[CREST_KEY_MAX];

    CrestUI UI;
    ui_renderer UIRenderer;

    C3DRenderer Renderer;

    editor_state EditorState;
} app;

#include "Zeravia/Zeravia.c"


#define UI_ID_OFFSET 20
enum Textures {
    TEXTURE_WHITE,
    TEXTURE_TEXT,

    TEXTURE_COUNT
};

global app * App;

internal b32
AppKeyJustDown(i32 Key) {
    return (App->KeyDown[Key] && !App->KeyWasDown[Key]);
}

internal b32
AppUpdate(Platform * platform) {
    b32 AppShouldQuit = 0;
    App = platform->PermenantStorage;
    static b32 OnMenu = 1;

    if(!App->Initialised) {
        App->Initialised = 1;
        CrestUIRendererInit(&App->UIRenderer);
        CrestUIRendererLoadFont(&App->UIRenderer, "../assets/LiberationMono-Regular.ttf");
        C3DInit(&App->Renderer);
        App->Renderer.Textures[0] = CasLoadTexture("../assets/White.png", GL_LINEAR);


        App->Renderer.ActiveTextures = TEXTURE_COUNT;
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        platform->TargetFPS = 60.0f;

        EditorStateInit(App);
    }

    //Note(Zen): Per-Frame initialisation
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        App->UIRenderer.Width = platform->ScreenWidth;
        App->UIRenderer.Height = platform->ScreenHeight;
        App->ScreenWidth = platform->ScreenWidth;
        App->ScreenHeight = platform->ScreenHeight;
        App->Delta = 1.f / platform->TargetFPS;
        memcpy(App->KeyWasDown, App->KeyDown, sizeof(b32) * CREST_KEY_MAX);
        memcpy(App->KeyDown, platform->KeyDown, sizeof(b32) * CREST_KEY_MAX);
        App->MousePosition = v2(platform->MouseEndX, platform->MouseEndY);

        App->LeftMouseDown = platform->LeftMouseDown;
        App->RightMouseDown = platform->RightMouseDown;
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

    EditorStateUpdate(App);

    C3DFlush(&App->Renderer);

    CrestUIEndFrame(&App->UI, &App->UIRenderer);




    return AppShouldQuit;
}
#undef UI_ID_OFFSET
