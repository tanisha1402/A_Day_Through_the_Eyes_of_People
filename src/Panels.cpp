#include "Panels.h"
#include "Characters.h"
#include "Environment.h"
#include "Particles.h"
#include "TimeOfDay.h"
#include "Viewports.h"
#include <GL/freeglut.h>
#include <cmath>

static Color C(float r, float g, float b) { return rgb(r, g, b); }
static float hz(int n) {                                   // repeatable pseudo-random 0..1 from an integer
    unsigned u = (unsigned)n * 2654435761u + 97u; u ^= u >> 15; u *= 2246822519u; u ^= u >> 13;
    return (u & 0xFFFF) / 65535.0f;
}

// ------------------------------------------------------- panel particles ----
static ParticlePool gDust, gNotes, gDrips, gDripRipples;
static RainField gRainAoi, gRainHana, gRainKenji;
static float gNoteTimer = 0, gDripTimer = 0, gBucket = 0;

void panelsReset(int scene) {
    seedRandom(2024u + scene);
    poolInit(gDust, 48); poolInit(gNotes, 20); poolInit(gDrips, 12); poolInit(gDripRipples, 16);
    PanelLayout g = layoutGrid2x2();
    rainInit(gRainAoi,   g.r[0].w, g.r[0].h, 0, 0, 260, 260);       // landMin/Max unused: drops hit the window glass
    rainInit(gRainHana,  g.r[1].w, g.r[1].h, 12, 74, 260, 300);
    rainInit(gRainKenji, g.r[3].w, g.r[3].h, 12, 74, 260, 300);
    gRainAoi.landMin = 20; gRainAoi.landMax = 200;
    gNoteTimer = gDripTimer = 0; gBucket = 0.25f;
    if (scene == 5) for (int i = 0; i < 90; ++i) panelsUpdate(1.0f / 60.0f, 5, 0.0f);      // rain is already falling when the chapter starts
    gBucket = 0.25f;
}

void panelsUpdate(float dt, int scene, float t) {
    if (scene == 3) {
        gNoteTimer -= dt;
        if (gNoteTimer <= 0) {                                       // Aoi's music floats up
            gNoteTimer = 0.55f + rand01() * 0.5f;
            Particle* n = poolSpawn(gNotes);
            if (n) { n->x = 150 + rand01() * 90; n->y = 250; n->vx = 6 + rand01() * 8; n->vy = 26 + rand01() * 14; n->life = n->maxLife = 5.5f; n->size = 0.8f + rand01() * 0.5f; }
        }
        if (poolAlive(gDust) < 30 && rand01() < 0.25f) {            // dust drifting in the classroom's sunbeam
            Particle* d = poolSpawn(gDust);
            if (d) { d->x = 60 + rand01() * 220; d->y = 90 + rand01() * 250; d->vx = 3 + rand01() * 4; d->vy = -2 + rand01() * 4; d->life = d->maxLife = 6; d->size = 1.5f + rand01() * 1.5f; }
        }
        poolUpdate(gNotes, dt, 0); poolUpdate(gDust, dt, 0);
    }
    if (scene == 5) {
        rainUpdate(gRainAoi, dt); rainUpdate(gRainHana, dt); rainUpdate(gRainKenji, dt);
        gDripTimer -= dt;
        if (gDripTimer <= 0) {                                       // Ren's leaking roof
            gDripTimer = 0.45f + rand01() * 0.4f;
            Particle* d = poolSpawn(gDrips);
            if (d) { d->x = 128; d->y = 252; d->vx = 0; d->vy = -20; d->life = d->maxLife = 3; d->size = 2.4f; }
        }
        for (size_t i = 0; i < gDrips.p.size(); ++i) {
            Particle& d = gDrips.p[i];
            if (d.life <= 0) continue;
            d.vy -= 700 * dt; d.y += d.vy * dt;
            if (d.y <= 98 + gBucket * 12) {                          // lands in the bucket: ripple, water level creeps up
                d.life = 0; gBucket = std::fmin(1.0f, gBucket + 0.012f);
                Particle* r = poolSpawn(gDripRipples);
                if (r) { r->x = d.x; r->y = 98 + gBucket * 12; r->life = r->maxLife = 0.6f; r->size = 3; }
            }
        }
        poolUpdate(gDripRipples, dt, 0);
    }
    (void)t;
}

float lightningLevel(float t) {
    static const float flashAt[] = { 2.6f, 6.2f, 6.55f, 10.4f, 13.1f };       // a double flash at 6.2 s
    float level = 0;
    for (int i = 0; i < 5; ++i) {
        float d = t - flashAt[i];
        if (d >= 0 && d < 0.6f) level = std::fmax(level, std::exp(-d * 8.0f));  // sudden flash, quick fade
    }
    return clamp01(level);
}

