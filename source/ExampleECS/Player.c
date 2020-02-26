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
        Set->Position[i].x += (platform->MouseEndX - Set->Position[i].x) * 0.5f * tempApp->Delta ;
        Set->Position[i].y += (platform->MouseEndY - Set->Position[i].y) * 0.5f * tempApp->Delta ;
        CrestUIButton(&tempApp->UI, GENERIC_ID(i), v4(Set->Position[i].x, Set->Position[i].y, Set->Sprite[i].Source.width, Set->Sprite[i].Source.height), "Player");
    }

}
