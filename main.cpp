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
/**
* TODO:
* - fix magic numbers
* - get rid of destructuring (not in c++11)
*/
#include "raylib.h"
#include <algorithm>
#include <cstdlib>
#include <vector>
using namespace std;

const char *const TITLE = "lamp";

constexpr int SCREEN_WIDTH = 1280,
              SCREEN_HEIGHT = 720,
              FPS = 60;

enum AppStatus { TERMINATED, RUNNING };
enum Weather { CLEAR, WINDY };

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
    Vector2 origin;

    DrawableObject(
        const Texture2D &texture,
        Vector2 screenCoords,
        int totalAnimationFrames,
        int animationFrame = 0,
        Vector2 scale = DEFAULT_TEXTURE_SCALE,
        float rotation = 0.0f,
        Vector2 origin = { 0.0f, 0.0f }
    ):
        texture(texture),
        screenCoords(screenCoords),
        totalAnimationFrames(totalAnimationFrames),
        animationFrame(animationFrame),
        scale(scale),
        rotation(rotation),
        origin(origin)
    {}
};

void drawToScreen(DrawableObject *obj) {
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
        obj->origin,
        obj->rotation,
        WHITE
    );
}

/**
* @brief clamps the value between minimum and maximum
* @return a value in range [minimum, maximum]
*/
float clamp(float original, float minimum, float maximum) {
    return min(maximum, max(original, minimum));
}

/**
* @brief linear interpolation between a and b
* @param t  a value between 0.0 and 1.0, representing the percentage of the way to b
* @param a  the starting value
* @param b  the ending value
* @return the interpolated value
*/
float lerp(float t, float a, float b) {
    return t * (b - a) + a;
}

AppStatus gAppStatus = RUNNING;
Weather currWeather = CLEAR;
Color bgColor = WHITE;

/* textures */
Texture2D lampTexture;
Texture2D postTexture;
Texture2D weedTexture;
Texture2D shadowTexture;
Texture2D particleTexture;

/* on-screen objects */
DrawableObject *lamppost, *lantern, *weed, *shadow;

// particles: a particle ptr paired with its velocity vector
const int PARTICLE_CT = 200; 
vector<pair<DrawableObject*, Vector2>> particles(PARTICLE_CT);

/* physics variables */
double lastTickTime;
double lastWindTime;
double lastBoilTime;

const float LANTERN_ROTATION_LIMIT = 50.0f;
const float LANTERN_GRAVITY = 200.0f;
const float LANTERN_FRICTION = 0.30f; // what percent of velocity the lantern loses per second
const float LANTERN_MAXVEL = 50.0f;
float lanternAngularVelocity = 60.0f; // how many degrees the lantern moves per second

float windStrength = 200.0f;

const float weedBaseline = SCREEN_HEIGHT / 2.0f + 650 * DEFAULT_TEXTURE_FACTOR;
float weedSpeed = (SCREEN_WIDTH + 200) / 8.0f; // how many pixels weed should move in a second
float weedYv = 100;


float randf(float min, float max) {
    float rand01 = (float)rand() / RAND_MAX;
    return rand01 * (max - min) + min;
}

void init() {
    const int LAMP_OFFSET_X = 400;
    const int LAMP_OFFSET_Y = 150;
    const int WEED_OFFSET = 650;
    const int SHADOW_OFFSET = 150;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, TITLE);
    SetTargetFPS(FPS);

    // load textures
    lampTexture = LoadTexture("./assets/lamp.png");
    postTexture = LoadTexture("./assets/post.png");
    weedTexture = LoadTexture("./assets/tumbleweed.png");
    shadowTexture = LoadTexture("./assets/shadow.png");
    particleTexture = LoadTexture("./assets/particle.png");

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

    lantern->origin = { lampTexture.width / 4.0f * DEFAULT_TEXTURE_FACTOR, 0.0f };

    weed = new DrawableObject(
        weedTexture,
        Vector2{
            SCREEN_WIDTH + 100.0f,
            SCREEN_HEIGHT / 2.0f + WEED_OFFSET * DEFAULT_TEXTURE_FACTOR
        },
        2
    );

    weed->origin = {
        weedTexture.width / 4.0f * DEFAULT_TEXTURE_FACTOR,
        weedTexture.height / 2.0f * DEFAULT_TEXTURE_FACTOR
    };

    shadow = new DrawableObject(
        shadowTexture,
        Vector2{
            weed->screenCoords.x,
            weed->screenCoords.y + SHADOW_OFFSET * DEFAULT_TEXTURE_FACTOR
        },
        1
    );

    shadow->origin = {
        shadowTexture.width / 2.0f * DEFAULT_TEXTURE_FACTOR,
        shadowTexture.height / 2.0f * DEFAULT_TEXTURE_FACTOR
    };

    // compress shadow on y axis to give perspective
    shadow->scale.y = 0.1;

    // populate particles
    for (size_t i = 0; i < particles.size(); i++) {
        float scale = DEFAULT_TEXTURE_FACTOR * 4.0f * randf(0.5f, 1.0f);
        particles[i] = {
            new DrawableObject(
                particleTexture,
                Vector2{
                    randf(0.0f, SCREEN_WIDTH),
                    randf(0.0f, SCREEN_HEIGHT)
                },
                1,
                0,
                Vector2{ scale, scale }
            ),
            Vector2{
                randf(-20, 20),
                randf(-20, 20)
            }
        };
    }

    lastTickTime = lastWindTime = lastBoilTime = GetTime();
}

