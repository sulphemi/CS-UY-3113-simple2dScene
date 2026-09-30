#include "raylib.h"

enum AppStatus { TERMINATED, RUNNING };

const char *const TITLE = "lamp";

constexpr int SCREEN_WIDTH = 1280,
              SCREEN_HEIGHT = 720,
              FPS = 12;

AppStatus gAppStatus = RUNNING;

void init() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, TITLE);
    SetTargetFPS(FPS);
}

void processInput() {
    if (WindowShouldClose()) gAppStatus = TERMINATED;
}

void update() {

}

void render() {
    BeginDrawing();

    EndDrawing();
}

void shutdown() {
    CloseWindow();
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