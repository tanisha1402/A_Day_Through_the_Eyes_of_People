#include "World.h"
#include "Camera.h"
#include "Environment.h"
#include "TimeOfDay.h"
#include <GL/freeglut.h>
#include <cmath>

static Color T(Color c) { return tint(c); }                 // lit by the current sky
static bool  lit(int id) { return windowLit(id, gSky.hour); }
static float night() { return gSky.darkness; }

float groundY(float x) {
    float hill = 330.0f - 170.0f * smoothstep01((x - 850.0f) / 550.0f);
    return hill + 6.0f * std::sin(x * 0.004f) * smoothstep01((x - 900.0f) / 300.0f);
}
float walkY(float x) { return groundY(x) - 6.0f; }

// -------------------------------------------------------------------- sky ----
void drawSky(float time) {
    drawGradientRect(0, 0, 1280, 720, gSky.bottom, gSky.top);
    drawStarField(0, 220, 1280, 500, 170, time, gSky.starAlpha);
    if (gSky.sunUp) {
        drawGlow(gSky.sunX, gSky.sunY, 230, rgba(1.0f, 0.78f, 0.45f, 0.65f), 18);
        drawCircle(gSky.sunX, gSky.sunY, 40, rgb(1.0f, 0.96f, 0.80f));
    }
    if (gSky.moonUp) {
        drawGlow(gSky.moonX, gSky.moonY, 150, rgba(0.62f, 0.74f, 1.0f, 0.55f), 16);
        drawCircle(gSky.moonX, gSky.moonY, 32, rgb(0.94f, 0.96f, 1.0f));
        drawCircle(gSky.moonX - 8, gSky.moonY + 6, 6, rgb(0.84f, 0.87f, 0.95f));
        drawCircle(gSky.moonX + 9, gSky.moonY - 8, 4, rgb(0.84f, 0.87f, 0.95f));
    }
}

void drawFarLayers(float time) {
    // distant clouds drift slowly; parallax 0.35
    cameraBegin(0.35f);
    for (int i = 0; i < 9; ++i) {
        float x = std::fmod(i * 700.0f + time * (6.0f + (i % 3) * 4.0f), 6800.0f) - 500.0f;
        float y = 250.0f + (i % 4) * 55.0f;
        drawCloud(x, y, 0.8f + (i % 3) * 0.3f, withAlpha(lerpColor(T(rgb(1, 1, 1)), gSky.bottom, 0.25f), 1.0f));
    }
    cameraEnd();
    Color haze = gSky.bottom;
    cameraBegin(0.25f);                                                   // far mountains
    drawHillLayerRange(-1700, 3800, 105, 75, 1100, lerpColor(T(rgb(0.46f, 0.56f, 0.82f)), haze, 0.28f),
                       lerpColor(T(rgb(0.46f, 0.56f, 0.82f)), haze, 0.5f), -700);
    cameraEnd();
    cameraBegin(0.5f);                                                    // nearer hills
    drawHillLayerRange(-1700, 4400, 55, 50, 760, lerpColor(T(rgb(0.34f, 0.52f, 0.52f)), haze, 0.25f),
                       lerpColor(T(rgb(0.30f, 0.46f, 0.46f)), haze, 0.4f), -700);
    cameraEnd();
}

// ------------------------------------------------------------- terrain ----
static void drawTerrain() {
    Color grassTop = T(rgb(0.42f, 0.64f, 0.34f)), soil = T(rgb(0.22f, 0.36f, 0.24f));
    glBegin(GL_QUADS);
        for (float x = -900; x < 6000; x += 20) {
            float y0 = groundY(x), y1 = groundY(x + 20);
            glColor4f(soil.r, soil.g, soil.b, 1);      glVertex2f(x, -700); glVertex2f(x + 20, -700);
            glColor4f(grassTop.r, grassTop.g, grassTop.b, 1); glVertex2f(x + 20, y1); glVertex2f(x, y0);
        }
    glEnd();
    // lighter grass edge line
    glBegin(GL_QUADS);
        Color edge = T(rgb(0.55f, 0.78f, 0.42f));
        glColor4f(edge.r, edge.g, edge.b, 1);
        for (float x = -900; x < 6000; x += 20) {
            glVertex2f(x, groundY(x) - 5); glVertex2f(x + 20, groundY(x + 20) - 5);
            glVertex2f(x + 20, groundY(x + 20) + 1); glVertex2f(x, groundY(x) + 1);
        }
    glEnd();
    // dirt path winding down the hill from the mansion gate
    glBegin(GL_QUADS);
        Color path = T(rgb(0.80f, 0.68f, 0.50f));
        glColor4f(path.r, path.g, path.b, 1);
        for (float x = 930; x < 1460; x += 20) {
            glVertex2f(x, groundY(x) - 18); glVertex2f(x + 20, groundY(x + 20) - 18);
            glVertex2f(x + 20, groundY(x + 20) - 5); glVertex2f(x, groundY(x) - 5);
        }
    glEnd();
    // downtown: sidewalk + road
    drawRect(2480, -700, 1620, 812, T(rgb(0.24f, 0.25f, 0.30f)));
    drawRect(2480, 112, 1620, 56, T(rgb(0.66f, 0.66f, 0.68f)));
    drawRect(2480, 108, 1620, 5, T(rgb(0.45f, 0.45f, 0.48f)));
    for (float x = 2490; x < 4100; x += 90) drawRect(x, 40, 48, 5, T(rgb(0.85f, 0.85f, 0.80f)));
    // countryside: dirt road, then rice paddies in front
    drawRect(4100, 112, 1300, 50, T(rgb(0.72f, 0.60f, 0.44f)));
    drawGradientRect(4100, -700, 1300, 812, T(rgb(0.20f, 0.36f, 0.22f)), T(rgb(0.36f, 0.62f, 0.32f)));
    for (int r = 0; r < 9; ++r) {                                      // paddy rows
        float y = 98 - r * 22.0f;
        drawRect(4100, y, 1300, 9, T(rgb(0.50f + 0.02f * r, 0.78f, 0.38f)));
        drawRect(4100, y - 2, 1300, 2, T(rgb(0.18f, 0.34f, 0.2f)));
    }
}

