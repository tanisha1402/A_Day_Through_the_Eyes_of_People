// Headless tests for Phase 2. Build + run instructions are in docs/PROGRESS.md.
#include "../src/Utils.h"
#include "../src/Animation.h"
#include "../src/Environment.h"
#include "TestScreen.h"
#include "../src/TimeOfDay.h"
#include "../src/Camera.h"
#include "../src/Scenes.h"
#include "../src/Viewports.h"
#include "../src/Particles.h"
#include "../src/Panels.h"
#include "../src/Fireworks.h"
#include "mock_gl.h"
#include <cstdio>
#include <cmath>
#include <algorithm>

static int gFail = 0, gPass = 0;
#define CHECK(cond, name) do { if (cond) { ++gPass; } else { ++gFail; std::printf("FAIL: %s\n", name); } } while (0)
static bool near(float a, float b, float e = 0.01f) { return std::fabs(a - b) <= e; }

static void testMath() {
    CHECK(near(lerp(0, 10, 0.5f), 5), "lerp midpoint");
    CHECK(clamp01(-3) == 0 && clamp01(7) == 1 && near(clamp01(0.4f), 0.4f), "clamp01");
    CHECK(near(smoothstep01(0), 0) && near(smoothstep01(1), 1) && near(smoothstep01(0.5f), 0.5f) && smoothstep01(0.25f) < 0.25f, "smoothstep01");
    Color m = lerpColor(rgb(0, 0, 0), rgb(1, 0.5f, 0.2f), 0.5f);
    CHECK(near(m.r, 0.5f) && near(m.g, 0.25f) && near(m.b, 0.1f), "lerpColor midpoint");
    CHECK(near(lerpColor(rgb(1,1,1), rgb(0,0,0), 2.0f).r, 0), "lerpColor clamps t");
    Color d = darken(rgba(1, 0.5f, 0.2f, 0.7f), 0.5f);
    CHECK(near(d.r, 0.5f) && near(d.g, 0.25f) && near(d.a, 0.7f), "darken keeps alpha");

    Viewport v = computeViewport(1280, 720);
    CHECK(v.x == 0 && v.y == 0 && v.w == 1280 && v.h == 720, "viewport exact 16:9");
    v = computeViewport(1000, 1000);
    CHECK(v.w == 1000 && v.h == 562 && v.x == 0 && v.y == 219, "viewport letterboxed (tall window)");
    v = computeViewport(800, 300);
    CHECK(v.h == 300 && v.w == 533 && v.x == 133 && v.y == 0, "viewport pillarboxed (wide window)");
    v = computeViewport(0, 0);
    CHECK(v.w >= 1 && v.h >= 1, "viewport survives zero-size (minimised) window");

    float lx, ly;
    windowToLogical(0, 0, 720, computeViewport(1280, 720), lx, ly);
    CHECK(near(lx, 0) && near(ly, 720), "mouse top-left -> logical (0,720)");
    windowToLogical(640, 360, 720, computeViewport(1280, 720), lx, ly);
    CHECK(near(lx, 640) && near(ly, 360), "mouse centre");
    windowToLogical(500, 500, 1000, computeViewport(1000, 1000), lx, ly);
    CHECK(near(lx, 640, 1.5f) && near(ly, 360, 1.5f), "mouse centre in letterboxed window");
    windowToLogical(1919, 1079, 1080, computeViewport(1920, 1080), lx, ly);
    CHECK(near(lx, 1280, 1.0f) && near(ly, 0, 1.0f), "mouse bottom-right at 1920x1080");

    ScissorRect s = logicalToScissor(100, 50, 200, 100, computeViewport(1920, 1080));
    CHECK(s.x == 150 && s.y == 75 && s.w == 300 && s.h == 150, "scissor scaled 1.5x");
    s = logicalToScissor(0, 0, 1280, 720, computeViewport(1000, 1000));
    CHECK(s.x == 0 && s.w == 1000 && s.y == 219 && s.h >= 562, "scissor of full screen = viewport");
}

