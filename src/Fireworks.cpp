#include "Fireworks.h"
#include "Particles.h"
#include <GL/freeglut.h>

static ParticlePool gRockets, gSparks;
static float gFlashes[6][5];                 // x, y, age, r, g  (b stored separately)
static float gFlashB[6];
static int   gFlashNext = 0;
static float gLevel = 0;

void fireworksReset() {
    poolInit(gRockets, 6); poolInit(gSparks, 360);
    for (int i = 0; i < 6; ++i) gFlashes[i][2] = 99.0f;
    gLevel = 0;
}

void fireworksLaunch(float x, float burstY, Color c) {
    Particle* r = poolSpawn(gRockets);
    if (!r) return;
    r->x = x; r->y = 50; r->vx = (rand01() - 0.5f) * 20.0f; r->vy = 430.0f + rand01() * 60.0f;
    r->life = r->maxLife = 4.0f; r->aux = burstY; r->color = c; r->size = 3;
}

static void burst(const Particle& r) {
    const int N = 44;
    for (int i = 0; i < N; ++i) {
        Particle* s = poolSpawn(gSparks);
        if (!s) break;
        float a = 2.0f * Config::PI * i / N + rand01() * 0.2f, sp = 120.0f + rand01() * 90.0f;
        s->x = r.x; s->y = r.y; s->vx = std::cos(a) * sp; s->vy = std::sin(a) * sp;
        s->life = s->maxLife = 1.3f + rand01() * 0.6f; s->size = 2.5f;
        float mix = rand01() * 0.35f;
        s->color = rgba(clamp01(r.color.r + mix), clamp01(r.color.g + mix * 0.5f), clamp01(r.color.b), 1.0f);
    }
    int f = gFlashNext; gFlashNext = (gFlashNext + 1) % 6;                 // brief flash of sky light at the burst
    gFlashes[f][0] = r.x; gFlashes[f][1] = r.y; gFlashes[f][2] = 0; gFlashes[f][3] = r.color.r; gFlashes[f][4] = r.color.g; gFlashB[f] = r.color.b;
}

void fireworksUpdate(float dt) {
    for (size_t i = 0; i < gRockets.p.size(); ++i) {
        Particle& r = gRockets.p[i];
        if (r.life <= 0) continue;
        r.x += r.vx * dt; r.y += r.vy * dt; r.life -= dt;
        if (r.y >= r.aux) { r.life = 0; burst(r); }                        // reached the burst height
    }
    for (size_t i = 0; i < gSparks.p.size(); ++i) {                        // sparks: gravity + air drag
        Particle& s = gSparks.p[i];
        if (s.life <= 0) continue;
        s.vx *= (1.0f - 1.2f * dt); s.vy = s.vy * (1.0f - 1.2f * dt) - 70.0f * dt;
        s.x += s.vx * dt; s.y += s.vy * dt; s.life -= dt;
    }
    gLevel = 0;
    for (int i = 0; i < 6; ++i) { gFlashes[i][2] += dt; if (gFlashes[i][2] < 0.5f) gLevel += 1.0f - gFlashes[i][2] / 0.5f; }
    if (gLevel > 1) gLevel = 1;
}

float fireworksFlash() { return gLevel; }
int   fireworksAlive() { return poolAlive(gRockets) + poolAlive(gSparks); }

void fireworksDraw() {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);                                       // light adds up
    for (int i = 0; i < 6; ++i)
        if (gFlashes[i][2] < 0.5f) {
            float k = 1.0f - gFlashes[i][2] / 0.5f;
            drawGlow(gFlashes[i][0], gFlashes[i][1], 140, rgba(gFlashes[i][3], gFlashes[i][4], gFlashB[i], 0.5f * k), 12);
        }
    glLineWidth(1.8f);
    glBegin(GL_LINES);
    for (size_t i = 0; i < gSparks.p.size(); ++i) {                          // each spark leaves a short trail
        const Particle& s = gSparks.p[i];
        if (s.life <= 0) continue;
        float a = clamp01(s.life / s.maxLife);
        glColor4f(s.color.r, s.color.g, s.color.b, 0.0f);     glVertex2f(s.x - s.vx * 0.06f, s.y - s.vy * 0.06f);
        glColor4f(s.color.r, s.color.g, s.color.b, a);        glVertex2f(s.x, s.y);
    }
    glEnd();
    glPointSize(3.0f);
    glBegin(GL_POINTS);
    for (size_t i = 0; i < gSparks.p.size(); ++i) {
        const Particle& s = gSparks.p[i];
        if (s.life > 0) { glColor4f(1.0f, 0.95f, 0.8f, clamp01(s.life / s.maxLife)); glVertex2f(s.x, s.y); }
    }
    for (size_t i = 0; i < gRockets.p.size(); ++i) { const Particle& r = gRockets.p[i]; if (r.life > 0) { glColor4f(1, 0.9f, 0.6f, 1); glVertex2f(r.x, r.y); } }
    glEnd();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
