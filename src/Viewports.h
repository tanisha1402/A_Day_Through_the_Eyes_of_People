// Viewports.h - split-screen panels. Each panel is drawn with its OWN glViewport,
// glScissor and projection, in panel-local coordinates (origin = panel's bottom-left).
#pragma once
#include "Utils.h"

struct PanelRect   { float x, y, w, h; };            // logical 1280x720 coordinates
struct PanelLayout { int count; PanelRect r[4]; };

namespace PanelGeom {
    const float GUTTER = 6.0f;                        // gap between panels
}

// Area between the letterbox bars.
inline PanelRect playArea() {
    PanelRect a; a.x = 0; a.y = Config::LETTERBOX; a.w = (float)Config::WINDOW_W; a.h = Config::WINDOW_H - 2 * Config::LETTERBOX; return a;
}
// n equal columns (2 or 3), left to right.
inline PanelLayout layoutColumns(int n) {
    PanelRect a = playArea(); PanelLayout L; L.count = n;
    float w = (a.w - PanelGeom::GUTTER * (n - 1)) / n;
    for (int i = 0; i < n; ++i) { L.r[i].x = i * (w + PanelGeom::GUTTER); L.r[i].y = a.y; L.r[i].w = w; L.r[i].h = a.h; }
    return L;
}
// 2x2 grid, order: top-left, top-right, bottom-left, bottom-right.
inline PanelLayout layoutGrid2x2() {
    PanelRect a = playArea(); PanelLayout L; L.count = 4;
    float w = (a.w - PanelGeom::GUTTER) / 2, h = (a.h - PanelGeom::GUTTER) / 2;
    for (int i = 0; i < 4; ++i) {
        L.r[i].w = w; L.r[i].h = h;
        L.r[i].x = (i % 2) * (w + PanelGeom::GUTTER);
        L.r[i].y = a.y + ((i / 2) == 0 ? h + PanelGeom::GUTTER : 0.0f);
    }
    return L;
}
// Mouse picking: which panel contains logical point (lx, ly)? Returns -1 if none (gutter / bars).
// px, py receive the point in that panel's local coordinates.
inline int panelAt(const PanelLayout& L, float lx, float ly, float& px, float& py) {
    for (int i = 0; i < L.count; ++i) {
        const PanelRect& r = L.r[i];
        if (lx >= r.x && lx < r.x + r.w && ly >= r.y && ly < r.y + r.h) { px = lx - r.x; py = ly - r.y; return i; }
    }
    return -1;
}

void beginPanel(const PanelRect& r);                       // glViewport + glScissor + local projection
void endPanel();                                           // back to the full 1280x720 view
void panelClipBegin(float lx, float ly, float w, float h); // nested clip rectangle, panel-local
void panelClipEnd();
void drawPanelFrames(const PanelLayout& L);                // dark gutters + thin light frames (manga panels)
