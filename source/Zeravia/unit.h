#define MAX_PLAYER_UNITS 16
#define MAX_UNIT_MOVE 8
#define UNIT_SPEED 5.f

typedef struct unit unit;
struct unit {
    u32 CellIndex;
    b32 HasMoved;
    b32 Exhausted;

    v3 Position;
};
