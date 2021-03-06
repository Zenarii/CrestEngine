//TODO(Zenarii): Unit UI should remain in the same place.
//maybe use trig to project to a position x distance away based on camera rotation
#include "debug.c"

#define STB_TRUETYPE_IMPLEMENTATION
#define ALLOW_UNALIGNED_TRUETYPE
#include "stb/stb_truetype.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "CMaths/Reals.c"
#include "CMaths/Vectors.c"
#include "CMaths/Matrices.c"
#include "CMaths/ProjectionMatrices.c"

#include "memory_arena.c"
#include "shader.c"
#include "ui/ui_renderer.c"
#include "ui/ui.c"

#include "CAssets/Textures.c"
#include "CAssets/Models.c"
#include "framebuffer.h"
#include "C3D/3DRenderer.c"

#include "CRandom/Random.c"
#include "CRandom/Noise.c"

#include "resource.h"

#include "Zeravia/Zeravia.h"

typedef struct screen_rect screen_rect;
struct screen_rect {
    u32 Shader;
    u32 VAO;
    u32 VBO;
};

internal screen_rect
ScreenRectInit() {
    float Vertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    u32 VAO, VBO;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), &Vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);


    u32 Shader = CrestLoadShader("assets/Shaders/quad_shader.vs",
                                 "assets/Shaders/quad_shader.fs", 0);

    glUseProgram(Shader);
    CrestShaderSetInt(Shader, "ScreenTexture", 0);

    screen_rect ScreenRect = {0};
    ScreenRect.Shader = Shader;
    ScreenRect.VAO = VAO;
    ScreenRect.VBO = VBO;

    return ScreenRect;
}


#define APP_STATE_EDITOR (1<<0)
#define APP_STATE_GAME (1<<2)

typedef struct app app;
struct app {
    b32 Initialised;
    r64 Delta;
    r32 TotalTime;
    r32 ScreenWidth;
    r32 ScreenHeight;

    struct {
        v2 Position;
        b32 LeftDown;
        b32 RightDown;
        b32 LeftWasDown;
        b32 RightWasDown;
        r32 Scroll;
    } Mouse;

    b32 KeyDown[CREST_KEY_MAX];
    b32 KeyWasDown[CREST_KEY_MAX];
    u32 Cursor;
    char PutCharacters[16];

    CrestUI UI;
    ui_renderer UIRenderer;

    C3DRenderer Renderer;
    fbo FBO;
    screen_rect ScreenRect;

    //Note(Zen): Resources
    struct {
        union {
            struct {
                #define INCLUDE_SHADERS
                #define APP_RESOURCE(Name, Path, States) resource Name;
                #include "app_resources.h"
            } Shaders;
            //TODO(Zen): Array
        };
    };

    //Note(Zen): Game stuff
    hex_grid Grid;
    editor_state EditorState;
    game_state GameState;
};

global app * App;

#include "Zeravia/Zeravia.c"
#include "resource.c"

#define UI_ID_OFFSET 20
enum Textures {
    TEXTURE_WHITE,
    TEXTURE_TEXT,

    TEXTURE_COUNT
};


internal void
AppResize(r32 Width, r32 Height) {
    App->ScreenWidth = Width;
    App->ScreenHeight = Height;

    //Note(Zen): There is likely a better way to do this with
    //arbitary framebuffers, rather than having to add framebuffers here
    //like some sort of tagging system idk
    //
    CrestDeleteFramebuffer(App->Grid.RefractionFBO);
    App->Grid.RefractionFBO = CrestCreateFramebuffer(Width, Height, 1);
    CrestDeleteFramebuffer(App->Grid.ReflectionFBO);
    App->Grid.ReflectionFBO = CrestCreateFramebuffer(Width, Height, 0);
}


internal b32
AppKeyJustDown(i32 Key) {
    return (App->KeyDown[Key] && !App->KeyWasDown[Key]);
}

internal b32
AppMouseJustDown(i32 Button) {
    if(Button == 0) return App->Mouse.LeftDown && !App->Mouse.LeftWasDown;
    else if(Button == 1) return App->Mouse.RightDown && !App->Mouse.RightWasDown;

    Assert(Button == 1 && Button == 0);
    return 0;
}

internal void
AppClearChars() {
    memset(App->PutCharacters, 0, sizeof(App->PutCharacters));
    App->Cursor = 0;
}


internal void
AppPutChar(char Char) {
    if(App->Cursor < 15) {
        App->PutCharacters[App->Cursor++] = Char;
    }
    else {
        AppClearChars();
    }
}


internal void
AppDeletePrevChar() {
    if(App->Cursor > 0) {
        App->PutCharacters[--App->Cursor] = 0;
    }
}