// ----------------------------------------------------------------- trees ----
static void townTree(float x, float yOffset, float s, float tone) {
    float y = groundY(x) + yOffset;
    drawTree(x, y, s, T(rgb(0.22f + 0.1f * tone, 0.52f + 0.1f * tone, 0.30f)));
}

// ------------------------------------------------------------- mansion ----
static void drawAoiRoom(float x, float by, float sunlight, float time) {
    const float w = 260, h = 140, rx = x - w / 2, ry = by + 40;
    Color wall = T(rgb(0.92f, 0.89f, 0.85f)), sky = lerpColor(gSky.bottom, rgb(1, 1, 1), 0.3f);
    drawRect(rx, ry, w, h, wall);
    drawGradientRect(rx, ry, w, 36, T(rgb(0.52f, 0.36f, 0.24f)), T(rgb(0.66f, 0.48f, 0.32f)));   // wooden floor
    for (int i = 1; i < 7; ++i) drawLine(rx + i * w / 7, ry, rx + w / 2 + (i - 3.5f) * 14, ry + 36, T(rgb(0.4f, 0.28f, 0.18f)), 1.0f);
    drawRect(rx, ry + 36, w, 4, T(rgb(0.78f, 0.74f, 0.70f)));                                     // skirting board
    for (int i = 0; i < 3; ++i) {                                                                // three tall arched windows
        float wx = rx + 30 + i * 76;
        drawRect(wx, ry + 52, 52, 66, sky); drawCircle(wx + 26, ry + 118, 26, sky);
        drawRectOutline(wx, ry + 52, 52, 66, T(rgb(0.5f, 0.45f, 0.4f)), 1.5f);
        drawLine(wx + 26, ry + 52, wx + 26, ry + 144, T(rgb(0.5f, 0.45f, 0.4f)), 1.2f);
    }
    drawRect(rx, ry + 36, 18, h - 36, T(rgb(0.55f, 0.18f, 0.22f)));                               // curtains
    drawRect(rx + w - 18, ry + 36, 18, h - 36, T(rgb(0.55f, 0.18f, 0.22f)));
    // grand piano silhouette on the left
    float pb[] = { rx + 24, ry + 38, rx + 112, ry + 38, rx + 118, ry + 62, rx + 96, ry + 70, rx + 24, ry + 66 };
    drawPolygon(pb, 5, T(rgb(0.07f, 0.07f, 0.10f)));
    drawRect(rx + 30, ry + 24, 4, 16, T(rgb(0.07f, 0.07f, 0.10f))); drawRect(rx + 104, ry + 24, 4, 16, T(rgb(0.07f, 0.07f, 0.10f)));
    drawRect(rx + 26, ry + 52, 70, 3, rgb(0.9f, 0.9f, 0.88f));
    // chandelier + a huge empty carpet: everything says "big and quiet"
    drawLine(x, ry + h, x, ry + h - 14, T(rgb(0.4f, 0.35f, 0.2f)), 1.5f);
    drawGlow(x, ry + h - 22, 38, rgba(1.0f, 0.85f, 0.55f, 0.35f + 0.25f * night()), 10);
    drawEllipse(x, ry + h - 22, 12, 6, rgb(0.95f, 0.85f, 0.55f));
    drawEllipse(x + 10, ry + 12, 78, 7, T(rgb(0.60f, 0.25f, 0.30f)), 24);
    // morning sunlight falling across the floor (additive)
    if (sunlight > 0.01f) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        for (int i = 0; i < 3; ++i) {
            float wx = rx + 30 + i * 76, a = 0.30f * sunlight * (0.8f + 0.2f * std::sin(time + i));
            glBegin(GL_QUADS);
                glColor4f(1.0f, 0.85f, 0.55f, a); glVertex2f(wx + 4, ry + 118); glVertex2f(wx + 50, ry + 118);
                glColor4f(1.0f, 0.85f, 0.55f, a * 0.4f); glVertex2f(wx + 118, ry + 4); glVertex2f(wx + 72, ry + 4);
            glEnd();
        }
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
}

