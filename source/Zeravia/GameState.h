//TODO(Zen): Increase this when an actual animation is present
#define NEW_TURN_TIME 0.1f

enum {
    GAME_STATE_NEW_TURN,

    GAME_STATE_OVERVIEW,
    GAME_STATE_UNIT_SELECTED,

    GAME_STATE_WATCH_MOVE,
    GAME_STATE_WATCH_BATTLE,

    GAME_STATE_TRANSITION_BATTLE_IN,
    GAME_STATE_TRANSITION_BATTLE_OUT,

    GAME_STATE_COUNT
};

enum {
    GAME_UI_START,
    GAME_UI_CHOOSE_TARGET,
    GAME_UI_INVENTORY
};

typedef struct game_state game_state;
struct game_state {
    i32 CurrentState;
    b32 IsPlayerTurn;
    camera Camera;
    //Note(Zen): state specific info
    struct {
        r32 Time;
    } NewTurn;
    struct {
        r32 Time;
        i32 Current;
        i32 StateAfter;
        i32 SubStateAfter;
        hex_path Path;
    } WatchMove;
    struct {
        i32 StartIndex;
        i32 UIState;
        b32 HideUI; //for when moving
        i32 CurrentTarget;
    } UnitSelected;

    //Teams
    struct player_info {
        u32 ActiveUnits;
        u32 SelectedUnit;
        u32 UnitCount;
        unit Units[MAX_PLAYER_UNITS];
        inventory Inventory[MAX_PLAYER_UNITS];
    } Player;

    struct enemy_info {
        u32 UnitCount;
        unit Units[MAX_ENEMY_UNITS];
    } Enemy;
};

global struct {
    b32 ShowCollisions;
} GameStateDebug;