// ---------------------------------------------------------------- shared ----
void drawWallClock(float cx, float cy, float r, float hour, float sec, Color face, Color rim, Color ink) {
    drawCircle(cx, cy, r + 4, rim); drawCircle(cx, cy, r, face);
    for (int i = 0; i < 12; ++i) {
        float a = i * Config::PI / 6;
        drawLine(cx + (r - 6) * std::cos(a), cy + (r - 6) * std::sin(a), cx + (r - 2) * std::cos(a), cy + (r - 2) * std::sin(a), ink, 1.5f);
    }
    float hr = std::fmod(hour, 12.0f), mn = (hour - std::floor(hour)) * 60.0f;
    float ah = Config::PI / 2 - hr / 12.0f * 2 * Config::PI, am = Config::PI / 2 - mn / 60.0f * 2 * Config::PI;
    float as = Config::PI / 2 - std::floor(sec) / 60.0f * 2 * Config::PI;                // the second hand TICKS
    drawLine(cx, cy, cx + r * 0.5f * std::cos(ah), cy + r * 0.5f * std::sin(ah), ink, 3.0f);
    drawLine(cx, cy, cx + r * 0.78f * std::cos(am), cy + r * 0.78f * std::sin(am), ink, 2.0f);
    drawLine(cx, cy, cx + r * 0.85f * std::cos(as), cy + r * 0.85f * std::sin(as), C(0.85f, 0.15f, 0.15f), 1.2f);
    drawCircle(cx, cy, 2.5f, ink);
}

static void drawCloudShared(float panelX, float cloudX, float cloudY) {      // the same cloud, drawn relative to THIS panel
    drawCloud(cloudX - panelX, cloudY, 1.5f, C(1, 1, 1));
}

static void skyBackdrop(float w, float h) { drawGradientRect(0, 0, w, h, gSky.bottom, gSky.top); }

static void flashOverlay(float w, float h, float flash) {
    if (flash > 0.01f) drawRect(0, 0, w, h, rgba(0.88f, 0.92f, 1.0f, 0.55f * flash));
}

