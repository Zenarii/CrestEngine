static r32 RedValue;
static r32 BlueValue;


internal void
DrawPaddle(app * App, paddle * Paddle) {
    r32 XOffset = (Paddle->PlayerNum == 0) ? 0.0f : App->Renderer.Width - PADDLE_WIDTH;
    C2DDrawColouredRect(&App->Renderer, v2(XOffset, Paddle->Offset), v2(30, PADDLE_HEIGHT), v3(RedValue, 1.f, BlueValue));
}

internal void
DrawBall(app * App, ball * Ball) {
    C2DDrawColouredRect(&App->Renderer, Ball->Position, v2(BALL_SIZE, BALL_SIZE), v3(RedValue, 1.f, BlueValue));
}

internal void
DrawScore(app * App, v2 Position, i32 Value) {
    i32 ScoreToDraw = NUMBERS[Value];
    for(int i = 0; i < 20; ++i) {
        b32 DrawSquare = (1 << i) & ScoreToDraw;
        if(DrawSquare) {
            v2 SquarePosition = Position;
            SquarePosition.x += (i % 4) * BALL_SIZE;
            SquarePosition.y += (i / 4) * BALL_SIZE;

            C2DDrawColouredRect(&App->Renderer, SquarePosition, v2(BALL_SIZE, BALL_SIZE), v3(RedValue, 1.f, BlueValue));
        }
    }
}

internal v2
RandomUnitVector() {
    r32 AngleFromHorizontal = rand() % 60 - 30.f;
    AngleFromHorizontal *= (3.14159265f / 180.f);
    return v2(cos(AngleFromHorizontal), sin(AngleFromHorizontal));
}

internal game_data
initGame(app * App) {
    r32 YOffset = (App->Renderer.Height - PADDLE_HEIGHT) * 0.5f;

    paddle Paddle0 = {0, YOffset};
    paddle Paddle1 = {1, YOffset};
    //r32 Angle =
    ball Ball = {0};
    Ball.Position = v2(App->Renderer.Width * 0.5f - 15.f, App->Renderer.Height * 0.5f - 15.f);
    Ball.Velocity = RandomUnitVector();;
    game_data result = {0, 0, //scores
                        Paddle0, //Player1
                        Paddle1,  //Player2
                        Ball
                       };
    return result;
}


