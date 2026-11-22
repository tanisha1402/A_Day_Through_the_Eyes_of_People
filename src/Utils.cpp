#include "Utils.h"
#include <GL/freeglut.h>

static Viewport gViewport = { 0, 0, Config::WINDOW_W, Config::WINDOW_H };
static int      gWinH     = Config::WINDOW_H;

static void setColor(Color c) { glColor4f(c.r, c.g, c.b, c.a); }

// ------------------------------------------------------------ window setup ----
void utilsOnResize(int winW, int winH) {
    gWinH = winH;
    gViewport = computeViewport(winW, winH);
    glViewport(gViewport.x, gViewport.y, gViewport.w, gViewport.h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, Config::WINDOW_W, 0, Config::WINDOW_H, -1, 1);   // logical coordinates
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
Viewport utilsGetViewport() { return gViewport; }
void utilsWindowToLogical(int px, int py, float& lx, float& ly) {
    windowToLogical(px, py, gWinH, gViewport, lx, ly);
}

// ------------------------------------------------------------- primitives ----
void drawRect(float x, float y, float w, float h, Color c) {
    setColor(c);
    glBegin(GL_QUADS);
        glVertex2f(x, y); glVertex2f(x + w, y); glVertex2f(x + w, y + h); glVertex2f(x, y + h);
    glEnd();
}

void drawRectOutline(float x, float y, float w, float h, Color c, float width) {
    setColor(c);
    glLineWidth(width);
    glBegin(GL_LINE_LOOP);
        glVertex2f(x, y); glVertex2f(x + w, y); glVertex2f(x + w, y + h); glVertex2f(x, y + h);
    glEnd();
}

void drawGradientRect(float x, float y, float w, float h, Color bottom, Color top) {
    glBegin(GL_QUADS);
        setColor(bottom); glVertex2f(x, y);     glVertex2f(x + w, y);        // a colour per vertex
        setColor(top);    glVertex2f(x + w, y + h); glVertex2f(x, y + h);    // = smooth gradient
    glEnd();
}

void drawGradientRectH(float x, float y, float w, float h, Color left, Color right) {
    glBegin(GL_QUADS);
        setColor(left);  glVertex2f(x, y);     glVertex2f(x, y + h);
        setColor(right); glVertex2f(x + w, y + h); glVertex2f(x + w, y);
    glEnd();
}

void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3, Color c) {
    setColor(c);
    glBegin(GL_TRIANGLES);
        glVertex2f(x1, y1); glVertex2f(x2, y2); glVertex2f(x3, y3);
    glEnd();
}

void drawPolygon(const float* xy, int count, Color c) {
    setColor(c);
    glBegin(GL_POLYGON);
        for (int i = 0; i < count; ++i) glVertex2f(xy[2 * i], xy[2 * i + 1]);
    glEnd();
}

void drawPolygonOutline(const float* xy, int count, Color c, float width) {
    setColor(c);
    glLineWidth(width);
    glBegin(GL_LINE_LOOP);
        for (int i = 0; i < count; ++i) glVertex2f(xy[2 * i], xy[2 * i + 1]);
    glEnd();
}

void drawCircle(float cx, float cy, float r, Color c, int segments) {
    setColor(c);
    glBegin(GL_TRIANGLE_FAN);                 // centre + points around the edge
        glVertex2f(cx, cy);
        for (int i = 0; i <= segments; ++i) {
            float a = 2.0f * Config::PI * i / segments;
            glVertex2f(cx + r * std::cos(a), cy + r * std::sin(a));
        }
    glEnd();
}

void drawEllipse(float cx, float cy, float rx, float ry, Color c, int segments) {
    setColor(c);
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (int i = 0; i <= segments; ++i) {
            float a = 2.0f * Config::PI * i / segments;
            glVertex2f(cx + rx * std::cos(a), cy + ry * std::sin(a));
        }
    glEnd();
}

void drawCircleOutline(float cx, float cy, float r, Color c, float width) {
    const int segments = 48;
    setColor(c);
    glLineWidth(width);
    glBegin(GL_LINE_LOOP);
        for (int i = 0; i < segments; ++i) {
            float a = 2.0f * Config::PI * i / segments;
            glVertex2f(cx + r * std::cos(a), cy + r * std::sin(a));
        }
    glEnd();
}