internal b32
AppUpdate(Platform * platform) {
    b32 AppShouldQuit = 0;
    App = platform->PermenantStorage;
    static b32 OnMenu = 1;

    if(!App->Initialised) {
        App->Initialised = 1;

        App->ScreenWidth = platform->ScreenWidth;
        App->ScreenHeight = platform->ScreenHeight;

        CrestUIRendererInit(&App->UIRenderer);
        CrestUIRendererLoadFont(&App->UIRenderer, "assets/Fonts/Roboto-Bold.ttf");
        CrestUIInit(&App->UI);
        C3DInit(&App->Renderer);
        App->Renderer.Textures[0] = CasLoadTexture("assets/White.png", GL_LINEAR);


        App->Renderer.ActiveTextures = TEXTURE_COUNT;
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glClearColor(CLEAR_COLOUR);
        platform->TargetFPS = 120.0f;

        LoadAllResources(APP_STATE_GAME);
        HexGridInit(&App->Grid);
        GameStateInit(App);
        EditorStateInit(App);

        App->ScreenRect = ScreenRectInit();

        LabelResources(App);


    }

    //Note(Zen): Per-Frame initialisation
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        App->UIRenderer.Width = platform->ScreenWidth;
        App->UIRenderer.Height = platform->ScreenHeight;
        App->ScreenWidth = platform->ScreenWidth;
        App->ScreenHeight = platform->ScreenHeight;

        App->Delta = platform->TimeTaken; //1.f / platform->TargetFPS;
        App->TotalTime += App->Delta;
        memcpy(App->KeyWasDown, App->KeyDown, sizeof(b32) * CREST_KEY_MAX);
        memcpy(App->KeyDown, platform->KeyDown, sizeof(b32) * CREST_KEY_MAX);


        App->Mouse.Position = v2(platform->MouseEndX, platform->MouseEndY);
        App->Mouse.LeftWasDown = App->Mouse.LeftDown;
        App->Mouse.LeftDown = platform->LeftMouseDown;
        App->Mouse.RightWasDown = App->Mouse.RightDown;
        App->Mouse.RightDown = platform->RightMouseDown;
        App->Mouse.Scroll = platform->MouseScroll;
    }

    //Note(Zen): Copy across UI Input
    CrestUIInput UIIn = {0};
    {
        UIIn.MouseX = platform->MouseEndX;
        UIIn.MouseY = platform->MouseEndY;
        UIIn.MouseStartX = platform->MouseStartX;
        UIIn.MouseStartY = platform->MouseStartY;
        UIIn.LeftMouseDown = platform->LeftMouseDown;
        UIIn.LeftMouseWasDown = App->Mouse.LeftWasDown;
        UIIn.RightMouseDown = platform->RightMouseDown;
        UIIn.RightMouseWasDown = App->Mouse.RightWasDown;
    }

    /*
        Temp frame buffer stuff
    */

    CrestUIBeginFrame(&App->UI, &UIIn, &App->UIRenderer);


    static b32 InEditor = 0;

    if(InEditor && AppKeyJustDown(KEY_F3)) {
        InEditor = 0;
        GameStateFromEditorState(&App->GameState, &App->EditorState);
    }
    else if(!InEditor && AppKeyJustDown(KEY_F3)) {
        InEditor = 1;
        EditorStateFromGameState(&App->EditorState, &App->GameState);
    }

    if(InEditor) {
        EditorStateUpdate(App);
    }
    else {
        GameStateUpdate(App);
    }

    C3DFlush(&App->Renderer);


    #define APP_FPS_PANEL_WIDTH 200
    CrestUIPushRow(&App->UI, v2(App->ScreenWidth - APP_FPS_PANEL_WIDTH - 32.f, 0), v2(APP_FPS_PANEL_WIDTH, 32), 1);
    {
        char Buffer[64];
        sprintf(Buffer, "Time for frame: %fms", platform->TimeTakenForFrame * 1000.f);
        CrestUITextLabel(&App->UI, GENERIC_ID(0), Buffer);
        sprintf(Buffer, "Time passed: %fms", platform->TimeTaken * 1000.f);
        CrestUITextLabel(&App->UI, GENERIC_ID(0), Buffer);
        sprintf(Buffer, "FPS: %3.f/s", 1.f/App->Delta);
        CrestUITextLabel(&App->UI, GENERIC_ID(0), Buffer);
    }
    CrestUIPopRow(&App->UI);

    CrestUIEndFrame(&App->UI, &App->UIRenderer);

    AppClearChars();





    return AppShouldQuit;
}
#undef UI_ID_OFFSET