void processInput() {
    if (WindowShouldClose()) gAppStatus = TERMINATED;
}

void startWind() {
    currWeather = WINDY;
    bgColor = GRAY;

    // move the tumbleweed at the left side of the screen
    weed->screenCoords.x = -100;
}

void endWind() {
    currWeather = CLEAR;
    bgColor = WHITE;
}

void update() {
    double currTime = GetTime();
    float deltaTime = static_cast<float>(currTime - lastTickTime);
    lastTickTime = currTime;

    /* wind */

    // wind happens every 5 seconds
    // start wind if it's clear and it's been 8 seconds since last wind ended
    if (currWeather == CLEAR && currTime - lastWindTime > 8.0) {
        lastWindTime = currTime;
        startWind();
    }

    // end wind if it's windy and it's been 8 seconds since the wind started
    if (currWeather == WINDY && currTime - lastWindTime > 8.0) {
        lastWindTime = currTime;
        endWind();
    }


    /* lantern */

    // if it's windy, accelerate rightward in direction of wind
    if (currWeather == WINDY) {
        lanternAngularVelocity -= windStrength * deltaTime;

        // limit lantern velocity
        lanternAngularVelocity = clamp(lanternAngularVelocity, -LANTERN_MAXVEL, LANTERN_MAXVEL);
    }

    // apply rotational velocity
    lantern->rotation += lanternAngularVelocity * deltaTime;
    lantern->rotation = clamp(lantern->rotation, -LANTERN_ROTATION_LIMIT, LANTERN_ROTATION_LIMIT);

    // apply acceleration toward neutral position
    // lantern will experience the maximum amount of acceleration at -90 and 90 degrees
    lanternAngularVelocity += lerp((lantern->rotation + 90.0f) / 180.0f, LANTERN_GRAVITY, -LANTERN_GRAVITY) * deltaTime;

    // apply friction to the lantern's velocity
    lanternAngularVelocity -= lanternAngularVelocity * LANTERN_FRICTION * deltaTime;


    /* tumbleweed */
    weed->rotation += 100 * deltaTime;
    weed->screenCoords.x += weedSpeed * deltaTime;
    weedYv += 100 * deltaTime;
    weed->screenCoords.y += weedYv * deltaTime;

    if (weed->screenCoords.y > weedBaseline) {
        weedYv = -weedYv;
        float diff = weed->screenCoords.y - weedBaseline;
        weed->screenCoords.y = weedBaseline - diff;
    }


    /* shadow */

    // shadow x always follows weed
    shadow->screenCoords.x = weed->screenCoords.x;

    // shadow size scales with height of weed
    shadow->scale.x = lerp(
        (weed->screenCoords.y - weedBaseline) / (450.0f - weedBaseline),
        DEFAULT_TEXTURE_FACTOR * 0.95f,
        DEFAULT_TEXTURE_FACTOR * 1.4f
    );


    /* boil animation */
    if (currTime - lastBoilTime >= 0.5) {
        lastBoilTime = currTime;

        // advance everything's animation frame by 1
        lamppost->animationFrame = (lamppost->animationFrame + 1) % lamppost->totalAnimationFrames;
        lantern->animationFrame = (lantern->animationFrame + 1) % lantern->totalAnimationFrames;
        weed->animationFrame = (weed->animationFrame + 1) % weed->totalAnimationFrames;
    }

    /* particles */
    for (auto [ particle, particleVel ] : particles) {
        particle->screenCoords.x += particleVel.x * deltaTime;
        particle->screenCoords.y += particleVel.y * deltaTime;

        // apply wind
        if (currWeather == WINDY) {
            particle->screenCoords.x += windStrength * deltaTime;
        }

        // if off screen, wrap to other side
        const int OFFSCREEN_BUFFER = 100;
        if (particle->screenCoords.x > SCREEN_WIDTH + OFFSCREEN_BUFFER) {
            particle->screenCoords.x = -OFFSCREEN_BUFFER;
        } else if (particle->screenCoords.x < -OFFSCREEN_BUFFER) {
            particle->screenCoords.x = SCREEN_WIDTH + OFFSCREEN_BUFFER;
        } else if (particle->screenCoords.y > SCREEN_HEIGHT + OFFSCREEN_BUFFER) {
            particle->screenCoords.y = -OFFSCREEN_BUFFER;
        } else if (particle->screenCoords.y < -OFFSCREEN_BUFFER) {
            particle->screenCoords.y = SCREEN_HEIGHT + OFFSCREEN_BUFFER;
        }
    }
}

void render() {
    BeginDrawing();

    ClearBackground(bgColor);
    
    drawToScreen(lamppost);
    drawToScreen(lantern);
    drawToScreen(weed);
    drawToScreen(shadow);
    for (auto [ particle, particleVel ] : particles) {
        drawToScreen(particle);
    }
    
    EndDrawing();
}

void shutdown() {
    // destroying objects
    delete lamppost;
    delete lantern;
    delete weed;
    delete shadow;
    for (size_t i = 0; i < particles.size(); i++) {
        delete particles[i].first;
    }

    // unloading textures
    UnloadTexture(lampTexture);
    UnloadTexture(postTexture);
    UnloadTexture(weedTexture);
    UnloadTexture(shadowTexture);
    UnloadTexture(particleTexture);

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