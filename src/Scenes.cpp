#include "Scenes.h"
#include "Animation.h"
#include "Camera.h"
#include "Characters.h"
#include "Environment.h"
#include "TimeOfDay.h"
#include "Utils.h"
#include "World.h"
#include <GL/freeglut.h>
#include <cmath>
#include <cstdio>

// ------------------------------------------------------------- chapters ----
struct SceneInfo { const char* clock; const char* title; float hourStart, hourEnd; };
static const SceneInfo kScenes[10] = {
    { "4:30 AM",  "Not Everyone's Day Starts at the Same Hour",  4.5f,  6.0f },
    { "6:00 AM",  "Same Sun",                                     6.0f,  7.5f },
    { "7:30 AM",  "Same Crossing",                                7.5f,  9.0f },
    { "9:00 AM",  "Same Clock",                                   9.0f, 12.0f },
    { "12:00 PM", "Same Cloud",                                  12.0f, 15.0f },
    { "3:00 PM",  "Same Rain",                                   15.0f, 16.5f },
    { "4:30 PM",  "After the Rain",                              16.5f, 18.0f },
    { "6:00 PM",  "Going Home",                                  18.0f, 20.0f },
    { "8:00 PM",  "The Festival",                                20.0f, 23.5f },
    { "11:30 PM", "Same Moon",                                   23.5f, 28.5f },
};
static const float SCENE_LENGTH   = 15.0f;   // seconds per chapter
static const int   ACTIVE_SCENES  = 3;       // chapters implemented so far; the loop wraps here
static const float FADE_TIME      = 0.9f;

static int   gIndex = 0;
static float gTime  = 0.0f;

int   sceneIndex() { return gIndex; }
float sceneTime()  { return gTime; }
void  scenesInit() { gIndex = 0; gTime = 0.0f; }
void  scenesGoto(int index, float time) { gIndex = index % ACTIVE_SCENES; gTime = time; }
void  scenesNext() { gIndex = (gIndex + 1) % ACTIVE_SCENES; gTime = 0.0f; }
void  scenesPrev() { gIndex = (gIndex + ACTIVE_SCENES - 1) % ACTIVE_SCENES; gTime = 0.0f; }
void  scenesUpdate(float dt) {
    gTime += dt;
    if (gTime >= SCENE_LENGTH) { gTime -= SCENE_LENGTH; gIndex = (gIndex + 1) % ACTIVE_SCENES; }
}

// ------------------------------------------------------ character tracks ----
// A character's walk is a list of (time, x) waypoints. Position, facing and
// walk cycle are pure functions of the chapter time, so jumping to any moment
// (N/P keys) or replaying is always consistent.
struct Waypoint { float t, x; };
struct Track { float x, facing, walk, dist; };

static Track trackAt(float t, const Waypoint* w, int n, float facing0) {
    Track r; r.x = w[0].x; r.facing = facing0; r.walk = 0; r.dist = 0;
    for (int i = 0; i + 1 < n; ++i) {
        float dx = w[i + 1].x - w[i].x;
        float dir = dx > 0 ? 1.0f : (dx < 0 ? -1.0f : 0.0f);
        if (t >= w[i + 1].t) { r.dist += std::fabs(dx); r.x = w[i + 1].x; if (dir != 0) r.facing = dir; }
        else if (t > w[i].t) {
            float u = (t - w[i].t) / (w[i + 1].t - w[i].t);
            r.x = lerp(w[i].x, w[i + 1].x, u); r.dist += std::fabs(dx) * u;
            if (dir != 0) {
                r.facing = dir;
                float ramp = std::fmin((t - w[i].t) / 0.3f, (w[i + 1].t - t) / 0.3f);   // ease into / out of the stride
                r.walk = clamp01(ramp);
            }
            return r;
        } else break;
    }
    return r;
}

static void drawActor(const PersonStyle& st, float scale, const Track& tr, float t, float y, float armUp) {
    PersonPose p = makePose(tr.facing, t);
    p.walkPhase = tr.dist * 0.115f;          // radians per unit walked: feet keep up with the ground
    p.walkAmount = tr.walk;
    p.armUp = armUp;
    drawPerson(tr.x, y, scale, st, p);
}

