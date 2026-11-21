#include "Environment.h"
#include <GL/freeglut.h>

// Cheap deterministic "random" number in [0,1] from an integer: the same star
// always lands in the same place every frame.
static float hash01(unsigned n) {
    n = n * 2654435761u + 1013904223u;
    n ^= n << 13; n ^= n >> 17; n ^= n << 5;
    return (n & 0xFFFFu) / 65535.0f;
}

void drawCloud(float x, float y, float s, Color c) {
    Color shade = darken(c, 0.82f);                       // flat anime-style underside
    drawEllipse(x, y - 8 * s, 95 * s, 22 * s, shade);
    drawCircle(x - 55 * s, y +  4 * s, 34 * s, c);        // overlapping puffs
    drawCircle(x - 18 * s, y + 24 * s, 44 * s, c);
    drawCircle(x + 28 * s, y + 16 * s, 40 * s, c);
    drawCircle(x + 62 * s, y -  2 * s, 30 * s, c);
    drawEllipse(x + 3 * s, y - 4 * s, 85 * s, 24 * s, c); // flat-ish bottom
}

void drawTree(float x, float y, float s, Color leaf) {
    Color trunk = rgb(0.36f, 0.24f, 0.16f);
    Color shade = darken(leaf, 0.72f);
    Color line  = darken(leaf, 0.35f);
    drawRect(x - 6 * s, y, 12 * s, 52 * s, trunk);
    drawRect(x + 1 * s, y, 5 * s, 52 * s, darken(trunk, 0.8f));
    // shadow layer first (offset down-right), light layer on top
    drawCircle(x + 6 * s, y + 62 * s, 38 * s, shade);
    drawCircle(x - 24 * s, y + 50 * s, 28 * s, shade);
    drawCircle(x + 26 * s, y + 50 * s, 28 * s, shade);
    drawCircle(x,          y + 70 * s, 36 * s, leaf);
    drawCircle(x - 26 * s, y + 52 * s, 27 * s, leaf);
    drawCircle(x + 26 * s, y + 52 * s, 27 * s, leaf);
    drawCircleOutline(x, y + 70 * s, 36 * s, line, 1.5f);
}

void drawWindow(float x, float y, float w, float h, bool lit) {
    Color frame = rgb(0.10f, 0.08f, 0.10f);
    if (lit) {
        drawGlow(x + w / 2, y + h / 2, w * 1.4f, rgba(1.0f, 0.78f, 0.38f, 0.6f), 6);
        drawRect(x, y, w, h, rgb(1.0f, 0.88f, 0.52f));
    } else {
        drawRect(x, y, w, h, rgb(0.13f, 0.16f, 0.27f));
    }
    drawRectOutline(x, y, w, h, frame, 1.5f);
    drawLine(x + w / 2, y, x + w / 2, y + h, frame, 1.0f);       // window cross
    drawLine(x, y + h / 2, x + w, y + h / 2, frame, 1.0f);
}

void drawHouse(float x, float y, float w, float h, Color wall, Color roof, bool lit) {
    float left = x - w / 2, right = x + w / 2;
    float roofH = w * 0.45f;
    Color line = darken(wall, 0.35f);

    drawRect(left, y, w, h, wall);
    drawRect(x + w * 0.30f, y, w * 0.20f, h, darken(wall, 0.84f));          // shadow side
    // roof: flat colour + a darker half for flat shading
    drawTriangle(left - 8, y + h, right + 8, y + h, x, y + h + roofH, roof);
    drawTriangle(x, y + h, right + 8, y + h, x, y + h + roofH, darken(roof, 0.82f));
    float roofOutline[6] = { left - 8, y + h, right + 8, y + h, x, y + h + roofH };
    drawPolygonOutline(roofOutline, 3, line, 1.5f);
    drawRectOutline(left, y, w, h, line, 1.5f);
    // door + window
    drawRect(left + w * 0.14f, y, w * 0.20f, h * 0.50f, darken(wall, 0.5f));
    drawWindow(x + w * 0.02f, y + h * 0.38f, w * 0.26f, h * 0.30f, lit);
}

static float hillHeight(float x, float baseY, float amp, float wavelen, float scroll) {
    float u = (x + scroll) / wavelen * 2.0f * Config::PI;
    return baseY + amp * (0.7f * std::sin(u) + 0.3f * std::sin(u * 2.7f + 1.3f));
}

void drawHillLayer(float baseY, float amp, float wavelen, float scrollX, Color top, Color bottom) {
    const float step = 16.0f;
    glBegin(GL_QUADS);                                        // one vertical strip per step
        for (float x = -step; x < Config::WINDOW_W + step; x += step) {
            float y0 = hillHeight(x, baseY, amp, wavelen, scrollX);
            float y1 = hillHeight(x + step, baseY, amp, wavelen, scrollX);
            glColor4f(bottom.r, bottom.g, bottom.b, bottom.a); glVertex2f(x, 0); glVertex2f(x + step, 0);
            glColor4f(top.r, top.g, top.b, top.a);             glVertex2f(x + step, y1); glVertex2f(x, y0);
        }
    glEnd();
}

void drawStarField(float x, float y, float w, float h, int count, float time, float alpha) {
    if (alpha <= 0.0f) return;
    glPointSize(2.0f);
    glBegin(GL_POINTS);
        for (int i = 0; i < count; ++i) {
            float px = x + hash01(i * 2) * w;
            float py = y + hash01(i * 2 + 1) * h;
            float twinkle = 0.5f + 0.5f * std::sin(time * (1.0f + 2.0f * hash01(i * 3)) + 6.28f * hash01(i * 5));
            glColor4f(1.0f, 1.0f, 0.92f, clamp01(alpha * (0.3f + 0.7f * twinkle)));
            glVertex2f(px, py);
        }
    glEnd();
}

void drawShootingStar(float hx, float hy, float tdx, float tdy, float alpha) {
    glLineWidth(2.0f);
    glBegin(GL_LINES);
        glColor4f(1, 1, 1, 0.0f);   glVertex2f(hx - tdx, hy - tdy);   // faded tail
        glColor4f(1, 1, 1, alpha);  glVertex2f(hx, hy);               // bright head
    glEnd();
    drawGlow(hx, hy, 14, rgba(1, 1, 1, alpha * 0.8f), 4);
}
