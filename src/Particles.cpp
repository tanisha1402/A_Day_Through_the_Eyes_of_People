#include "Particles.h"
#include <GL/freeglut.h>

static unsigned gSeed = 12345u;
void  seedRandom(unsigned seed) { gSeed = seed ? seed : 1u; }
float rand01() {                                   // xorshift: fast, repeatable
    gSeed ^= gSeed << 13; gSeed ^= gSeed >> 17; gSeed ^= gSeed << 5;
    return (gSeed & 0xFFFFFFu) / 16777216.0f;
}

void poolInit(ParticlePool& pool, int capacity) {
    Particle dead; dead.x = dead.y = dead.vx = dead.vy = 0; dead.life = 0; dead.maxLife = 1; dead.size = 1; dead.aux = 0; dead.color = rgb(1, 1, 1);
    pool.p.assign(capacity, dead);
    pool.cursor = 0;
}
void poolClear(ParticlePool& pool) { for (size_t i = 0; i < pool.p.size(); ++i) pool.p[i].life = 0; pool.cursor = 0; }

Particle* poolSpawn(ParticlePool& pool) {
    int n = (int)pool.p.size();
    for (int k = 0; k < n; ++k) {                  // start from the cursor so free slots are found quickly
        int i = (pool.cursor + k) % n;
        if (pool.p[i].life <= 0) {
            pool.cursor = (i + 1) % n;
            Particle& q = pool.p[i];
            q.x = q.y = q.vx = q.vy = 0; q.life = q.maxLife = 1; q.size = 1; q.aux = 0; q.color = rgb(1, 1, 1);
            return &q;
        }
    }
    return 0;                                      // pool full: skip this spawn (hard cap)
}
int poolAlive(const ParticlePool& pool) {
    int n = 0; for (size_t i = 0; i < pool.p.size(); ++i) if (pool.p[i].life > 0) ++n; return n;
}
void poolUpdate(ParticlePool& pool, float dt, float gravityY) {
    for (size_t i = 0; i < pool.p.size(); ++i) {
        Particle& q = pool.p[i];
        if (q.life <= 0) continue;
        q.vy += gravityY * dt; q.x += q.vx * dt; q.y += q.vy * dt;
        q.life -= dt;
    }
}

// ------------------------------------------------------------------- rain ----
void rainInit(RainField& f, float w, float h, float landMin, float landMax, int maxDrops, float rate) {
    f.w = w; f.h = h; f.landMin = landMin; f.landMax = landMax; f.rate = rate; f.spawnAcc = 0;
    poolInit(f.drops, maxDrops); poolInit(f.ripples, 48);
}

void rainUpdate(RainField& f, float dt) {
    f.spawnAcc += f.intensity * f.rate * dt;
    while (f.spawnAcc >= 1.0f) {
        f.spawnAcc -= 1.0f;
        Particle* d = poolSpawn(f.drops);
        if (!d) { f.spawnAcc = 0; break; }
        d->x = rand01() * (f.w + 160.0f) - 40.0f; d->y = f.h + 8.0f;
        d->vx = f.wind; d->vy = -(560.0f + rand01() * 200.0f);
        d->life = d->maxLife = 100.0f; d->size = 9.0f + rand01() * 8.0f;
        d->aux = f.landMin + rand01() * (f.landMax - f.landMin);      // where this drop will land
    }
    for (size_t i = 0; i < f.drops.p.size(); ++i) {
        Particle& d = f.drops.p[i];
        if (d.life <= 0) continue;
        d.x += d.vx * dt; d.y += d.vy * dt;
        if (d.y <= d.aux) {                                           // hit the ground: free the drop, start a ripple
            d.life = 0;
            if (d.x > 0 && d.x < f.w) {
                Particle* r = poolSpawn(f.ripples);
                if (r) { r->x = d.x; r->y = d.aux; r->life = r->maxLife = 0.5f; r->size = 3.0f; }
            }
        }
    }
    poolUpdate(f.ripples, dt, 0.0f);
}

void rainDraw(const RainField& f, Color dc, Color rc) {
    glLineWidth(1.3f);
    glBegin(GL_LINES);
        for (size_t i = 0; i < f.drops.p.size(); ++i) {
            const Particle& d = f.drops.p[i];
            if (d.life <= 0) continue;
            float k = d.size / 560.0f;                                // streak length follows the speed
            glColor4f(dc.r, dc.g, dc.b, 0.0f);   glVertex2f(d.x - d.vx * k, d.y - d.vy * k);   // faint tail
            glColor4f(dc.r, dc.g, dc.b, dc.a);   glVertex2f(d.x, d.y);
        }
    glEnd();
    for (size_t i = 0; i < f.ripples.p.size(); ++i) {                 // expanding, fading ellipses
        const Particle& r = f.ripples.p[i];
        if (r.life <= 0) continue;
        float u = 1.0f - r.life / r.maxLife;
        glLineWidth(1.0f);
        glColor4f(rc.r, rc.g, rc.b, rc.a * (1.0f - u));
        glBegin(GL_LINE_LOOP);
            for (int s = 0; s < 12; ++s) { float a = 2 * Config::PI * s / 12; glVertex2f(r.x + (r.size + 14 * u) * std::cos(a), r.y + (r.size + 14 * u) * 0.35f * std::sin(a)); }
        glEnd();
    }
}