// ---------------------------------------------------------------- scene 1 ----
// 4:30 AM - the town is asleep; Ren is already out on the river with his lantern.
static void scene1(float t, TownState& ts) {
    gCamera = cameraBlend(makeCamera(1850, 320, 0.95f), makeCamera(2750, 300, 0.95f), t / SCENE_LENGTH);
    ts.mist = 0.75f - 0.2f * t / SCENE_LENGTH;
    ts.lantern = true;
}
static void scene1Actors(float t, const TownState& ts) {
    static const PersonStyle ren = makeRen();
    PersonPose p = makePose(1, t);
    drawPerson(ts.boatX - 18, boatDeckY() + boatBobY(ts.time), 1.15f, ren, p);       // Ren stands in the boat
    drawBoatFront(ts);
    static const Waypoint catPath[] = { {0, 2420}, {11, 2640}, {15, 2640} };
    Track c = trackAt(t, catPath, 3, 1);
    float cy = lerp(walkY(c.x) + 2, 140.0f, smoothstep01((c.x - 2380.0f) / 160.0f));   // bank -> sidewalk
    drawCat(c.x, cy, 0.85f, c.facing, c.dist * 0.22f, c.walk, t, c.walk < 0.05f);
}
static void scene1SkyFx(float t) {                      // a shooting star across the fading night sky
    float p = (t - 6.0f) / 1.3f;
    if (p > 0 && p < 1) drawShootingStar(260 + p * 520, 600 - p * 230, 86, -34, std::sin(p * Config::PI));
}

// ---------------------------------------------------------------- scene 2 ----
// 6:00 AM - the same sun falls on Aoi's huge empty room, then on Ren's tiny hut.
static void scene2(float t, TownState& ts) {
    Camera a = makeCamera(640, 500, 1.1f), b = makeCamera(1790, 240, 1.5f);
    gCamera = cameraBlend(a, b, clamp01((t - 3.5f) / 7.5f));
    ts.sunlight = clamp01(t / 2.5f) * (1.0f - 0.7f * smoothstep01((t - 6.0f) / 4.0f));
    ts.mist = 0.4f * (1.0f - t / SCENE_LENGTH);
    ts.hutCups = 1;
}
static void scene2Actors(float t, const TownState& ts) {
    static const PersonStyle aoi = makeAoi(), ren = makeRen();
    float by = groundY(World::MANSION_X) - 4;
    drawPerson(World::MANSION_X + 78, by + 54, 0.46f, aoi, makePose(-1, t));                // tiny Aoi in a vast room
    // Ren fishes from the dock
    PersonPose rp = makePose(1, t);
    float rx = 2025, ry = World::WATER_TOP + 12;
    drawPerson(rx, ry, 1.0f, ren, rp);
    float sway = std::sin(t * 1.3f) * 3;
    drawLine(rx + 17, ry + 50, rx + 92, ry + 98 + sway, rgb(0.25f, 0.17f, 0.1f), 2.0f);           // rod
    drawLine(rx + 92, ry + 98 + sway, rx + 120, World::WATER_TOP - 14 + sway * 0.3f, rgba(1, 1, 1, 0.8f), 1.0f);
    drawCircle(rx + 120, World::WATER_TOP - 14 + sway * 0.3f, 3.5f, rgb(0.95f, 0.25f, 0.2f));
    drawCat(1862, walkY(1862) + 2, 0.8f, 1, 0, 0, t, true);                                  // the cat watches Ren's hut
    (void)ts;
}

// ---------------------------------------------------------------- scene 3 ----
// 7:30 AM - everyone waits at the same crossing while the train passes.
static const float CROSS_Y = 140.0f;      // feet line on the sidewalk
static void scene3(float t, TownState& ts) {
    gCamera = cameraBlend(makeCamera(3300, 305, 1.12f), makeCamera(3300, 297, 1.2f), t / SCENE_LENGTH);
    ts.arms = (t < 9.6f) ? smoothstep01((t - 3.4f) / 1.0f) : 1.0f - smoothstep01((t - 9.6f) / 1.0f);
    ts.signalOn = (t > 3.4f && t < 10.8f);
    ts.trainOn = (t > 4.4f);
    ts.trainHeadX = 2400.0f + 560.0f * (t - 4.5f);
}
static void scene3Actors(float t, const TownState& ts) {
    static const PersonStyle hana = makeHana(), kenji = makeKenji(), neighbor = makeNeighbor();
    static const Waypoint hanaPath[] = { {0, 2950}, {3.4f, 3150}, {10.8f, 3150}, {15, 3450} };
    static const Waypoint kenjiPath[] = { {0, 3830}, {2.2f, 3440}, {10.8f, 3440}, {14, 3010} };
    static const Waypoint nbPath[] = { {0, 3700}, {2.8f, 3515}, {11.2f, 3515}, {15, 3345} };
    float watch = 0;                                         // Kenji keeps checking his watch
    if ((t > 3.0f && t < 5.2f) || (t > 7.2f && t < 8.6f)) watch = smoothstep01(std::fmin((t < 6 ? t - 3.0f : t - 7.2f) / 0.4f, 1.0f));
    Track h = trackAt(t, hanaPath, 4, 1), k = trackAt(t, kenjiPath, 4, -1), n = trackAt(t, nbPath, 4, -1);
    // the cat: sits on the rail, hops down when the lights start to blink, then sits by Hana
    static const Waypoint catPath[] = { {0, 3300}, {3.7f, 3300}, {4.8f, 3185}, {15, 3185} };
    Track c = trackAt(t, catPath, 4, 1);
    float cy = lerp(World::TRACK_Y + 3, CROSS_Y, smoothstep01((t - 3.7f) / 1.1f));
    drawCat(c.x, cy, 0.85f, c.walk > 0.05f ? c.facing : 1.0f, c.dist * 0.22f, c.walk, t, c.walk < 0.05f);
    drawActor(hana, 1.0f, h, t, CROSS_Y, 0);
    drawActor(neighbor, 1.0f, n, t, CROSS_Y - 6, 0);
    drawActor(kenji, 1.0f, k, t, CROSS_Y + 2, watch);
    (void)ts;
}

