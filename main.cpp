/**
* Author: Jing Qian
* Assignment: Simple 2D Scene
* Date due: 10/05/2026
*
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/
#include "raylib.h"

enum AppStatus { TERMINATED, RUNNING };

struct DrawableObject {
    const Texture2D &texture;
    Vector2 screenCoords;
    int totalAnimationFrames;
    int animationFrame;
    Vector2 scale;
    float rotation;

    DrawableObject(
        const Texture2D &texture,
        Vector2 screenCoords,
        int totalAnimationFrames,
        int animationFrame = 0,
        Vector2 scale = { 1.0f, 1.0f },
        float rotation = 0.0f
    ):
        texture(texture),
        screenCoords(screenCoords),
        totalAnimationFrames(totalAnimationFrames),
        animationFrame(animationFrame),
        scale(scale),
        rotation(rotation)
    {}
};

void drawToScreen(DrawableObject &obj) {
    static constexpr Vector2 DRAW_ORIGIN{ 0, 0 };
    float textureWidth = static_cast<float>(obj.texture.width) / obj.totalAnimationFrames;
    float textureHeight = static_cast<float>(obj.texture.height);

    DrawTexturePro(
        obj.texture,
        Rectangle{ textureWidth * obj.animationFrame, 0.0f, textureWidth, textureHeight },
        Rectangle{
            obj.screenCoords.x,
            obj.screenCoords.y,
            textureWidth * obj.scale.x,
            textureHeight * obj.scale.y
        },
        DRAW_ORIGIN,
        obj.rotation,
        WHITE
    );
}

const char *const TITLE = "lamp";

constexpr int SCREEN_WIDTH = 1280,
              SCREEN_HEIGHT = 720,
              FPS = 12;

AppStatus gAppStatus = RUNNING;

/* textures */
Texture2D lampTexture;
Texture2D postTexture;
Texture2D weedTexture;
Texture2D shadowTexture;

/* on-screen objects */
DrawableObject lamppost(postTexture, Vector2{ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f }, 2);

void init() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, TITLE);
    SetTargetFPS(FPS);

    // load textures
    lampTexture = LoadTexture("./assets/lamp.png");
    postTexture = LoadTexture("./assets/post.png");
    weedTexture = LoadTexture("./assets/tumbleweed.png");
    shadowTexture = LoadTexture("./assets/shadow.png");
}

void processInput() {
    if (WindowShouldClose()) gAppStatus = TERMINATED;
}

void update() {

}

void render() {
    BeginDrawing();

    ClearBackground(WHITE);
    
    
    
    EndDrawing();
}

void shutdown() {
    CloseWindow();
    UnloadTexture(lampTexture);
    UnloadTexture(postTexture);
    UnloadTexture(weedTexture);
    UnloadTexture(shadowTexture);
}

int main() {
    init();

    while (gAppStatus == RUNNING) {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}