// Headless tests for Phase 2. Build + run instructions are in docs/PROGRESS.md.
#include "../src/Utils.h"
#include "../src/Animation.h"
#include "../src/Environment.h"
#include "../src/TestScreen.h"
#include "mock_gl.h"
#include <cstdio>
#include <cmath>

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

int main() {
    testMath();
    testClock();
    testHeadless();
    std::printf("\n%d checks passed, %d failed\n", gPass, gFail);
    return gFail ? 1 : 0;
}