// ------------------------------------------------------------ overlays ----
static void drawCaption(float t) {
    float a = smoothstep01(t / 0.7f) * (1.0f - smoothstep01((t - 4.2f) / 1.0f));
    if (t < 0.5f) a = 0;
    if (a <= 0.01f) return;
    drawGradientRectH(0, 50, 760, 150, rgba(0, 0, 0, 0.5f * a), rgba(0, 0, 0, 0));
    drawText(60, 118, kScenes[gIndex].clock, rgba(1.0f, 0.82f, 0.5f, a), true);
    drawText(60, 90, kScenes[gIndex].title, rgba(1, 1, 1, a), true);
}

static void drawHud() {
    char s[96];
    int h24 = (int)std::fmod(gSky.hour, 24.0f), m = (int)((gSky.hour - std::floor(gSky.hour)) * 60.0f);
    std::snprintf(s, sizeof s, "%02d:%02d", h24, m);
    drawText(1170, 692, s, rgb(1, 1, 1), true);                                           // clock in the corner
    drawText(24, 692, "A Day Through the Eyes of People", rgba(1, 1, 1, 0.8f));
    std::snprintf(s, sizeof s, "Chapter %d/10   FPS %.0f   speed %.2fx   %s", gIndex + 1, gClock.fps, gClock.speed,
                  gClock.paused ? "[PAUSED]" : "");
    drawText(24, 20, s, rgba(1, 1, 1, 0.75f));
    drawText(820, 20, "Space: pause   N/P: chapter   Left/Right: speed   Esc: exit", rgba(1, 1, 1, 0.6f));
}

// ------------------------------------------------------------------ draw ----
void scenesDraw() {
    const SceneInfo& info = kScenes[gIndex];
    float hour = lerp(info.hourStart, info.hourEnd, gTime / SCENE_LENGTH);   // story hour glides through the chapter
    gSky = skyAtHour(hour);

    TownState ts; ts.time = gClock.time;
    switch (gIndex) {
        case 0: scene1(gTime, ts); break;
        case 1: scene2(gTime, ts); break;
        default: scene3(gTime, ts); break;
    }

    drawSky(gClock.time);
    if (gIndex == 0) scene1SkyFx(gTime);
    drawFarLayers(gClock.time);

    cameraBegin(1.0f);
    drawTownBack(ts);
    switch (gIndex) {
        case 0: scene1Actors(gTime, ts); break;
        case 1: scene2Actors(gTime, ts); break;
        default: scene3Actors(gTime, ts); break;
    }
    drawTownFront(ts);
    cameraEnd();

    if (gIndex == 1) drawLightRays(gSky.sunX, gSky.sunY, gClock.time, 0.9f, rgb(1.0f, 0.86f, 0.58f));
    drawVignette(0.4f);

    // fade in at the start / out at the end of every chapter
    float fade = 1.0f - smoothstep01(gTime / FADE_TIME);
    fade = std::fmax(fade, smoothstep01((gTime - (SCENE_LENGTH - FADE_TIME)) / FADE_TIME));
    drawFade(rgb(0, 0, 0), fade);

    drawLetterbox();
    drawCaption(gTime);
    drawHud();
}
