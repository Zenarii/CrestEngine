//################
//# Zenarii 2020 #
//################

#include "crest.h"

global_variable mesh GlobalMesh;
global_variable mesh GlobalMesh2;

global_variable CrestShader ShaderProgram;
global_variable CrestShader Shader2;


internal void
GameUpdateAndRender(platform *Platform) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    CrestRenderMesh(&GlobalMesh, ShaderProgram, Platform);
    CrestRenderMesh(&GlobalMesh2, Shader2, Platform);

}

internal void
CrestInit() {
    //Note(Zen): Enables transparency/3D
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    ShaderProgram = CrestShaderInit("C:/Dev/Crest/source/VertexShader.vs",
                                    "C:/Dev/Crest/source/FragmentShader.fs");
    Shader2 = CrestShaderInit("C:/Dev/Crest/source/VertexShader2.vs",
                              "C:/Dev/Crest/source/FragmentShader2.fs");
    GlobalMesh = CrestLoadMesh("C:/Dev/Crest/data/Cube.obj");
    GlobalMesh2 = CrestLoadMesh("C:/Dev/Crest/data/Cube.obj");
    
}