internal b32
doGame(app * App) {
    paddle * Paddle0 = &App->Game.Paddle0;
    paddle * Paddle1 = &App->Game.Paddle1;
    ball * Ball = &App->Game.Ball;

    static r32 TimeSinceScore = 0.f;
    TimeSinceScore += App->Delta;
    r32 Speed = 5.f * TimeSinceScore + BALL_BASE_SPEED;

    static r32 TimeRunning = 0.f;
    TimeRunning += App->Delta;

    RedValue = sinf(TimeRunning * 0.3f);
    BlueValue = cosf(TimeRunning * 0.2f);

    //Note(Zen): move everything
    {
        Paddle0->Offset += (r32)App->KeyDown[KEY_S] * App->Delta * PLAYER_SPEED;
        Paddle0->Offset -= (r32)App->KeyDown[KEY_W] * App->Delta * PLAYER_SPEED;

        if(Paddle0->Offset <= 0.0f) Paddle0->Offset = 0.0f;
        else if(Paddle0->Offset + PADDLE_HEIGHT >= App->Renderer.Height) Paddle0->Offset = App->Renderer.Height - PADDLE_HEIGHT;

        Paddle1->Offset += (r32)App->KeyDown[KEY_DOWN] * App->Delta * PLAYER_SPEED;
        Paddle1->Offset -= (r32)App->KeyDown[KEY_UP]   * App->Delta * PLAYER_SPEED;

        if(Paddle1->Offset <= 0.0f) Paddle1->Offset = 0.0f;
        else if(Paddle1->Offset + PADDLE_HEIGHT >= App->Renderer.Height) Paddle1->Offset = App->Renderer.Height - PADDLE_HEIGHT;

        Ball->Position = v2(Ball->Position.x + (Ball->Velocity.x * App->Delta * Speed),
                            Ball->Position.y + (Ball->Velocity.y * App->Delta * Speed));
    }

    //Note(Zen): check collisions
    {
        //Note(Zen): Ball against top/bottom
        if(Ball->Position.y < 0.f) {
            Ball->Velocity.y *= -1.f;
            Ball->Position.y = 0.0f;
        }
        if(Ball->Position.y + BALL_SIZE > App->Renderer.Height) {
            Ball->Velocity.y *= -1.f;
            Ball->Position.y = App->Renderer.Height - BALL_SIZE;
        };

        //Note(Zen): Ball against Paddle0
        r32 XOffset = 0.f;
        if(Ball->Position.x >= XOffset && Ball->Position.x <= XOffset + PADDLE_WIDTH &&
           Ball->Position.y <= (Paddle0->Offset + PADDLE_HEIGHT) && Ball->Position.y >= Paddle0->Offset) {
               r32 DistFromCenter = (Ball->Position.y + BALL_SIZE * 0.5f) - (Paddle0->Offset + PADDLE_HEIGHT * 0.5f);
               DistFromCenter /= (PADDLE_HEIGHT * 0.5f);
               Ball->Velocity = v2(cosf(DistFromCenter), sinf(DistFromCenter));
        }
        else if(Ball->Position.x >= XOffset && Ball->Position.x <= XOffset + PADDLE_WIDTH &&
           Ball->Position.y + BALL_SIZE <= (Paddle0->Offset + PADDLE_HEIGHT) && Ball->Position.y + BALL_SIZE >= Paddle0->Offset) {
               r32 DistFromCenter = (Ball->Position.y + BALL_SIZE * 0.5f) - (Paddle0->Offset + PADDLE_HEIGHT * 0.5f);
               DistFromCenter /= (PADDLE_HEIGHT * 0.5f);
               Ball->Velocity = v2(cosf(DistFromCenter), sinf(DistFromCenter));
        }

        //Note(Zen): Ball against Paddle1
        XOffset = App->Renderer.Width;
        if(Ball->Position.x + BALL_SIZE <= XOffset && Ball->Position.x + BALL_SIZE >= XOffset - PADDLE_WIDTH &&
           Ball->Position.y <= (Paddle1->Offset + PADDLE_HEIGHT) && Ball->Position.y >= Paddle1->Offset) {
               r32 DistFromCenter = (Ball->Position.y + BALL_SIZE * 0.5f) - (Paddle1->Offset + PADDLE_HEIGHT * 0.5f);
               DistFromCenter /= (PADDLE_HEIGHT * 0.5f);
               Ball->Velocity = v2(-cosf(DistFromCenter), sinf(DistFromCenter));
        }
        else if(Ball->Position.x + BALL_SIZE <= XOffset && Ball->Position.x + BALL_SIZE >= XOffset - PADDLE_WIDTH &&
           Ball->Position.y + BALL_SIZE <= (Paddle1->Offset + PADDLE_HEIGHT) && Ball->Position.y + BALL_SIZE >= Paddle1->Offset) {
               r32 DistFromCenter = (Ball->Position.y + BALL_SIZE * 0.5f) - (Paddle1->Offset + PADDLE_HEIGHT * 0.5f);
               DistFromCenter /= (PADDLE_HEIGHT * 0.5f);
               Ball->Velocity = v2(-cosf(DistFromCenter), sinf(DistFromCenter));
        }

        //Note(Zen): Ball against left edge
        if(Ball->Position.x <= 0) {
            App->Game.P2Score += 1;
            Ball->Position = v2(App->Renderer.Width * 0.5f - 15.f, App->Renderer.Height * 0.5f - 15.f);
            TimeSinceScore = 1.f;
            Ball->Velocity = RandomUnitVector();
        }
        //Note(Zen): Ball against Right Edge
        if(Ball->Position.x + BALL_SIZE >= App->Renderer.Width) {
            App->Game.P1Score += 1;
            Ball->Position = v2(App->Renderer.Width * 0.5f - 15.f, App->Renderer.Height * 0.5f - 15.f);
            TimeSinceScore = 1.f;
            Ball->Velocity = RandomUnitVector();
            Ball->Velocity.x *= -1;
        }
    }


    //Note(Zen): Draw Everything
    DrawPaddle(App, Paddle0);
    DrawPaddle(App, Paddle1);

    v2 P1ScorePosition = v2(App->Renderer.Width * 0.5 - 5 * BALL_SIZE, BALL_SIZE);
    DrawScore(App, P1ScorePosition, App->Game.P1Score);
    v2 P2ScorePosition = v2(App->Renderer.Width * 0.5 + BALL_SIZE, BALL_SIZE);
    DrawScore(App, P2ScorePosition, App->Game.P2Score);

    DrawBall(App, Ball);

    b32 GameWon = (App->Game.P1Score == 11) || (App->Game.P2Score == 11);
    return GameWon;
}

internal void
doWin(app * App) {
    i32 Winner = (App->Game.P1Score == 11) ? 1 : 2;
    paddle * Paddle0 = &App->Game.Paddle0;
    paddle * Paddle1 = &App->Game.Paddle1;
    ball * Ball = &App->Game.Ball;

    static r32 TimeSinceBlink = 0.f;
    TimeSinceBlink += App->Delta;
    if(TimeSinceBlink > 2.f) TimeSinceBlink -= 2.f;



    DrawPaddle(App, Paddle0);
    DrawPaddle(App, Paddle1);

    if((Winner == 1 && TimeSinceBlink > 1.f) || Winner == 2) {
        v2 P1ScorePosition = v2(App->Renderer.Width * 0.5 - 6 * BALL_SIZE, BALL_SIZE);
        DrawScore(App, P1ScorePosition, App->Game.P1Score);
    }

    if((Winner == 2 && TimeSinceBlink > 1.f) || Winner == 1) {
        v2 P2ScorePosition = v2(App->Renderer.Width * 0.5 + BALL_SIZE, BALL_SIZE);
        DrawScore(App, P2ScorePosition, App->Game.P2Score);
    }

    DrawBall(App, Ball);
}
