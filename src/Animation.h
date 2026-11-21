// Animation.h - the ONE central animation clock.
// main's single glutTimerFunc calls clockTick(); everything animated reads
// gClock.time (total animation seconds) or gClock.dt (seconds since last tick).
// Both already include the speed multiplier and are frozen while paused.
#pragma once

struct AnimClock {
    float time       = 0.0f;   // animation seconds (speed-scaled, stops when paused)
    float dt         = 0.0f;   // speed-scaled seconds since the previous tick
    float speed      = 1.0f;
    bool  paused     = false;
    float fps        = 0.0f;   // measured frames per second (for the overlay)
    int   lastMs     = 0;
    int   frames     = 0;
    int   fpsStartMs = 0;
};

extern AnimClock gClock;

void clockStart(int nowMs);   // call once, before the first tick
void clockTick(int nowMs);    // call from the timer; nowMs = glutGet(GLUT_ELAPSED_TIME)
void clockTogglePause();
void clockSpeedUp();
void clockSlowDown();
