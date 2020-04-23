typedef struct game_state game_state;
struct game_state {
    camera Camera;
};

global struct {
    b32 ShowCollisions;
} GameStateDebug;