static void drawMansion(float x, float by, const TownState& ts) {
    Color wall = T(rgb(0.90f, 0.87f, 0.82f)), roof = T(rgb(0.30f, 0.33f, 0.44f)), line = darken(wall, 0.4f);
    // wings
    drawRect(x - 345, by, 160, 150, wall); drawRect(x + 185, by, 160, 150, wall);
    drawTriangle(x - 360, by + 150, x - 170, by + 150, x - 265, by + 215, roof);
    drawTriangle(x + 170, by + 150, x + 360, by + 150, x + 265, by + 215, roof);
    drawTriangle(x - 265, by + 150, x - 170, by + 150, x - 265, by + 215, darken(roof, 0.82f));
    // main hall
    drawRect(x - 190, by, 380, 232, wall);
    drawRect(x + 150, by, 40, 232, darken(wall, 0.9f));
    float hallRoof[] = { x - 208, by + 232, x + 208, by + 232, x + 150, by + 285, x - 150, by + 285 };
    drawPolygon(hallRoof, 4, roof);
    drawTriangle(x - 70, by + 285, x + 70, by + 285, x, by + 340, T(rgb(0.36f, 0.39f, 0.50f)));   // central gable
    drawCircle(x, by + 300, 9, lit(900) ? rgb(1.0f, 0.85f, 0.5f) : T(rgb(0.9f, 0.9f, 0.92f)));
    drawRectOutline(x - 190, by, 380, 232, line, 1.5f);
    // cutaway room: Aoi's huge, empty, perfect room
    drawAoiRoom(x, by, ts.sunlight, ts.time);
    drawRectOutline(x - 130, by + 40, 260, 140, rgb(0.28f, 0.25f, 0.25f), 3.0f);
    // upper row of small windows + wing windows
    for (int i = 0; i < 5; ++i) drawWindow(x - 150 + i * 62, by + 190, 24, 30, lit(910 + i));
    for (int s = 0; s < 2; ++s)
        for (int r = 0; r < 2; ++r)
            for (int c = 0; c < 2; ++c) drawWindow(x + (s ? 205 : -325) + c * 70, by + 24 + r * 70, 26, 36, lit(920 + s * 4 + r * 2 + c));
    // entrance steps and door on the right wing
    drawRect(x + 245, by, 36, 52, T(rgb(0.35f, 0.22f, 0.16f)));
    drawRect(x + 238, by - 6, 50, 6, T(rgb(0.7f, 0.68f, 0.64f)));
}

static void drawHillGateAndFence() {
    float gx = 1010, gy = groundY(gx);
    for (int s = 0; s < 2; ++s) {
        drawRect(gx - 44 + s * 80, gy - 6, 14, 78, T(rgb(0.78f, 0.75f, 0.70f)));
        drawRect(gx - 47 + s * 80, gy + 70, 20, 8, T(rgb(0.66f, 0.63f, 0.58f)));
    }
    drawLine(gx - 30, gy + 50, gx + 36, gy + 50, T(rgb(0.15f, 0.15f, 0.18f)), 3.0f);
    drawLine(gx - 30, gy + 20, gx + 36, gy + 20, T(rgb(0.15f, 0.15f, 0.18f)), 3.0f);
    for (float x = 960; x < 1000; x += 12) drawLine(x, groundY(x) - 4, x, groundY(x) + 34, T(rgb(0.15f, 0.15f, 0.18f)), 2.0f);
    for (float x = 1050; x < 1100; x += 12) drawLine(x, groundY(x) - 4, x, groundY(x) + 34, T(rgb(0.15f, 0.15f, 0.18f)), 2.0f);
}

// ------------------------------------------------------------ river side ----
static void drawRiver(const TownState& ts) {
    const float x0 = World::RIVER_X0, x1 = World::RIVER_X1, top = World::WATER_TOP;
    // stone bank wall between the grass and the water
    drawRect(x0, top - 2, x1 - x0, 28, T(rgb(0.50f, 0.46f, 0.42f)));
    for (float x = x0; x < x1; x += 34) drawLine(x, top - 2, x, top + 26, T(rgb(0.34f, 0.31f, 0.30f)), 1.0f);
    // water
    Color wt = lerpColor(T(rgb(0.36f, 0.58f, 0.66f)), gSky.bottom, 0.40f), wb = T(rgb(0.08f, 0.20f, 0.30f));
    glBegin(GL_QUADS);                                       // trapezoid: the banks slope down into the river
        glColor4f(wb.r, wb.g, wb.b, 1); glVertex2f(x0 + 140, -700); glVertex2f(x1 - 140, -700);
        glColor4f(wt.r, wt.g, wt.b, 1); glVertex2f(x1, top);        glVertex2f(x0, top);
    glEnd();
    for (int i = 0; i < 26; ++i) {                                     // ripples
        float rx = x0 + std::fmod(i * 97.0f + ts.time * (6.0f + (i % 4) * 3.0f), x1 - x0 - 60);
        float ry = top - 8 - (i % 9) * 11.0f;
        drawLine(rx, ry, rx + 26 + (i % 3) * 10, ry, withAlpha(lerpColor(wt, rgb(1, 1, 1), 0.6f), 0.35f), 1.5f);
    }
}

