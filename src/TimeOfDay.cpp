#include "TimeOfDay.h"

SkyState gSky;
float    gMoonScale = 1.0f;

// Keyframes at the hour of each chapter (plus 28.5 = 4:30 AM again for the loop).
static const int N = 11;
static const float kHour[N]    = { 4.5f, 6.0f, 7.5f, 9.0f, 12.0f, 15.0f, 16.5f, 18.0f, 20.0f, 23.5f, 28.5f };
static const float kDark[N]    = { 0.85f, 0.35f, 0.10f, 0.0f, 0.0f, 0.15f, 0.05f, 0.35f, 0.75f, 1.0f, 0.85f };
static const Color kTop[N]     = { rgb(0.06f,0.08f,0.22f), rgb(0.35f,0.38f,0.68f), rgb(0.38f,0.62f,0.95f), rgb(0.30f,0.60f,0.98f),
                                   rgb(0.22f,0.55f,0.98f), rgb(0.40f,0.45f,0.55f), rgb(0.35f,0.62f,0.95f), rgb(0.30f,0.28f,0.55f),
                                   rgb(0.10f,0.10f,0.30f), rgb(0.02f,0.03f,0.12f), rgb(0.06f,0.08f,0.22f) };
static const Color kBottom[N]  = { rgb(0.30f,0.28f,0.50f), rgb(1.00f,0.66f,0.42f), rgb(0.98f,0.85f,0.72f), rgb(0.78f,0.90f,1.00f),
                                   rgb(0.72f,0.90f,1.00f), rgb(0.62f,0.66f,0.72f), rgb(0.80f,0.92f,1.00f), rgb(1.00f,0.55f,0.25f),
                                   rgb(0.45f,0.28f,0.45f), rgb(0.10f,0.12f,0.30f), rgb(0.30f,0.28f,0.50f) };
static const Color kLight[N]   = { rgb(0.42f,0.48f,0.75f), rgb(1.00f,0.82f,0.72f), rgb(1.00f,0.95f,0.88f), rgb(1.00f,1.00f,0.97f),
                                   rgb(1.00f,1.00f,1.00f), rgb(0.80f,0.84f,0.90f), rgb(1.00f,0.97f,0.90f), rgb(1.00f,0.72f,0.52f),
                                   rgb(0.50f,0.50f,0.78f), rgb(0.28f,0.34f,0.62f), rgb(0.42f,0.48f,0.75f) };

static void arc(float a, float x0, float x1, float horizonY, float height, float& x, float& y) {
    float ang = a * Config::PI;
    x = lerp(x0, x1, (1.0f - std::cos(ang)) * 0.5f);
    y = horizonY + height * std::sin(ang);
}

SkyState skyAtHour(float hour) {
    if (hour < kHour[0]) hour += 24.0f;
    if (hour > kHour[N - 1]) hour = kHour[N - 1];
    int i = 0;
    while (i < N - 2 && hour > kHour[i + 1]) ++i;
    float t = smoothstep01((hour - kHour[i]) / (kHour[i + 1] - kHour[i]));   // eased blend

    SkyState s;
    s.hour = hour;
    s.top = lerpColor(kTop[i], kTop[i + 1], t);
    s.bottom = lerpColor(kBottom[i], kBottom[i + 1], t);
    s.light = lerpColor(kLight[i], kLight[i + 1], t);
    s.darkness = lerp(kDark[i], kDark[i + 1], t);
    s.starAlpha = clamp01((s.darkness - 0.35f) / 0.5f);

    // Sun crosses the sky from 5:36 to 18:24, the moon from 18:24 to 5:36.
    float sa = (hour - 5.6f) / 12.8f;
    s.sunUp = (sa >= 0.0f && sa <= 1.0f);
    arc(clamp01(sa), 170, 1110, 300, 300, s.sunX, s.sunY);
    float mh = hour < 5.6f ? hour + 24.0f : hour;
    float ma = (mh - 18.4f) / 11.2f;
    s.moonUp = (ma >= 0.0f && ma <= 1.0f);
    arc(clamp01(ma), 170, 1110, 300, 300, s.moonX, s.moonY);
    return s;
}

Color applyLight(Color c, const SkyState& s) {
    return rgba(c.r * s.light.r, c.g * s.light.g, c.b * s.light.b, c.a);
}

bool windowLit(int id, float hour) {
    // A few households wake early; in the evening each window switches on and off at its own time.
    bool early   = (id % 6 == 0) && hour < 5.2f + (id % 3) * 0.5f;
    bool evening = hour > 17.3f + (id % 5) * 0.5f && hour < 25.5f + (id % 4) * 1.2f;
    return early || evening;
}
