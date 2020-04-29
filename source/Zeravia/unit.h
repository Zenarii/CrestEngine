#define MAX_PLAYER_UNITS 16
#define MAX_ENEMY_UNITS 32
#define MAX_UNIT_MOVE 8
#define UNIT_SPEED 5.f

typedef struct unit unit;
struct unit {
    //Note(Zen): Gameplay Info
    u32 CellIndex;
    b32 HasMoved;
    b32 Exhausted;

    //Note(Zen): Visual Info
    v3 Position;
};