static void drawBoat(const TownState& ts) {
    float bx = ts.boatX, by = World::WATER_TOP - 22 + boatBobY(ts.time);
    glPushMatrix(); glTranslatef(bx, by, 0); glRotatef(std::sin(ts.time * 0.9f) * 1.4f, 0, 0, 1);
    float hull[] = { -66, 22, 66, 22, 50, -2, -50, -2 };
    drawPolygon(hull, 4, T(rgb(0.40f, 0.26f, 0.17f)));
    drawLine(-62, 14, 62, 14, T(rgb(0.25f, 0.16f, 0.10f)), 2.0f);
    drawPolygonOutline(hull, 4, T(rgb(0.15f, 0.1f, 0.07f)), 1.5f);
    if (ts.lantern) {                                                   // Ren's lantern on a pole at the bow
        drawLine(58, 22, 58, 86, T(rgb(0.25f, 0.18f, 0.12f)), 3.0f);
        drawLine(58, 86, 40, 86, T(rgb(0.25f, 0.18f, 0.12f)), 3.0f);
        drawGlow(40, 74, 130, rgba(1.0f, 0.72f, 0.35f, 0.8f), 16);        // Ren's lantern: the warm heart of the scene
        drawRect(34, 64, 12, 18, rgb(1.0f, 0.84f, 0.48f));
        drawRectOutline(34, 64, 12, 18, rgb(0.3f, 0.15f, 0.08f), 1.5f);
        drawLine(34, 82, 46, 82, rgb(0.3f, 0.15f, 0.08f), 2.5f);
    }
    glPopMatrix();
    if (ts.lantern) {                                                   // reflection streak on the water
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        for (int i = 0; i < 6; ++i) {
            float wob = std::sin(ts.time * 2.0f + i) * 5;
            drawEllipse(bx + 40 + wob, World::WATER_TOP - 40 - i * 15.0f, 7 + i * 1.5f, 3.5f, rgba(1.0f, 0.75f, 0.4f, 0.28f - i * 0.035f), 16);
        }
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
}

static void drawDock() {
    const float x0 = World::DOCK_X0, x1 = World::DOCK_X1, top = World::WATER_TOP + 4;
    for (float x = x0 + 8; x < x1; x += 46) drawRect(x, 30, 8, top - 30, T(rgb(0.28f, 0.19f, 0.13f)));       // posts into the water
    drawRect(x0, top, x1 - x0, 8, T(rgb(0.52f, 0.37f, 0.25f)));
    for (float x = x0; x < x1; x += 22) drawLine(x, top, x, top + 8, T(rgb(0.32f, 0.22f, 0.14f)), 1.0f);
    drawRectOutline(x0, top, x1 - x0, 8, T(rgb(0.2f, 0.13f, 0.08f)), 1.2f);
}

static void drawHut(float x, float by, const TownState& ts) {
    Color wood = T(rgb(0.52f, 0.38f, 0.26f)), dark = T(rgb(0.30f, 0.20f, 0.14f));
    // interior (cutaway front)
    drawRect(x - 62, by + 4, 124, 72, T(rgb(0.62f, 0.46f, 0.32f)));
    for (int i = 1; i < 9; ++i) drawLine(x - 62 + i * 13.8f, by + 4, x - 62 + i * 13.8f, by + 76, T(rgb(0.45f, 0.32f, 0.22f)), 1.0f);
    drawRect(x - 62, by + 4, 124, 8, T(rgb(0.40f, 0.28f, 0.19f)));                                // floor
    // shelf with the late wife's photo
    drawRect(x - 56, by + 50, 40, 4, dark);
    drawRect(x - 50, by + 54, 11, 14, rgb(0.92f, 0.85f, 0.70f)); drawRectOutline(x - 50, by + 54, 11, 14, dark, 1.5f);
    drawCircle(x - 44.5f, by + 63, 2.6f, rgb(0.35f, 0.25f, 0.2f)); drawTriangle(x - 48, by + 55, x - 41, by + 55, x - 44.5f, by + 60, rgb(0.5f, 0.4f, 0.5f));
    drawRect(x - 30, by + 54, 8, 9, rgb(0.75f, 0.6f, 0.4f));                                       // tea tin
    // stove glow
    drawRect(x - 58, by + 12, 22, 20, dark); drawGlow(x - 47, by + 22, 26, rgba(1.0f, 0.55f, 0.2f, 0.5f), 8);
    // hanging lamp
    drawLine(x + 28, by + 76, x + 28, by + 62, dark, 1.5f);
    drawGlow(x + 28, by + 58, 52, rgba(1.0f, 0.8f, 0.45f, 0.35f + 0.35f * night()), 12);
    drawCircle(x + 28, by + 58, 4, rgb(1.0f, 0.9f, 0.6f));
    // table with cups
    drawRect(x - 12, by + 28, 52, 4, dark); drawRect(x - 8, by + 12, 4, 17, dark); drawRect(x + 33, by + 12, 4, 17, dark);
    for (int c = 0; c < ts.hutCups; ++c) {
        float cx = x + 4 + c * 20.0f;
        drawRect(cx, by + 32, 9, 8, rgb(0.95f, 0.95f, 0.92f)); drawRectOutline(cx, by + 32, 9, 8, rgb(0.4f, 0.4f, 0.45f), 1.2f);
        drawRect(cx + 9, by + 34, 3, 4, rgb(0.95f, 0.95f, 0.92f));
        for (int k = 0; k < 3; ++k) {                                                              // steam
            float ph = std::fmod(ts.time * 9.0f + k * 6.0f, 18.0f);
            drawCircle(cx + 4.5f + std::sin(ts.time * 1.6f + k) * 2.5f, by + 42 + ph, 2.2f + ph * 0.08f, rgba(1, 1, 1, 0.5f * (1 - ph / 18.0f)), 12);
        }
    }
    // frame, roof
    drawRect(x - 68, by, 8, 80, wood); drawRect(x + 60, by, 8, 80, wood);
    drawRect(x - 72, by + 76, 144, 6, dark);
    float roof[] = { x - 96, by + 78, x + 96, by + 78, x + 4, by + 134, x - 4, by + 134 };
    drawPolygon(roof, 4, T(rgb(0.46f, 0.30f, 0.24f)));
    float roofShade[] = { x, by + 78, x + 96, by + 78, x + 4, by + 134, x, by + 134 };
    drawPolygon(roofShade, 4, T(rgb(0.38f, 0.24f, 0.20f)));
    for (int i = 1; i < 4; ++i) drawLine(x - (94 - i * 21.0f), by + 78 + i * 13, x + (94 - i * 21.0f), by + 78 + i * 13, T(rgb(0.30f, 0.19f, 0.15f)), 1.0f);
    drawPolygonOutline(roof, 4, dark, 1.5f);
    // net, barrel, crate
    for (int i = 0; i < 4; ++i) drawLine(x + 70 + i * 4, by + 74, x + 70 + i * 4, by + 20, T(rgb(0.6f, 0.55f, 0.4f)), 1.0f);
    drawRect(x + 78, by, 22, 26, T(rgb(0.42f, 0.30f, 0.20f))); drawRect(x + 78, by + 8, 22, 3, dark);
    drawRect(x + 104, by, 28, 14, T(rgb(0.58f, 0.44f, 0.30f)));
    // chimney smoke (only when the stove is lit: always, it is a fisherman's hut)
    drawRect(x + 52, by + 108, 10, 24, dark);
    for (int k = 0; k < 4; ++k) {
        float ph = std::fmod(ts.time * 12.0f + k * 14.0f, 56.0f);
        drawCircle(x + 57 + ph * 0.25f + std::sin(ts.time + k) * 3, by + 136 + ph, 5 + ph * 0.12f, rgba(0.85f, 0.85f, 0.9f, 0.4f * (1 - ph / 56.0f)), 14);
    }
}

// --------------------------------------------------------------- downtown ----
static void drawBakery(float x, float by, const TownState& ts) {
    Color wall = T(rgb(0.90f, 0.72f, 0.54f)), dark = T(rgb(0.30f, 0.18f, 0.12f));
    drawRect(x - 88, by, 176, 120, wall);
    drawRect(x + 50, by, 38, 120, darken(wall, 0.88f));
    drawRect(x - 92, by + 120, 184, 8, T(rgb(0.55f, 0.32f, 0.24f)));
    // sign board with a loaf of bread
    drawRect(x - 56, by + 132, 112, 26, T(rgb(0.45f, 0.28f, 0.20f))); drawRectOutline(x - 56, by + 132, 112, 26, dark, 1.5f);
    drawEllipse(x, by + 145, 26, 9, rgb(0.86f, 0.6f, 0.3f)); for (int i = -1; i <= 1; ++i) drawLine(x + i * 12 - 3, by + 150, x + i * 12 + 3, by + 140, rgb(0.6f, 0.38f, 0.18f), 2.0f);
    // shop window: always warm and lit; a baker kneads inside
    drawGlow(x - 24, by + 48, 120, rgba(1.0f, 0.75f, 0.4f, 0.35f + 0.3f * night()), 14);
    drawRect(x - 68, by + 18, 88, 62, rgb(1.0f, 0.84f, 0.52f));
    drawCircle(x - 30, by + 52, 8.5f, T(rgb(0.30f, 0.20f, 0.18f)));                              // baker's head
    drawRect(x - 42, by + 18, 24, 32, T(rgb(0.34f, 0.24f, 0.2f)));                                // baker's body
    drawRect(x - 36, by + 60, 12, 6, rgb(1, 1, 1));                                               // hat
    glPushMatrix(); glTranslatef(-18 + x, by + 44, 0); glRotatef(-35 + std::sin(ts.time * 3.0f) * 14.0f, 0, 0, 1);
    drawRect(-3, -22, 6, 22, T(rgb(0.34f, 0.24f, 0.2f))); glPopMatrix();                          // kneading arm
    drawRect(x - 68, by + 18, 88, 8, T(rgb(0.55f, 0.35f, 0.22f)));                                // counter
    for (int i = 0; i < 4; ++i) drawEllipse(x - 56 + i * 19.0f, by + 32, 8, 4.5f, rgb(0.88f, 0.62f, 0.3f));   // loaves
    drawRectOutline(x - 68, by + 18, 88, 62, dark, 2.0f);
    // door + striped awning
    drawRect(x + 36, by, 34, 70, T(rgb(0.45f, 0.28f, 0.2f))); drawCircle(x + 63, by + 34, 2, rgb(0.9f, 0.8f, 0.4f));
    for (int i = 0; i < 8; ++i)
        drawRect(x - 90 + i * 22.5f, by + 84, 22.5f, 18, (i % 2) ? T(rgb(0.96f, 0.93f, 0.88f)) : T(rgb(0.85f, 0.25f, 0.25f)));
    drawRectOutline(x - 90, by + 84, 180, 18, dark, 1.3f);
    // chimney + smoke
    drawRect(x + 60, by + 128, 16, 36, T(rgb(0.50f, 0.34f, 0.28f)));
    for (int k = 0; k < 5; ++k) {
        float ph = std::fmod(ts.time * 14.0f + k * 14.0f, 70.0f);
        drawCircle(x + 68 + ph * 0.35f + std::sin(ts.time * 0.9f + k) * 4, by + 168 + ph, 6 + ph * 0.14f, rgba(0.92f, 0.92f, 0.95f, 0.45f * (1 - ph / 70.0f)), 14);
    }
}

static void drawOffice(float x, float by, float w, float h, int id, Color wallBase) {
    Color wall = T(wallBase);
    drawRect(x - w / 2, by, w, h, wall);
    drawRect(x + w * 0.28f, by, w * 0.22f, h, darken(wall, 0.88f));
    drawRectOutline(x - w / 2, by, w, h, darken(wall, 0.4f), 1.5f);
    int cols = (int)(w / 30), rows = (int)((h - 30) / 40);
    bool workHours = gSky.hour > 7.8f && gSky.hour < 18.6f;
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c) {
            float wx = x - w / 2 + 10 + c * ((w - 20) / cols), wy = by + 24 + r * 40;
            int wid = id * 40 + r * 7 + c;
            bool office = workHours && (wid % 5 != 0);
            if (office) {                                                    // cool white office light
                drawRect(wx, wy, (w - 20) / cols - 8, 24, rgb(0.82f, 0.92f, 1.0f));
                drawRectOutline(wx, wy, (w - 20) / cols - 8, 24, darken(wall, 0.4f), 1.0f);
            } else drawWindow(wx, wy, (w - 20) / cols - 8, 24, lit(wid));
        }
    drawRect(x - w / 2 - 4, by + h, w + 8, 7, darken(wall, 0.7f));
    drawRect(x - 14, by + h + 7, 28, 12, T(rgb(0.55f, 0.57f, 0.6f)));       // rooftop unit
    drawLine(x + w / 3, by + h + 7, x + w / 3, by + h + 40, T(rgb(0.3f, 0.3f, 0.35f)), 1.5f);
}

