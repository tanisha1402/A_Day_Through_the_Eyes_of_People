// Particles.h - one particle structure reused for rain, ripples, dust, music notes, drips...
// Every pool has a hard capacity, so the number of live particles can never grow without limit.
#pragma once
#include "Utils.h"
#include <vector>

struct Particle {
    float x, y;          // position
    float vx, vy;        // velocity (units per second)
    float life;          // seconds left; <= 0 means the slot is free
    float maxLife;       // starting life (for fading)
    float size;
    float aux;           // free use: e.g. the height at which a raindrop lands
    Color color;
};

struct ParticlePool {
    std::vector<Particle> p;
    int cursor;
    ParticlePool() : cursor(0) {}
};

float rand01();                       // deterministic pseudo-random number 0..1
void  seedRandom(unsigned seed);

void      poolInit(ParticlePool& pool, int capacity);
void      poolClear(ParticlePool& pool);
Particle* poolSpawn(ParticlePool& pool);           // nullptr when the pool is full
int       poolAlive(const ParticlePool& pool);
void      poolUpdate(ParticlePool& pool, float dt, float gravityY);   // move, accelerate, age, free dead ones

// A sheet of rain for one panel. Drops fall, then leave a ripple where they land.
struct RainField {
    ParticlePool drops, ripples;
    float w, h;                      // panel size
    float landMin, landMax;          // drops land at a random height in this range (ground depth)
    float rate;                      // drops per second at intensity 1
    float intensity;                 // 0 = off, 1 = full (R key toggles this in Phase 6)
    float wind;                      // horizontal speed
    float spawnAcc;
    RainField() : w(0), h(0), landMin(0), landMax(0), rate(200), intensity(1), wind(-60), spawnAcc(0) {}
};
void rainInit(RainField& f, float w, float h, float landMin, float landMax, int maxDrops, float rate);
void rainUpdate(RainField& f, float dt);
void rainDraw(const RainField& f, Color dropColor, Color rippleColor);