static void testClock() {
    clockStart(1000);
    clockTick(1016);
    CHECK(near(gClock.dt, 0.016f, 0.0005f) && near(gClock.time, 0.016f, 0.0005f), "clock advances by real dt");
    clockSpeedUp(); clockTick(1032);
    CHECK(near(gClock.dt, 0.016f * 1.25f, 0.0005f), "speed multiplier applied");
    for (int i = 0; i < 30; ++i) clockSpeedUp();
    CHECK(near(gClock.speed, Config::SPEED_MAX), "speed capped at max");
    for (int i = 0; i < 60; ++i) clockSlowDown();
    CHECK(near(gClock.speed, Config::SPEED_MIN), "speed floored at min");
    clockTogglePause(); float before = gClock.time; clockTick(1048);
    CHECK(gClock.dt == 0 && gClock.time == before, "pause freezes time");
    clockTogglePause(); clockTick(6000);
    CHECK(gClock.dt <= Config::MAX_DT * Config::SPEED_MIN + 1e-4f, "long stall is clamped");
    clockStart(0);
    for (int i = 1; i <= 125; ++i) clockTick(i * 16);   // 2 seconds at 16 ms
    CHECK(near(gClock.fps, 62.5f, 3.0f), "fps measurement ~62.5 for 16 ms ticks");
}

static void testHeadless() {
    const int sizes[][2] = { {1280, 720}, {1920, 1080}, {1000, 1000}, {800, 300} };
    for (int s = 0; s < 4; ++s) {
        utilsOnResize(sizes[s][0], sizes[s][1]);
        clockStart(0);
        for (int page = 0; page < 2; ++page) {
            for (int frame = 1; frame <= 1800; ++frame) {         // 30 s of animation, covers whole day cycle
                clockTick(frame * 16);
                gMock.vertices = 0;
                testScreenDraw();
                mockFrameCheck("frame");
            }
            testScreenNextPage();
        }
    }
    CHECK(gMock.errors == 0, "no GL misuse over 14,400 frames (4 window sizes, both pages)");
    if (gMock.errors) std::printf("  first error: %s\n", gMock.firstError);
    std::printf("  max vertices per frame: %ld   text calls total: %ld\n", gMock.maxVertices, gMock.textCalls);
    CHECK(gMock.maxVertices < 60000, "vertex count per frame stays modest");
}


static void testTimeOfDay() {
    // 1. the sky never jumps: tiny hour steps give tiny colour changes, over the whole looping day
    float maxStep = 0; SkyState prev = skyAtHour(4.5f);
    for (float h = 4.505f; h <= 28.5f; h += 0.005f) {
        SkyState s = skyAtHour(h);
        float d = std::fabs(s.top.r - prev.top.r) + std::fabs(s.top.g - prev.top.g) + std::fabs(s.top.b - prev.top.b)
                + std::fabs(s.bottom.r - prev.bottom.r) + std::fabs(s.bottom.g - prev.bottom.g) + std::fabs(s.bottom.b - prev.bottom.b)
                + std::fabs(s.light.r - prev.light.r) + std::fabs(s.darkness - prev.darkness);
        if (d > maxStep) maxStep = d;
        prev = s;
    }
    std::printf("  largest sky change per 0.005 h step: %.4f\n", maxStep);
    CHECK(maxStep < 0.03f, "sky colours blend smoothly through the whole day");
    SkyState a = skyAtHour(4.5f), b = skyAtHour(28.5f), c = skyAtHour(3.0f), d = skyAtHour(27.0f);
    CHECK(near(a.top.r, b.top.r, 0.001f) && near(a.bottom.g, b.bottom.g, 0.001f), "loop closes: 28:30 sky == 4:30 sky");
    CHECK(near(c.top.b, d.top.b, 0.001f), "hours below 4:30 wrap to the next day");
    SkyState noon = skyAtHour(12.0f), night = skyAtHour(23.5f), dawn = skyAtHour(4.5f);
    CHECK(noon.sunUp && !noon.moonUp && noon.darkness < 0.05f && noon.starAlpha == 0, "noon: sun up, no moon, no stars");
    CHECK(!night.sunUp && night.moonUp && night.darkness > 0.95f && night.starAlpha > 0.9f, "11:30 PM: moon up, stars out");
    CHECK(dawn.moonUp && !dawn.sunUp, "4:30 AM: moon still up");
    CHECK(skyAtHour(12.0f).sunY > skyAtHour(7.0f).sunY && skyAtHour(12.0f).sunY > skyAtHour(17.0f).sunY, "sun is highest at noon");
    int lit4 = 0, lit12 = 0, lit21 = 0;
    for (int id = 0; id < 100; ++id) { lit4 += windowLit(id, 4.5f); lit12 += windowLit(id, 12.0f); lit21 += windowLit(id, 21.0f); }
    std::printf("  lit windows out of 100:  4:30 -> %d   12:00 -> %d   21:00 -> %d\n", lit4, lit12, lit21);
    CHECK(lit4 > 5 && lit4 < 30 && lit12 == 0 && lit21 > 50, "window schedule: few at dawn, none at noon, most in the evening");
}

