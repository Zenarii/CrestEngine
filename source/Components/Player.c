#include "Player.h"

internal void
PushPlayer(PlayerSet * Set, PositionComponent Position, SpriteComponent Sprite) {
    u32 index = Set->Count++;
    Set->Position[index] = Position;
    Set->Sprite[index] = Sprite;
}

internal void
doPlayerSet(PlayerSet * Set, Platform * platform, app * tempApp) {
    for(u32 i = 0; i < Set->Count; ++i) {
        //Change Position
        Set->Position[i].x += (r32)(platform->KeyDown[KEY_D] - platform->KeyDown[KEY_A]) * 128.0f * tempApp->Delta;
        Set->Position[i].y += (r32)(platform->KeyDown[KEY_S] - platform->KeyDown[KEY_W]) * 128.0f * tempApp->Delta;
    }

    for(u32 i = 0; i < Set->Count; ++i) {
        CrestUIButton(&tempApp->UI, GENERIC_ID(0), v4(Set->Position[i].x, Set->Position[i].y, 32.0f, 32.0f), "Player");
    }
}
