#include "TestScreen.h"
#include "../src/Animation.h"
#include "../src/Environment.h"
#include "../src/Utils.h"
#include <GL/freeglut.h>
#include <cmath>
#include <cstdio>

static int gPage = 0;   // 0 = utility grid, 1 = composed mini-scene

// ---------------------------------------------------- day/night keyframes ----
// pre-dawn, sunrise, noon, sunset, night  (blended with lerpColor)
static const int KEYS = 5;
static const Color kTop[KEYS]  = { rgb(0.05f,0.07f,0.22f), rgb(0.35f,0.30f,0.60f), rgb(0.25f,0.55f,0.95f), rgb(0.30f,0.22f,0.50f), rgb(0.02f,0.03f,0.10f) };
static const Color kBot[KEYS]  = { rgb(0.30f,0.25f,0.45f), rgb(1.00f,0.65f,0.40f), rgb(0.70f,0.88f,1.00f), rgb(1.00f,0.50f,0.25f), rgb(0.08f,0.10f,0.25f) };
static const float kDark[KEYS] = { 0.85f, 0.35f, 0.0f, 0.40f, 1.0f };

// phase 0..1 loops through the five keyframes; darkness 0 = full day, 1 = night.
static void skyAt(float phase, Color& top, Color& bottom, float& darkness) {
    float p = phase * KEYS;
    int i = (int)p % KEYS;
    float t = smoothstep01(p - std::floor(p));
    int j = (i + 1) % KEYS;
    top = lerpColor(kTop[i], kTop[j], t);
    bottom = lerpColor(kBot[i], kBot[j], t);
    darkness = lerp(kDark[i], kDark[j], t);
}

// Position along a half-circle arc for a < 0..1 (false when outside the arc).
static bool arcPos(float a01, float x0, float x1, float horizonY, float height, float& x, float& y) {
    if (a01 < 0.0f || a01 > 1.0f) return false;
    float a = a01 * Config::PI;
    x = x0 + (x1 - x0) * (1.0f - std::cos(a)) / 2.0f;
    y = horizonY + height * std::sin(a);
    return true;
}

// darken + cool scenery at night
static Color tinted(Color c, float darkness) {
    return lerpColor(c, rgb(c.r * 0.18f, c.g * 0.22f, c.b * 0.42f), darkness * 0.85f);
}

// ------------------------------------------------------------- grid cells ----
struct Cell { float x, y, w, h; };
static Cell cellAt(int col, int row) {
    Cell c; c.w = 290; c.h = 280; c.x = 30 + col * 310; c.y = (row == 0) ? 370.0f : 70.0f; return c;
}
static void beginCell(const Cell& c, Color bg) {
    beginClip(c.x, c.y, c.w, c.h);
    drawRect(c.x, c.y, c.w, c.h, bg);
}
static void endCell(const Cell& c, const char* label) {
    drawText(c.x + 8, c.y + 8, label, rgb(0.95f, 0.95f, 0.95f));
    drawRectOutline(c.x, c.y, c.w, c.h, rgb(0.5f, 0.5f, 0.55f), 1.0f);
    endClip();
}

static void cellRects(float) {
    Cell c = cellAt(0, 0); beginCell(c, rgb(0.16f, 0.17f, 0.20f));
    drawGradientRect(c.x + 15, c.y + 60, 260, 110, rgb(1.0f, 0.62f, 0.38f), rgb(0.12f, 0.16f, 0.42f));
    drawRect(c.x + 15, c.y + 60, 260, 24, rgb(0.10f, 0.12f, 0.20f));
    drawRectOutline(c.x + 15, c.y + 60, 260, 110, rgb(1, 1, 1), 1.5f);
    drawRect(c.x + 20, c.y + 190, 110, 70, rgb(0.90f, 0.80f, 0.50f));
    drawRectOutline(c.x + 20, c.y + 190, 110, 70, rgb(0.25f, 0.18f, 0.10f), 2.0f);
    drawGradientRectH(c.x + 150, c.y + 190, 125, 70, rgb(0.30f, 0.80f, 0.90f), rgb(0.90f, 0.40f, 0.70f));
    endCell(c, "drawRect / gradient V+H / outline");
}