static void testCameraAndScenes() {
    Camera p = makeCamera(100, 200, 1.0f), q = makeCamera(500, 400, 2.0f);
    Camera m0 = cameraBlend(p, q, 0), m1 = cameraBlend(p, q, 1), mh = cameraBlend(p, q, 0.5f);
    CHECK(near(m0.x, 100) && near(m1.x, 500) && near(m1.zoom, 2.0f), "camera blend endpoints");
    CHECK(near(mh.x, 300) && near(mh.y, 300) && near(mh.zoom, 1.5f), "camera blend midpoint (smoothstep(0.5) = 0.5)");

    scenesInit();
    CHECK(sceneIndex() == 0 && sceneTime() == 0, "scenes start at chapter 1");
    for (int i = 0; i < 899; ++i) scenesUpdate(1.0f / 60.0f);     // 14.98 s
    CHECK(sceneIndex() == 0, "chapter 1 lasts 15 s");
    for (int i = 0; i < 3; ++i) scenesUpdate(1.0f / 60.0f);
    CHECK(sceneIndex() == 1 && sceneTime() < 0.1f, "chapter 2 starts after 15 s");
    for (int i = 0; i < 60 * 135; ++i) scenesUpdate(1.0f / 60.0f); // 135 more seconds: chapters 2..10, then wrap
    CHECK(sceneIndex() == 0, "ten chapters (150 s) loop back to chapter 1");
    scenesNext(); CHECK(sceneIndex() == 1 && sceneTime() == 0, "N goes to the next chapter");
    scenesPrev(); scenesPrev(); CHECK(sceneIndex() == 9, "P wraps backwards (chapter 1 -> chapter 10)");
}

