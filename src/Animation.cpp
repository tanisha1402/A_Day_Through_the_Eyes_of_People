#include "Animation.h"
#include "Config.h"

AnimClock gClock;

void clockStart(int nowMs) {
    gClock = AnimClock();
    gClock.lastMs = nowMs;
    gClock.fpsStartMs = nowMs;
}

void clockTick(int nowMs) {
    // Real elapsed time, clamped so a stall (window drag, breakpoint) never
    // makes the animation jump.
    float real = (nowMs - gClock.lastMs) / 1000.0f;
    gClock.lastMs = nowMs;
    if (real < 0.0f) real = 0.0f;
    if (real > Config::MAX_DT) real = Config::MAX_DT;

    gClock.dt = gClock.paused ? 0.0f : real * gClock.speed;
    gClock.time += gClock.dt;

    // FPS counter, refreshed twice a second
    ++gClock.frames;
    int span = nowMs - gClock.fpsStartMs;
    if (span >= 500) {
        gClock.fps = gClock.frames * 1000.0f / span;
        gClock.frames = 0;
        gClock.fpsStartMs = nowMs;
    }
}

void clockTogglePause() { gClock.paused = !gClock.paused; }

void clockSpeedUp() {
    gClock.speed *= Config::SPEED_STEP;
    if (gClock.speed > Config::SPEED_MAX) gClock.speed = Config::SPEED_MAX;
}

void clockSlowDown() {
    gClock.speed /= Config::SPEED_STEP;
    if (gClock.speed < Config::SPEED_MIN) gClock.speed = Config::SPEED_MIN;
}