static void drawBolt(float x, float y0, float y1, int seed, float a) {
    if (a < 0.02f) return;
    float px = x, py = y0;
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    for (int pass = 0; pass < 2; ++pass) {
        px = x; py = y0;
        glLineWidth(pass == 0 ? 7.0f : 2.0f);
        glBegin(GL_LINES);
        glColor4f(0.8f, 0.85f, 1.0f, pass == 0 ? a * 0.35f : a);
        for (int i = 0; i < 9; ++i) {
            float nx = px + (hz(seed + i * 3) - 0.5f) * 46.0f, ny = py - (y0 - y1) / 9.0f;
            glVertex2f(px, py); glVertex2f(nx, ny); px = nx; py = ny;
        }
        glEnd();
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

static void drawSkyline(float w, float baseY, Color col, int seed, int count, float minH, float maxH, bool windows) {
    float bw = w / count;
    for (int i = 0; i < count; ++i) {
        float bh = minH + hz(seed + i) * (maxH - minH);
        drawRect(i * bw, baseY, bw - 2, bh, col);
        if (windows)
            for (int r = 0; r < (int)(bh / 26); ++r)
                for (int c = 0; c < (int)(bw / 18); ++c)
                    if (hz(seed * 7 + i * 31 + r * 5 + c) > 0.62f) drawRect(i * bw + 6 + c * 18, baseY + 10 + r * 26, 8, 12, rgba(1.0f, 0.88f, 0.55f, 0.8f));
    }
}

static PersonStyle classmate(int k) {
    PersonStyle s = makeHana();
    s.prop = PROP_NONE;
    if (k == 0) { s.hair = C(0.12f, 0.1f, 0.12f); s.hairStyle = HAIR_SHORT; s.skirt = false; s.uniform = false; s.top = C(0.96f, 0.96f, 0.98f); s.bottom = C(0.16f, 0.2f, 0.4f); s.height = 0.95f; }
    else        { s.hair = C(0.5f, 0.3f, 0.15f); s.hairStyle = HAIR_BOB; s.height = 0.9f; }
    return s;
}

static void drawChairDesk(float x, float feetY, float s, bool desk) {
    Color wood = C(0.62f, 0.42f, 0.26f), dark = C(0.35f, 0.23f, 0.15f);
    drawRect(x - 34 * s * 0.55f, feetY + 17 * s, 22 * s, 3 * s, wood);                       // seat
    drawRect(x - 34 * s * 0.55f, feetY + 17 * s, 3 * s, 32 * s, wood);                       // back
    drawRect(x - 30 * s * 0.55f, feetY, 2 * s, 17 * s, dark);
    if (desk) {
        drawRect(x + 14 * s, feetY + 31 * s, 52 * s, 3.5f * s, C(0.75f, 0.55f, 0.35f));
        drawRect(x + 18 * s, feetY, 2.5f * s, 31 * s, dark); drawRect(x + 60 * s, feetY, 2.5f * s, 31 * s, dark);
    }
}

// -------------------------------------------------------------- scene 4 ----
void drawClassroomPanel(float w, float h, float t) {
    const float floorH = 170;
    drawGradientRect(0, floorH, w, h - floorH, C(0.96f, 0.80f, 0.58f), C(1.0f, 0.91f, 0.74f));
    drawGradientRect(0, 0, w, floorH, C(0.58f, 0.37f, 0.22f), C(0.78f, 0.55f, 0.34f));
    for (int k = -7; k <= 7; ++k) drawLine(w / 2 + k * 80, 0, w / 2 + k * 30, floorH, C(0.45f, 0.28f, 0.16f), 1.2f);
    static const float rows[4] = { 30, 70, 108, 142 };
    for (int i = 0; i < 4; ++i) drawLine(0, rows[i], w, rows[i], C(0.45f, 0.28f, 0.16f), 1.0f);
    drawRect(0, floorH, w, 7, C(0.45f, 0.30f, 0.20f));
    // window with sun beam
    drawGradientRect(22, 330, 150, 210, C(0.65f, 0.85f, 1.0f), C(0.40f, 0.70f, 1.0f));
    drawCloud(100, 470, 0.35f, C(1, 1, 1));
    drawRectOutline(22, 330, 150, 210, C(0.95f, 0.92f, 0.85f), 4.0f);
    drawLine(97, 330, 97, 540, C(0.95f, 0.92f, 0.85f), 3.0f); drawLine(22, 435, 172, 435, C(0.95f, 0.92f, 0.85f), 3.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glBegin(GL_QUADS);
        glColor4f(1.0f, 0.9f, 0.6f, 0.30f); glVertex2f(30, 330); glVertex2f(172, 330);
        glColor4f(1.0f, 0.9f, 0.6f, 0.05f); glVertex2f(370, 40); glVertex2f(210, 40);
    glEnd();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // blackboard
    drawRect(199, 354, 212, 162, C(0.5f, 0.34f, 0.2f)); drawGradientRect(205, 360, 200, 150, C(0.16f, 0.30f, 0.24f), C(0.20f, 0.36f, 0.28f));
    for (int i = 0; i < 7; ++i) drawLine(220, 490 - i * 19.0f, 220 + 50 + (int)(hz(i + 3) * 110), 490 - i * 19.0f, rgba(1, 1, 1, 0.7f), 1.5f);
    drawRect(205, 354, 200, 5, C(0.85f, 0.85f, 0.8f));
    drawWallClock(w - 50, 580, 27, gSky.hour, t, C(1.0f, 0.98f, 0.92f), C(0.5f, 0.32f, 0.18f), C(0.2f, 0.12f, 0.08f));
    // classmates (back row)
    PersonStyle a = classmate(0), b = classmate(1);
    PersonPose pa = makePose(1, t), pb = makePose(1, t + 1.3f);
    pa.sit = pb.sit = 1; pa.armsFwd = 0.8f; pb.armsFwd = 0.6f + 0.1f * std::sin(t * 2);
    drawChairDesk(110, 190, 1.3f, true); drawPerson(110, 190, 1.3f, a, pa);
    drawChairDesk(300, 196, 1.3f, true); drawPerson(300, 196, 1.3f, b, pb);
    // Hana, front row: writing in her notebook
    PersonStyle hana = makeHana(); PersonPose p = makePose(1, t);
    p.sit = 1; p.armsFwd = 0.85f + 0.1f * std::sin(t * 9.0f);
    drawChairDesk(150, 58, 1.9f, true);
    drawPerson(150, 58, 1.9f, hana, p);
    drawRect(194, 121, 62, 6, C(0.98f, 0.98f, 0.95f)); drawLine(200, 125, 245, 125, C(0.5f, 0.5f, 0.6f), 1.0f);
    for (size_t i = 0; i < gDust.p.size(); ++i) {                                          // dust in the sunbeam
        const Particle& d = gDust.p[i];
        if (d.life > 0) drawCircle(d.x, d.y, d.size, rgba(1.0f, 0.95f, 0.75f, 0.6f * std::sin(Config::PI * d.life / d.maxLife)), 8);
    }
}

void drawOfficePanel(float w, float h, float t) {
    drawGradientRect(0, 0, w, h, C(0.50f, 0.53f, 0.54f), C(0.62f, 0.65f, 0.64f));
    drawGradientRect(0, 0, w, 140, C(0.28f, 0.29f, 0.32f), C(0.38f, 0.40f, 0.42f));            // dull carpet
    drawRect(0, h - 80, w, 80, C(0.76f, 0.78f, 0.78f));                                         // low ceiling
    float flick = (std::sin(t * 41.0f) > 0.96f) ? 0.45f : 1.0f;                                 // tired fluorescent tube
    drawGlow(w / 2, h - 66, 160, rgba(0.85f, 0.95f, 1.0f, 0.4f * flick), 14);
    drawRect(w / 2 - 110, h - 72, 220, 14, flick > 0.9f ? C(0.95f, 1.0f, 1.0f) : C(0.55f, 0.58f, 0.6f));
    // small window, grey city
    drawGradientRect(w / 2 - 60, 370, 120, 100, C(0.72f, 0.76f, 0.80f), C(0.58f, 0.62f, 0.70f));
    drawSkyline(120, 370, C(0.45f, 0.48f, 0.55f), 11, 6, 20, 60, false);
    drawRectOutline(w / 2 - 60, 370, 120, 100, C(0.3f, 0.3f, 0.32f), 4.0f);
    drawWallClock(w - 85, 478, 24, gSky.hour, t, C(0.92f, 0.93f, 0.9f), C(0.25f, 0.26f, 0.3f), C(0.1f, 0.1f, 0.12f));
    // calendar full of red circles, sticky notes
    drawRect(34, 400, 52, 66, C(0.95f, 0.95f, 0.92f)); for (int i = 0; i < 9; ++i) drawCircleOutline(44 + (i % 3) * 16.0f, 412 + (i / 3) * 16.0f, 5, C(0.85f, 0.15f, 0.15f), 1.5f);
    // cubicle partitions close in from both sides: cramped
    drawRect(0, 0, 56, h - 80, C(0.40f, 0.44f, 0.50f)); drawRect(w - 56, 0, 56, h - 80, C(0.40f, 0.44f, 0.50f));
    drawRect(56, 0, 5, h - 80, C(0.3f, 0.33f, 0.38f)); drawRect(w - 61, 0, 5, h - 80, C(0.3f, 0.33f, 0.38f));
    // desk, monitor, paper piles
    drawRect(70, 150, w - 140, 14, C(0.52f, 0.46f, 0.40f)); drawRect(76, 0, 8, 150, C(0.36f, 0.32f, 0.28f)); drawRect(w - 86, 0, 8, 150, C(0.36f, 0.32f, 0.28f));
    drawGlow(w - 150, 232, 130, rgba(0.7f, 0.85f, 1.0f, 0.28f), 14);
    drawRect(w - 160, 164, 12, 22, C(0.2f, 0.2f, 0.22f));
    drawRect(w - 215, 186, 130, 92, C(0.12f, 0.13f, 0.15f)); drawRect(w - 209, 192, 118, 80, C(0.80f, 0.90f, 1.0f));
    for (int i = 0; i < 6; ++i) drawLine(w - 200, 262 - i * 12.0f, w - 200 + 25 + (int)(hz(i + 20) * 60), 262 - i * 12.0f, C(0.25f, 0.35f, 0.5f), 2.0f);
    drawRect(w - 205, 255, 14, 14, C(1.0f, 0.9f, 0.3f)); drawRect(w - 124, 238, 14, 14, C(1.0f, 0.75f, 0.85f));
    for (int i = 0; i < 6; ++i) drawRect(w - 78 + (i % 2) * 3.0f, 164 + i * 12.0f, 40, 11, i % 2 ? C(0.96f, 0.95f, 0.88f) : C(0.9f, 0.92f, 0.95f));      // paper tower
    for (int i = 0; i < 4; ++i) drawRect(72 + (i % 2) * 2.0f, 164 + i * 11.0f, 38, 10, i % 2 ? C(0.96f, 0.95f, 0.88f) : C(0.88f, 0.9f, 0.95f));
    drawRect(116, 164, 11, 13, C(0.9f, 0.9f, 0.9f));                                              // cold coffee
    // phone: lights up now and then (missed calls from home)
    bool ring = std::fmod(t, 5.0f) > 3.4f;
    drawRect(w - 262, 164, 24, 5, C(0.1f, 0.1f, 0.12f));
    if (ring) { drawGlow(w - 250, 168, 30, rgba(0.6f, 0.8f, 1.0f, 0.8f), 8); drawRect(w - 262, 164, 24, 5, C(0.7f, 0.85f, 1.0f)); }
    // Kenji at the keyboard, shoulders hunched
    PersonStyle k = makeKenji(); k.hunch = 16; k.prop = PROP_NONE;
    PersonPose p = makePose(1, t); p.sit = 1; p.armsFwd = 0.8f + 0.2f * std::sin(t * 15.0f);
    drawChairDesk(140, 78, 1.85f, false);
    drawPerson(140, 78, 1.85f, k, p);
}

void drawPianoPanel(float w, float h, float t) {
    const float floorH = 190;
    drawGradientRect(0, floorH, w, h - floorH, C(0.40f, 0.48f, 0.66f), C(0.70f, 0.78f, 0.92f));
    drawGradientRect(0, 0, w, floorH, C(0.10f, 0.12f, 0.22f), C(0.22f, 0.26f, 0.42f));          // polished, dark floor
    // tall arched windows with cold light
    for (int i = 0; i < 2; ++i) {
        float wx = 40 + i * 190;
        drawRect(wx, 280, 120, 250, C(0.80f, 0.90f, 1.0f)); drawCircle(wx + 60, 530, 60, C(0.80f, 0.90f, 1.0f));
        drawRectOutline(wx, 280, 120, 250, C(0.92f, 0.94f, 0.98f), 3.0f); drawLine(wx + 60, 280, wx + 60, 590, C(0.92f, 0.94f, 0.98f), 2.5f);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glBegin(GL_QUADS);
            glColor4f(0.7f, 0.85f, 1.0f, 0.22f); glVertex2f(wx + 10, 280); glVertex2f(wx + 110, 280);
            glColor4f(0.7f, 0.85f, 1.0f, 0.0f);  glVertex2f(wx + 150, 20); glVertex2f(wx + 40, 20);
        glEnd();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    drawWallClock(w - 38, 590, 20, gSky.hour, t, C(0.92f, 0.95f, 1.0f), C(0.55f, 0.6f, 0.75f), C(0.15f, 0.18f, 0.3f));
    drawGlow(w / 2, h - 30, 70, rgba(0.8f, 0.9f, 1.0f, 0.4f), 12);                               // distant chandelier
    drawLine(w / 2, h, w / 2, h - 26, C(0.5f, 0.55f, 0.7f), 1.5f); drawEllipse(w / 2, h - 30, 16, 6, C(0.85f, 0.9f, 1.0f));
    for (int i = 0; i < 4; ++i) {                                                                 // empty chairs along the wall
        float cx = 60 + i * 100; drawRect(cx, floorH, 22, 3, C(0.2f, 0.22f, 0.34f)); drawRect(cx, floorH, 3, 26, C(0.2f, 0.22f, 0.34f)); drawRect(cx + 19, floorH, 3, 14, C(0.2f, 0.22f, 0.34f));
    }
    drawEllipse(w / 2, 70, 170, 34, C(0.22f, 0.26f, 0.5f), 32); drawEllipse(w / 2, 70, 150, 28, C(0.30f, 0.34f, 0.6f), 32);   // rug
    // grand piano and Aoi
    float body[] = { 40, 96, 300, 96, 316, 120, 250, 150, 70, 150 };
    drawPolygon(body, 5, C(0.06f, 0.07f, 0.10f));
    float lid[] = { 70, 150, 250, 150, 290, 205, 100, 205 };
    drawPolygon(lid, 4, C(0.10f, 0.11f, 0.16f)); drawPolygonOutline(lid, 4, C(0.35f, 0.4f, 0.55f), 1.0f);
    drawRect(60, 40, 9, 60, C(0.06f, 0.07f, 0.10f)); drawRect(270, 40, 9, 60, C(0.06f, 0.07f, 0.10f));
    drawRect(258, 118, 70, 9, C(0.92f, 0.94f, 0.98f)); for (int i = 0; i < 8; ++i) drawRect(262 + i * 8.0f, 121, 4, 6, C(0.05f, 0.05f, 0.08f));
    drawRect(332, 64, 34, 5, C(0.12f, 0.12f, 0.2f)); drawRect(336, 40, 4, 24, C(0.12f, 0.12f, 0.2f)); drawRect(358, 40, 4, 24, C(0.12f, 0.12f, 0.2f));    // bench
    PersonStyle aoi = makeAoi(); PersonPose p = makePose(-1, t); p.sit = 1;
    p.armsFwd = 0.85f + 0.15f * std::sin(t * 6.0f);
    drawPerson(350, 58, 1.2f, aoi, p);
    for (size_t i = 0; i < gNotes.p.size(); ++i) {                                                // music notes
        const Particle& n = gNotes.p[i];
        if (n.life <= 0) continue;
        float a = std::sin(Config::PI * n.life / n.maxLife) * 0.9f, s = n.size;
        Color c = rgba(0.85f, 0.92f, 1.0f, a);
        drawGlow(n.x, n.y, 14 * s, withAlpha(c, a * 0.5f), 6);
        drawEllipse(n.x, n.y, 5 * s, 3.5f * s, c, 14); drawLine(n.x + 4.5f * s, n.y, n.x + 4.5f * s, n.y + 17 * s, c, 1.6f);
        drawLine(n.x + 4.5f * s, n.y + 17 * s, n.x + 10 * s, n.y + 11 * s, c, 1.6f);
    }
}

// -------------------------------------------------------------- scene 5 ----
static bool phoneRinging(float t) { return t > 4.2f && t < 10.0f; }
static void phonePulse(float x, float y, float t, Color c) {            // expanding rings: the call that connects two panels
    if (!phoneRinging(t)) return;
    for (int i = 0; i < 2; ++i) {
        float u = std::fmod(t * 1.1f + i * 0.5f, 1.0f);
        drawCircleOutline(x, y, 8 + u * 46, withAlpha(c, 0.8f * (1 - u)), 2.0f);
    }
}

void drawRooftopPanel(float w, float h, float t, float panelX, float cloudX, float cloudY) {
    skyBackdrop(w, h);
    drawCloudShared(panelX, cloudX, cloudY);
    Color farColor = lerpColor(gSky.bottom, C(0.55f, 0.6f, 0.7f), 0.6f);
    drawSkyline(w, 150, farColor, 5, 8, 60, 220, false);
    drawSkyline(w, 140, lerpColor(farColor, C(0.3f, 0.34f, 0.42f), 0.6f), 41, 6, 90, 260, true);
    drawGradientRect(0, 0, w, 150, C(0.46f, 0.47f, 0.50f), C(0.62f, 0.63f, 0.65f));               // concrete roof
    drawRect(0, 150, w, 26, C(0.70f, 0.71f, 0.73f)); drawRect(0, 172, w, 5, C(0.52f, 0.53f, 0.56f));
    for (int i = 0; i < 9; ++i) drawLine(i * 80 + 20, 176, i * 80 + 20, 236, C(0.3f, 0.32f, 0.36f), 2.0f);
    drawLine(0, 236, w, 236, C(0.3f, 0.32f, 0.36f), 2.5f); drawLine(0, 205, w, 205, C(0.3f, 0.32f, 0.36f), 1.5f);
    drawRect(w - 150, 176, 8, 70, C(0.35f, 0.36f, 0.4f)); drawRect(w - 80, 176, 8, 70, C(0.35f, 0.36f, 0.4f));    // water tank
    drawRect(w - 160, 246, 100, 80, C(0.56f, 0.58f, 0.62f)); drawEllipse(w - 110, 326, 50, 12, C(0.64f, 0.66f, 0.7f)); drawLine(w - 160, 286, w - 60, 286, C(0.4f, 0.42f, 0.46f), 2.0f);
    drawRect(60, 176, 80, 38, C(0.6f, 0.62f, 0.65f)); drawCircleOutline(100, 195, 12, C(0.35f, 0.36f, 0.4f), 1.5f);
    drawRect(230, 176, 40, 14, C(0.95f, 0.9f, 0.7f)); drawRect(234, 190, 12, 16, C(0.7f, 0.25f, 0.2f));       // lunch on the ledge
    // phone on the ledge: buzzing but unanswered
    float shake = phoneRinging(t) ? std::sin(t * 70.0f) * 1.3f : 0.0f;
    drawRect(318 + shake, 177, 16, 26, C(0.12f, 0.12f, 0.14f)); drawRect(320 + shake, 180, 12, 19, phoneRinging(t) ? C(0.75f, 0.9f, 1.0f) : C(0.2f, 0.25f, 0.3f));
    phonePulse(326, 190, t, C(0.7f, 0.85f, 1.0f));
    if (phoneRinging(t)) drawGlow(326, 190, 40, rgba(0.7f, 0.85f, 1.0f, 0.5f), 10);
    // Kenji: eats, hears the phone, looks at it, then turns away and keeps eating
    PersonStyle k = makeKenji(); k.prop = PROP_NONE; PersonPose p = makePose(1, t);
    float facing = (t < 8.6f) ? 1.0f : -1.0f; p.facing = facing;
    bool eating = (t > 0.8f && t < 3.6f) || t > 11.0f;
    p.armUp = eating ? (0.5f + 0.5f * std::sin(t * 4.0f)) : 0.0f;
    k.hunch = (t > 9.5f) ? 18.0f : 6.0f;
    drawPerson(230 - (facing < 0 ? 40.0f : 0.0f), 50, 1.9f, k, p);
}

void drawFieldPanel(float w, float h, float t, float panelX, float cloudX, float cloudY) {
    skyBackdrop(w, h);
    drawCloudShared(panelX, cloudX, cloudY);
    drawHillLayerRange(-20, w + 20, 280, 22, 260, C(0.50f, 0.64f, 0.72f), C(0.45f, 0.6f, 0.66f), 200);
    drawRect(70, 300, 70, 34, C(0.93f, 0.9f, 0.8f)); drawTriangle(60, 334, 150, 334, 105, 372, C(0.32f, 0.36f, 0.44f));   // distant farmhouse
    drawRect(0, 150, w, 160, C(0.34f, 0.58f, 0.34f));
    for (int i = 0; i < 9; ++i) {                                                                    // rice paddies in perspective
        float y = 290 - i * 18.0f;
        drawRect(0, y, w, 12, lerpColor(C(0.55f, 0.80f, 0.40f), C(0.32f, 0.60f, 0.30f), i / 9.0f));
        drawRect(0, y - 3, w, 3, rgba(0.6f, 0.8f, 0.9f, 0.5f));
    }
    drawGradientRect(0, 0, w, 150, C(0.46f, 0.34f, 0.22f), C(0.70f, 0.56f, 0.38f));                 // earth ridge in front
    drawEllipse(w * 0.58f, 58, 120, 30, C(0.78f, 0.66f, 0.4f), 28);                                   // straw mat
    // lunch cloth with bento, thermos and the old phone
    float cx = w * 0.30f;
    drawRect(cx - 50, 44, 100, 10, C(0.85f, 0.35f, 0.40f)); drawRect(cx - 40, 54, 44, 22, C(0.35f, 0.25f, 0.2f));
    drawEllipse(cx - 24, 77, 11, 7, C(0.98f, 0.98f, 0.95f)); drawEllipse(cx - 6, 77, 11, 7, C(0.98f, 0.98f, 0.95f));
    drawRect(cx + 12, 54, 14, 30, C(0.6f, 0.65f, 0.7f));
    float shake = phoneRinging(t) ? std::sin(t * 60.0f) * 1.0f : 0.0f;
    drawRect(cx + 32 + shake, 54, 16, 12, C(0.22f, 0.22f, 0.26f));
    phonePulse(cx + 40, 62, t, C(1.0f, 0.85f, 0.55f));
    if (phoneRinging(t)) drawGlow(cx + 40, 62, 36, rgba(1.0f, 0.85f, 0.5f, 0.5f), 10);
    // Granny Sato, eating alone and waiting: she calls, listens to it ring, puts the phone down
    PersonStyle g = makeGranny(); PersonPose p = makePose(-1, t); p.sit = 1;
    bool onPhone = t > 4.2f && t < 9.6f;
    p.armUp = onPhone ? 1.0f : 0.0f; p.armsFwd = onPhone ? 0.0f : 0.55f + 0.1f * std::sin(t * 3.0f);
    g.hunch = (t > 9.8f) ? 20.0f : 13.0f;
    drawPerson(w * 0.60f, 58, 1.75f, g, p);
}

// -------------------------------------------------------------- scene 6 ----
void drawAoiWindowPanel(float w, float h, float t, float flash) {
    drawGradientRect(0, 0, w, h, C(0.26f, 0.31f, 0.44f), C(0.40f, 0.46f, 0.62f));
    drawGradientRect(0, 0, w, 52, C(0.14f, 0.16f, 0.26f), C(0.20f, 0.23f, 0.36f));
    const float wx = 250, wy = 58, ww = 340, wh = 215;
    drawGradientRect(wx, wy, ww, wh, C(0.34f, 0.38f, 0.48f), C(0.22f, 0.25f, 0.34f));              // storm outside
    drawSkyline(ww, 0, C(0.18f, 0.2f, 0.28f), 3, 7, 30, 90, false);
    drawRect(wx, wy, ww, wh, rgba(0.9f, 0.93f, 1.0f, 0.5f * flash));
    panelClipBegin(wx, wy, ww, wh);
    drawBolt(wx + 220, wy + wh, wy + 40, 9, flash);
    rainDraw(gRainAoi, rgba(0.75f, 0.82f, 0.95f, 0.7f), rgba(0.7f, 0.8f, 0.9f, 0.0f));
    panelClipEnd();
    for (int i = 0; i < 7; ++i) {                                                                    // streaks running down the dry side of the glass
        float x = wx + 30 + i * 46.0f, y = wy + wh - std::fmod(t * (14.0f + i * 3.0f) + i * 31.0f, wh);
        drawLine(x, y, x, y - 18, rgba(0.85f, 0.9f, 1.0f, 0.35f), 2.0f);
    }
    drawRectOutline(wx, wy, ww, wh, C(0.82f, 0.85f, 0.92f), 6.0f); drawLine(wx + ww / 2, wy, wx + ww / 2, wy + wh, C(0.82f, 0.85f, 0.92f), 4.0f);
    drawRect(wx - 30, wy - 10, 26, wh + 30, C(0.34f, 0.28f, 0.42f)); drawRect(wx + ww + 4, wy - 10, 26, wh + 30, C(0.34f, 0.28f, 0.42f));   // curtains
    float pb[] = { 18, 52, 190, 52, 196, 86, 150, 112, 30, 112 };
    drawPolygon(pb, 5, C(0.08f, 0.09f, 0.14f)); drawRect(34, 20, 7, 34, C(0.08f, 0.09f, 0.14f)); drawRect(166, 20, 7, 34, C(0.08f, 0.09f, 0.14f));   // her piano, unplayed
    PersonStyle a = makeAoi(); PersonPose p = makePose(1, t); p.armUp = 0.55f;                         // hand toward the glass
    drawPerson(222, 54, 1.5f, a, p);
    flashOverlay(w, h, flash * 0.5f);
}

static void streetScene(float w, float h, float t, float speed, bool downtown, const RainField& rain, float flash, bool bolt) {
    drawGradientRect(0, 0, w, h, C(0.50f, 0.53f, 0.60f), C(0.26f, 0.28f, 0.37f));
    if (bolt) drawBolt(w * 0.72f, h, h - 150, 31, flash);
    const float P = 560.0f;
    float off = std::fmod(t * speed, P);
    for (int k = -1; k < (int)(w / P) + 2; ++k) {
        float x0 = k * P - off;
        for (int b = 0; b < 3; ++b) {
            float bx = x0 + b * 190.0f + 10, bh = (downtown ? 110.0f : 70.0f) + hz(b + 5) * (downtown ? 100.0f : 50.0f);
            Color wall = downtown ? C(0.30f, 0.32f, 0.40f) : C(0.38f, 0.36f, 0.40f);
            drawRect(bx, 88, 170, bh, wall);
            if (!downtown) drawTriangle(bx - 8, 88 + bh, bx + 178, 88 + bh, bx + 85, 88 + bh + 44, C(0.26f, 0.24f, 0.30f));
            for (int c = 0; c < 4; ++c) if (hz(b * 7 + c + k * 3) > 0.35f) {
                drawGlow(bx + 22 + c * 38.0f, 88 + bh * 0.55f, 26, rgba(1.0f, 0.8f, 0.45f, 0.35f), 6);
                drawRect(bx + 14 + c * 38.0f, 88 + bh * 0.55f - 10, 16, 20, C(1.0f, 0.86f, 0.52f));
            }
        }
        drawLamp(x0 + 140, 84, 0.9f, true);
    }
    drawGradientRect(0, 62, w, 30, C(0.40f, 0.41f, 0.46f), C(0.50f, 0.51f, 0.56f));                    // sidewalk
    drawGradientRect(0, 0, w, 62, C(0.14f, 0.15f, 0.20f), C(0.28f, 0.30f, 0.38f));                      // wet road
    for (int k = 0; k < 6; ++k) drawRect(std::fmod(k * 140.0f - t * speed, w + 140) - 20, 30, 60, 3, rgba(0.8f, 0.8f, 0.9f, 0.35f));
    rainDraw(rain, rgba(0.78f, 0.85f, 0.97f, 0.65f), rgba(0.85f, 0.9f, 1.0f, 0.55f));
}

void drawHanaStreetPanel(float w, float h, float t, float flash) {
    const float speed = 55.0f, scale = 1.25f;
    streetScene(w, h, t, speed, false, gRainHana, flash, true);
    PersonStyle s = makeHana(); PersonPose p = makePose(1, t);
    p.walkAmount = 1; p.walkPhase = t * speed * 0.115f / scale; p.armUp = 1.0f;                          // schoolbag held over her head
    float x = w * 0.45f, y = 66;
    drawPerson(x, y, scale, s, p);
    glPushMatrix(); glTranslatef(x + 20, y + 122, 0); glRotatef(-12, 0, 0, 1);
    drawRect(-20, -8, 40, 22, C(0.85f, 0.45f, 0.30f)); drawRectOutline(-20, -8, 40, 22, C(0.35f, 0.15f, 0.1f), 1.5f); glPopMatrix();
    flashOverlay(w, h, flash * 0.5f);
}

void drawKenjiRunPanel(float w, float h, float t, float flash) {
    const float speed = 150.0f, scale = 1.25f;
    streetScene(w, h, t, speed, true, gRainKenji, flash, false);
    PersonStyle s = makeKenji(); s.hunch = 14; PersonPose p = makePose(1, t);
    p.walkAmount = 1; p.walkPhase = t * speed * 0.115f / scale; p.armUp = 1.0f; p.propAlways = true;    // briefcase over his head
    float x = w * 0.40f, y = 64;
    float step = std::sin(p.walkPhase);
    drawPerson(x, y, scale, s, p);
    float sp = std::fabs(step);                                                                         // splashes at each footfall
    for (int i = 0; i < 4; ++i) drawLine(x + 8 + i * 6, y + 2, x + 2 + i * 9, y + 8 + 10 * (1 - sp), rgba(0.85f, 0.9f, 1.0f, 0.6f * (1 - sp)), 1.5f);
    drawEllipse(x + 6, y - 2, 26 + 6 * (1 - sp), 5, rgba(0.8f, 0.88f, 1.0f, 0.25f), 16);
    flashOverlay(w, h, flash * 0.5f);
}

void drawRenHutPanel(float w, float h, float t, float flash) {
    drawGradientRect(0, 0, w, h, C(0.30f, 0.20f, 0.14f), C(0.50f, 0.36f, 0.24f));
    for (int i = 1; i < 14; ++i) drawLine(i * w / 14, 52, i * w / 14, h, C(0.26f, 0.17f, 0.12f), 1.2f);
    drawGradientRect(0, 0, w, 52, C(0.18f, 0.12f, 0.09f), C(0.28f, 0.19f, 0.13f));
    for (int i = 0; i < 6; ++i) drawLine(0, h - 12 - i * 8.0f, w, h - 12 - i * 8.0f, C(0.20f, 0.13f, 0.09f), 1.0f);     // roof boards
    drawRect(0, h - 22, w, 22, C(0.2f, 0.13f, 0.09f)); drawRect(121, h - 22, 14, 22, C(0.12f, 0.1f, 0.12f));   // the crack, right above the bucket
    drawRect(w - 150, 120, 90, 100, C(0.35f, 0.4f, 0.5f)); drawRect(w - 150, 120, 90, 100, rgba(0.9f, 0.93f, 1.0f, 0.5f * flash));
    for (int i = 0; i < 9; ++i) drawLine(w - 146 + i * 10.0f, 220 - std::fmod(t * 160.0f + i * 23.0f, 100.0f), w - 148 + i * 10.0f, 232 - std::fmod(t * 160.0f + i * 23.0f, 100.0f), rgba(0.8f, 0.88f, 1.0f, 0.6f), 1.3f);
    drawRectOutline(w - 150, 120, 90, 100, C(0.22f, 0.15f, 0.1f), 5.0f);
    float sw = std::sin(t * 1.3f) * 3.0f + flash * 4.0f;                                                              // hanging lamp, swaying
    drawLine(w * 0.55f, h, w * 0.55f + sw, h - 40, C(0.15f, 0.1f, 0.08f), 1.5f);
    drawGlow(w * 0.55f + sw, h - 48, 90, rgba(1.0f, 0.78f, 0.42f, 0.55f), 14); drawCircle(w * 0.55f + sw, h - 48, 6, C(1.0f, 0.92f, 0.65f));
    drawGlow(34, 80, 60, rgba(1.0f, 0.5f, 0.2f, 0.4f), 10); drawRect(14, 52, 40, 46, C(0.12f, 0.1f, 0.1f));            // stove
    // bucket under the leak
    float bx = 128, lvl = gBucket * 12;
    drawRect(bx - 26, 52, 52, 44, C(0.5f, 0.56f, 0.62f)); drawEllipse(bx, 96, 26, 6, C(0.3f, 0.34f, 0.4f)); drawEllipse(bx, 96 + lvl - 12 + 6, 22, 4, C(0.45f, 0.62f, 0.8f));
    for (size_t i = 0; i < gDrips.p.size(); ++i) { const Particle& d = gDrips.p[i]; if (d.life > 0) drawEllipse(d.x, d.y, 2.2f, 4.2f, C(0.6f, 0.78f, 0.95f), 10); }
    for (size_t i = 0; i < gDripRipples.p.size(); ++i) {
        const Particle& r = gDripRipples.p[i]; if (r.life <= 0) continue;
        float u = 1 - r.life / r.maxLife; drawCircleOutline(r.x, r.y, 4 + 14 * u, rgba(0.8f, 0.9f, 1.0f, 0.8f * (1 - u)), 1.2f);
    }
    // table with the single cup, and Ren watching the ceiling
    drawRect(w * 0.40f, 100, 120, 7, C(0.40f, 0.27f, 0.17f)); drawRect(w * 0.40f + 8, 52, 6, 48, C(0.3f, 0.2f, 0.13f)); drawRect(w * 0.40f + 106, 52, 6, 48, C(0.3f, 0.2f, 0.13f));
    float cx = w * 0.40f + 60;
    drawRect(cx, 107, 14, 12, C(0.96f, 0.96f, 0.92f)); drawRectOutline(cx, 107, 14, 12, C(0.4f, 0.4f, 0.45f), 1.2f);
    for (int k = 0; k < 3; ++k) { float ph = std::fmod(t * 9.0f + k * 7.0f, 22.0f); drawCircle(cx + 7 + std::sin(t * 1.6f + k) * 3, 122 + ph, 2.5f + ph * 0.1f, rgba(1, 1, 1, 0.5f * (1 - ph / 22.0f)), 12); }
    PersonStyle r = makeRen(); PersonPose p = makePose(-1, t); p.sit = 1; p.armsFwd = 0.3f;
    drawRect(w * 0.60f - 30, 70, 6, 34, C(0.32f, 0.2f, 0.12f)); drawRect(w * 0.60f - 30, 70, 36, 5, C(0.4f, 0.26f, 0.16f));
    drawPerson(w * 0.60f, 56, 1.35f, r, p);
    flashOverlay(w, h, flash * 0.35f);
}