static void testPanelsAndParticles() {
    // layouts: inside the area between the letterbox bars, no overlaps, gutters between panels
    PanelLayout L3 = layoutColumns(3), L2 = layoutColumns(2), G = layoutGrid2x2();
    CHECK(L3.count == 3 && L2.count == 2 && G.count == 4, "layout panel counts");
    bool inside = true, apart = true;
    const PanelLayout* all[3] = { &L3, &L2, &G };
    for (int k = 0; k < 3; ++k) {
        const PanelLayout& L = *all[k];
        for (int i = 0; i < L.count; ++i) {
            const PanelRect& a = L.r[i];
            if (a.x < -0.01f || a.y < Config::LETTERBOX - 0.01f || a.x + a.w > 1280.01f || a.y + a.h > 720 - Config::LETTERBOX + 0.01f) inside = false;
            for (int j = i + 1; j < L.count; ++j) {
                const PanelRect& b = L.r[j];
                bool sepX = a.x + a.w <= b.x + 0.01f || b.x + b.w <= a.x + 0.01f, sepY = a.y + a.h <= b.y + 0.01f || b.y + b.h <= a.y + 0.01f;
                if (!(sepX || sepY)) apart = false;
            }
        }
    }
    CHECK(inside, "panels stay between the letterbox bars");
    CHECK(apart, "panels never overlap");
    CHECK(near(L3.r[2].x + L3.r[2].w, 1280.0f, 0.1f) && near(L2.r[1].x + L2.r[1].w, 1280.0f, 0.1f), "columns span the full width");
    CHECK(near(G.r[0].y, G.r[1].y) && G.r[0].y > G.r[2].y, "grid: top row above bottom row");

    // picking: window click -> logical -> panel -> panel-local (also in a letterboxed window)
    float px, py;
    int i0 = panelAt(G, G.r[3].x + 100, G.r[3].y + 40, px, py);
    CHECK(i0 == 3 && near(px, 100) && near(py, 40), "pick bottom-right panel, local coordinates");
    CHECK(panelAt(L3, L3.r[0].x + L3.r[0].w + 3, 300, px, py) == -1, "click in the gutter hits no panel");
    CHECK(panelAt(L3, 500, 20, px, py) == -1 && panelAt(L3, 500, 700, px, py) == -1, "click on a letterbox bar hits no panel");
    Viewport vp = computeViewport(1000, 1000);
    float lx, ly; windowToLogical(300, 500, 1000, vp, lx, ly);                 // a point left of centre
    CHECK(panelAt(L2, lx, ly, px, py) >= 0, "a window click maps into a panel through the letterboxed viewport");
    ScissorRect sr = logicalToScissor(G.r[1].x, G.r[1].y, G.r[1].w, G.r[1].h, computeViewport(1920, 1080));
    CHECK(sr.x == (int)std::floor(G.r[1].x * 1.5f) && sr.w >= (int)(G.r[1].w * 1.5f), "panel scissor scales with the window");

    // particles: hard caps, determinism, rain settles and drains
    ParticlePool pool; poolInit(pool, 10);
    for (int i = 0; i < 25; ++i) { Particle* q = poolSpawn(pool); if (q) q->life = 1; }
    CHECK(poolAlive(pool) == 10 && poolSpawn(pool) == 0, "pool never exceeds its capacity");
    poolUpdate(pool, 2.0f, 0); CHECK(poolAlive(pool) == 0, "particles expire");
    seedRandom(5); float a1 = rand01(), a2 = rand01(); seedRandom(5); float b1 = rand01(), b2 = rand01();
    CHECK(a1 == b1 && a2 == b2 && a1 >= 0 && a1 < 1, "random numbers are repeatable and in [0,1)");
    RainField rf; rainInit(rf, 637, 307, 12, 74, 100, 5000.0f);               // absurdly heavy rain to stress the caps
    int maxDrops = 0, maxRipples = 0; bool finite = true;
    for (int f = 0; f < 600; ++f) {
        rainUpdate(rf, 1.0f / 60.0f);
        maxDrops = std::max(maxDrops, poolAlive(rf.drops)); maxRipples = std::max(maxRipples, poolAlive(rf.ripples));
        for (size_t i = 0; i < rf.drops.p.size(); ++i) if (!std::isfinite(rf.drops.p[i].x) || !std::isfinite(rf.drops.p[i].y)) finite = false;
    }
    CHECK(maxDrops <= 100 && maxRipples <= 48 && finite, "rain respects its caps even under a flood of spawns");
    rf.intensity = 0; for (int f = 0; f < 180; ++f) rainUpdate(rf, 1.0f / 60.0f);
    CHECK(poolAlive(rf.drops) == 0 && poolAlive(rf.ripples) == 0, "rain drains completely when switched off");
    bool lightningOk = true; float peak = 0, quiet = 0;
    for (float tt = 0; tt < 15.0f; tt += 0.01f) { float v = lightningLevel(tt); if (v < 0 || v > 1) lightningOk = false; peak = std::max(peak, v); }
    quiet = lightningLevel(1.0f);
    CHECK(lightningOk && peak > 0.95f && quiet == 0, "lightning stays in 0..1, flashes, and is dark between flashes");
}

static void checkViewportRestored() {
    int vx, vy, vw, vh; mockGetViewport(vx, vy, vw, vh); Viewport v = utilsGetViewport();
    if (vx != v.x || vy != v.y || vw != v.w || vh != v.h) { ++gMock.errors; std::snprintf(gMock.firstError, sizeof gMock.firstError, "viewport not restored after panels"); }
}