static void drawSchool(float x, float by) {
    Color wall = T(rgb(0.90f, 0.86f, 0.74f)), roof = T(rgb(0.45f, 0.30f, 0.28f));
    drawRect(x - 170, by, 340, 128, wall);
    drawRect(x + 100, by, 70, 128, darken(wall, 0.9f));
    drawRect(x - 176, by + 128, 352, 8, roof);
    drawRect(x - 34, by + 136, 68, 84, wall); drawRectOutline(x - 34, by + 136, 68, 84, darken(wall, 0.4f), 1.5f);   // clock tower
    drawTriangle(x - 44, by + 220, x + 44, by + 220, x, by + 262, roof);
    drawCircle(x, by + 184, 24, rgb(0.97f, 0.97f, 0.94f)); drawCircleOutline(x, by + 184, 24, T(rgb(0.3f, 0.25f, 0.2f)), 2.0f);
    float hr = std::fmod(gSky.hour, 12.0f), mn = (gSky.hour - std::floor(gSky.hour)) * 60.0f;     // the clock tells the story time
    float ah = Config::PI / 2 - hr / 12.0f * 2 * Config::PI, am = Config::PI / 2 - mn / 60.0f * 2 * Config::PI;
    drawLine(x, by + 184, x + 11 * std::cos(ah), by + 184 + 11 * std::sin(ah), rgb(0.1f, 0.1f, 0.12f), 2.5f);
    drawLine(x, by + 184, x + 18 * std::cos(am), by + 184 + 18 * std::sin(am), rgb(0.1f, 0.1f, 0.12f), 1.8f);
    for (int i = 0; i < 6; ++i) { if (i == 2 || i == 3) continue; drawWindow(x - 150 + i * 55, by + 74, 32, 36, lit(300 + i)); drawWindow(x - 150 + i * 55, by + 20, 32, 36, lit(310 + i)); }
    drawRect(x - 14, by, 28, 44, T(rgb(0.4f, 0.28f, 0.2f)));
    drawLine(x + 130, by + 128, x + 130, by + 230, T(rgb(0.7f, 0.7f, 0.72f)), 2.5f);            // flagpole
    drawRect(x + 130, by + 206, 28, 18, rgb(0.95f, 0.95f, 0.95f)); drawCircle(x + 144, by + 215, 5.5f, rgb(0.85f, 0.15f, 0.2f));
}

