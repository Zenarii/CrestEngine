
typedef struct Sprite {
    i32 TextureID;
    v4 Rect;
} Sprite;

typedef struct Animation {
    i32 NumberOfFrames;
    r32 TimePerFrame;
    i32 CurrentFrame;
    r32 TimeOnFrame;
} Animation;

internal Sprite
SpriteInit(i32 TextureID, v4 Rect_pixelspace) {
    v4 Rect_texturespace = v4(Rect_pixelspace.x/512.0f, Rect_pixelspace.y/512.0f,
        Rect_pixelspace.z/512.0f, Rect_pixelspace.w/512.0f);

    Sprite Result = {TextureID, Rect_texturespace};
    return Result;
}

#define Sprite(TextureID, Rect) SpriteInit(TextureID, Rect)

internal void
DrawSprite(C2DRenderer * Renderer, v2 Position, Sprite * TestSprite, Animation * TestAnimation) {
    v4 Rect = v4(TestSprite->Rect.x + (TestSprite->Rect.width * (r32)TestAnimation->CurrentFrame),
                 TestSprite->Rect.y,
                 TestSprite->Rect.width, TestSprite->Rect.height);

    C2DDrawTexturedSlice(Renderer, Position, v2(128.f, 128.f), Rect, TestSprite->TextureID);
}
