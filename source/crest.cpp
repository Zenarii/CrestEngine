//################
//# Zenarii 2020 #
//################

#include "crest.h"

global_variable mesh GlobalMesh;
global_variable CrestShader ShaderProgram;

internal void
GameUpdateAndRender(platform *Platform) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    RenderMesh(&GlobalMesh, ShaderProgram, Platform);
}

internal void
CrestInit() {
    //Note(Zen): Enables transparency/3D
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    ShaderProgram = CrestShaderInit("C:/Dev/Crest/source/VertexShader.vs",
                                    "C:/Dev/Crest/source/FragmentShader.fs");
    GlobalMesh = CrestLoadMesh("C:/Dev/Crest/data/Cube.obj");
}