// ---------------------------------------------------------- countryside ----
static void drawFarmhouse(float x, float by, const TownState& ts) {
    Color wall = T(rgb(0.94f, 0.91f, 0.82f)), wood = T(rgb(0.38f, 0.26f, 0.18f)), roof = T(rgb(0.30f, 0.34f, 0.42f));
    drawRect(x - 130, by, 260, 84, wall);
    for (int i = 0; i <= 5; ++i) drawRect(x - 130 + i * 51.0f, by, 4, 84, wood);                    // wooden posts
    drawRect(x - 130, by + 78, 260, 6, wood);
    for (int i = 0; i < 5; ++i) {                                                                    // sliding screens
        bool glow = ts.grannyLight && i == 2;
        drawRect(x - 124 + i * 51.0f, by + 8, 43, 66, glow ? rgb(1.0f, 0.86f, 0.52f) : T(rgb(0.97f, 0.95f, 0.88f)));
        for (int j = 1; j < 3; ++j) drawLine(x - 124 + i * 51.0f + j * 14.3f, by + 8, x - 124 + i * 51.0f + j * 14.3f, by + 74, wood, 1.0f);
        if (glow) drawGlow(x - 124 + i * 51.0f + 21, by + 40, 80, rgba(1.0f, 0.78f, 0.42f, 0.5f), 12);
    }
    drawRect(x - 138, by - 4, 276, 8, T(rgb(0.55f, 0.40f, 0.28f)));                                 // porch
    float r1[] = { x - 160, by + 84, x + 160, by + 84, x + 110, by + 150, x - 110, by + 150 };
    drawPolygon(r1, 4, roof);
    float r2[] = { x, by + 84, x + 160, by + 84, x + 110, by + 150, x, by + 150 };
    drawPolygon(r2, 4, darken(roof, 0.84f));
    drawRect(x - 112, by + 150, 224, 8, darken(roof, 0.7f));
    for (int i = 1; i < 5; ++i) drawLine(x - (156 - i * 11.0f * 1.0f) + 0, by + 84 + i * 13, x + (156 - i * 11.0f), by + 84 + i * 13, darken(roof, 0.65f), 1.0f);
    drawPolygonOutline(r1, 4, darken(roof, 0.5f), 1.5f);
}