static void cellCircles(float) {
    Cell c = cellAt(1, 0); beginCell(c, rgb(0.16f, 0.17f, 0.20f));
    const Color band[6] = { rgb(0.9f,0.2f,0.2f), rgb(1.0f,0.6f,0.1f), rgb(1.0f,0.9f,0.2f),
                            rgb(0.3f,0.8f,0.3f), rgb(0.2f,0.5f,0.95f), rgb(0.6f,0.3f,0.8f) };
    for (int i = 0; i < 6; ++i)                                      // rainbow preview (Scene 7)
        drawArcBand(c.x + 145, c.y + 45, 70.0f + i * 10, 80.0f + i * 10, 0.0f, Config::PI, band[i]);
    drawCircle(c.x + 45, c.y + 225, 28, rgb(0.95f, 0.75f, 0.30f));
    drawCircleOutline(c.x + 45, c.y + 225, 28, rgb(0.3f, 0.2f, 0.05f), 2.0f);
    drawEllipse(c.x + 130, c.y + 225, 42, 22, rgb(0.5f, 0.8f, 0.95f));
    drawArcBand(c.x + 235, c.y + 225, 14, 28, 0.0f, 2.0f * Config::PI, rgb(0.9f, 0.5f, 0.7f));
    endCell(c, "circle / ellipse / outline / arc band");
}

static void cellPolygons(float t) {
    Cell c = cellAt(2, 0); beginCell(c, rgb(0.16f, 0.17f, 0.20f));
    drawTriangle(c.x + 20, c.y + 170, c.x + 90, c.y + 240, c.x + 160, c.y + 170, rgb(0.75f, 0.25f, 0.2f));
    float pent[10];
    for (int i = 0; i < 5; ++i) {
        float a = Config::PI / 2 + i * 2 * Config::PI / 5;
        pent[2 * i] = c.x + 225 + 42 * std::cos(a); pent[2 * i + 1] = c.y + 205 + 42 * std::sin(a);
    }
    drawPolygon(pent, 5, rgb(0.4f, 0.7f, 0.45f));
    drawPolygonOutline(pent, 5, rgb(0.05f, 0.2f, 0.1f), 2.0f);
    for (int i = 0; i < 5; ++i)                                      // line widths 1..5
        drawLine(c.x + 20, c.y + 140 - i * 18, c.x + 140, c.y + 140 - i * 18, rgb(1, 1, 1), 1.0f + i);
    for (int i = 0; i < 8; ++i) {                                    // animated rain streaks
        float x = c.x + 165 + i * 14;
        float y = c.y + 150 - std::fmod(t * 120.0f + i * 37.0f, 110.0f);
        drawLine(x, y, x - 3, y - 12, rgba(0.6f, 0.75f, 1.0f, 0.85f), 1.5f);
    }
    endCell(c, "triangle / polygon / lines (widths 1-5)");
}

static void cellGlow(float t) {
    Cell c = cellAt(3, 0); beginCell(c, rgb(0.04f, 0.05f, 0.12f));
    drawGlow(c.x + 90, c.y + 170, 95, rgba(1.0f, 0.72f, 0.30f, 0.9f));
    drawCircle(c.x + 90, c.y + 170, 24, rgb(1.0f, 0.95f, 0.80f));
    drawGlow(c.x + 215, c.y + 215, 60, rgba(0.55f, 0.70f, 1.0f, 0.8f));
    drawCircle(c.x + 215, c.y + 215, 16, rgb(0.9f, 0.95f, 1.0f));
    drawRect(c.x + 20, c.y + 30, 250, 70, rgb(0.08f, 0.09f, 0.16f));   // dark wall
    drawWindow(c.x + 40, c.y + 50, 40, 36, true);
    drawWindow(c.x + 120, c.y + 50, 40, 36, false);
    drawWindow(c.x + 200, c.y + 50, 40, 36, std::fmod(t, 3.0f) < 1.5f);   // blinks
    endCell(c, "drawGlow (additive) + windows");
}

static void cellSky(float t) {
    Cell c = cellAt(0, 1); beginCell(c, rgb(0, 0, 0));
    float phase = std::fmod(t / 10.0f, 1.0f);
    Color top, bot; float dark;
    skyAt(phase, top, bot, dark);
    drawGradientRect(c.x, c.y, c.w, c.h, bot, top);
    drawStarField(c.x, c.y + 80, c.w, c.h - 80, 40, t, dark);
    float x, y;
    if (arcPos((phase - 0.2f) / 0.4f, c.x + 30, c.x + c.w - 30, c.y + 60, 150, x, y)) {
        drawGlow(x, y, 55, rgba(1.0f, 0.8f, 0.4f, 0.8f));
        drawCircle(x, y, 18, rgb(1.0f, 0.95f, 0.75f));
    }
    float mp = phase < 0.1f ? phase + 1.0f : phase;
    if (arcPos((mp - 0.6f) / 0.5f, c.x + 30, c.x + c.w - 30, c.y + 60, 150, x, y)) {
        drawGlow(x, y, 40, rgba(0.6f, 0.7f, 1.0f, 0.7f));
        drawCircle(x, y, 13, rgb(0.92f, 0.95f, 1.0f));
    }
    drawRect(c.x, c.y, c.w, 60, tinted(rgb(0.25f, 0.45f, 0.30f), dark));
    endCell(c, "lerpColor sky cycle (dawn-noon-dusk-night)");
}

