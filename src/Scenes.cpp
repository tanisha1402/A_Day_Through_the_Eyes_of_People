#include "Scenes.h"
#include "Animation.h"
#include "Camera.h"
#include "Characters.h"
#include "Environment.h"
#include "Panels.h"
#include "Fireworks.h"
#include "Particles.h"
#include "Viewports.h"
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
    { "9:00 AM",  "Same Clock",                                   9.0f, 10.0f },   // clocks are on screen: one hour per chapter keeps their hands readable
    { "12:00 PM", "Same Cloud",                                  12.0f, 15.0f },
    { "3:00 PM",  "Same Rain",                                   15.0f, 16.5f },
    { "4:30 PM",  "After the Rain",                              16.5f, 18.0f },
    { "6:00 PM",  "Going Home",                                  18.0f, 20.0f },
    { "8:00 PM",  "The Festival",                                20.0f, 23.5f },
    { "11:30 PM", "Same Moon",                                   23.5f, 24.5f },   // deep night all chapter, so the moon stays high; the fade-out then loops to 4:30 AM
};
static const float SCENE_LENGTH   = 15.0f;   // seconds per chapter
static const int   ACTIVE_SCENES  = 10;      // chapters implemented so far; the loop wraps here
static const float FADE_TIME      = 0.9f;

static int   gIndex = 0;
static float gTime  = 0.0f;

float sceneHourAt(int index, float time) { return lerp(kScenes[index].hourStart, kScenes[index].hourEnd, time / SCENE_LENGTH); }
int   sceneIndex() { return gIndex; }
float sceneTime()  { return gTime; }
// The festival's fireworks show: launch times, screen positions and colours are scripted, so it is identical every loop.
struct ShowEntry { float t, x, y, r, g, b; };
static const ShowEntry kShow[] = {
    { 1.0f, 250, 560, 1.0f, 0.35f, 0.35f }, { 2.8f, 1140, 590, 0.4f, 0.7f, 1.0f }, { 4.4f, 480, 600, 1.0f, 0.85f, 0.3f },
    { 5.2f, 1190, 520, 0.7f, 1.0f, 0.5f },  { 7.0f, 330, 590, 0.9f, 0.5f, 1.0f },  { 8.6f, 1100, 600, 1.0f, 0.5f, 0.3f },
    { 9.8f, 200, 520, 0.5f, 0.9f, 1.0f },   { 10.6f, 1150, 570, 1.0f, 0.4f, 0.6f }, { 11.8f, 560, 600, 1.0f, 0.9f, 0.5f },
    { 12.6f, 1060, 545, 0.6f, 0.6f, 1.0f }, { 13.4f, 380, 560, 1.0f, 0.7f, 0.3f }
};
static void scheduleFireworks(float from, float to) {
    for (size_t i = 0; i < sizeof(kShow) / sizeof(kShow[0]); ++i)
        if (kShow[i].t > from && kShow[i].t <= to) fireworksLaunch(kShow[i].x, kShow[i].y, rgb(kShow[i].r, kShow[i].g, kShow[i].b));
}

