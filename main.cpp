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
#include <memory>
using namespace std;

const char *const TITLE = "lamp";

constexpr int SCREEN_WIDTH = 1280,
              SCREEN_HEIGHT = 720,
              FPS = 12;

enum AppStatus { TERMINATED, RUNNING };

// the textures were originally drawn for 4k 16:9 resolution (3840:2160)
constexpr float DEFAULT_TEXTURE_FACTOR = static_cast<float>(SCREEN_WIDTH) / 3840;
constexpr Vector2 DEFAULT_TEXTURE_SCALE{ DEFAULT_TEXTURE_FACTOR, DEFAULT_TEXTURE_FACTOR };

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
        Vector2 scale = DEFAULT_TEXTURE_SCALE,
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

void drawToScreen(DrawableObject *obj) {
    static constexpr Vector2 DRAW_ORIGIN{ 0, 0 };
    float textureWidth = static_cast<float>(obj->texture.width) / obj->totalAnimationFrames;
    float textureHeight = static_cast<float>(obj->texture.height);

    DrawTexturePro(
        obj->texture,
        Rectangle{ textureWidth * obj->animationFrame, 0.0f, textureWidth, textureHeight },
        Rectangle{
            obj->screenCoords.x,
            obj->screenCoords.y,
            textureWidth * obj->scale.x,
            textureHeight * obj->scale.y
        },
        DRAW_ORIGIN,
        obj->rotation,
        WHITE
    );
}

AppStatus gAppStatus = RUNNING;

/* textures */
Texture2D lampTexture;
Texture2D postTexture;
Texture2D weedTexture;
Texture2D shadowTexture;

/* on-screen objects */
DrawableObject *lamppost, *lantern, *weed, *shadow;

void init() {
    const int LAMP_OFFSET_X = 300;
    const int LAMP_OFFSET_Y = 160;
    const int WEED_OFFSET = 650;
    const int SHADOW_OFFSET = 150;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, TITLE);
    SetTargetFPS(FPS);

    // load textures
    lampTexture = LoadTexture("./assets/lamp.png");
    postTexture = LoadTexture("./assets/post.png");
    weedTexture = LoadTexture("./assets/tumbleweed.png");
    shadowTexture = LoadTexture("./assets/shadow.png");

    // create the objects
    lamppost = new DrawableObject(
        postTexture,
        Vector2{
            SCREEN_WIDTH / 2.0f - lampTexture.width / 4.0f,
            SCREEN_HEIGHT / 2.0f - lampTexture.height
        },
        2
    );

    lantern = new DrawableObject(
        lampTexture,
        Vector2{
            SCREEN_WIDTH / 2.0f - lampTexture.width / 4.0f + LAMP_OFFSET_X * DEFAULT_TEXTURE_FACTOR,
            SCREEN_HEIGHT / 2.0f - lampTexture.height + LAMP_OFFSET_Y * DEFAULT_TEXTURE_FACTOR
        },
        2
    );

    weed = new DrawableObject(
        weedTexture,
        Vector2{
            0.0f,
            SCREEN_HEIGHT / 2.0f + WEED_OFFSET * DEFAULT_TEXTURE_FACTOR
        },
        2
    );

    shadow = new DrawableObject(
        shadowTexture,
        Vector2{
            weed->screenCoords.x,
            weed->screenCoords.y + SHADOW_OFFSET * DEFAULT_TEXTURE_FACTOR
        },
        1
    );

    // compress shadow on y axis to give perspective
    shadow->scale.y = 0.1;
}

void processInput() {
    if (WindowShouldClose()) gAppStatus = TERMINATED;
}

int frameCt = 0;
void update() {
    if (++frameCt % 12 == 0) {
        lamppost->animationFrame++;
        lamppost->animationFrame %= 2;

        lantern->animationFrame = ++lantern->animationFrame % 2;
    }

}

void render() {
    BeginDrawing();

    ClearBackground(WHITE);
    
    drawToScreen(lamppost);
    drawToScreen(lantern);
    drawToScreen(weed);
    drawToScreen(shadow);
    
    EndDrawing();
}

void shutdown() {
    CloseWindow();

    // unloading textures
    UnloadTexture(lampTexture);
    UnloadTexture(postTexture);
    UnloadTexture(weedTexture);
    UnloadTexture(shadowTexture);

    // destroying objects

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