static void cellAlpha(float) {
    Cell c = cellAt(1, 1); beginCell(c, rgb(0.9f, 0.9f, 0.9f));
    for (int i = 0; i < 12; ++i)                                     // stripes show what is behind
        drawRect(c.x + i * 24.2f, c.y, 12, c.h, rgb(0.25f, 0.25f, 0.3f));
    drawCircle(c.x + 110, c.y + 190, 50, rgba(1.0f, 0.1f, 0.1f, 0.55f));
    drawCircle(c.x + 170, c.y + 190, 50, rgba(0.1f, 1.0f, 0.1f, 0.55f));
    drawCircle(c.x + 140, c.y + 140, 50, rgba(0.1f, 0.2f, 1.0f, 0.55f));
    drawGradientRect(c.x, c.y, c.w, 100, rgba(0.0f, 0.0f, 0.2f, 0.95f), rgba(0.0f, 0.0f, 0.2f, 0.0f));
    endCell(c, "GL_BLEND: translucent circles + fade");
}

static void cellTransform(float t) {
    Cell c = cellAt(2, 1); beginCell(c, rgb(0.16f, 0.17f, 0.20f));
    // Nested transforms: the forearm is drawn inside the upper arm's matrix, so it
    // swings WITH the upper arm. Characters' limbs will work exactly like this.
    glPushMatrix();
        glTranslatef(c.x + 60, c.y + 215, 0);                   // move origin to the shoulder
        glRotatef(35.0f * std::sin(t * 3.0f), 0, 0, 1);         // rotate about the shoulder
        drawRect(-6, -60, 12, 60, rgb(0.95f, 0.75f, 0.6f));
        glTranslatef(0, -60, 0);                                // move origin to the elbow
        glRotatef(30.0f + 30.0f * std::sin(t * 3.0f + 1.0f), 0, 0, 1);
        drawRect(-5, -50, 10, 50, rgb(0.9f, 0.7f, 0.55f));
        drawCircle(0, -52, 8, rgb(0.95f, 0.75f, 0.6f));
    glPopMatrix();
    drawCircle(c.x + 60, c.y + 215, 8, rgb(0.3f, 0.3f, 0.35f));
    float s = 0.85f + 0.15f * std::sin(t * 2.0f);               // scale about the base point
    glPushMatrix();
        glTranslatef(c.x + 205, c.y + 60, 0);
        glScalef(s, s, 1);
        drawHouse(0, 0, 90, 60, rgb(0.85f, 0.75f, 0.6f), rgb(0.7f, 0.3f, 0.25f), true);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(c.x + 200, c.y + 220, 0);
        glRotatef(t * 60.0f, 0, 0, 1);
        drawRect(-18, -18, 36, 36, rgb(0.5f, 0.8f, 0.9f));
    glPopMatrix();
    endCell(c, "glPush/Pop, Translate, Rotate, Scale");
}

static void cellStars(float t) {
    Cell c = cellAt(3, 1); beginCell(c, rgb(0.02f, 0.03f, 0.10f));
    drawStarField(c.x, c.y, c.w, c.h, 70, t, 1.0f);
    drawGlow(c.x + 225, c.y + 210, 55, rgba(0.7f, 0.8f, 1.0f, 0.7f));
    drawCircle(c.x + 225, c.y + 210, 18, rgb(0.92f, 0.95f, 1.0f));
    float p = std::fmod(t, 4.0f);                                // one streak every 4 s
    if (p < 1.0f)
        drawShootingStar(c.x + 40 + p * 200, c.y + 250 - p * 120, 43, -26, std::sin(p * Config::PI));
    endCell(c, "GL_POINTS stars + shooting star");
}