static void enterScene() { panelsReset(gIndex); fireworksReset(); }       // fresh particles every time a chapter starts
void  scenesInit() { gIndex = 0; gTime = 0.0f; enterScene(); }
void  scenesGoto(int index, float time) { gIndex = index % ACTIVE_SCENES; gTime = time; enterScene(); }
void  scenesNext() { gIndex = (gIndex + 1) % ACTIVE_SCENES; gTime = 0.0f; enterScene(); }
void  scenesPrev() { gIndex = (gIndex + ACTIVE_SCENES - 1) % ACTIVE_SCENES; gTime = 0.0f; enterScene(); }
void  scenesUpdate(float dt) {
    float before = gTime;
    gTime += dt;
    panelsUpdate(dt, gIndex, gTime);
    if (gIndex == 8) scheduleFireworks(before, gTime);
    fireworksUpdate(dt);
    if (gTime >= SCENE_LENGTH) { gTime -= SCENE_LENGTH; gIndex = (gIndex + 1) % ACTIVE_SCENES; enterScene(); }
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
    p.walkPhase = tr.dist * 0.115f / scale;  // radians per unit walked: feet keep up with the ground (smaller people take shorter steps)
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


// ---------------------------------------------------------------- scene 7 ----
// 4:30 PM - After the Rain: the cat walks through the puddles up to the mansion, and Aoi opens her window.
static const Waypoint kCat7[] = { {0, 1960}, {12.2f, 1010}, {15, 1010} };
static void scene7(float t, TownState& ts) {
    Track c = trackAt(t, kCat7, 3, -1);
    Camera follow = makeCamera(c.x - 100, walkY(c.x) + 140, 1.1f);               // the camera follows the cat uphill...
    Camera room = makeCamera(World::MANSION_X + 10, groundY(World::MANSION_X) + 110, 1.5f);   // ...then settles on Aoi's room
    gCamera = cameraBlend(follow, room, clamp01((t - 9.0f) / 5.5f));
    ts.wet = 1.0f - 0.3f * t / SCENE_LENGTH;
    ts.windowOpen = smoothstep01((t - 11.6f) / 2.0f);
    ts.sunlight = 0.15f + 0.8f * ts.windowOpen;
}
static void scene7Actors(float t, const TownState& ts) {
    static const PersonStyle aoi = makeAoi();
    Track c = trackAt(t, kCat7, 3, -1);
    float cy = walkY(c.x) + 2;
    if (puddleDepth(c.x) > 0 && c.walk > 0.1f)                                   // splash rings while the cat is in a puddle
        for (int i = 0; i < 2; ++i) { float u = std::fmod(t * 2.2f + i * 0.5f, 1.0f); drawEllipse(c.x, cy - 2, 10 + u * 30, 3 + u * 8, rgba(1, 1, 1, 0.4f * (1 - u)), 20); }
    drawCat(c.x, cy, 0.9f, c.facing, c.dist * 0.22f, c.walk, t, c.walk < 0.05f);
    static const Waypoint aoiPath[] = { {0, World::MANSION_X + 78}, {5.5f, World::MANSION_X + 78}, {9.2f, World::MANSION_X + 26} };
    Track a = trackAt(t, aoiPath, 3, -1);
    float reach = smoothstep01((t - 10.0f) / 1.0f) * (1.0f - 0.5f * smoothstep01((t - 13.5f) / 1.0f));   // reaches up and opens the window
    drawActor(aoi, 0.46f, a, t, groundY(World::MANSION_X) - 4 + 54, reach);
    (void)ts;
}

// ---------------------------------------------------------------- scene 9 ----
// 8:00 PM - The Festival: Aoi slips out, Hana is there, and the two of them share the fireworks.
static PersonStyle crowdStyle(int k) {
    PersonStyle s = (k % 2) ? makeKenji() : makeNeighbor();
    static const Color tops[6] = { rgb(0.85f, 0.45f, 0.40f), rgb(0.35f, 0.55f, 0.75f), rgb(0.90f, 0.78f, 0.40f), rgb(0.50f, 0.70f, 0.50f), rgb(0.70f, 0.50f, 0.75f), rgb(0.88f, 0.60f, 0.35f) };
    s.prop = PROP_NONE; s.suit = false; s.hat = false; s.uniform = false; s.apron = false;
    s.top = tops[k % 6]; s.bottom = rgb(0.22f + 0.04f * (k % 3), 0.24f, 0.36f); s.skirt = (k % 3 != 1);
    s.hair = rgb(0.10f + 0.05f * (k % 4), 0.08f + 0.03f * (k % 3), 0.08f); s.height = 0.85f + 0.05f * (k % 4);
    return s;
}
static void scene9(float t, TownState& ts) {
    gCamera = cameraBlend(makeCamera(2800, 300, 0.95f), makeCamera(2810, 266, 1.22f), t / SCENE_LENGTH);
    ts.festival = true;
}
static void scene9Actors(float t, const TownState& ts) {
    // townspeople strolling between the stalls
    static const float cx[9] = { 2515, 2560, 2655, 2700, 2765, 2915, 2965, 3040, 3120 };
    for (int i = 0; i < 9; ++i) {
        PersonStyle st = crowdStyle(i);
        float x = cx[i], y = 112.0f + (i % 3) * 7.0f;
        PersonPose p = makePose((i % 2) ? -1.0f : 1.0f, t + i);
        if (i % 4 == 0) { float run = std::fmod(t * 14.0f + i * 90.0f, 240.0f) - 120.0f; x += run; p.walkAmount = 1; p.walkPhase = (t * 14.0f) * 0.115f / st.height + i; }
        drawPerson(x, y, st.height, st, p);
    }
    static const PersonStyle yukata = makeAoi();
    PersonStyle aoi = yukata; aoi.top = rgb(0.33f, 0.38f, 0.75f); aoi.bottom = rgb(0.30f, 0.34f, 0.70f); aoi.accent = rgb(0.95f, 0.7f, 0.75f);
    PersonStyle hana = makeHana(); hana.uniform = false; hana.prop = PROP_NONE; hana.top = rgb(0.95f, 0.55f, 0.62f); hana.bottom = rgb(0.92f, 0.48f, 0.58f);
    static const Waypoint aoiW[] = { {0, 2430}, {1.0f, 2430}, {8.0f, 2780} };
    static const Waypoint hanaW[] = { {0, 3260}, {1.5f, 3260}, {8.0f, 2852}, {10.0f, 2852}, {11.0f, 2826} };
    static const Waypoint catW[]  = { {0, 3300}, {9.0f, 3300}, {10.8f, 2872}, {15, 2872} };
    Track a = trackAt(t, aoiW, 3, 1), h = trackAt(t, hanaW, 5, -1), c = trackAt(t, catW, 4, -1);
    float wave = (t > 8.2f && t < 9.8f) ? 0.5f + 0.5f * std::sin((t - 8.2f) * 10.0f) : 0.0f;   // a shy little wave
    if (t > 10.0f) { h.facing = 1; a.facing = 1; }                                                 // both look up at the fireworks
    drawCat(c.x, 108, 0.95f, c.facing, c.dist * 0.22f, c.walk, t, c.walk < 0.05f);
    drawActor(aoi, 1.0f, a, t, 116, wave);
    drawActor(hana, 1.0f, h, t, 114, wave);
    (void)ts;
}

// --------------------------------------------------------------- scene 10 ----
// 11:30 PM - Same Moon: a pan across the lit windows, then the camera rises to the moon.
static Camera cam10(float t) {
    const Camera c1 = makeCamera(1780, 235, 1.45f), c2 = makeCamera(2330, 245, 1.3f), c3 = makeCamera(4600, 262, 1.15f), c4 = makeCamera(4560, 880, 0.85f);
    if (t < 3.2f) return c1;
    if (t < 5.0f) return cameraBlend(c1, c2, (t - 3.2f) / 1.8f);
    if (t < 8.2f) return c2;
    if (t < 9.8f) return cameraBlend(c2, c3, (t - 8.2f) / 1.6f);
    if (t < 11.6f) return c3;
    return cameraBlend(c3, c4, (t - 11.6f) / 3.4f);
}
static void scene10(float t, TownState& ts) {
    gCamera = cam10(t);
    ts.hutCups = (t > 3.4f) ? 2 : 1;                                   // Ren pours a second cup
    ts.hanaHome = smoothstep01((t - 6.8f) / 0.8f);                    // the doorway lights up when her mother comes home
    ts.gateLight = t > 10.4f; ts.grannyLight = t > 11.0f;
    gMoonScale = 1.0f + 0.8f * smoothstep01((t - 11.5f) / 3.5f);
}
static void scene10Actors(float t, const TownState& ts) {
    static const PersonStyle ren = makeRen(), mother = makeMother(), granny = makeGranny();
    PersonStyle hana = makeHana(); hana.prop = PROP_NONE; hana.uniform = false; hana.top = rgb(0.95f, 0.9f, 0.95f); hana.bottom = rgb(0.45f, 0.5f, 0.7f);
    PersonStyle kenji = makeKenji();
    float hutY = groundY(World::HUT_X) + 6;
    // Ren, seated at his table, pours the second cup
    PersonPose rp = makePose(-1, t); rp.sit = 1;
    float pour = (t > 1.6f && t < 3.6f) ? smoothstep01((t - 1.6f) / 0.5f) * (1.0f - smoothstep01((t - 3.0f) / 0.6f)) : 0.0f;
    rp.armsFwd = 0.35f + 0.65f * pour;
    glPushMatrix(); glTranslatef(World::HUT_X + 38, hutY + 36, 0); glRotatef(28.0f * pour, 0, 0, 1);              // teapot tilts
    drawEllipse(0, 5, 8, 6.5f, rgb(0.55f, 0.40f, 0.35f), 16); drawLine(-8, 7, -14, 12 + 0, rgb(0.55f, 0.40f, 0.35f), 2.0f); drawLine(8, 6, 13, 10, rgb(0.55f, 0.40f, 0.35f), 1.6f);
    glPopMatrix();
    if (pour > 0.2f) drawLine(World::HUT_X + 22, hutY + 46, World::HUT_X + 27, hutY + 41, rgba(0.9f, 0.8f, 0.5f, 0.9f), 1.5f);
    drawPerson(World::HUT_X + 54, hutY + 12, 0.6f, ren, rp);
    // Hana's mother comes home; Hana runs out to meet her
    static const Waypoint momW[] = { {0, 2560}, {5.0f, 2560}, {7.2f, 2348}, {15, 2348} };
    static const Waypoint hanaW[] = { {0, 2289}, {6.9f, 2289}, {7.7f, 2322}, {15, 2322} };
    Track m = trackAt(t, momW, 4, -1), h = trackAt(t, hanaW, 4, 1);
    float homeY = groundY(2320) - 6;
    drawActor(mother, 1.0f, m, t, homeY, 0);
    if (t > 6.8f) drawActor(hana, 0.92f, h, t, homeY + 2, 0);
    // Kenji reaches Granny Sato's gate; her light is on
    static const Waypoint kenjiW[] = { {0, 3560}, {10.9f, 4545}, {15, 4545} };
    static const Waypoint grannyW[] = { {0, 4800}, {11.4f, 4800}, {13.4f, 4628}, {15, 4628} };
    Track k = trackAt(t, kenjiW, 3, 1), g = trackAt(t, grannyW, 3, -1);
    drawCat(4660, walkY(4660) + 2, 0.85f, -1, 0, 0, t, true);                     // the cat waits just inside the gate
    if (t > 11.2f) drawActor(granny, 1.0f, g, t, walkY(g.x) + 2, 0);
    drawActor(kenji, 1.0f, k, t, walkY(k.x) + 2, 0);
    (void)ts;
}
static void drawFinalLine(float t) {                                              // the film's closing thought
    float a = smoothstep01((t - 12.3f) / 1.0f) * (1.0f - smoothstep01((t - 14.0f) / 0.5f));
    if (a < 0.01f) return;
    const char* s = "Same sky. Different lives.";
    float scale = 0.34f, w = textStrokeWidth(s, scale);
    drawTextStroke((Config::WINDOW_W - w) / 2 + 2, 392 - 2, s, rgba(0, 0, 0, 0.5f * a), scale, 4.0f);     // soft shadow
    drawTextStroke((Config::WINDOW_W - w) / 2, 392, s, rgba(1.0f, 0.97f, 0.9f, a), scale, 2.6f);
}

// ------------------------------------------------------------ overlays ----
static void drawCaption(float t) {
    float a = smoothstep01(t / 0.7f) * (1.0f - smoothstep01((t - 4.2f) / 1.0f));
    if (t < 0.5f) a = 0;
    if (a <= 0.01f) return;
    drawGradientRectH(0, 50, 760, 150, rgba(0, 0, 0, 0.5f * a), rgba(0, 0, 0, 0));
    drawTextStroke(60, 118, kScenes[gIndex].clock, rgba(1.0f, 0.82f, 0.5f, a), 0.19f, 2.2f);   // scalable stroke font for the title card
    drawTextStroke(60, 86, kScenes[gIndex].title, rgba(1, 1, 1, a), 0.16f, 1.8f);
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

static bool isPanelScene(int i) { return i == 3 || i == 4 || i == 5 || i == 7; }

// ----------------------------------------------------- split-screen scenes ----
// Panels appear one after another, each with its own viewport, scissor and projection.
static float panelReveal(float t, int i) { return smoothstep01((t - (0.3f + 0.6f * i)) / 0.8f); }

static void drawPanelScene(float t) {
    PanelLayout L;
    if (gIndex == 3 || gIndex == 7) L = layoutColumns(3);
    else if (gIndex == 4) L = layoutColumns(2);
    else L = layoutGrid2x2();
    float flash = (gIndex == 5) ? lightningLevel(t) : 0.0f;
    // Scene 5: ONE cloud, one position in screen space, drawn in both panels
    float cloudX = lerp(150.0f, 1150.0f, t / SCENE_LENGTH), cloudY = 470.0f;
    float flockX = lerp(-160.0f, 1480.0f, t / SCENE_LENGTH);       // Scene 8: the crows cross all three panels
    for (int i = 0; i < L.count; ++i) {
        const PanelRect& r = L.r[i];
        beginPanel(r);
        if (gIndex == 3) {
            if (i == 0) drawClassroomPanel(r.w, r.h, t);
            if (i == 1) drawOfficePanel(r.w, r.h, t);
            if (i == 2) drawPianoPanel(r.w, r.h, t);
        } else if (gIndex == 7) {
            if (i == 0) drawRenReturnPanel(r.w, r.h, t, r.x, flockX);
            if (i == 1) drawHanaWaitPanel(r.w, r.h, t, r.x, flockX);
            if (i == 2) drawKenjiTrainPanel(r.w, r.h, t, r.x, flockX);
        } else if (gIndex == 4) {
            if (i == 0) drawRooftopPanel(r.w, r.h, t, r.x, cloudX, cloudY);
            if (i == 1) drawFieldPanel(r.w, r.h, t, r.x, cloudX, cloudY);
        } else {
            if (i == 0) drawAoiWindowPanel(r.w, r.h, t, flash);
            if (i == 1) drawHanaStreetPanel(r.w, r.h, t, flash);
            if (i == 2) drawRenHutPanel(r.w, r.h, t, flash);
            if (i == 3) drawKenjiRunPanel(r.w, r.h, t, flash);
        }
        float cover = 1.0f - panelReveal(t, i);                       // black cover fades away
        if (cover > 0.001f) drawRect(0, 0, r.w, r.h, rgba(0, 0, 0, cover));
        endPanel();
    }
    drawPanelFrames(L);
}

// ------------------------------------------------------------------ draw ----
void scenesDraw() {
    const SceneInfo& info = kScenes[gIndex];
    float hour = lerp(info.hourStart, info.hourEnd, gTime / SCENE_LENGTH);   // story hour glides through the chapter
    gSky = skyAtHour(hour);

    gMoonScale = 1.0f;
    if (isPanelScene(gIndex)) {
        drawPanelScene(gTime);
    } else {
        TownState ts; ts.time = gClock.time;
        switch (gIndex) {
            case 0: scene1(gTime, ts); break;
            case 1: scene2(gTime, ts); break;
            case 2: scene3(gTime, ts); break;
            case 6: scene7(gTime, ts); break;
            case 8: scene9(gTime, ts); break;
            default: scene10(gTime, ts); break;
        }
        drawSky(gClock.time);
        if (gIndex == 0) scene1SkyFx(gTime);
        if (gIndex == 8) fireworksDraw();                                      // sky show, behind the town
        drawFarLayers(gClock.time);
        if (gIndex == 6) drawRainbow(smoothstep01((gTime - 0.4f) / 2.5f) * (1.0f - 0.55f * smoothstep01((gTime - 11.0f) / 4.0f)));
        cameraBegin(1.0f);
        drawTownBack(ts);
        switch (gIndex) {
            case 0: scene1Actors(gTime, ts); break;
            case 1: scene2Actors(gTime, ts); break;
            case 2: scene3Actors(gTime, ts); break;
            case 6: scene7Actors(gTime, ts); break;
            case 8: scene9Actors(gTime, ts); break;
            default: scene10Actors(gTime, ts); break;
        }
        drawTownFront(ts);
        cameraEnd();
        if (gIndex == 1) drawLightRays(gSky.sunX, gSky.sunY, gClock.time, 0.9f, rgb(1.0f, 0.86f, 0.58f));
        if (gIndex == 8 && fireworksFlash() > 0.01f) drawRect(0, 0, 1280, 720, rgba(1.0f, 0.85f, 0.6f, 0.07f * fireworksFlash()));   // light from the bursts
        drawVignette(0.4f);
        if (gIndex == 9) drawFinalLine(gTime);
    }

    // fade in at the start / out at the end of every chapter
    float fade = 1.0f - smoothstep01(gTime / FADE_TIME);
    fade = std::fmax(fade, smoothstep01((gTime - (SCENE_LENGTH - FADE_TIME)) / FADE_TIME));
    drawFade(rgb(0, 0, 0), fade);

    drawLetterbox();
    drawCaption(gTime);
    drawHud();
}
