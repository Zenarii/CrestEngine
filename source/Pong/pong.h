#include "math.h"

#define PADDLE_WIDTH 30.0f
#define PADDLE_HEIGHT 100.f
#define BALL_SIZE 30.0f

#define PLAYER_SPEED 400.f
#define BALL_BASE_SPEED 300.f

#define ZERO 432534
#define ONE 4473956
#define TWO 988807
#define THREE 493191
#define FOUR 324964
#define FIVE 493087
#define SIX 431894
#define SEVEN 140423
#define EIGHT 431766
#define NINE 560790
#define TEN 375733
#define ELEVEN 699130

static i32 NUMBERS[16] = {ZERO, ONE, TWO, THREE, FOUR, FIVE,
                          SIX, SEVEN, EIGHT, NINE, TEN, ELEVEN};

typedef struct paddle paddle;
struct paddle {
    i32 PlayerNum;
    r32 Offset;
};

typedef struct ball ball;
struct ball {
    v2 Position;
    v2 Velocity;
};

typedef struct game_data {
    i32 P1Score;
    i32 P2Score;
    union {
        struct {
            paddle Paddle0;
            paddle Paddle1;
        };
        paddle Paddles[2];
    };
    ball Ball;
} game_data;
