#ifndef PLAYER_H
#define PLAYER_H

#define MAX_PLAYERS 4

typedef u32 EntityID;

typedef struct SpriteComponent {
    EntityID id;
    v4 Source;
} SpriteComponent;

typedef struct PositionComponent {
    EntityID id;
    union {
        struct {
            r32 x;
            r32 y;
        };
        v2 Pos;
    };
} PositionComponent;

typedef struct PlayerSet {
    u32 Count;
    PositionComponent Position[MAX_PLAYERS];
    SpriteComponent Sprite[MAX_PLAYERS];
} PlayerSet;

#endif
