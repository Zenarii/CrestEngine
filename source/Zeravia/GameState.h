enum {
    GAME_STATE_OVERVIEW,
    GAME_STATE_UNIT_SELECTED,

    GAME_STATE_WATCH_MOVE,
    GAME_STATE_WATCH_BATTLE,

    GAME_STATE_TRANSITION_BATTLE_IN,
    GAME_STATE_TRANSITION_BATTLE_OUT,

    GAME_STATE_COUNT,
};

typedef struct game_state game_state;
struct game_state {
    camera Camera;
    i32 CurrentState;
    //Note(Zen): state specific info
    struct {
        i32 Current;
        hex_path Path;
    } WatchMove;
    struct {
        b32 HideUI; //for when moving
    } UnitSelected;

    struct player_info {
        u32 ActiveUnits;
        u32 SelectedUnit;
        u32 UnitCount;
        unit Units[MAX_PLAYER_UNITS];
    } Player;

    struct enemy_info {
        unit Unit;
    } Enemy;
};

global struct {
    b32 ShowCollisions;
} GameStateDebug;
