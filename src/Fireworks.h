// Fireworks.h - rockets that rise and burst into fading sparks (uses the shared Particle pool).
// Coordinates are screen-space (logical 1280x720). Works from the scheduled show in Scene 9
// and will also serve the right-click "launch a firework" control (Phase 6).
#pragma once
#include "Utils.h"

void  fireworksReset();
void  fireworksLaunch(float x, float burstY, Color c);   // returns silently if every rocket slot is busy
void  fireworksUpdate(float dt);
void  fireworksDraw();                                    // additive sparks + glow
float fireworksFlash();                                   // 0..1: how much the sky is lit right now
int   fireworksAlive();                                   // live rockets + sparks (for tests)
