// TimeOfDay.h - sky, light and window schedules for any hour of the story day.
// Hours run 4.5 (4:30 AM) .. 28.5 (4:30 AM next day) so the loop blends smoothly.
#pragma once
#include "Utils.h"

struct SkyState {
    Color top, bottom;     // sky gradient
    Color light;           // ambient light colour multiplied onto scenery
    float darkness;        // 0 = bright day, 1 = deep night
    float starAlpha;       // star visibility
    float sunX, sunY, moonX, moonY;   // screen-space positions
    bool  sunUp, moonUp;
    float hour;
};

extern SkyState gSky;                         // current sky, set once per frame by the scene manager

SkyState skyAtHour(float hour);               // blend between the keyframes
Color    applyLight(Color c, const SkyState& s);
inline Color tint(Color c) { return applyLight(c, gSky); }   // "lit by the current sky"
bool     windowLit(int id, float hour);       // each window id has its own schedule
