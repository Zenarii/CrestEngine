//################
//# Zenarii 2020 #
//################

#include "crest.h"

internal void
GameUpdateAndRender() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    RenderTriangle();
}
