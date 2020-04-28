enum game_states {
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
    enum game_states CurrentState;
    //Note(Zen): state specific info
    struct {
        i32 Current;
        hex_path Path;
    } WatchMove;
    struct {
        b32 HideUI; //for when moving
    } UnitSelected;

    camera Camera;
    //Units
    //TODO(Zen): Consider this as a struct?
    //would mean less typing using player_info * ...
    u32 PlayerUnitsCount;
    u32 ActiveUnits;
    u32 SelectedUnit;
    unit PlayerUnits[MAX_PLAYER_UNITS];
};

global struct {
    b32 ShowCollisions;
} GameStateDebug;
