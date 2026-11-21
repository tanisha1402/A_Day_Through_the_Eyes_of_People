// Config.h - project-wide constants (no magic numbers elsewhere).
#pragma once
namespace Config {
    const int   WINDOW_W  = 1280;     // logical + initial window width
    const int   WINDOW_H  = 720;      // logical + initial window height
    const int   TIMER_MS  = 16;       // requested timer period (~60 fps)
    const float LETTERBOX = 50.0f;    // cinematic bar height (logical pixels)
    const float MAX_DT    = 0.05f;    // longest frame time we accept (seconds)
    const float SPEED_MIN = 0.25f;
    const float SPEED_MAX = 4.0f;
    const float SPEED_STEP = 1.25f;   // multiplier per Left/Right key press
    const float PI        = 3.14159265f;
}