static void drawFarmGate(float x) {
    float gy = groundY(x) - 2;
    Color wood = T(rgb(0.36f, 0.24f, 0.16f));
    drawRect(x - 36, gy, 9, 78, wood); drawRect(x + 27, gy, 9, 78, wood);
    drawRect(x - 46, gy + 70, 92, 9, wood);
    drawTriangle(x - 52, gy + 79, x + 52, gy + 79, x, gy + 96, T(rgb(0.30f, 0.33f, 0.4f)));
    for (int s = -1; s <= 1; s += 2)
        for (int i = 0; i < 5; ++i) drawLine(x + s * (60 + i * 16), gy, x + s * (60 + i * 16), gy + 34, wood, 2.0f);   // fence
    drawLine(x + 52, gy + 28, x + 132, gy + 28, wood, 2.0f); drawLine(x - 52, gy + 28, x - 132, gy + 28, wood, 2.0f);
}

// ----------------------------------------------------------- rails + train ----
static void drawTrack() {
    const float x0 = 2350, x1 = 5400, y = World::TRACK_Y;
    drawRect(x0, y - 14, x1 - x0, 16, T(rgb(0.40f, 0.37f, 0.35f)));                                  // ballast
    for (float x = x0; x < x1; x += 18) drawRect(x, y - 12, 8, 5, T(rgb(0.30f, 0.22f, 0.16f)));       // sleepers
    drawLine(x0, y, x1, y, T(rgb(0.62f, 0.62f, 0.66f)), 2.5f);
    drawLine(x0, y - 7, x1, y - 7, T(rgb(0.52f, 0.52f, 0.56f)), 2.0f);
}

static void drawTrain(float headX) {
    const float carW = 152, gap = 6, y = World::TRACK_Y + 4, h = 70;
    const int cars = 7;
    Color body = T(rgb(0.93f, 0.94f, 0.96f)), stripe = T(rgb(0.15f, 0.45f, 0.75f)), dark = T(rgb(0.18f, 0.2f, 0.26f));
    for (int c = 0; c < cars; ++c) {
        float x1 = headX - c * (carW + gap), x0 = x1 - carW;
        if (c == 0) {                                                                                 // rounded nose
            float nose[] = { x0, y + 10, x1 - 4, y + 10, x1 + 14, y + 26, x1 + 14, y + 50, x1 - 6, y + h, x0, y + h };
            drawPolygon(nose, 6, body); drawPolygonOutline(nose, 6, dark, 1.5f);
        } else { drawRect(x0, y + 10, carW, h - 10, body); drawRectOutline(x0, y + 10, carW, h - 10, dark, 1.5f); }
        drawRect(x0, y + 22, carW + (c == 0 ? 12 : 0), 7, stripe);                                   // blue stripe
        for (int w = 0; w < 5; ++w) {                                                                 // lit windows
            float wx = x0 + 12 + w * 28;
            drawRect(wx, y + 38, 20, 22, rgb(1.0f, 0.94f, 0.74f));
            drawRectOutline(wx, y + 38, 20, 22, dark, 1.2f);
            if (night() > 0.4f) drawGlow(wx + 10, y + 49, 24, rgba(1.0f, 0.85f, 0.5f, 0.35f), 6);
        }
        drawRect(x0 + 4, y + 8, carW - 8, 4, dark);                                                   // underframe
        drawCircle(x0 + 24, y + 8, 7, dark); drawCircle(x1 - 24, y + 8, 7, dark);                     // wheels
        drawCircle(x0 + 24, y + 8, 2.5f, rgb(0.6f, 0.6f, 0.65f)); drawCircle(x1 - 24, y + 8, 2.5f, rgb(0.6f, 0.6f, 0.65f));
        drawLine(x0 + 40, y + h, x0 + 70, y + h + 12, dark, 1.5f);                                    // pantograph
        drawLine(x0 + 70, y + h + 12, x0 + 100, y + h, dark, 1.5f);
    }
    drawCircle(headX + 8, y + 26, 4, rgb(1.0f, 0.95f, 0.7f));                                         // headlight
    drawGlow(headX + 8, y + 26, 40, rgba(1.0f, 0.95f, 0.7f, 0.5f), 8);
}

// -------------------------------------------------------- crossing gates ----
static void drawCrossingSide(float x, float side, const TownState& ts) {
    Color dark = T(rgb(0.15f, 0.15f, 0.18f));
    float py = 150;
    drawRect(x - 4, py - 4, 8, 96, dark);                                     // post
    drawRect(x - 12, py - 6, 24, 6, dark);
    // signal head: two lamps blinking in turn, plus the cross-buck
    drawRect(x - 12 - side * 0, py + 82, 24, 14, dark);
    bool a = ts.signalOn && std::fmod(ts.time * 2.2f, 2.0f) < 1.0f, b = ts.signalOn && !a;
    drawCircle(x - 6, py + 89, 4.2f, a ? rgb(1, 0.2f, 0.18f) : T(rgb(0.35f, 0.1f, 0.1f)));
    drawCircle(x + 6, py + 89, 4.2f, b ? rgb(1, 0.2f, 0.18f) : T(rgb(0.35f, 0.1f, 0.1f)));
    if (a) drawGlow(x - 6, py + 89, 34, rgba(1, 0.2f, 0.15f, 0.7f), 10);
    if (b) drawGlow(x + 6, py + 89, 34, rgba(1, 0.2f, 0.15f, 0.7f), 10);
    drawLine(x - 14, py + 112, x + 14, py + 130, rgb(0.95f, 0.95f, 0.95f), 4.0f);
    drawLine(x - 14, py + 130, x + 14, py + 112, rgb(0.95f, 0.95f, 0.95f), 4.0f);
    // barrier arm: pivots at the post; 80 degrees up when open, 0 (horizontal) when closed
    glPushMatrix();
    glTranslatef(x, py + 60, 0);
    glScalef(side, 1, 1);                                                      // mirror for the right-hand gate
    glRotatef(lerp(82.0f, 0.0f, smoothstep01(ts.arms)), 0, 0, 1);
    for (int i = 0; i < 7; ++i) drawRect(i * 12.5f, -3, 12.5f, 6, (i % 2) ? rgb(0.97f, 0.97f, 0.97f) : rgb(0.9f, 0.15f, 0.15f));
    drawRectOutline(0, -3, 87.5f, 6, dark, 1.2f);
    glPopMatrix();
}