static void testFireworksAndStory() {
    fireworksReset();
    for (int i = 0; i < 300; ++i) fireworksLaunch(100 + (i % 10) * 100.0f, 400.0f, rgb(1, 0.5f, 0.2f));   // flood the launcher
    int peak = 0; float maxFlash = 0; bool ok = true;
    for (int f = 0; f < 1200; ++f) {
        if (f % 5 == 0) fireworksLaunch(200 + (f % 7) * 120.0f, 450.0f, rgb(0.5f, 0.8f, 1.0f));
        fireworksUpdate(1.0f / 60.0f);
        peak = std::max(peak, fireworksAlive()); maxFlash = std::max(maxFlash, fireworksFlash());
        if (fireworksFlash() < 0 || fireworksFlash() > 1) ok = false;
    }
    std::printf("  fireworks: peak live particles %d (cap 366), peak sky flash %.2f\n", peak, maxFlash);
    CHECK(peak <= 6 + 360 && ok && maxFlash > 0.5f, "fireworks respect their caps and the flash stays in 0..1");
    for (int f = 0; f < 600; ++f) fireworksUpdate(1.0f / 60.0f);
    CHECK(fireworksAlive() == 0, "fireworks burn out completely");

    // story clock: the sky at the end of a chapter should match the start of the next (fades hide the two deliberate jumps)
    float worst = 0; int worstAt = -1;
    for (int i = 0; i < 9; ++i) {
        SkyState a = skyAtHour(sceneHourAt(i, 15.0f)), b = skyAtHour(sceneHourAt(i + 1, 0.0f));
        float d = std::fabs(a.top.r - b.top.r) + std::fabs(a.top.g - b.top.g) + std::fabs(a.top.b - b.top.b) + std::fabs(a.bottom.r - b.bottom.r) + std::fabs(a.bottom.g - b.bottom.g) + std::fabs(a.bottom.b - b.bottom.b) + std::fabs(a.darkness - b.darkness);
        std::printf("  chapter %d -> %d sky difference %.3f\n", i + 1, i + 2, d);
        if (d > worst) { worst = d; worstAt = i; }
    }
    CHECK(worst < 0.35f, "sky is continuous across chapter boundaries (4->5 is a time skip with near-identical day sky)");
    (void)worstAt;
    SkyState end = skyAtHour(sceneHourAt(9, 15.0f)), start = skyAtHour(sceneHourAt(0, 0.0f));
    CHECK(end.darkness > 0.8f && start.darkness > 0.8f, "loop: the last chapter ends in deep night, chapter 1 starts in pre-dawn darkness (hidden by the fade)");
}

static void testSceneRendering() {
    // 1. the whole film at 1280x720: all ten chapters + the wrap
    utilsOnResize(1280, 720);
    clockStart(0); scenesInit();
    long frames = 0;
    for (int frame = 1; frame <= 60 * 152; ++frame) {
        clockTick(frame * 16); scenesUpdate(gClock.dt);
        gMock.vertices = 0; scenesDraw(); mockFrameCheck("scene frame"); checkViewportRestored(); ++frames;
    }
    // 2. every chapter at awkward window sizes
    const int sizes[][2] = { {1920, 1080}, {1000, 1000}, {800, 300} };
    for (int s = 0; s < 3; ++s) {
        utilsOnResize(sizes[s][0], sizes[s][1]);
        for (int ch = 0; ch < 10; ++ch) {
            clockStart(0); scenesGoto(ch, 0);
            for (int frame = 1; frame <= 60 * 5; ++frame) {
                clockTick(frame * 16); scenesUpdate(gClock.dt);
                gMock.vertices = 0; scenesDraw(); mockFrameCheck("scene frame"); checkViewportRestored(); ++frames;
            }
        }
    }
    // 3. pause and speed extremes
    utilsOnResize(1280, 720); clockStart(0); scenesGoto(8, 0);
    for (int frame = 1; frame <= 600; ++frame) {
        if (frame == 100) clockTogglePause();
        if (frame == 200) clockTogglePause();
        if (frame == 250) for (int i = 0; i < 10; ++i) clockSpeedUp();
        clockTick(frame * 16); scenesUpdate(gClock.dt); scenesDraw(); mockFrameCheck("speed frame"); checkViewportRestored(); ++frames;
    }
    std::printf("  rendered %ld frames\n", frames);
    CHECK(gMock.errors == 0, "all ten chapters render with no GL misuse (4 window sizes, pause, max speed)");
    if (gMock.errors) std::printf("  first error: %s\n", gMock.firstError);
    std::printf("  max vertices per frame overall: %ld\n", gMock.maxVertices);
    CHECK(gMock.maxVertices < 150000, "vertex count per frame stays within budget");
}

int main() {
    testMath();
    testClock();
    testHeadless();
    testTimeOfDay();
    testCameraAndScenes();
    testPanelsAndParticles();
    testFireworksAndStory();
    testSceneRendering();
    std::printf("\n%d checks passed, %d failed\n", gPass, gFail);
    return gFail ? 1 : 0;
}
