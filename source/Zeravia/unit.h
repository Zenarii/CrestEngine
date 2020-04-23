#define MAX_PLAYER_UNITS 16
#define MAX_UNIT_MOVE 8

typedef struct unit unit;
struct unit {
    //Used by game state
    u32 CellIndex;

    //Important for animations
    i32 AnimPosition;
    //Pathing
    u32 PathCount;
    u32 CellsInPath[MAX_UNIT_MOVE];
};