// ------------------------------------------------------------ whole town ----
float boatBobY(float time) { return std::sin(time * 1.1f) * 2.0f; }
float boatDeckY() { return World::WATER_TOP - 22 + 16.0f; }

void drawBoatFront(const TownState& ts) {          // gunwale in front of the rider's feet
    glPushMatrix(); glTranslatef(ts.boatX, World::WATER_TOP - 22 + boatBobY(ts.time), 0); glRotatef(std::sin(ts.time * 0.9f) * 1.4f, 0, 0, 1);
    float rim[] = { -66, 22, 66, 22, 62, 10, -62, 10 };
    drawPolygon(rim, 4, T(rgb(0.48f, 0.32f, 0.21f)));
    drawPolygonOutline(rim, 4, T(rgb(0.15f, 0.1f, 0.07f)), 1.5f);
    glPopMatrix();
}

static void drawMistLayer(const TownState& ts, float amount, float yBase, int count, float seed) {
    if (amount <= 0.01f) return;
    Color m = lerpColor(gSky.bottom, rgb(0.9f, 0.92f, 1.0f), 0.5f);
    for (int i = 0; i < count; ++i) {
        float x = World::RIVER_X0 - 200 + std::fmod(i * 330.0f + seed + ts.time * (5.0f + (i % 3) * 3.0f), World::RIVER_X1 - World::RIVER_X0 + 400);
        float y = yBase + (i % 4) * 9.0f;
        drawSoftEllipse(x, y, 270, 34, withAlpha(m, 0.30f * amount), 12);
    }
}

void drawTownBack(const TownState& ts) {
    drawTerrain();
    // back row of small town blocks, dimmer, to give the downtown some depth
    for (int i = 0; i < 9; ++i) {
        float x = 2520 + i * 175.0f; float h = 110 + (i * 37) % 90;
        if (x > 3150 && x < 3470) continue;                                        // keep the crossing area open
        drawRect(x - 60, World::BUILDING_Y + 40, 120, h, lerpColor(T(rgb(0.62f, 0.64f, 0.72f)), gSky.bottom, 0.35f));
    }
    // hilltop
    drawHillGateAndFence();
    townTree(120, 4, 1.15f, 0.2f); townTree(250, -4, 0.95f, 0.5f); townTree(1120, -6, 0.85f, 0.1f); townTree(1215, -6, 0.7f, 0.6f);
    drawMansion(World::MANSION_X, groundY(World::MANSION_X) - 4, ts);
    drawLamp(1040, groundY(1040) - 4, 1.0f, night() > 0.45f); drawLamp(1300, groundY(1300) - 4, 1.0f, night() > 0.45f);
    // riverside (houses sit behind the bank)
    drawHouse(1560, groundY(1560) + 8, 110, 70, T(rgb(0.86f, 0.78f, 0.62f)), T(rgb(0.40f, 0.32f, 0.30f)), lit(601));
    drawHouse(2320, groundY(2320) + 8, 120, 76, T(rgb(0.78f, 0.74f, 0.66f)), T(rgb(0.34f, 0.38f, 0.48f)), lit(602));
    townTree(1450, 6, 1.0f, 0.3f); townTree(2440, 6, 1.15f, 0.0f);
    drawRiver(ts);
    drawHut(World::HUT_X, groundY(World::HUT_X) + 6, ts);
    drawDock();
    drawBoat(ts);
    // downtown
    drawBakery(World::BAKERY_X, World::BUILDING_Y, ts);
    drawOffice(2900, World::BUILDING_Y, 150, 330, 1, rgb(0.72f, 0.76f, 0.84f));
    drawOffice(3085, World::BUILDING_Y, 120, 250, 2, rgb(0.80f, 0.74f, 0.70f));
    drawSchool(World::SCHOOL_X, World::BUILDING_Y);
    drawTrack();
    if (ts.trainOn) drawTrain(ts.trainHeadX);
    for (int i = 0; i < 6; ++i) { float lx = 2540 + i * 290.0f; if (lx > 3200 && lx < 3420) continue; drawLamp(lx, 152, 0.9f, night() > 0.45f); }
    // countryside
    townTree(4180, 8, 1.2f, 0.4f); townTree(4350, 8, 0.95f, 0.1f); townTree(5080, 8, 1.3f, 0.5f); townTree(5200, 8, 0.9f, 0.2f);
    drawFarmhouse(World::FARMHOUSE_X, groundY(World::FARMHOUSE_X) + 4, ts);
    drawFarmGate(World::GATE_X);
    drawMistLayer(ts, ts.mist, World::WATER_TOP + 6, 8, 0);
}

void drawTownFront(const TownState& ts) {
    drawCrossingSide(World::CROSSING_X - 85, 1.0f, ts);
    drawCrossingSide(World::CROSSING_X + 85, -1.0f, ts);
    drawMistLayer(ts, ts.mist * 0.8f, World::WATER_TOP - 14, 6, 120);
}