// ------------------------------------------------- page 2: composed scene ----
static void miniScene(float t) {
    float phase = std::fmod(t / 24.0f, 1.0f);
    Color top, bot; float dark;
    skyAt(phase, top, bot, dark);
    drawGradientRect(0, 0, 1280, 720, bot, top);
    drawStarField(0, 300, 1280, 420, 150, t, dark);

    float x, y;
    if (arcPos((phase - 0.2f) / 0.4f, 120, 1160, 250, 360, x, y)) {
        drawGlow(x, y, 190, rgba(1.0f, 0.78f, 0.40f, 0.7f), 10);
        drawCircle(x, y, 40, rgb(1.0f, 0.96f, 0.78f));
    }
    float mp = phase < 0.1f ? phase + 1.0f : phase;
    if (arcPos((mp - 0.6f) / 0.5f, 120, 1160, 250, 360, x, y)) {
        drawGlow(x, y, 130, rgba(0.6f, 0.72f, 1.0f, 0.6f), 10);
        drawCircle(x, y, 30, rgb(0.93f, 0.96f, 1.0f));
    }
    // clouds drift and wrap around the screen
    Color cloud = tinted(rgb(1.0f, 1.0f, 1.0f), dark);
    drawCloud(std::fmod(t * 14.0f + 200, 1500.0f) - 110, 560, 1.0f, cloud);
    drawCloud(std::fmod(t * 9.0f + 800, 1500.0f) - 110, 620, 0.7f, cloud);
    drawCloud(std::fmod(t * 20.0f + 1200, 1500.0f) - 110, 500, 0.5f, cloud);

    // three parallax hill layers: far scrolls slowly, near scrolls faster
    drawHillLayer(340, 50, 700, t * 6.0f,  tinted(rgb(0.58f, 0.68f, 0.88f), dark), tinted(rgb(0.45f, 0.55f, 0.78f), dark));
    drawHillLayer(255, 45, 520, t * 12.0f, tinted(rgb(0.38f, 0.58f, 0.58f), dark), tinted(rgb(0.28f, 0.45f, 0.48f), dark));
    drawHillLayer(175, 32, 380, t * 24.0f, tinted(rgb(0.28f, 0.52f, 0.36f), dark), tinted(rgb(0.20f, 0.40f, 0.30f), dark));
    drawGradientRect(0, 0, 1280, 140, tinted(rgb(0.16f, 0.32f, 0.22f), dark), tinted(rgb(0.28f, 0.52f, 0.36f), dark));

    bool lit = dark > 0.3f;                                       // windows glow when it is dim
    drawHouse(210, 105, 130, 80, tinted(rgb(0.95f, 0.88f, 0.75f), dark), tinted(rgb(0.72f, 0.30f, 0.26f), dark), lit);
    drawHouse(560, 100, 100, 70, tinted(rgb(0.85f, 0.80f, 0.90f), dark), tinted(rgb(0.30f, 0.38f, 0.60f), dark), lit);
    drawHouse(930, 108, 140, 85, tinted(rgb(0.92f, 0.82f, 0.68f), dark), tinted(rgb(0.45f, 0.30f, 0.22f), dark), lit);
    drawTree(90,  98, 1.0f,  tinted(rgb(0.25f, 0.58f, 0.32f), dark));
    drawTree(390, 92, 0.85f, tinted(rgb(0.30f, 0.62f, 0.34f), dark));
    drawTree(745, 96, 1.1f,  tinted(rgb(0.22f, 0.52f, 0.30f), dark));
    drawTree(1150, 94, 0.95f, tinted(rgb(0.28f, 0.60f, 0.34f), dark));

    drawVignette(0.45f);
}

// ---------------------------------------------------------------- entry ----
void testScreenNextPage() { gPage = (gPage + 1) % 2; }

void testScreenDraw() {
    float t = gClock.time;
    if (gPage == 0) {
        drawRect(0, 0, 1280, 720, rgb(0.10f, 0.10f, 0.12f));
        cellRects(t); cellCircles(t); cellPolygons(t); cellGlow(t);
        cellSky(t); cellAlpha(t); cellTransform(t); cellStars(t);
    } else {
        miniScene(t);
    }
    drawLetterbox();

    char line[160];
    drawText(24, 692, gPage == 0 ? "PHASE 2  -  Core utilities test   |   Page 1/2: primitives, gradients, glow, blending, transforms"
                                 : "PHASE 2  -  Core utilities test   |   Page 2/2: composed mini-scene (parallax, clouds, houses, day/night)",
             rgb(1, 1, 1), true);
    std::snprintf(line, sizeof line, "FPS %.0f    t = %.1f s    speed %.2fx    %s",
                  gClock.fps, gClock.time, gClock.speed, gClock.paused ? "[PAUSED]" : "");
    drawText(24, 20, line, rgb(1, 1, 1), true);
    drawText(800, 20, "T: page   Space: pause   Left/Right: speed   Esc: exit", rgb(0.85f, 0.85f, 0.85f));
}
