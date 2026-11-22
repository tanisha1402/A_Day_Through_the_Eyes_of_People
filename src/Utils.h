// Utils.h - colour maths and basic drawing utilities.
// Coordinates are the logical 1280x720 space, origin bottom-left.
// Pure maths helpers are inline here so they can be unit-tested without OpenGL.
#pragma once
#include "Config.h"
#include <cmath>

// ---------------------------------------------------------------- colour ----
struct Color { float r, g, b, a; };

inline Color rgba(float r, float g, float b, float a) { Color c; c.r = r; c.g = g; c.b = b; c.a = a; return c; }
inline Color rgb(float r, float g, float b)           { return rgba(r, g, b, 1.0f); }
inline Color withAlpha(Color c, float a)              { c.a = a; return c; }

inline float clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }
inline float lerp(float a, float b, float t) { return a + (b - a) * t; }
// Ease-in/ease-out curve: makes blends start and end gently (no abrupt changes).
inline float smoothstep01(float t) { t = clamp01(t); return t * t * (3.0f - 2.0f * t); }

// Blend two colours (used for gradient skies and day/night changes).
inline Color lerpColor(Color a, Color b, float t) {
    t = clamp01(t);
    return rgba(lerp(a.r, b.r, t), lerp(a.g, b.g, t), lerp(a.b, b.b, t), lerp(a.a, b.a, t));
}
// Multiply RGB by f (<1 = darker). Used for the flat "shadow" shapes.
inline Color darken(Color c, float f) { return rgba(c.r * f, c.g * f, c.b * f, c.a); }

// -------------------------------------------------- viewport / mouse maths ----
struct Viewport { int x, y, w, h; };

// Largest centred 16:9 rectangle inside a window of winW x winH pixels.
inline Viewport computeViewport(int winW, int winH) {
    if (winW < 1) winW = 1;
    if (winH < 1) winH = 1;
    const float aspect = (float)Config::WINDOW_W / (float)Config::WINDOW_H;
    int vw = winW, vh = (int)(winW / aspect);
    if (vh > winH) { vh = winH; vw = (int)(winH * aspect); }
    if (vw < 1) vw = 1;      // never 0: later maths divides by the viewport size
    if (vh < 1) vh = 1;
    Viewport v; v.x = (winW - vw) / 2; v.y = (winH - vh) / 2; v.w = vw; v.h = vh;
    return v;
}

// Window pixel (GLUT: origin top-left) -> logical 1280x720 (origin bottom-left).
// Needed for mouse picking in Phase 6.
inline void windowToLogical(int px, int py, int winH, const Viewport& vp, float& lx, float& ly) {
    lx = (px - vp.x) * (float)Config::WINDOW_W / (float)vp.w;
    ly = ((winH - py) - vp.y) * (float)Config::WINDOW_H / (float)vp.h;
}

struct ScissorRect { int x, y, w, h; };
// Logical rectangle -> window-pixel rectangle for glScissor (which uses pixels).
inline ScissorRect logicalToScissor(float x, float y, float w, float h, const Viewport& vp) {
    float sx = vp.w / (float)Config::WINDOW_W, sy = vp.h / (float)Config::WINDOW_H;
    int x0 = (int)std::floor(vp.x + x * sx),       y0 = (int)std::floor(vp.y + y * sy);
    int x1 = (int)std::ceil (vp.x + (x + w) * sx), y1 = (int)std::ceil (vp.y + (y + h) * sy);
    ScissorRect r; r.x = x0; r.y = y0; r.w = x1 - x0; r.h = y1 - y0;
    return r;
}

// ------------------------------------------------------------ window setup ----
void     utilsOnResize(int winW, int winH);   // sets viewport + projection
Viewport utilsGetViewport();
void     utilsWindowToLogical(int px, int py, float& lx, float& ly);

// ------------------------------------------------------------- primitives ----
// (x, y) of rectangles is the BOTTOM-LEFT corner; circles use their centre.
void drawRect(float x, float y, float w, float h, Color c);
void drawRectOutline(float x, float y, float w, float h, Color c, float width = 1.0f);
void drawGradientRect(float x, float y, float w, float h, Color bottom, Color top);   // vertical
void drawGradientRectH(float x, float y, float w, float h, Color left, Color right);  // horizontal
void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3, Color c);
void drawPolygon(const float* xy, int count, Color c);                 // convex only
void drawPolygonOutline(const float* xy, int count, Color c, float width = 1.0f);
void drawCircle(float cx, float cy, float r, Color c, int segments = 40);
void drawEllipse(float cx, float cy, float rx, float ry, Color c, int segments = 40);
void drawCircleOutline(float cx, float cy, float r, Color c, float width = 1.0f);
// Band between two radii, from angle a0 to a1 (radians). Full ring: 0..2*PI.
// Rainbow = several bands with half-circle angles 0..PI.
void drawArcBand(float cx, float cy, float rIn, float rOut, float a0, float a1, Color c, int segments = 48);
void drawLine(float x1, float y1, float x2, float y2, Color c, float width = 1.0f);

// ------------------------------------------------------------ light / fx ----
// Soft glow: stacked translucent circles drawn with additive blending.
void drawGlow(float cx, float cy, float radius, Color c, int layers = 16);
void drawSoftEllipse(float cx, float cy, float rx, float ry, Color c, int layers = 10);   // mist, soft shadows
void drawVignette(float strength);          // darkened screen edges
void drawLetterbox();                       // black cinematic bars
void drawFade(Color c, float alpha);        // full-screen colour overlay (scene fades)
void drawText(float x, float y, const char* s, Color c, bool large = false);

// Clip drawing to a logical rectangle (glScissor). Used by split-screen panels.
void beginClip(float x, float y, float w, float h);
void endClip();