void drawArcBand(float cx, float cy, float rIn, float rOut, float a0, float a1, Color c, int segments) {
    setColor(c);
    glBegin(GL_QUADS);                        // one thin quad per segment
        for (int i = 0; i < segments; ++i) {
            float t0 = a0 + (a1 - a0) * i / segments;
            float t1 = a0 + (a1 - a0) * (i + 1) / segments;
            glVertex2f(cx + rIn  * std::cos(t0), cy + rIn  * std::sin(t0));
            glVertex2f(cx + rOut * std::cos(t0), cy + rOut * std::sin(t0));
            glVertex2f(cx + rOut * std::cos(t1), cy + rOut * std::sin(t1));
            glVertex2f(cx + rIn  * std::cos(t1), cy + rIn  * std::sin(t1));
        }
    glEnd();
}

void drawLine(float x1, float y1, float x2, float y2, Color c, float width) {
    setColor(c);
    glLineWidth(width);
    glBegin(GL_LINES);
        glVertex2f(x1, y1); glVertex2f(x2, y2);
    glEnd();
}

// ------------------------------------------------------------ light / fx ----
void drawGlow(float cx, float cy, float radius, Color c, int layers) {
    // Additive blending (GL_ONE) adds light instead of covering what is behind,
    // so overlapping glows brighten like real light. The big faint circles are
    // drawn first; every smaller circle adds a bit more, giving a soft falloff.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    // big glows need more layers, otherwise the rings become visible (8-bit colour banding)
    int n = layers;
    if (n < (int)(radius / 5.0f)) n = (int)(radius / 5.0f);
    if (n > 56) n = 56;
    layers = n;
    for (int i = layers; i >= 1; --i) {
        float t = (float)i / layers;
        drawCircle(cx, cy, radius * t, withAlpha(c, c.a / layers), 32);
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);   // restore normal blending
}

void drawSoftEllipse(float cx, float cy, float rx, float ry, Color c, int layers) {
    for (int i = layers; i >= 1; --i) {                     // shrinking translucent ellipses = soft edge
        float t = (float)i / layers;
        drawEllipse(cx, cy, rx * t, ry * t, withAlpha(c, c.a / layers), 32);
    }
}

void drawVignette(float strength) {
    const float W = Config::WINDOW_W, H = Config::WINDOW_H;
    const float sideW = 260.0f, edgeH = 150.0f;
    Color edge = rgba(0, 0, 0, strength), clear = rgba(0, 0, 0, 0);
    drawGradientRectH(0, 0, sideW, H, edge, clear);                // left
    drawGradientRectH(W - sideW, 0, sideW, H, clear, edge);        // right
    drawGradientRect(0, 0, W, edgeH, edge, clear);                 // bottom
    drawGradientRect(0, H - edgeH, W, edgeH, clear, edge);         // top
}

void drawLetterbox() {
    Color black = rgb(0, 0, 0);
    drawRect(0, 0, Config::WINDOW_W, Config::LETTERBOX, black);
    drawRect(0, Config::WINDOW_H - Config::LETTERBOX, Config::WINDOW_W, Config::LETTERBOX, black);
}

void drawFade(Color c, float alpha) {
    drawRect(0, 0, Config::WINDOW_W, Config::WINDOW_H, withAlpha(c, clamp01(alpha)));
}

void drawText(float x, float y, const char* s, Color c, bool large) {
    setColor(c);
    glRasterPos2f(x, y);
    glutBitmapString(large ? GLUT_BITMAP_HELVETICA_18 : GLUT_BITMAP_HELVETICA_12,
                     (const unsigned char*)s);
}

void beginClip(float x, float y, float w, float h) {
    ScissorRect r = logicalToScissor(x, y, w, h, gViewport);   // scissor works in window pixels
    glEnable(GL_SCISSOR_TEST);
    glScissor(r.x, r.y, r.w, r.h);
}
void endClip() { glDisable(GL_SCISSOR_TEST); }
